// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
package com.labsii.voices.runner

internal object KokoroPitch {
    fun effective(requested: Float): Float {
        if (!requested.isFinite() || requested <= 0f) return 1f
        val pitch = requested.coerceIn(0.76f, 1.24f)
        return if (pitch in 0.98f..1.02f) 1f else pitch
    }
}
