// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#include "pitch_shift.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <limits>

static int checks = 0, cancel_at = 0;
static bool cancelled() { return cancel_at > 0 && ++checks >= cancel_at; }
static std::vector<int16_t> tone(size_t size) {
    std::vector<int16_t> pcm(size);
    for (size_t i = 0; i < size; ++i)
        pcm[i] = int16_t(12000 * std::sin(2 * 3.141592653589793 * 220 * i / 24000));
    return pcm;
}
static double frequency(const std::vector<int16_t>& pcm) {
    size_t crossings = 0, begin = pcm.size() / 4, end = 3 * pcm.size() / 4;
    for (size_t i = begin + 1; i < end; ++i)
        if (pcm[i - 1] <= 0 && pcm[i] > 0) ++crossings;
    return crossings * 24000.0 / (end - begin);
}
int main() {
    using namespace nnopt_tts;
    for (float p : {0.98f, 0.999999f, 1.f, 1.00001f, 1.02f,
                    std::numeric_limits<float>::quiet_NaN(), -1.f, 0.f}) {
        auto pcm = tone(48000), original = pcm;
        const auto* data = pcm.data();
        assert(effective_pitch(p) == 1.f);
        assert(shift_pitch(pcm, effective_pitch(p), cancelled));
        assert(pcm == original && pcm.data() == data);
    }
    assert(effective_pitch(0.5f) == 0.76f && effective_pitch(2.f) == 1.24f);
    assert(effective_pitch(1.02001f) == 1.02001f);
    assert(effective_pitch(0.97999f) == 0.97999f);
    const auto started = std::chrono::steady_clock::now();
    for (float p : {0.76f, 0.9f, 0.979f, 1.021f, 1.1f, 1.24f}) {
        auto pcm = tone(48000);
        assert(shift_pitch(pcm, p, cancelled));
        assert(pcm.size() == 48000);
        const double hz = frequency(pcm);
        assert(std::abs(hz - 220 * p) < 4.0);
        std::cout << "ratio=" << p << " measuredHz=" << hz << '\n';
    }
    for (size_t length : {size_t(1), size_t(240), size_t(1023), size_t(1024), size_t(1025), size_t(1440000)}) {
        for (float p : {0.76f, 1.24f}) {
            auto pcm = tone(length);
            assert(shift_pitch(pcm, p, cancelled));
            assert(pcm.size() == length);
        }
    }
    std::vector<int16_t> silence(24000);
    assert(shift_pitch(silence, 1.24f, cancelled));
    for (auto sample : silence) assert(sample == 0);
    for (int stop : {1, 10, 55, 70}) {
        checks = 0; cancel_at = stop;
        auto pcm = tone(48000), original = pcm;
        assert(!shift_pitch(pcm, 0.76f, cancelled));
        assert(pcm == original);
    }
    cancel_at = 0;
    auto next = tone(48000);
    assert(shift_pitch(next, 1.24f, cancelled));
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    std::cout << "Pitch ratio, duration, neutral bypass, silence, bounds and cancellation passed ("
              << seconds << " s host test time)\n";
}
