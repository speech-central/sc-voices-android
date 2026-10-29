package com.labsii.voices.system

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

class DefaultVoicePolicyTest {
    private val available = setOf("af_heart", "bf_emma", "af_bella")

    @Test fun explicitClientVoiceTakesPriority() {
        assertEquals(
            "af_bella",
            DefaultVoicePolicy.select("af_bella", "bf_emma", "af_heart", available),
        )
    }

    @Test fun savedEngineDefaultIsUsedWithoutAnExplicitVoice() {
        assertEquals(
            "bf_emma",
            DefaultVoicePolicy.select(null, "bf_emma", "af_heart", available),
        )
    }

    @Test fun unavailableChoicesFallBackToLanguageThenNull() {
        assertEquals(
            "af_heart",
            DefaultVoicePolicy.select("missing", "also_missing", "af_heart", available),
        )
        assertNull(DefaultVoicePolicy.select("missing", null, null, available))
    }
}
