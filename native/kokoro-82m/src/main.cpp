// Auto-generated graph-mode main.cpp for hexgrad/Kokoro-82M.
// Modality: input=tokens, output=waveform
//
// In graph mode main.cpp is small: tokenize/load input, instantiate Model,
// loop a generate step (or single forward for non-autoregressive), write
// output. The interesting code lives in src/ops/*.cpp.
//
// API REFERENCE (DO NOT GUESS — these are the canonical signatures the
// scaffold emits; matching them avoids the iteration tax of inventing
// non-existent methods like load_from_disk / decode_one / sample_argmax):
//
//   class Weights {
//     Weights();                                 // default ctor only — no OpenCLContext arg
//     bool load(const std::string& bin_path,
//               const std::string& meta_path,
//               cl_context ctx);
//     cl_mem get_buffer(const std::string& key, bool optional=false);
//     // --- Introspection (use these when CLBlast returns kInsufficientMemoryB
//     //     or any dim-mismatch error: compare your op's expected dim against
//     //     the actual on-disk shape BEFORE editing model_config.h or kernels):
//     bool has_tensor(const std::string& key) const;
//     std::vector<int> get_shape(const std::string& key) const;       // [] if missing
//     size_t get_num_elements(const std::string& key) const;          // 0 if missing
//     size_t get_size_bytes(const std::string& key) const;            // 0 if missing
//     std::string get_dtype(const std::string& key) const;            // "" if missing
//   };
//   class Tokenizer {
//     bool load(const std::string& vocab_path); // singular path, NOT (bin, meta)
//     std::vector<int> encode(const std::string& text);
//     std::string decode(const std::vector<int32_t>& ids); // takes vector, NOT single int
//     int eos_token_id() const;
//   };
//   class Sampler {                              // class-based, NOT a free function
//     Sampler(const SamplerConfig& cfg);
//     int sample(std::vector<float>& logits,
//                const std::vector<int32_t>& generated_ids) const;
//   };

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "model.h"
#include "model_config.h"
#include "opencl_context.h"
#include "weights.h"
#include "sampler.h"
#include "tokenizer.h"
#include "debug_utils.h"
#include "version.h"
#include "benchmark.h"  // BenchmarkTimer + NNOPT_BENCH_FIRST_TOKEN — see prefill/decode call sites below
#include "profiler.h"   // KernelProfiler::dump_summary — dormant unless NNOPT_PROFILE=1.

extern "C" void nnopt_gen_reclaim_scratch_after_drain();


#include "load_bin.h"    // load_int32_bin, load_float_bin (fixture loaders)
#include "write_wav.h"   // write_wav (RIFF int16 PCM, 16kHz mono)

// On-device streaming (--stream): G2P (espeak-ng) -> chunked synth -> raw int16
// PCM on stdout. Compiled only when espeak-ng is vendored (CMake defines
// NNOPT_TTS_STREAMING); without it --stream reports an error.
#ifdef NNOPT_TTS_STREAMING
#include "phonemizer.h"
#include <cstdio>
#endif


#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include "cancellation.h"
#include "stream_commands.h"

// Read prompt input ids from a binary file (int32 little-endian) when the
// caller passes \`--token-ids /path/to/test_input_ids.bin\`. This path is
// the deterministic-evaluation contract — Evaluate compares the C++ output
// against the PyTorch reference for the SAME input ids, so the binary
// MUST consume the file rather than re-tokenizing the prompt string (which
// can produce different ids if the encoder isn't perfectly aligned with
// HF's tokenizer for this model family).
static bool read_input_ids_bin(const std::string& path, std::vector<int32_t>& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    const std::streamsize n_bytes = f.tellg();
    f.seekg(0, std::ios::beg);
    if (n_bytes <= 0 || (n_bytes % sizeof(int32_t)) != 0) return false;
    out.resize(static_cast<size_t>(n_bytes) / sizeof(int32_t));
    f.read(reinterpret_cast<char*>(out.data()), n_bytes);
    return f.good() || f.eof();
}

