/*
 * Android system-engine integration based on the VoxSherpa TTS architecture.
 * VoxSherpa TTS copyright (C) 2025 CodeBySonu95.
 * SPDX-License-Identifier: GPL-3.0-or-later
 * See NOTICE for complete attribution and modification information.
 */
package com.labsii.voices.system

import android.content.BroadcastReceiver
import android.content.ComponentCallbacks2
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.media.AudioFormat
import android.os.Build
import android.os.PowerManager
import android.os.SystemClock
import android.speech.tts.SynthesisCallback
import android.speech.tts.SynthesisRequest
import android.speech.tts.TextToSpeech
import android.speech.tts.TextToSpeechService
import android.speech.tts.Voice
import android.util.Log
import com.labsii.voices.BuildConfig
import com.labsii.voices.runner.KokoroModelManager
import com.labsii.voices.runner.KokoroSession
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.TimeoutCancellationException
import kotlinx.coroutines.runBlocking
import java.util.Locale
import java.util.concurrent.locks.ReentrantLock

/** Android system TTS facade for the Adreno Kokoro-82M runtime. */
class AdrenoTtsService : TextToSpeechService() {
    private lateinit var catalog: InstalledKokoroVoiceCatalog
    private lateinit var session: KokoroSession
    private val synthesisWatchdog = SynthesisWatchdog()
    private lateinit var powerManager: PowerManager
    private var powerReceiverRegistered = false
    private val powerReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            logPowerState(intent.action ?: "unknown", activeRequest?.id)
        }
    }
    /**
     * Android can create a replacement TTS client while the previous request is
     * still being cancelled.  A single mutable `cancelled` Boolean makes those
     * two requests race: the replacement clears it and revives the old callback.
     * A request now owns its cancellation state. Only Android's onStop() may
     * cancel the currently active request; a new TTS client merely queues
     * behind it and cannot invalidate it.
     */
    private val synthesisLock = ReentrantLock()
    @Volatile private var activeRequest: KokoroSession.Request? = null

    override fun onCreate() {
        // TextToSpeechService.onCreate() synchronously calls onLoadLanguage().
        catalog = InstalledKokoroVoiceCatalog(applicationContext)
        session = KokoroSession(applicationContext, diagnosticsEnabled = BuildConfig.DEBUG)
        powerManager = getSystemService(PowerManager::class.java)
        if (BuildConfig.DEBUG) Log.i(
            TAG,
            "Service created (${BuildConfig.VERSION_NAME}, background reading exemption); installed voices=${catalog.installed().size}, " +
                "runtimeReady=${KokoroModelManager.isRuntimeReady(applicationContext)}",
        )
        super.onCreate()
        if (BuildConfig.DEBUG) {
            val filter = IntentFilter().apply {
                addAction(Intent.ACTION_SCREEN_OFF)
                addAction(Intent.ACTION_SCREEN_ON)
                addAction(PowerManager.ACTION_DEVICE_IDLE_MODE_CHANGED)
                addAction(PowerManager.ACTION_POWER_SAVE_MODE_CHANGED)
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    addAction(PowerManager.ACTION_DEVICE_LIGHT_IDLE_MODE_CHANGED)
                }
            }
            runCatching {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    registerReceiver(powerReceiver, filter, Context.RECEIVER_NOT_EXPORTED)
                } else {
                    @Suppress("DEPRECATION")
                    registerReceiver(powerReceiver, filter)
                }
                powerReceiverRegistered = true
            }.onFailure {
                Log.w(TAG, "KOKORO_POWER receiver unavailable: ${it.javaClass.simpleName}")
            }
            logPowerState("service_created", null)
        }
    }

    override fun onGetLanguage(): Array<String> = legacyLanguage(catalog.preferred()?.locale)

    override fun onIsLanguageAvailable(lang: String?, country: String?, variant: String?): Int {
        val voice = catalog.findForLanguage(lang, country) ?: return TextToSpeech.LANG_NOT_SUPPORTED
        if (country.isNullOrBlank()) return TextToSpeech.LANG_AVAILABLE
        return if (
            country.equals(voice.locale.country, ignoreCase = true) ||
            country.equals(voice.locale.isO3Country, ignoreCase = true)
        ) TextToSpeech.LANG_COUNTRY_AVAILABLE else TextToSpeech.LANG_AVAILABLE
    }

    override fun onLoadLanguage(lang: String?, country: String?, variant: String?): Int =
        onIsLanguageAvailable(lang, country, variant)

    override fun onGetVoices(): MutableList<Voice> {
        val installed = catalog.installed()
        if (BuildConfig.DEBUG) Log.i(TAG, "Android requested voices; returning ${installed.size}")
        return installed.mapTo(mutableListOf()) { voice ->
            Voice(
                voice.voiceName,
                voice.locale,
                Voice.QUALITY_HIGH,
                Voice.LATENCY_HIGH,
                false,
                emptySet(),
            )
        }
    }

    override fun onGetDefaultVoiceNameFor(lang: String?, country: String?, variant: String?): String? =
        catalog.findForLanguage(lang, country)?.voiceName

    override fun onIsValidVoiceName(voiceName: String?): Int =
        if (catalog.findByVoiceName(voiceName) != null) TextToSpeech.SUCCESS else TextToSpeech.ERROR

    override fun onLoadVoice(voiceName: String?): Int = onIsValidVoiceName(voiceName)

    override fun onSynthesizeText(request: SynthesisRequest, callback: SynthesisCallback) {
        val enteredRealMs = if (BuildConfig.DEBUG) SystemClock.elapsedRealtime() else 0L
        if (BuildConfig.DEBUG) {
            Log.i(TAG, "KOKORO_DISPATCH event=entered elapsedRealtimeMs=$enteredRealMs " +
                "uptimeMs=${SystemClock.uptimeMillis()} lockHeld=${synthesisLock.isLocked}")
        }
        synthesisLock.lock()
        if (BuildConfig.DEBUG) {
            Log.i(TAG, "KOKORO_DISPATCH event=acquired waitMs=${SystemClock.elapsedRealtime() - enteredRealMs}")
        }
        val startedAt = if (BuildConfig.DEBUG) System.nanoTime() else 0L
        val startedRealMs = if (BuildConfig.DEBUG) SystemClock.elapsedRealtime() else 0L
        var state: KokoroSession.Request? = null
        var callbackStarted = false
        var callbackFinished = false
        var chunkIndex = 0
        var chunkCount = 0
        val debug = if (BuildConfig.DEBUG) KokoroSession.DebugTiming() else null
        var textChars = 0
        var startupAwakeNs = 0L
        var startupElapsedMs = 0L
        var speakAwakeNs = 0L
        var speakElapsedMs = 0L
        var callbackAwakeNs = 0L
        var callbackElapsedMs = 0L
        var callbackSamples = 0L
        try {
            val requestStageMs = if (BuildConfig.DEBUG) SystemClock.elapsedRealtime() else 0L
            if (BuildConfig.DEBUG) Log.i(TAG, "KOKORO_PREP stage=begin_request elapsedRealtimeMs=$requestStageMs")
            val current = session.beginRequest()
            state = current
            activeRequest = current
            val voiceStageMs = if (BuildConfig.DEBUG) SystemClock.elapsedRealtime() else 0L
            if (BuildConfig.DEBUG) {
                Log.i(TAG, "KOKORO_PREP stage=request_created id=${current.id} " +
                    "requestMs=${voiceStageMs - requestStageMs} elapsedRealtimeMs=$voiceStageMs")
            }
            val voice = catalog.findByVoiceName(request.voiceName)
                ?: catalog.findForLanguage(request.language, request.country)
                ?: throw IllegalStateException("Kokoro is not installed or does not support ${request.language}")
            if (BuildConfig.DEBUG) {
                Log.i(TAG, "KOKORO_PREP stage=voice_resolved id=${current.id} " +
                    "lookupMs=${SystemClock.elapsedRealtime() - voiceStageMs}")
            }
            if (BuildConfig.DEBUG) Log.i(TAG, "Synthesis begin id=${current.id}, voice=${voice.voiceName}")
            logPowerState("request_begin", current.id)
            val chunks = SystemTtsText.chunks(request.charSequenceText ?: "", voice.locale)
            chunkCount = chunks.size
            if (debug != null) {
                textChars = chunks.sumOf { it.length }
                Log.i(TAG, "KOKORO_PREP stage=text_prepared id=${current.id} " +
                    "chunkChars=${chunks.joinToString(",") { it.length.toString() }}")
            }
            val speechRate = request.speechRate
                .takeIf { it > 0 }
                ?.div(100.0f)
                ?.coerceIn(MIN_SPEECH_RATE, MAX_SPEECH_RATE)
                ?: DEFAULT_SPEECH_RATE

            if (callback.start(SAMPLE_RATE, AudioFormat.ENCODING_PCM_16BIT, 1) != TextToSpeech.SUCCESS) {
                throw IllegalStateException("TTS client rejected the Kokoro audio format")
            }
            callbackStarted = true
            if (chunks.isEmpty()) {
                if (!current.cancelled.get()) {
                    callback.done()
                    callbackFinished = true
                }
                return
            }

            val startupStartedNs = if (debug != null) System.nanoTime() else 0L
            val startupStartedRealMs = if (debug != null) SystemClock.elapsedRealtime() else 0L
            try {
                runBlocking(Dispatchers.IO) {
                    session.start(current, voice.voicePackPath, voice.phonemizerVoice,
                        KokoroSession.GpuPriority.LOW)
                }
            } finally {
                if (debug != null) {
                    startupAwakeNs += System.nanoTime() - startupStartedNs
                    startupElapsedMs += SystemClock.elapsedRealtime() - startupStartedRealMs
                }
            }
            var recoveryAttempted = false
            for (chunk in chunks) {
                chunkIndex++
                if (current.cancelled.get()) return
                if (debug != null) Log.i(TAG, "KOKORO_CHUNK event=begin id=${current.id} " +
                    "chunk=$chunkIndex/$chunkCount chars=${chunk.length}")
                // PCM arrives asynchronously; callbacks remain on Android's
                // dedicated synthesis thread, never the reader/control threads.
                val speakStartedNs = if (debug != null) System.nanoTime() else 0L
                val speakStartedRealMs = if (debug != null) SystemClock.elapsedRealtime() else 0L
                var chunkDelivered = false
                var measuredAudioMs = 0.0
                var measuredGenerationMs = 0L
                var generationStartedMs = SystemClock.elapsedRealtime()
                val deliver: (KokoroSession.PcmAudio) -> Unit = { result ->
                    val audioArrivedMs = SystemClock.elapsedRealtime()
                    measuredGenerationMs += audioArrivedMs - generationStartedMs
                    measuredAudioMs += result.samples.size * 1000.0 / result.sampleRate
                    if (result.sampleRate != SAMPLE_RATE) {
                        throw IllegalStateException("Unexpected Kokoro sample rate ${result.sampleRate}")
                    }
                    if (!current.cancelled.get()) {
                        // Mark before entering a possibly blocking callback. Once
                        // any audio can reach Android, replay could duplicate it.
                        chunkDelivered = true
                        if (debug == null) {
                            streamPcm(result.samples, callback) { current.cancelled.get() }
                        } else {
                            val callbackStartedNs = System.nanoTime()
                            val callbackStartedRealMs = SystemClock.elapsedRealtime()
                            Log.i(TAG, "KOKORO_CALLBACK event=audio_begin id=${current.id} " +
                                "chunk=$chunkIndex/$chunkCount samples=${result.samples.size}")
                            try {
                                callbackSamples += streamPcm(result.samples, callback) { current.cancelled.get() }
                            } finally {
                                callbackAwakeNs += System.nanoTime() - callbackStartedNs
                                callbackElapsedMs += SystemClock.elapsedRealtime() - callbackStartedRealMs
                                Log.i(TAG, "KOKORO_CALLBACK event=audio_end id=${current.id} " +
                                    "chunk=$chunkIndex/$chunkCount")
                            }
                        }
                    }
                    generationStartedMs = SystemClock.elapsedRealtime()
                }
                val timeoutMs = synthesisWatchdog.timeoutMs(chunk.length, speechRate)
                if (debug != null) Log.i(TAG, "KOKORO_WATCHDOG id=${current.id} " +
                    "chars=${chunk.length} rtf=${synthesisWatchdog.estimatedRtf} timeoutMs=$timeoutMs")
                try {
                    try {
                        runBlocking { session.speak(current, chunk, speechRate, debug, timeoutMs, deliver) }
                    } catch (stalled: KokoroSession.InferenceDeadlineException) {
                        if (recoveryAttempted || chunkDelivered || current.cancelled.get()) throw stalled
                        recoveryAttempted = true
                        Log.w(TAG, "Kokoro inference deadline id=${current.id}, chunk=$chunkIndex/$chunkCount; " +
                            "retiring worker and retrying once at low GPU priority")
                        runBlocking(Dispatchers.IO) {
                            session.start(current, voice.voicePackPath, voice.phonemizerVoice,
                                KokoroSession.GpuPriority.LOW)
                        }
                        if (current.cancelled.get()) return
                        runBlocking { session.speak(current, chunk, speechRate, debug, timeoutMs, deliver) }
                    }
                } finally {
                    if (debug != null) {
                        speakAwakeNs += System.nanoTime() - speakStartedNs
                        speakElapsedMs += SystemClock.elapsedRealtime() - speakStartedRealMs
                    }
                }
                if (!current.cancelled.get() && !recoveryAttempted) {
                    synthesisWatchdog.record(measuredGenerationMs, measuredAudioMs, timeoutMs)
                }
                if (current.cancelled.get()) {
                    if (BuildConfig.DEBUG) Log.i(TAG, "Cancelled native utterance drained; preserving warm session")
                    return
                }
            }

            if (!current.cancelled.get()) {
                if (debug != null) Log.i(TAG, "KOKORO_CALLBACK event=done_begin id=${current.id}")
                callback.done()
                callbackFinished = true
                if (debug != null) Log.i(TAG, "KOKORO_CALLBACK event=done_end id=${current.id}")
            }
        } catch (t: Throwable) {
            // A timeout, dead pipe, or rejected chunk can leave the framed
            // native stream out of sync. Normal cancellation does not throw;
            // failures do, and require a clean process on the next request.
            state?.let { session.failRequest(it) }
            if (state?.cancelled?.get() != true) {
                Log.e(TAG, "Kokoro system synthesis failed id=${state?.id}, " +
                    "chunk=$chunkIndex/$chunkCount", t)
                callback.error()
                callback.done()
                callbackFinished = true
            }
        } finally {
            if (callbackStarted && !callbackFinished && state?.cancelled?.get() != true) callback.done()
            if (BuildConfig.DEBUG) Log.i(TAG, "Synthesis finished id=${state?.id}, cancelled=${state?.cancelled?.get() == true}, " +
                "elapsedMs=${(System.nanoTime() - startedAt) / 1_000_000}, " +
                "elapsedRealtimeMs=${SystemClock.elapsedRealtime() - startedRealMs}")
            if (debug != null) {
                val totalAwakeMs = (System.nanoTime() - startedAt) / 1_000_000
                val totalElapsedMs = SystemClock.elapsedRealtime() - startedRealMs
                Log.i(TAG, "KOKORO_TIMING id=${state?.id} chunks=$chunkIndex/$chunkCount chars=$textChars " +
                    "startup=${startupAwakeNs / 1_000_000}/${startupElapsedMs}ms " +
                    "speak=${speakAwakeNs / 1_000_000}/${speakElapsedMs}ms " +
                    "g2p=${debug.phonemizeNs.get() / 1_000_000}ms " +
                    "inference=${debug.inferenceNs.get() / 1_000_000}ms " +
                    "pipeWrite=${debug.nativeWriteNs.get() / 1_000_000}ms " +
                    "pipeRead=${debug.pcmReadNs.get() / 1_000_000}ms " +
                    "callback=${callbackAwakeNs / 1_000_000}/${callbackElapsedMs}ms " +
                    "audio=${callbackSamples * 1000 / SAMPLE_RATE}ms " +
                    "total=$totalAwakeMs/${totalElapsedMs}ms " +
                    "suspendGap=${totalElapsedMs - totalAwakeMs}ms engineWakeLock=disabled")
            }
            logPowerState("request_end", state?.id)
            state?.let { session.finishRequest(it) }
            if (activeRequest === state) {
                activeRequest = null
            }
            synthesisLock.unlock()
        }
    }

    override fun onStop() {
        val state = activeRequest
        state?.cancelled?.set(true)
        if (BuildConfig.DEBUG) Log.i(TAG, "Synthesis stop requested; active=${state != null}, id=${state?.id}")
        if (state != null && ::session.isInitialized) {
            // Request cancellation from a separate native stdin reader. The
            // graph checks it between safe GPU stages and keeps the warm model
            // on the normal path. A wedged driver still gets a bounded escape
            // hatch, but `onStop()` itself never blocks Android's caller.
            session.requestCancel(state)
        }
    }

    override fun onDestroy() {
        if (powerReceiverRegistered) {
            unregisterReceiver(powerReceiver)
            powerReceiverRegistered = false
        }
        activeRequest?.cancelled?.set(true)
        if (::session.isInitialized) session.close()
        super.onDestroy()
    }

    @Suppress("DEPRECATION") // Includes UI-hidden/background hints on newer Android releases.
    override fun onTrimMemory(level: Int) {
        super.onTrimMemory(level)
        if (level >= ComponentCallbacks2.TRIM_MEMORY_RUNNING_LOW && ::session.isInitialized) {
            session.releaseIdleWorker("Android memory pressure ($level)")
        }
    }

    private fun streamPcm(
        pcm: ShortArray,
        callback: SynthesisCallback,
        isCancelled: () -> Boolean,
    ): Int {
        val maxBytes = (callback.maxBufferSize.takeIf { it >= 2 } ?: DEFAULT_CALLBACK_BUFFER_BYTES)
            .let { it - (it % 2) }
        val buffer = ByteArray(maxBytes)
        var sampleOffset = 0
        while (sampleOffset < pcm.size) {
            if (isCancelled()) return sampleOffset
            val count = minOf(buffer.size / 2, pcm.size - sampleOffset)
            for (index in 0 until count) {
                val sample = pcm[sampleOffset + index].toInt()
                buffer[index * 2] = (sample and 0xff).toByte()
                buffer[index * 2 + 1] = ((sample ushr 8) and 0xff).toByte()
            }
            val byteCount = count * 2
            if (callback.audioAvailable(buffer, 0, byteCount) != TextToSpeech.SUCCESS) {
                // Android may invalidate the callback concurrently with
                // onStop(). That is a normal cancellation, not a stream error.
                if (isCancelled()) return sampleOffset
                throw IllegalStateException("TTS client rejected a Kokoro audio chunk")
            }
            sampleOffset += count
        }
        return sampleOffset
    }

    /** Debug-only device power observations. */
    private fun logPowerState(event: String, requestId: Long?) {
        if (!BuildConfig.DEBUG || !::powerManager.isInitialized) return
        // A diagnostic must never turn an otherwise successful synthesis into an error.
        val details = runCatching {
            val standby = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                powerManager.isLowPowerStandbyEnabled.toString()
            } else "unsupported"
            val standbyExempt = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
                powerManager.isExemptFromLowPowerStandby.toString()
            } else "unsupported"
            val lightIdle = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                powerManager.isDeviceLightIdleMode.toString()
            } else "unsupported"
            "engineWakeLock=disabled interactive=${powerManager.isInteractive} " +
                "batterySaver=${powerManager.isPowerSaveMode} " +
                "deviceIdle=${powerManager.isDeviceIdleMode} lightIdle=$lightIdle " +
                "lowPowerStandby=$standby standbyExempt=$standbyExempt " +
                "batteryOptimizationExempt=${powerManager.isIgnoringBatteryOptimizations(packageName)} " +
                "elapsedRealtimeMs=${SystemClock.elapsedRealtime()} uptimeMs=${SystemClock.uptimeMillis()}"
        }.getOrElse { "probeError=${it.javaClass.simpleName}" }
        Log.i(TAG, "KOKORO_POWER event=$event id=$requestId $details")
    }

    private fun legacyLanguage(locale: Locale?): Array<String> {
        if (locale == null || locale == Locale.ROOT) return arrayOf("", "", "")
        val language = locale.safeIso3Language().ifBlank { locale.language }
        val country = try { locale.isO3Country } catch (_: Throwable) { locale.country }
        return arrayOf(language, country, locale.variant)
    }

    companion object {
        private const val TAG = "AdrenoTtsService"
        private const val SAMPLE_RATE = 24_000
        private const val DEFAULT_CALLBACK_BUFFER_BYTES = 8 * 1024
        private const val DEFAULT_SPEECH_RATE = 1.0f
        private const val MIN_SPEECH_RATE = 0.5f
        private const val MAX_SPEECH_RATE = 2.0f
    }
}
