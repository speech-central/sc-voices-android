// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#include "pitch_shift.h"
#include <rubberband/RubberBandStretcher.h>
#include <array>
#include <stdexcept>
#include <string>

namespace nnopt_tts {
bool shift_pitch(std::vector<int16_t>& pcm, float pitch, bool (*cancelled)()) {
    if (pitch == 1.f || pcm.empty()) return !cancelled();
    if (!std::isfinite(pitch) || pitch < 0.76f || pitch > 1.24f || pcm.size() > 1440000)
        throw std::invalid_argument("Unsafe pitch processing request");
    if (cancelled()) return false;

    using R = RubberBand::RubberBandStretcher;
    constexpr size_t block = 1024;
    // Give the analysis window enough input for tiny fragments; discard this
    // trailing padding from the result, never extend the utterance duration.
    const size_t inputSize = std::max(pcm.size(), size_t(4096));
    R processor(24000, 1, R::OptionProcessOffline | R::OptionEngineFiner |
        R::OptionFormantPreserved | R::OptionThreadingNever, 1.0, pitch);
    processor.setExpectedInputDuration(inputSize);
    processor.setMaxProcessSize(block);
    std::array<float, block> input{}, output{};
    const float* in[] = {input.data()};
    float* out[] = {output.data()};
    auto fill = [&](size_t offset, size_t count) {
        for (size_t i = 0; i < count; ++i)
            input[i] = offset + i < pcm.size() ? pcm[offset + i] / 32768.f : 0.f;
    };
    for (size_t offset = 0; offset < inputSize; offset += block) {
        if (cancelled()) return false;
        const size_t count = std::min(block, inputSize - offset);
        fill(offset, count);
        processor.study(in, count, offset + count == inputSize);
    }

    std::vector<int16_t> shifted;
    shifted.reserve(pcm.size());
    size_t outputSize = 0;
    auto drain = [&]() {
        int available;
        while ((available = processor.available()) > 0) {
            if (cancelled()) return false;
            const size_t count = processor.retrieve(out, std::min(block, size_t(available)));
            if (!count || outputSize + count > inputSize)
                throw std::runtime_error("Unexpected pitch output length");
            outputSize += count;
            for (size_t i = 0; i < count; ++i) {
                if (!std::isfinite(output[i]))
                    throw std::runtime_error("Non-finite pitch output");
                if (shifted.size() < pcm.size())
                    shifted.push_back(static_cast<int16_t>(std::lround(
                        std::clamp(output[i] * 32768.f, -32768.f, 32767.f))));
            }
        }
        return true;
    };
    for (size_t offset = 0; offset < inputSize; offset += block) {
        if (cancelled()) return false;
        const size_t count = std::min(block, inputSize - offset);
        fill(offset, count);
        processor.process(in, count, offset + count == inputSize);
        if (!drain()) return false;
    }
    // Offline, single-thread mode must finish synchronously, with duration unchanged.
    if (processor.available() != -1 || shifted.size() != pcm.size())
        throw std::runtime_error("Incomplete pitch output: " + std::to_string(shifted.size()) +
            "/" + std::to_string(pcm.size()) + " available=" + std::to_string(processor.available()));
    if (cancelled()) return false;
    pcm.swap(shifted);
    return true;
}
}
