/* Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0 */
package com.labsii.voices.runner

internal object DownloadConnectionPolicy {
    enum class Decision { DOWNLOAD, ASK, OFFLINE }

    fun decide(connected: Boolean, metered: Boolean, approved: Boolean): Decision = when {
        !connected -> Decision.OFFLINE
        metered && !approved -> Decision.ASK
        else -> Decision.DOWNLOAD
    }
}
