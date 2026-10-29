/*
 * Android system-engine integration based on the VoxSherpa TTS architecture.
 * VoxSherpa TTS copyright (C) 2025 CodeBySonu95.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
package com.labsii.voices.system

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.speech.tts.TextToSpeech
import android.util.Log

/** Legacy compatibility endpoint used by Android TTS settings on some devices. */
class GetSampleTextActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val language = intent.getStringExtra("language").orEmpty()
        val baseLanguage = language.substringBefore('-').substringBefore('_')
        val supported = baseLanguage.isBlank() ||
            baseLanguage.equals("en", true) || baseLanguage.equals("eng", true)
        if (supported && InstalledKokoroVoiceCatalog(applicationContext).installed().isNotEmpty()) {
            setResult(
                RESULT_OK,
                Intent().putExtra(
                    TextToSpeech.Engine.EXTRA_SAMPLE_TEXT,
                    "Kokoro SC is ready to read this sample aloud.",
                ),
            )
            Log.i(TAG, "Returned English sample text")
        } else {
            setResult(RESULT_CANCELED)
            Log.i(TAG, "Sample text unavailable for language=$language")
        }
        finish()
    }

    private companion object {
        const val TAG = "GetSampleTextActivity"
    }
}
