// SPDX-License-Identifier: GPL-3.0-or-later
// Integration test: real eSpeak NG 1.52 + the shipped English data and vocabulary.
#include "phonemizer.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
using Ids = std::vector<int32_t>;
Ids marks(const Ids& ids) {
    Ids out;
    for (int32_t id : ids) if (id >= 1 && id <= 15) out.push_back(id);
    return out;
}
Ids words(Ids ids) {
    ids.erase(std::remove_if(ids.begin(), ids.end(), [](int32_t id) {
        return id >= 1 && id <= 15;
    }), ids.end());
    return ids;
}
void require(bool condition, const std::string& message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}
int main(int argc, char** argv) {
    if (argc != 4) { std::cerr << "Usage: test DATA_PARENT VOCAB_TSV VOICE\n"; return 2; }
    {
        const char* voice = argv[3];
        nnopt_tts::Phonemizer ph;
        require(ph.init(argv[1], argv[2], voice), "initialize phonemizer");
        const auto expect = [&](const std::string& text, const Ids& expected) {
            const auto ids = ph.phonemize(text);
            require(!ids.empty() && ids.front() == 0 && ids.back() == 0, "BOS/EOS: " + text);
            require(marks(ids) == expected, "punctuation: " + text);
            return ids;
        };
        const auto statement = expect("You are coming.", {4});
        const auto question = expect("You are coming?", {6});
        const auto exclamation = expect("You are coming!", {5});
        require(words(statement) == words(question) && words(question) == words(exclamation),
                "punctuation must not replace or duplicate words");
        expect("Hello, Alice; wait: really? Yes!", {3, 1, 2, 6, 5});
        expect("She said, \"Are you sure?!\" Then left.", {3, 6, 5, 4});
        expect("She said, “Are you sure?!” Then left.", {3, 6, 5, 4});
        expect("Hello, \"Alice!\"", {3, 5});
        expect("Hello, élan.", {3, 4});
        expect("Hello, 🐇 Alice!", {3, 5});
        expect("Hello, a", {3});
        expect("Hello, a!", {3, 5});
        expect("Hello, I.", {3, 4});
        expect("Wait... Go!", {10, 5});
        expect("Wait… Go!", {10, 5});
        expect("Wait — really?", {9, 6});
        expect("Wait – really?", {9, 6});
        expect("Dr. Smith paid $3.14 at 10:30 a.m. today.", {4});
        expect("Prof. Smith met Mr. Jones?", {6});
        expect("A. Smith works in the U.S. office.", {4});
        expect("This is e.g. a test.", {4});
        expect("1,000 people paid 1.50 each.", {4});
        expect("Don't stop. Well-known words stay intact.", {4, 4});
        expect("This is a test (really).", {4});
        expect("No punctuation", {});
        expect("No punctuation\n", {});
        expect("Hi! Next? End.", {5, 6, 4});
        expect("Will you come?\nYes!", {6, 5});
        expect("Well!   ", {5});
        expect("Well!\xc2\xa0", {5});
        require(ph.phonemize("").empty(), "empty text");
        const auto wait = expect("Wait.", {4});
        expect("Wait?", {6});
        require(ph.phonemize("Wait.") == wait, "no punctuation leaks across requests");
        std::string long_text;
        for (int i = 0; i < 200; ++i) long_text += "ordinary words ";
        expect(long_text + "finished?", {6});
        std::cout << voice << ": punctuation integration checks passed\n";
    }
}
