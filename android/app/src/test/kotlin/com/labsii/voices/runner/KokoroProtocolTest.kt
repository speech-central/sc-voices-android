package com.labsii.voices.runner

import org.junit.Assert.*
import org.junit.Test
import java.io.IOException
import java.util.Locale

class KokoroProtocolTest {
    @Test fun commandsAreLocaleIndependentAndCannotInjectNewlines() {
        val old = Locale.getDefault()
        try {
            Locale.setDefault(Locale.GERMANY)
            assertEquals("SAY 42 1.250 Hello CANCEL 99", KokoroProtocol.say(42, 1.25f, "Hello\nCANCEL 99"))
            assertEquals("SAY 1 1.000 Hello", KokoroProtocol.say(1, Float.NaN, "Hello"))
        } finally { Locale.setDefault(old) }
    }
    @Test fun strictFraming() {
        assertEquals(7L, KokoroProtocol.audioHeader("KOKORO_PCM_BEGIN 7 240 24000").id)
        assertEquals("CANCELLED", KokoroProtocol.end("KOKORO_UTT_END 7 CANCELLED").status)
        listOf("KOKORO_PCM_BEGIN 0 1 24000", "KOKORO_PCM_BEGIN 1 1440001 24000", "KOKORO_PCM_BEGIN 1 1 22050").forEach {
            assertThrows(IOException::class.java) { KokoroProtocol.audioHeader(it) }
        }
        assertThrows(IOException::class.java) { KokoroProtocol.end("KOKORO_UTT_END") }
        assertThrows(IOException::class.java) { KokoroProtocol.end("KOKORO_UTT_END 7 MAYBE") }
    }
}
