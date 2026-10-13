# 0.6.25 — Download retry feedback

- Every download attempt publishes an immediate preparing state with a spinner.
- Immediate failures retain visible attempt feedback for at least 700 ms before showing the result.
- Retry remains single-flight and is available again when the failure is displayed.
- Offline, unreachable-host and timeout failures receive actionable connection messages.
- Adds ACCESS_NETWORK_STATE (no runtime permission prompt) to distinguish no active network.
- Existing file validation, resumable downloads and synthesis behavior remain unchanged.

Validation: Kotlin compilation against Android API 36 and AndroidX/Material dependencies with resource-ID/BuildConfig stand-ins; XML parsing. Full Gradle/NDK APK build and on-device network testing were not available.
