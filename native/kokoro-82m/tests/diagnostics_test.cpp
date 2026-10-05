// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#include "tts_gpu_guard.h"

// Invoked twice in fresh processes, once with NNOPT_DIAGNOSTICS=0 and once =1.
// No OpenCL driver or GPU is needed: stage logging must not submit/wait on GPU.
int main() {
    nnopt_tts_begin(42);
    nnopt_tts_stage("diagnostic_test value=%d", 7);
    return 0;
}
