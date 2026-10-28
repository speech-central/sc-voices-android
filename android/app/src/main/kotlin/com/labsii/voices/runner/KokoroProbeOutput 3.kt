/* Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0 */
package com.labsii.voices.runner

internal class KokoroProbeOutput {
    var renderer: String? = null
        private set
    var compatible = false
        private set
    var reason: String? = null
        private set
    private val tail = ArrayDeque<String>()

    @Synchronized
    fun accept(line: String) {
        if (line.startsWith("KOKORO_DEVICE ")) {
            renderer = line.removePrefix("KOKORO_DEVICE ").trim().takeIf { it.isNotEmpty() }
        }
        if (line.startsWith("KOKORO_COMPATIBLE ")) {
            val fields = line.removePrefix("KOKORO_COMPATIBLE ").trim().split(' ', limit = 2)
            compatible = fields.firstOrNull() == "1"
            if (!compatible && reason == null) reason = fields.getOrNull(1)
        }
        if (line.startsWith("KOKORO_OPENCL_FAILURE ")) reason = line.removePrefix("KOKORO_OPENCL_FAILURE ")
        if (tail.size == 64) tail.removeFirst()
        tail.addLast(line.take(512))
    }

    @Synchronized
    fun result(exitCode: Int?, failure: String? = null): KokoroDeviceProbe.Result = KokoroDeviceProbe.Result(
        compatible = exitCode == 0 && compatible && failure == null,
        renderer = renderer,
        reason = failure ?: reason ?: if (exitCode != 0 || !compatible) "OpenCL check did not complete (exit $exitCode)." else null,
        diagnostics = "Probe exit=$exitCode\n" + tail.joinToString("\n"),
    )
}
