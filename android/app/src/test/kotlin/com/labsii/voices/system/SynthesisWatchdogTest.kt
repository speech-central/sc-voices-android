package com.labsii.voices.system

import org.junit.Assert.*
import org.junit.Test

class SynthesisWatchdogTest {
    private fun learned(): SynthesisWatchdog = SynthesisWatchdog().apply {
        repeat(5) { record(7000, 5000.0, 30000) }
    }

    @Test fun learnsDeviceSpeedAndUsesRateOnce() {
        val w = learned()
        assertEquals(1.4, w.estimatedRtf, 0.001)
        assertEquals(29300.0, w.timeoutMs(162, 1.15f).toDouble(), 10.0)
        assertTrue(w.timeoutMs(162, 2f) < w.timeoutMs(162, 1f))
    }

    @Test fun removesCharacterCliff() {
        val w = learned()
        assertTrue(w.timeoutMs(161, 1f) - w.timeoutMs(160, 1f) < 200)
    }

    @Test fun conservativeWarmupAndBounds() {
        val w = SynthesisWatchdog()
        repeat(2) { w.record(5000, 5000.0, 20000) }
        assertEquals(2.0, w.estimatedRtf, 0.0)
        assertEquals(10000L, w.timeoutMs(1, 1f))
        assertEquals(120000L, w.timeoutMs(Int.MAX_VALUE, 1f))
        assertEquals(w.timeoutMs(100, 1f), w.timeoutMs(100, Float.NaN))
    }

    @Test fun stallsCannotPoisonEstimate() {
        val w = learned()
        repeat(20) { w.record(68000, 8325.0, 78000) }
        w.record(30000, 5000.0, 30000)
        w.record(500, Double.NaN, 30000)
        assertEquals(1.4, w.estimatedRtf, 0.001)
    }

    @Test fun adaptsToSustainedOrdinarySlowdown() {
        val w = learned()
        repeat(9) { w.record(9000, 5000.0, 30000) }
        assertEquals(1.8, w.estimatedRtf, 0.001)
    }
}
