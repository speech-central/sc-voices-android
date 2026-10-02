/* Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0 */
package com.labsii.voices.runner

import java.io.IOException
import java.util.Locale

/** Strict v2 framing, shared by the worker and JVM regression tests. */
internal object KokoroProtocol {
    sealed interface Event { val id: Long }
    data class Audio(override val id: Long, val samples: ShortArray, val sampleRate: Int) : Event {
        override fun equals(other: Any?): Boolean {
            if (this === other) return true
            if (javaClass != other?.javaClass) return false

            other as Audio

            if (id != other.id) return false
            if (sampleRate != other.sampleRate) return false
            if (!samples.contentEquals(other.samples)) return false

            return true
        }

        override fun hashCode(): Int {
            var result = id.hashCode()
            result = 31 * result + sampleRate
            result = 31 * result + samples.contentHashCode()
            return result
        }
    }

    data class End(override val id: Long, val status: String) : Event
    data class Header(val id: Long, val samples: Int, val rate: Int)

    fun say(id: Long, rate: Float, text: String): String {
        require(id > 0)
        val clean = text.replace(Regex("[\\r\\n\\u0000]+"), " ").trim()
        require(clean.isNotEmpty() && clean.toByteArray(Charsets.UTF_8).size <= 12_000)
        val speed = if (rate.isFinite()) rate.coerceIn(0.5f, 2.0f) else 1.0f
        return "SAY %d %.3f %s".format(Locale.ROOT, id, speed, clean)
    }

    fun audioHeader(line: String): Header {
        val parts = line.split(' ')
        if (parts.size != 4 || parts[0] != "KOKORO_PCM_BEGIN") throw IOException("Invalid PCM header")
        val id = parts[1].toLongOrNull() ?: throw IOException("Invalid PCM request ID")
        val count = parts[2].toIntOrNull() ?: throw IOException("Invalid PCM length")
        val rate = parts[3].toIntOrNull() ?: throw IOException("Invalid PCM rate")
        if (id <= 0 || count !in 1..1_440_000 || rate != 24_000) throw IOException("Unsafe PCM header")
        return Header(id, count, rate)
    }

    fun end(line: String): End {
        val parts = line.split(' ')
        if (parts.size != 3 || parts[0] != "KOKORO_UTT_END") throw IOException("Invalid terminal frame")
        val id = parts[1].toLongOrNull() ?: throw IOException("Invalid terminal request ID")
        if (id <= 0 || parts[2] !in setOf("OK", "CANCELLED", "ERROR")) throw IOException("Invalid terminal status")
        return End(id, parts[2])
    }
}
