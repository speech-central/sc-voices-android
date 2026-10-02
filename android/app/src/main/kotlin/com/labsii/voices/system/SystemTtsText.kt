/*
 * Adreno System TTS integration.
 * SPDX-License-Identifier: GPL-3.0-or-later
 * See NOTICE for upstream copyright and attribution information.
 */
package com.labsii.voices.system

import java.text.BreakIterator
import java.util.Locale

/** Preserves normal sentences while keeping pathological native requests bounded. */
internal object SystemTtsText {
    private const val MAX_CHARS_PER_REQUEST = 320
    private const val MIN_PREFERRED_SPLIT = MAX_CHARS_PER_REQUEST / 2

    fun chunks(text: CharSequence, locale: Locale): List<String> {
        val normalized = text.toString().replace(Regex("\\s+"), " ").trim()
        if (normalized.isEmpty()) return emptyList()

        val sentences = mutableListOf<String>()
        val iterator = BreakIterator.getSentenceInstance(locale.takeUnless { it == Locale.ROOT } ?: Locale.getDefault())
        iterator.setText(normalized)
        var start = iterator.first()
        var end = iterator.next()
        while (end != BreakIterator.DONE) {
            appendBounded(normalized.substring(start, end).trim(), sentences)
            start = end
            end = iterator.next()
        }
        if (sentences.isEmpty()) appendBounded(normalized, sentences)
        return sentences
    }

    private fun appendBounded(value: String, output: MutableList<String>) {
        var rest = value.trim()
        while (rest.length > MAX_CHARS_PER_REQUEST) {
            // BreakIterator already separated ordinary sentences. For an
            // exceptionally long/complex sentence, prefer its latest clause
            // boundary in the latter half of the safe window. Only fall back
            // to a word boundary when there is no linguistically useful cut.
            var splitAt = preferredClauseBoundary(rest)
                ?: rest.lastIndexOf(' ', startIndex = MAX_CHARS_PER_REQUEST)
                    .takeIf { it > 0 }
                ?: MAX_CHARS_PER_REQUEST
            if (splitAt > 0 && Character.isLowSurrogate(rest[splitAt])) splitAt--
            output += rest.substring(0, splitAt).trim()
            rest = rest.substring(splitAt).trimStart()
        }
        if (rest.isNotEmpty()) output += rest
    }

    private fun preferredClauseBoundary(value: String): Int? {
        for (index in MAX_CHARS_PER_REQUEST downTo MIN_PREFERRED_SPLIT) {
            if (index >= value.length) continue
            if (value[index - 1] in ".!?;:," && value[index].isWhitespace()) return index
        }
        return null
    }
}
