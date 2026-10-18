# Vendored pitch dependency

Rubber Band Library 4.0.0, unmodified upstream source distribution.
Source: https://github.com/breakfastquay/rubberband/tree/v4.0.0
Archive: https://codeload.github.com/breakfastquay/rubberband/tar.gz/refs/tags/v4.0.0
Archive SHA-256: `24300f48a8014b7c863b573a9647e61b1b19b37875e2cdd92005e64c6424d266`

Copyright 2007–2024 Particular Programs Ltd. GPL-2.0-or-later;
see rubberband/COPYING and the source file headers. This application uses
it under GPLv3. No additional build-time download is needed.

CMake compiles `single/RubberBandSingle.cpp` into a static target. This
uses the built-in FFT/resampler on Android and disables helper threads.
The adapter is `kokoro-82m/src/pitch_shift.cpp`, outside the upstream tree.