// The installer uses this before downloading ~170 MiB of model data. Test the
// exact context selected by inference, require fp16, then compile and dispatch
// a tiny fp16 kernel. An extension string alone is insufficient: some Android
// drivers advertise cl_khr_fp16 but fail when a program actually uses it.
static bool probe_kokoro_opencl(OpenCLContext& cl_ctx, std::string& reason) {
    size_t extension_size = 0;
    if (clGetDeviceInfo(cl_ctx.device(), CL_DEVICE_EXTENSIONS, 0, nullptr, &extension_size) != CL_SUCCESS) {
        reason = "Could not query OpenCL extensions";
        return false;
    }
    std::string extensions(extension_size, '\0');
    if (extension_size == 0 ||
        clGetDeviceInfo(cl_ctx.device(), CL_DEVICE_EXTENSIONS, extension_size, extensions.data(), nullptr) != CL_SUCCESS ||
        extensions.find("cl_khr_fp16") == std::string::npos) {
        reason = "OpenCL fp16 is unavailable";
        return false;
    }

    static const char* source = R"CLC(
        #pragma OPENCL EXTENSION cl_khr_fp16 : enable
        __kernel void kokoro_fp16_probe(__global half* output) {
            output[0] = convert_half(1.0f);
        }
    )CLC";
    cl_program program = cl_ctx.build_program(source);
    if (!program) {
        reason = "The OpenCL driver cannot compile Kokoro fp16 kernels";
        return false;
    }
    cl_int error = CL_SUCCESS;
    cl_kernel kernel = clCreateKernel(program, "kokoro_fp16_probe", &error);
    cl_mem output = error == CL_SUCCESS
        ? clCreateBuffer(cl_ctx.context(), CL_MEM_WRITE_ONLY, sizeof(cl_half), nullptr, &error)
        : nullptr;
    if (error == CL_SUCCESS) error = clSetKernelArg(kernel, 0, sizeof(output), &output);
    const size_t global_size = 1;
    if (error == CL_SUCCESS) {
        error = clEnqueueNDRangeKernel(
            cl_ctx.queue(), kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
    }
    if (error == CL_SUCCESS) error = clFinish(cl_ctx.queue());
    if (output) clReleaseMemObject(output);
    if (kernel) clReleaseKernel(kernel);
    clReleaseProgram(program);
    if (error != CL_SUCCESS) {
        reason = "The OpenCL driver cannot execute Kokoro fp16 kernels";
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    // Build marker — lets us confirm from the engine log which binary is
    // actually running. Bump the tag on every device-facing engine change.
    fprintf(stderr, "[engine] kokoro build: 0.6.10 protocol=2\n");
    // (No version banner — debug_utils does not define one, and emitting an
    // undefined macro here was breaking every fresh-port build.)
    // Argument parsing: positional "prompt" + optional flags.
    //   ./binary "<prompt>" [max_new_tokens] [--token-ids <file>]
    std::string prompt = "The teacher worked at the ";
    int max_new_tokens = 16;
    std::string token_ids_path;
    bool serve_mode = false;   // --serve: persistent streaming server (see below)
    bool stream_mode = false;  // --stream: on-device G2P + chunked PCM to stdout
    bool serve_stream_mode = false;  // --serve-stream: persistent --stream REPL (app)
    bool device_info_mode = false;  // --device-info: OpenCL renderer probe for the app UI
    std::string voice = "en-us";
    // Style-embedding voice pack. Defaults to the bundled af_heart; the app
    // passes --voicepack assets/voices/<name>.bin to switch among downloaded
    // voices (the 82M model is shared across all 54 voices).
    std::string voicepack = "assets/voice_pack_af_heart.bin";

    SamplerConfig sampler_config;
    sampler_config.temperature = 0.0f;          // greedy by default — matches PyTorch reference
    sampler_config.top_k = 1;
    sampler_config.top_p = 1.0f;
    sampler_config.repetition_penalty = 1.0f;
    sampler_config.seed = 42u;
    {
        int positional = 0;
        for (int i = 1; i < argc; i++) {
            const std::string a = argv[i];
            if (a == "--token-ids" && i + 1 < argc) {
                token_ids_path = argv[++i];
            } else if (a == "--serve") {
                serve_mode = true;
            } else if (a == "--stream") {
                stream_mode = true;
            } else if (a == "--serve-stream") {
                serve_stream_mode = true;
            } else if (a == "--device-info") {
                device_info_mode = true;
            } else if (a == "--voice" && i + 1 < argc) {
                voice = argv[++i];
            } else if (a == "--voicepack" && i + 1 < argc) {
                voicepack = argv[++i];
            } else if (a == "--temperature" && i + 1 < argc) {
                sampler_config.temperature = std::stof(argv[++i]);
            } else if (a == "--top-k" && i + 1 < argc) {
                sampler_config.top_k = std::stoi(argv[++i]);
            } else if (a == "--top-p" && i + 1 < argc) {
                sampler_config.top_p = std::stof(argv[++i]);
            } else if (a == "--seed" && i + 1 < argc) {
                sampler_config.seed = static_cast<uint32_t>(std::stoul(argv[++i]));
            } else if (a.rfind("--", 0) == 0) {
                // Unknown flag — skip the value too if present.
                if (i + 1 < argc && argv[i + 1][0] != '-') ++i;
            } else if (positional == 0) {
                prompt = a; positional++;
            } else if (positional == 1) {
                max_new_tokens = std::stoi(a); positional++;
            }
        }
    }

    // Benchmark instrumentation — emits BENCHMARK <key>: <value> lines on stderr,
    // parsed by runUtils.ts::parseInferenceMetrics. Five call sites total:
    //   1. mark_inference_start()        — here, immediately after arg parsing
    //   2. mark_prefill_start()          — immediately before the first model.forward()
    //   3. NNOPT_BENCH_FIRST_TOKEN()     — inside generate loop, right after first sampler.sample()
    //   4. mark_end()                    — after the decode loop exits
    //   5. print_summary(prompt_len, gen) — just before return 0
    // If you restructure prefill+decode (e.g. split into a single batched prefill
    // forward followed by per-token decode forwards), KEEP all five sites — they
    // partition wall-clock into ttft / prefill / decode correctly regardless of
    // loop shape. Removing any of them silently emits -1 for that metric.
    BenchmarkTimer& bench = BenchmarkTimer::instance();
    bench.mark_inference_start();

    OpenCLContext cl_ctx;
    if (!cl_ctx.initialize()) {
        if (device_info_mode) {
            std::cout << "KOKORO_COMPATIBLE 0 OpenCL initialization failed" << std::endl;
        }
        NNOPT_ERROR("OpenCL init failed");
        return 1;
    }
    if (device_info_mode) {
        std::cout << "KOKORO_DEVICE " << cl_ctx.device_name()
                  << " · " << cl_ctx.device_version() << std::endl;
        std::string reason;
        const bool compatible = probe_kokoro_opencl(cl_ctx, reason);
        std::cout << "KOKORO_COMPATIBLE " << (compatible ? "1" : "0")
                  << (reason.empty() ? "" : " ") << reason << std::endl;
        return compatible ? 0 : 1;
    }

    // Kokoro consumes pre-phonemized ids. The Android streaming modes obtain
    // them from the bundled eSpeak phonemizer; the deterministic probe reads
    // them from assets/test_input_ids.bin. There is no tokenizer_vocab.bin in
    // the published Kokoro bundle, so requiring one here prevents every real
    // text-to-speech mode from starting.

    // Weights — fp16 build pulls weights/model.fp16.bin; fp32 uses weights/model.bin.
    Weights weights;
#ifdef NNOPT_USE_FP16
    const char* nnopt_weights_bin  = "weights/model.fp16.bin";
    const char* nnopt_weights_meta = "weights/model.fp16.meta.json";
#else
    const char* nnopt_weights_bin  = "weights/model.bin";
    const char* nnopt_weights_meta = "weights/model.meta.json";
#endif
    if (!weights.load(nnopt_weights_bin, nnopt_weights_meta, cl_ctx.context())) {
        NNOPT_ERROR_FMT("weights load failed: %s", nnopt_weights_bin);
        return 1;
    }

    // Pre-allocate all GPU buffers upfront so the first inference doesn't pay
    // ~5s of per-tensor allocation cost spread across its kernel launches.
    // Gated by NNOPT_PRE_ALLOC_WEIGHTS=0 to disable (defaults ON).
    {
        const char* env = std::getenv("NNOPT_PRE_ALLOC_WEIGHTS");
        bool pre_alloc = !env || env[0] != '0';
        if (pre_alloc) {
            std::fprintf(stderr, "[engine] GPU weight preallocation begin\n");
            std::fflush(stderr);
            if (!weights.pre_allocate_all()) {
                NNOPT_ERROR("GPU weight preallocation failed; insufficient device memory");
                return 6;
            }
            std::fprintf(stderr, "[engine] GPU weight preallocation end\n");
            std::fflush(stderr);
        }
    }

    Sampler sampler(sampler_config);

    Model model(cl_ctx, weights);
    if (!model.initialize()) {
        NNOPT_ERROR("Model::initialize() failed — see prior NNOPT_ERROR for the layer that failed");
        return 1;
    }



    // ── Streaming server mode (--serve) ──────────────────────────────────
    // Persistent process: weights + CL context stay hot across utterance
    // chunks, so per-chunk cost is pure synthesis (no ~0.7s init/load tax).
    // Line protocol (stdin → stdout, one line each, stdout flushed):
    //   in:  SAY <ids_bin_path> <out_wav_path>
    //   in:  QUIT
    //   out: SERVER_READY                       (once, after model init)
    //   out: AUDIO_READY <out_wav_path> <n_samples> <synth_sec>
    //   out: ERROR <message>
    // The host streamer (scripts/stream.sh) chunks text at clause boundaries,
    // feeds ids files, and plays each WAV as it becomes ready.
    if (serve_mode) {
        auto vp = load_float_bin("assets/voice_pack_af_heart.bin");
        if (vp.size() != 510u * 256u) {
            std::cout << "ERROR voice_pack size " << vp.size() << " != 510*256" << std::endl;
            return 5;
        }
        std::cout << "SERVER_READY" << std::endl;
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == "QUIT" || line == "quit") break;
            if (line.rfind("SAY ", 0) != 0) {
                if (!line.empty()) std::cout << "ERROR bad command: " << line << std::endl;
                continue;
            }
            const size_t sp = line.find(' ', 4);
            if (sp == std::string::npos) { std::cout << "ERROR SAY needs <ids> <out>" << std::endl; continue; }
            const std::string ids_path = line.substr(4, sp - 4);
            const std::string out_path = line.substr(sp + 1);
            std::vector<int32_t> ids;
            if (!read_input_ids_bin(ids_path, ids) || ids.empty()) {
                std::cout << "ERROR cannot read ids: " << ids_path << std::endl; continue;
            }
            int vidx = (int)ids.size() - 1;
            if (vidx < 0) vidx = 0;
            if (vidx > 509) vidx = 509;
            std::vector<float> style(vp.begin() + (size_t)vidx * 256,
                                     vp.begin() + (size_t)(vidx + 1) * 256);
            std::vector<int16_t> chunk_pcm;
            const auto t0 = std::chrono::steady_clock::now();
            const int rc2 = model.forward_graph(ids, style, chunk_pcm);
            const double synth_s = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - t0).count();
            if (rc2 != 0) { std::cout << "ERROR forward_graph rc=" << rc2 << std::endl; continue; }
            if (!write_wav(out_path, chunk_pcm.data(), (int)chunk_pcm.size(), 24000)) {
                std::cout << "ERROR write_wav " << out_path << std::endl; continue;
            }
            std::cout << "AUDIO_READY " << out_path << " " << chunk_pcm.size()
                      << " " << synth_s << std::endl;
        }
        return 0;
    }

    // ── On-device streaming (--stream) ───────────────────────────────────
    // End-to-end in C++, no Python: prompt -> espeak-ng G2P -> chunk -> per-chunk
    // forward_graph synth -> raw int16 LE PCM flushed to STDOUT (so a host
    // consumer plays chunk N while chunk N+1 synthesizes on the GPU). Same
    // voice-pack + style-by-length selection + forward_graph as --serve above;
    // only the sink differs (stdout PCM instead of WAV files + line protocol).
    //
    // stdout carries ONLY raw int16 LE PCM @ 24kHz mono. Diagnostics go to
    // stderr. Binary stdout is mangled by `adb shell` (LF->CRLF) — the consumer
    // MUST use `adb exec-out`:
    //   adb exec-out 'cd <dir> && ./<bin> --stream "text"' | ffplay -f s16le -ar 24000 -ac 1 -nodisp -
    if (stream_mode) {
#ifdef NNOPT_TTS_STREAMING
        auto vp = load_float_bin(voicepack.c_str());
        if (vp.size() != 510u * 256u) {
            NNOPT_ERROR_FMT("voice_pack %s size %zu != 510*256", voicepack.c_str(), vp.size());
            return 5;
        }
        nnopt_tts::Phonemizer ph;
        // espeak wants the PARENT of espeak-ng-data/. Deploy pushes assets/* to
        // the run dir, so espeak-ng-data/ lives at assets/espeak-ng-data/.
        if (!ph.init(/*espeak_data_parent=*/"assets", "assets/phoneme_vocab.tsv", voice))
            return 5;

        const auto chunks = nnopt_tts::chunk_text(prompt);
        std::fprintf(stderr, "STREAM_CHUNKS %zu\n", chunks.size());
        std::fflush(stderr);

        for (size_t c = 0; c < chunks.size(); ++c) {
            std::vector<int32_t> ids = ph.phonemize(chunks[c]);
            if (ids.size() <= 2) {  // only pad/pad — nothing phonemized
                std::fprintf(stderr, "STREAM_SKIP empty chunk %zu: \"%s\"\n",
                             c, chunks[c].c_str());
                continue;
            }
            // Style-by-length, identical to the --serve handler.
            int vidx = (int)ids.size() - 1;
            if (vidx < 0) vidx = 0;
            if (vidx > 509) vidx = 509;
            std::vector<float> style(vp.begin() + (size_t)vidx * 256,
                                     vp.begin() + (size_t)(vidx + 1) * 256);

            std::vector<int16_t> chunk_pcm;
            const auto t0 = std::chrono::steady_clock::now();
            const int rc2 = model.forward_graph(ids, style, chunk_pcm);
            const double synth_s = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - t0).count();
            if (rc2 != 0) {
                NNOPT_ERROR_FMT("forward_graph chunk %zu rc=%d", c, rc2);
                return 7;
            }

            // Raw int16 LE PCM to stdout, flushed per chunk so playback is gapless.
            std::fwrite(chunk_pcm.data(), sizeof(int16_t), chunk_pcm.size(), stdout);
            std::fflush(stdout);

            const double audio_s = (double)chunk_pcm.size() / 24000.0;
            std::fprintf(stderr,
                         "STREAM_CHUNK %zu synth_s=%.3f samples=%zu audio_s=%.3f rtf=%.3f \"%s\"\n",
                         c, synth_s, chunk_pcm.size(), audio_s,
                         audio_s > 0 ? synth_s / audio_s : 0.0, chunks[c].c_str());
            std::fflush(stderr);
        }
        std::fprintf(stderr, "NNOPT_EXIT_CLEAN exit_code=0\n");
        std::fflush(stderr);
        return 0;
