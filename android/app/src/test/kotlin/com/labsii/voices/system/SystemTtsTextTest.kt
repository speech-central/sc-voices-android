/* SPDX-License-Identifier: GPL-3.0-or-later */
package com.labsii.voices.system

import java.util.Locale
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class SystemTtsTextTest {
    @Test
    fun `empty text has no chunks`() {
        assertTrue(SystemTtsText.chunks("  \n ", Locale.ENGLISH).isEmpty())
    }

    @Test
    fun `sentences are separated and long input is bounded`() {
        val text = "First sentence. Second sentence! " + "word ".repeat(100)
        val chunks = SystemTtsText.chunks(text, Locale.ENGLISH)

        assertEquals("First sentence.", chunks[0])
        assertEquals("Second sentence!", chunks[1])
        assertTrue(chunks.all { it.length <= 320 })
        assertEquals(text.replace(Regex("\\s+"), " ").trim(), chunks.joinToString(" "))
    }

    @Test
    fun `hard splitting does not break a surrogate pair`() {
        val text = "a".repeat(319) + "🙂".repeat(10)
        val chunks = SystemTtsText.chunks(text, Locale.ENGLISH)

        assertEquals(text, chunks.joinToString(""))
        assertTrue(chunks.none { it.lastOrNull()?.let { char -> Character.isHighSurrogate(char) } == true })
    }
}
