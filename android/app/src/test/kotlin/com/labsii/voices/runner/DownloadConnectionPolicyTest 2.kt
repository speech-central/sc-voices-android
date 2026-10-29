package com.labsii.voices.runner

import org.junit.Assert.assertEquals
import org.junit.Test

class DownloadConnectionPolicyTest {
    @Test fun connectionChangesRequireConsentBeforeMeteredTransfer() {
        assertEquals(DownloadConnectionPolicy.Decision.DOWNLOAD, decide(true, false, false))
        assertEquals(DownloadConnectionPolicy.Decision.ASK, decide(true, true, false))
        assertEquals(DownloadConnectionPolicy.Decision.DOWNLOAD, decide(true, true, true))
        assertEquals(DownloadConnectionPolicy.Decision.OFFLINE, decide(false, true, true))
        assertEquals(DownloadConnectionPolicy.Decision.OFFLINE, decide(false, false, false))
        // Consent is supplied by the current attempt, not persisted for later downloads.
        assertEquals(DownloadConnectionPolicy.Decision.ASK, decide(true, true, false))
    }

    private fun decide(connected: Boolean, metered: Boolean, approved: Boolean) =
        DownloadConnectionPolicy.decide(connected, metered, approved)
}
