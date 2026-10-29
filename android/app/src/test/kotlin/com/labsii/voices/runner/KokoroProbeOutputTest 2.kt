package com.labsii.voices.runner

import org.junit.Assert.*
import org.junit.Test

class KokoroProbeOutputTest {
    @Test fun protocolSurvivesVerboseDriverOutput() {
        val output = KokoroProbeOutput()
        repeat(100) { output.accept("initial driver log $it") }
        output.accept("KOKORO_DEVICE Mali-G715")
        output.accept("KOKORO_COMPATIBLE 1")
        repeat(100) { output.accept("later driver log $it") }
        val result = output.result(0)
        assertTrue(result.compatible)
        assertEquals("Mali-G715", result.renderer)
        assertNull(result.reason)
        assertEquals(65, result.diagnostics.lines().size)
        assertTrue(result.diagnostics.endsWith("later driver log 99"))
    }

    @Test fun rejectedDriverKeepsSpecificFailure() {
        val output = KokoroProbeOutput()
        output.accept("KOKORO_OPENCL_FAILURE No usable OpenCL driver could be loaded.")
        output.accept("KOKORO_COMPATIBLE 0 OpenCL initialization failed")
        assertFalse(output.result(1).compatible)
        assertEquals("No usable OpenCL driver could be loaded.", output.result(1).reason)
    }

    @Test fun successRequiresExactFlagAndSuccessfulExit() {
        val output = KokoroProbeOutput()
        output.accept("KOKORO_COMPATIBLE 10")
        assertFalse(output.result(0).compatible)
        output.accept("KOKORO_COMPATIBLE 1")
        assertFalse(output.result(1).compatible)
        assertFalse(output.result(0, "Timed out").compatible)
        assertEquals("Timed out", output.result(0, "Timed out").reason)
    }
}
