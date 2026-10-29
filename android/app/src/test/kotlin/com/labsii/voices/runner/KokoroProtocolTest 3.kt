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
            assertEquals("SAY 42 1.250 1.0 Hello CANCEL 99", KokoroProtocol.say(42, 1.25f, "Hello\nCANCEL 99"))
            assertEquals("SAY 1 1.000 1.0 Hello", KokoroProtocol.say(1, Float.NaN, "Hello"))
        } finally { Locale.setDefault(old) }
    }
    @Test fun pitchNeutralBandAndDecimals() {
        for (pitch in listOf(0.98f, 1f, 1.02f, Float.NaN, Float.POSITIVE_INFINITY, 0f, -1f)) {
            assertEquals("SAY 1 1.000 1.0 Hello", KokoroProtocol.say(1, 1f, "Hello", pitch))
        }
        assertEquals("SAY 1 1.000 1.02001 Hello", KokoroProtocol.say(1, 1f, "Hello", 1.02001f))
        assertEquals("SAY 1 1.000 0.97999 Hello", KokoroProtocol.say(1, 1f, "Hello", 0.97999f))
        assertEquals("SAY 1 1.000 0.76 Hello", KokoroProtocol.say(1, 1f, "Hello", 0.1f))
        assertEquals("SAY 1 1.000 1.24 Hello", KokoroProtocol.say(1, 1f, "Hello", 4f))
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
