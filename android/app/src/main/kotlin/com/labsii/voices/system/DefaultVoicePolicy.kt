/*
 * Adreno System TTS integration.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
package com.labsii.voices.system

internal object DefaultVoicePolicy {
    fun select(
        requested: String?,
        preferred: String?,
        languageFallback: String?,
        available: Set<String>,
    ): String? = sequenceOf(requested, preferred, languageFallback)
        .filterNotNull()
        .firstOrNull(available::contains)
}
