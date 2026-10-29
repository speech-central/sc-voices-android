package com.labsii.voices.system

import kotlin.math.ceil

/**
 * Service-lifetime estimate, accessed only on the serialized synthesis thread.
 * Audio duration is measured at the engine's requested rate, so observed RTF
 * must not be rate-adjusted again. The 15 chars/s proxy is only for prediction.
 */
internal class SynthesisWatchdog {
    private val ratios = ArrayDeque<Double>()

    val estimatedRtf: Double
        get() {
            if (ratios.size < 3) return 2.0
            val sorted = ratios.sorted()
            // Upper quartile tolerates ordinary variation without allowing a
            // single slow completion to dominate the next deadline.
            return sorted[((sorted.size - 1) * 0.75).toInt()].coerceAtLeast(0.1)
        }

    fun timeoutMs(chars: Int, speechRate: Float): Long {
        val rate = speechRate.toDouble().takeIf { it.isFinite() && it > 0 } ?: 1.0
        val predictedAudioMs = chars.coerceAtLeast(0) * 1000.0 / (15.0 * rate)
        return ceil(3000.0 + 2.0 * predictedAudioMs * estimatedRtf)
            .toLong().coerceIn(10_000L, 120_000L)
    }

    /** Call only after successful, uncancelled synthesis without any retry. */
    fun record(generationMs: Long, audioMs: Double, deadlineMs: Long) {
        if (generationMs <= 0 || generationMs >= deadlineMs ||
            !audioMs.isFinite() || audioMs < 1000.0) return
        val ratio = generationMs / audioMs
        // A recovered long stall must not normalize long waits. A fresh service
        // starts conservatively again; timeout/retry paths never call this.
        if (!ratio.isFinite() || ratio > estimatedRtf * 2.0) return
        ratios.addLast(ratio)
        if (ratios.size > 9) ratios.removeFirst()
    }
}