#else
        NNOPT_ERROR("--stream needs espeak-ng vendored at src/third_party/espeak-ng/ "
                    "and a rebuild (CMake auto-enables NNOPT_TTS_STREAMING when present)");
        return 2;
#endif
    }

    // ── Persistent on-device streaming REPL (--serve-stream) ─────────────
    // Same pipeline as --stream (on-device G2P -> chunked synth -> raw int16 LE
    // PCM on stdout), but the process STAYS RESIDENT: weights, CL context, and
    // the phonemizer load ONCE, then we loop reading one utterance per stdin
    // line. This is what the See & Say app drives — per-utterance cost is pure
    // synthesis (no ~0.7s reload tax), so back-to-back "Speak" presses are
    // gapless and real-time.
    //
    // Per-chunk framing (mirrors mms-tts's TTS_PCM_BEGIN/END so the Kotlin
    // reader knows exactly how many bytes to slurp before the next stderr line):
    //   stderr: KOKORO_PCM_BEGIN <id> <n_samples> 24000 (flushed BEFORE PCM)
    //   stdout: <n_samples> int16 LE samples            (flushed)
    //   ...one BEGIN+PCM pair per chunk...
    //   stderr: KOKORO_UTT_END <id> OK|CANCELLED|ERROR
    // Readiness: prints `ready.` on stderr once, after model + phonemizer init.
    // A blank line or EOF exits the REPL cleanly.
    if (serve_stream_mode) {
#ifdef NNOPT_TTS_STREAMING
        auto vp = load_float_bin(voicepack.c_str());
        if (vp.size() != 510u * 256u) {
            NNOPT_ERROR_FMT("voice_pack %s size %zu != 510*256", voicepack.c_str(), vp.size());
            return 5;
        }
        nnopt_tts::Phonemizer ph;
        if (!ph.init(/*espeak_data_parent=*/"assets", "assets/phoneme_vocab.tsv", voice))
            return 5;

        // Protocol v2 pairs every audio/terminal frame with a command ID.
        std::fprintf(stderr, "ready. protocol=2 build=0.6.10\n");
        std::fflush(stderr);
        TtsCommandReader reader(STDIN_FILENO);
        TtsCommand command;
        while (reader.next(command)) {
            nnopt_tts_begin(command.id);
            auto finish = [&](const char* status) {
                std::fprintf(stderr, "KOKORO_UTT_END %llu %s\n",
                             (unsigned long long)command.id, status);
                std::fflush(stderr);
            };
            std::deque<std::string> chunks;
            if (command.text.size() <= 400) chunks.push_back(command.text);
            else {
                auto split = nnopt_tts::chunk_text(command.text, 24, 40);
                chunks.assign(split.begin(), split.end());
            }
            bool cancelled = false;
            bool produced = false;
            int scratch_retries = 0;
            std::chrono::steady_clock::time_point first_scratch_failure;
            auto split_for_retry = [&](const std::string& text) {
                size_t cut = text.rfind(' ', text.size() / 2);
                if (cut == std::string::npos || cut == 0) cut = text.find(' ', text.size() / 2);
                if (cut == std::string::npos || cut == 0 || cut + 1 >= text.size()) return false;
                chunks.push_front(text.substr(cut + 1));
                chunks.push_front(text.substr(0, cut));
                return true;
            };
            while (!chunks.empty()) {
                if (nnopt_tts_cancelled()) { cancelled = true; break; }
                std::string text = std::move(chunks.front());
                chunks.pop_front();
                auto ids = ph.phonemize(text);
                if (ids.size() <= 2) continue;
                // Character count is not a model-token bound (numbers expand).
                // Split only pathological phonemized input, retaining normal
                // sentence prosody. Never silently truncate tokens or style.
                if (ids.size() > 512) {
                    if (!split_for_retry(text)) {
                        NNOPT_ERROR("Phonemized input exceeds 512 tokens with no safe split");
                        finish("ERROR");
                        return 7;
                    }
                    continue;
                }
                int vidx = std::min(509, (int)ids.size() - 1);
                std::vector<float> style(vp.begin() + (size_t)vidx * 256,
                                         vp.begin() + (size_t)(vidx + 1) * 256);
                std::vector<int16_t> pcm;
                int rc = -1;
                try { rc = model.forward_graph(ids, style, pcm, command.rate); }
                catch (const std::exception& error) {
                    if (std::string(error.what()).find("Kokoro scratch-memory safety budget exceeded") != std::string::npos) {
                        // An aborted graph may still have GPU work queued.
                        // Drain before releasing all retained arena slots;
                        // otherwise every smaller retry inherits the same
                        // nearly-full memory budget.
                        if (clFinish(cl_ctx.queue()) != CL_SUCCESS) {
                            NNOPT_ERROR("Scratch recovery could not drain the GPU queue");
                            finish("ERROR"); return 7;
                        }
                        nnopt_gen_reclaim_scratch_after_drain();
                        const auto now = std::chrono::steady_clock::now();
                        if (scratch_retries == 0) first_scratch_failure = now;
                        const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                            now - first_scratch_failure).count();
                        if (scratch_retries >= 3 || elapsed >= 20 || !split_for_retry(text)) {
                            NNOPT_ERROR_FMT("Scratch recovery stopped after %d splits (%lld s)",
                                            scratch_retries, (long long)elapsed);
                            finish("ERROR"); return 7;
                        }
                        ++scratch_retries;
                        std::fprintf(stderr, "KOKORO_MEMORY split_retry chars=%zu attempt=%d\n",
                                     text.size(), scratch_retries);
                        std::fflush(stderr);
                        continue;
                    }
                    NNOPT_ERROR_FMT("Inference exception: %s", error.what());
                }
                if (rc == -2 || nnopt_tts_cancelled()) { cancelled = true; break; }
                if (rc == -3) {
                    if (!split_for_retry(text)) {
                        NNOPT_ERROR("Predicted speech exceeds safe memory capacity with no word boundary");
                        finish("ERROR"); return 7;
                    }
                    continue;
                }
                if (rc != 0 || pcm.empty() || pcm.size() > 1440000) {
                    NNOPT_ERROR_FMT("Synthesis failed rc=%d samples=%zu", rc, pcm.size());
                    finish("ERROR");
                    return 7;
                }
                std::fprintf(stderr, "KOKORO_PCM_BEGIN %llu %zu 24000\n",
                             (unsigned long long)command.id, pcm.size());
                std::fflush(stderr);
                if (std::fwrite(pcm.data(), sizeof(int16_t), pcm.size(), stdout) != pcm.size() ||
                    std::fflush(stdout) != 0) return 7;
                produced = true;
            }
            if (cancelled) {
                // Acknowledgement means the GPU has actually drained. If this
                // hangs, Android's request-scoped watchdog retires this worker.
                if (clFinish(cl_ctx.queue()) != CL_SUCCESS) { finish("ERROR"); return 7; }
                finish("CANCELLED");
            } else {
                finish(produced ? "OK" : "ERROR");
            }
        }
        return 0;
