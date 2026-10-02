// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#include "stream_commands.h"
#include "cl_owned.h"
#include <cassert>
#include <chrono>
#include <iostream>

struct _cl_mem { int value; };
int releases = 0;
extern "C" cl_int CL_API_CALL clReleaseMemObject(cl_mem m) { ++releases; delete m; return CL_SUCCESS; }

int main() {
    TtsCommand command;
    assert(tts_parse_say("SAY 1 1.25 A regular sentence.", command));
    assert(command.id == 1 && command.rate == 1.25f && command.text == "A regular sentence.");
    assert(!tts_parse_say("SAY -1 1 Bad.", command));
    assert(!tts_parse_say("SAY 1 nan Bad.", command));
    assert(!tts_parse_say("SAY 0 1 Bad.", command));
    assert(!tts_parse_say("SAY 1 1", command));
    assert(!tts_parse_say("SAY 1 1 " + std::string(12001, 'a'), command));
    nnopt_tts_cancel(10); // Cancel before dequeue must not be erased by begin.
    nnopt_tts_begin(10);
    assert(nnopt_tts_cancelled());
    nnopt_tts_begin(11);
    nnopt_tts_cancel(10); // Late old cancellation must not stop replacement.
    assert(!nnopt_tts_cancelled());
    nnopt_tts_cancel(11);
    assert(nnopt_tts_cancelled());
    {
        ClOwnedBuffers owned;
        auto first = owned.own(new _cl_mem{1});
        owned.own(new _cl_mem{2});
        owned.release(first);
        owned.release(first); // no double release
    }
    assert(releases == 2);
    try {
        ClOwnedBuffers owned;
        owned.own(new _cl_mem{3});
        owned.own(nullptr);
        assert(false);
    } catch (const std::runtime_error&) {}
    assert(releases == 3);
    int fds[2]; assert(pipe(fds) == 0);
    const auto start = std::chrono::steady_clock::now();
    {
        TtsCommandReader reader(fds[0]);
        const std::string input = "SAY 20 1 Hello.\nCANCEL 20\n";
        assert(write(fds[1], input.data(), input.size()) == (ssize_t)input.size());
        assert(reader.next(command) && command.id == 20);
        // Destructor must join even though stdin is still open and idle.
    }
    assert(std::chrono::steady_clock::now() - start < std::chrono::seconds(1));
    close(fds[0]); close(fds[1]);
    std::cout << "Native protocol, cancellation ownership, cleanup and reader-shutdown checks passed\n";
}
