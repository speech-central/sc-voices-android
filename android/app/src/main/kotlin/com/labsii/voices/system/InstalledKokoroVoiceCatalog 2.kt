/*
 * Android system voice integration based on the VoxSherpa TTS architecture.
 * VoxSherpa TTS copyright (C) 2025 CodeBySonu95.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
package com.labsii.voices.system

import android.content.Context
import com.labsii.voices.runner.KokoroModelManager
import com.labsii.voices.runner.KokoroVoiceSpec
import com.labsii.voices.runner.KokoroVoices
import java.util.Locale

internal data class InstalledKokoroVoice(
    val spec: KokoroVoiceSpec,
) {
    val id: String get() = spec.id
    val displayName: String get() = "Kokoro ${spec.displayName}"
    val locale: Locale get() = spec.locale
    val voiceName: String get() = spec.voiceName
    val voicePackPath: String get() = spec.localPath
    val phonemizerVoice: String get() = spec.phonemizerVoice
}

/** Synchronous catalog used by Android's Binder voice-discovery callbacks. */
internal class InstalledKokoroVoiceCatalog(private val context: Context) {
    fun installed(): List<InstalledKokoroVoice> =
        if (KokoroModelManager.isRuntimeReady(context)) {
            KokoroVoices.all
                .filter { KokoroModelManager.isVoiceInstalled(context, it) }
                .map(::InstalledKokoroVoice)
        } else {
            emptyList()
        }

    fun preferred(): InstalledKokoroVoice? = installed().firstOrNull()

    fun findByVoiceName(voiceName: String?): InstalledKokoroVoice? =
        installed().firstOrNull { it.voiceName == voiceName }

    fun findForLanguage(language: String?, country: String? = null): InstalledKokoroVoice? {
        val voices = installed()
        val voice = voices.firstOrNull() ?: return null
        if (language.isNullOrBlank()) return voice
        val normalized = language.substringBefore('-').substringBefore('_').lowercase(Locale.ROOT)
        if (normalized != "en" && normalized != "eng") return null
        val wantsUk = country.equals("GB", ignoreCase = true) ||
            country.equals("GBR", ignoreCase = true) ||
            country.equals("UK", ignoreCase = true) ||
            language.contains("GB", ignoreCase = true) || language.contains("UK", ignoreCase = true)
        return voices.firstOrNull { (it.locale == Locale.UK) == wantsUk } ?: voice
    }
}

internal fun Locale.safeIso3Language(): String = try {
    isO3Language
} catch (_: Throwable) {
    ""
}
