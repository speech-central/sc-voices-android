// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdio>
#include <cstdlib>

inline void startup_marker(const char* stage, const char* detail = "") {
    const char* enabled = std::getenv("NNOPT_DIAGNOSTICS");
    if (!enabled || enabled[0] != '1') return;
    std::fprintf(stderr, "KOKORO_STARTUP stage=%s %s\n", stage, detail);
    std::fflush(stderr);
}