#else
        NNOPT_ERROR("--serve-stream requires the eSpeak NG-enabled application build");
        return 2;
#endif
    }

    // ── TTS single-shot pipeline (VITS family) ──────────────────────────
    // No autoregressive decode loop. We load three fixture buffers and
    // call Model::forward_graph(...) ONCE, then write a WAV.
    //
    // The RNG fixtures (duration_noise, prior_noise) are captured by the
    // reference Python and SHIPPED inside assets/ on the device. Loading
    // them — never sampling RNG in C++ — is what makes per-layer cosine
    // compare against the reference meaningful. See tts.md axis D.
    //
    // Build-time gates (buildTs.ts) refuse if any of:
    //   - load_int32_bin("assets/test_input_ids.bin")
    //   - load_float_bin("assets/duration_noise.bin")
    //   - load_float_bin("assets/prior_noise.bin")
    //   - write_wav(...)
    //   - the seven model_forward_graph op_* calls
    // is missing. Do not delete them.
    auto input_ids = load_int32_bin("assets/test_input_ids.bin");
    // --token-ids overrides the fixture (run_android.sh passes it for both
    // Evaluate and ad-hoc prompts). Previously parsed but never consumed in
    // the TTS path — every run synthesized the stale assets fixture.
    if (!token_ids_path.empty()) {
        std::vector<int32_t> cli_ids;
        if (read_input_ids_bin(token_ids_path, cli_ids) && !cli_ids.empty()) {
            input_ids = cli_ids;
        } else {
            NNOPT_ERROR_FMT("--token-ids %s unreadable, using assets fixture", token_ids_path.c_str());
        }
    }
    if (input_ids.empty()) {
        NNOPT_ERROR("required fixture missing: assets/test_input_ids.bin");
        return 5;
    }
    // Load Kokoro voice pack: 510 * 256 fp32 = 522240 bytes.
    // Index by len(input_ids) - 1 to get the per-length style vector (256 floats).
    auto voice_pack = load_float_bin("assets/voice_pack_af_heart.bin");
    if (voice_pack.size() != 510u * 256u) {
        NNOPT_ERROR_FMT("voice_pack size %zu != 510*256", voice_pack.size());
        return 5;
    }
    int ref_idx = (int)input_ids.size() - 1;
    if (ref_idx < 0) ref_idx = 0;
    if (ref_idx > 509) ref_idx = 509;
    std::vector<float> ref_s(voice_pack.begin() + (size_t)ref_idx * 256,
                             voice_pack.begin() + (size_t)(ref_idx + 1) * 256);

    std::vector<int16_t> pcm;
    bench.mark_prefill_start();
    NNOPT_BENCH_FIRST_TOKEN();   // repurposed: marks first sample produced
    const int rc = model.forward_graph(input_ids, ref_s, pcm);
    bench.mark_end();
    if (rc != 0) {
        NNOPT_ERROR_FMT("forward_graph returned %d", rc);
        return 7;
    }

    // WAV output — 16-bit signed mono at Kokoro's native 24kHz.
    // (Scaffolder previously hardcoded 16000 from a VITS template; Kokoro's
    // KPipeline ships 24kHz, see kokoro/pipeline.py:296. Fix tracked in
    // ~/.claude/plans/kokoro-port-journal.md as scaffolder bug #1.)
    const std::string out_wav = "output.wav";
    if (!write_wav(out_wav, pcm.data(), (int)pcm.size(), 24000)) {
        NNOPT_ERROR_FMT("write_wav failed: %s", out_wav.c_str());
        return 8;
    }

    // Evaluate-parsed stderr contract.
    std::cerr << "TTS_OUTPUT_PCM_SAMPLES " << pcm.size() << std::endl;
    std::cerr << "TTS_OUTPUT_SAMPLE_RATE 24000" << std::endl;

    KernelProfiler::dump_summary();
    bench.print_summary((int)input_ids.size(), (int)pcm.size());
    return 0;


}
