/* Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0 */
package com.labsii.voices.runner

import android.content.Context
import android.os.Bundle
import android.speech.tts.TextToSpeech
import android.speech.tts.UtteranceProgressListener
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.coroutines.withTimeout
import java.io.File
import java.io.IOException
import java.util.UUID
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.time.Duration.Companion.milliseconds

/** Tests real synthesis through the shared Android service, not a second GPU worker. */
internal object KokoroSelfTest {
    suspend fun run(context: Context) = withContext(Dispatchers.IO) {
        val initialized = CompletableDeferred<Int>()
        val finished = CompletableDeferred<Unit>()
        val audible = AtomicBoolean()
        val utteranceId = "self-test-${UUID.randomUUID()}"
        val output = File.createTempFile("kokoro-self-test-", ".wav", context.cacheDir)
        val tts = TextToSpeech(context.applicationContext, { initialized.complete(it) }, context.packageName)
        try {
            withTimeout(150_000L.milliseconds) {
                if (initialized.await() != TextToSpeech.SUCCESS) throw IOException("Could not connect to the engine")
                // Android can fall back to another engine. Require our exact voice.
                val voice = tts.voices?.firstOrNull { it.name == KokoroVoices.all.first().voiceName }
                    ?: throw IOException("The installed Kokoro voice is not advertised by Android")
                if (tts.setVoice(voice) != TextToSpeech.SUCCESS) throw IOException("Could not select the Kokoro voice")
                tts.setOnUtteranceProgressListener(object : UtteranceProgressListener() {
                    override fun onStart(id: String?) = Unit
                    override fun onAudioAvailable(id: String?, audio: ByteArray?) {
                        if (id != utteranceId || audio == null) return
                        var index = 0
                        while (index + 1 < audio.size) {
                            val sample = ((audio[index].toInt() and 255) or (audio[index + 1].toInt() shl 8)).toShort().toInt()
                            if (kotlin.math.abs(sample) > 32) { audible.set(true); break }
                            index += 2
                        }
                    }
                    override fun onDone(id: String?) {
                        if (id == utteranceId) finished.complete(Unit)
                    }
                    @Deprecated("Android legacy callback")
                    override fun onError(id: String?) {
                        if (id == utteranceId) finished.completeExceptionally(IOException("Kokoro synthesis failed on this driver"))
                    }
                    override fun onStop(id: String?, interrupted: Boolean) {
                        if (id == utteranceId) finished.completeExceptionally(IOException("Compatibility test interrupted; retry when reading has stopped"))
                    }
                })
                if (tts.synthesizeToFile("This is a short voice compatibility test.", Bundle.EMPTY, output, utteranceId) != TextToSpeech.SUCCESS)
                    throw IOException("Android rejected the compatibility test")
                finished.await()
                if (!audible.get() || output.length() <= 44) throw IOException("The driver returned empty or silent audio")
            }
        } finally {
            tts.stop()
            tts.shutdown()
            output.delete()
        }
    }
}
