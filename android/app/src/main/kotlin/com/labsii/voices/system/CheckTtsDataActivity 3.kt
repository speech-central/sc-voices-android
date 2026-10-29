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

/** Compatibility endpoint used by Android's TTS settings screen to query installed voice data. */
class CheckTtsDataActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val voices = InstalledKokoroVoiceCatalog(applicationContext).installed()
        // ACTION_CHECK_TTS_DATA predates BCP-47. Its documented wire format is
        // lang-COUNTRY-variant using ISO 639-2 and ISO 3166-1 alpha-3 codes
        // (for example eng-USA), not Locale.toLanguageTag() values such as
        // en-US. Some Android builds tolerate BCP-47 while others reject the
        // complete result, so expose the legacy language plus country forms.
        val available = arrayListOf<String>()
        if (voices.isNotEmpty()) available += "eng"
        if (voices.any { it.locale.country.equals("US", ignoreCase = true) }) available += "eng-USA"
        if (voices.any { it.locale.country.equals("GB", ignoreCase = true) }) available += "eng-GBR"
        val unavailable = if (voices.isEmpty()) {
            arrayListOf("eng")
        } else {
            arrayListOf()
        }
        val data = Intent().apply {
            putStringArrayListExtra(TextToSpeech.Engine.EXTRA_AVAILABLE_VOICES, available)
            putStringArrayListExtra(TextToSpeech.Engine.EXTRA_UNAVAILABLE_VOICES, unavailable)
        }
        val result = if (voices.isEmpty()) {
            TextToSpeech.Engine.CHECK_VOICE_DATA_FAIL
        } else {
            TextToSpeech.Engine.CHECK_VOICE_DATA_PASS
        }
        Log.i(TAG, "Voice-data check: installed=${voices.size}, available=$available, result=$result")
        setResult(result, data)
        finish()
    }

    private companion object {
        const val TAG = "CheckTtsDataActivity"
    }
}
