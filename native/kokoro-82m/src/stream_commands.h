// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#pragma once
#include "cancellation.h"
#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cmath>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <poll.h>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

struct TtsCommand { uint64_t id = 0; float rate = 1; std::string text; };

inline bool tts_parse_id(const std::string& text, uint64_t& id) {
    const auto r = std::from_chars(text.data(), text.data() + text.size(), id);
    return r.ec == std::errc() && r.ptr == text.data() + text.size() && id > 0;
}

inline bool tts_parse_say(const std::string& line, TtsCommand& out) {
    out = TtsCommand{};
    std::istringstream input(line);
    std::string verb, id, rate;
    if (!(input >> verb >> id >> rate) || verb != "SAY" || !tts_parse_id(id, out.id)) return false;
    char* end = nullptr;
    const float speed = std::strtof(rate.c_str(), &end);
    if (!end || end == rate.c_str() || *end || !std::isfinite(speed)) return false;
    std::getline(input >> std::ws, out.text);
    if (out.text.empty() || out.text.size() > 12000 || out.text.find('\0') != std::string::npos) return false;
    out.rate = std::max(0.5f, std::min(2.0f, speed));
    return true;
}

// Joinable input reader: never captures model/phonemizer stack references and
// never waits indefinitely in getline() during inference-error shutdown.
class TtsCommandReader {
    int fd_;
    std::mutex mutex_;
    std::condition_variable changed_;
    std::deque<TtsCommand> pending_;
    std::atomic_bool stopping_{false};
    bool closed_ = false;
    std::thread reader_;
    void read_loop() {
        std::string line;
        uint64_t latest = 0;
        bool failed = false;
        while (!stopping_.load() && !failed) {
            pollfd descriptor{fd_, POLLIN, 0};
            const int rc = poll(&descriptor, 1, 100);
            if (rc < 0) { if (errno == EINTR) continue; break; }
            if (rc == 0) continue;
            char bytes[1024];
            const auto count = read(fd_, bytes, sizeof(bytes));
            if (count <= 0) { if (count < 0 && errno == EINTR) continue; break; }
            for (ssize_t i = 0; i < count; ++i) {
                if (bytes[i] != '\n') {
                    line += bytes[i];
                    if (line.size() > 16384) { failed = true; break; }
                    continue;
                }
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (line.empty() || line == "QUIT") { failed = true; break; }
                if (line.rfind("CANCEL ", 0) == 0) {
                    uint64_t id = 0;
                    if (!tts_parse_id(line.substr(7), id) || id > latest) { failed = true; break; }
                    nnopt_tts_cancel(id);
                } else {
                    TtsCommand command;
                    if (!tts_parse_say(line, command) || command.id <= latest) { failed = true; break; }
                    latest = command.id;
                    std::lock_guard<std::mutex> guard(mutex_);
                    if (pending_.size() >= 4) { failed = true; break; }
                    pending_.push_back(std::move(command));
                    changed_.notify_one();
                }
                line.clear();
            }
        }
        nnopt_tts_input_closed.store(true, std::memory_order_release);
        { std::lock_guard<std::mutex> guard(mutex_); closed_ = true; }
        changed_.notify_one();
    }
public:
    explicit TtsCommandReader(int fd) : fd_(fd), reader_([this] { read_loop(); }) {}
    ~TtsCommandReader() { stopping_.store(true); reader_.join(); }
    bool next(TtsCommand& command) {
        std::unique_lock<std::mutex> guard(mutex_);
        changed_.wait(guard, [&] { return closed_ || !pending_.empty(); });
        if (pending_.empty()) return false;
        command = std::move(pending_.front()); pending_.pop_front();
        return true;
    }
};
