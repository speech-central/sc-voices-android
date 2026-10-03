/*
 * Android system-engine integration based on the VoxSherpa TTS architecture.
 * VoxSherpa TTS copyright (C) 2025 CodeBySonu95.
 * SPDX-License-Identifier: GPL-3.0-or-later
 * See NOTICE for complete attribution and modification information.
 */
package com.labsii.voices.system

import android.content.ComponentCallbacks2
import android.media.AudioFormat
import android.speech.tts.SynthesisCallback
import android.speech.tts.SynthesisRequest
import android.speech.tts.TextToSpeech
import android.speech.tts.TextToSpeechService
import android.speech.tts.Voice
import android.util.Log
import com.labsii.voices.runner.KokoroModelManager
import com.labsii.voices.runner.KokoroSession
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import java.util.Locale
import java.util.concurrent.locks.ReentrantLock

/** Android system TTS facade for the Adreno Kokoro-82M runtime. */
class AdrenoTtsService : TextToSpeechService() {
    private lateinit var catalog: InstalledKokoroVoiceCatalog
    private lateinit var session: KokoroSession
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
        session = KokoroSession(applicationContext)
        Log.i(
            TAG,
            "Service created (0.6.10, request-scoped protocol v2); installed voices=${catalog.installed().size}, " +
                "runtimeReady=${KokoroModelManager.isRuntimeReady(applicationContext)}",
        )
        super.onCreate()
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
        Log.i(TAG, "Android requested voices; returning ${installed.size}")
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
        synthesisLock.lock()
        val startedAt = System.nanoTime()
        var state: KokoroSession.Request? = null
        var callbackStarted = false
        var callbackFinished = false
        var chunkIndex = 0
        var chunkCount = 0
        try {
            val current = session.beginRequest()
            state = current
            activeRequest = current
            val voice = catalog.findByVoiceName(request.voiceName)
                ?: catalog.findForLanguage(request.language, request.country)
                ?: throw IllegalStateException("Kokoro is not installed or does not support ${request.language}")
            Log.i(TAG, "Synthesis begin id=${current.id}, voice=${voice.voiceName}")
            val chunks = SystemTtsText.chunks(request.charSequenceText ?: "", voice.locale)
            chunkCount = chunks.size
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

            runBlocking(Dispatchers.IO) {
                session.start(current, voice.voicePackPath, voice.phonemizerVoice)
            }
            for (chunk in chunks) {
                chunkIndex++
                if (current.cancelled.get()) return
                // PCM arrives asynchronously; callbacks remain on Android's
                // dedicated synthesis thread, never the reader/control threads.
                runBlocking {
                    session.speak(current, chunk, speechRate) { result ->
                        if (result.sampleRate != SAMPLE_RATE) {
                            throw IllegalStateException("Unexpected Kokoro sample rate ${result.sampleRate}")
                        }
                        if (!current.cancelled.get()) {
                            streamPcm(result.samples, callback) { current.cancelled.get() }
                        }
                    }
                }
                if (current.cancelled.get()) {
                    Log.i(TAG, "Cancelled native utterance drained; preserving warm session")
                    return
                }
            }

            if (!current.cancelled.get()) {
                callback.done()
                callbackFinished = true
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
            Log.i(TAG, "Synthesis finished id=${state?.id}, cancelled=${state?.cancelled?.get() == true}, " +
                "elapsedMs=${(System.nanoTime() - startedAt) / 1_000_000}")
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
        Log.i(TAG, "Synthesis stop requested; active=${state != null}, id=${state?.id}")
        if (state != null && ::session.isInitialized) {
            // Request cancellation from a separate native stdin reader. The
            // graph checks it between safe GPU stages and keeps the warm model
            // on the normal path. A wedged driver still gets a bounded escape
            // hatch, but `onStop()` itself never blocks Android's caller.
            session.requestCancel(state)
        }
    }

    override fun onDestroy() {
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
    ) {
        val maxBytes = (callback.maxBufferSize.takeIf { it >= 2 } ?: DEFAULT_CALLBACK_BUFFER_BYTES)
            .let { it - (it % 2) }
        val buffer = ByteArray(maxBytes)
        var sampleOffset = 0
        while (sampleOffset < pcm.size) {
            if (isCancelled()) return
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
                if (isCancelled()) return
                throw IllegalStateException("TTS client rejected a Kokoro audio chunk")
            }
            sampleOffset += count
        }
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
