// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace nnopt_tts {
inline float effective_pitch(float requested) {
    if (!std::isfinite(requested) || requested <= 0.f) return 1.f;
    const float pitch = std::clamp(requested, 0.76f, 1.24f);
    return pitch >= 0.98f && pitch <= 1.02f ? 1.f : pitch;
}

// pitch is already normalized per command. Neutral returns without allocating.
// Returns false on cancellation; pcm is unchanged until processing succeeds.
bool shift_pitch(std::vector<int16_t>& pcm, float pitch,
                 bool (*cancelled)());
}
