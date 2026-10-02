package com.labsii.voices.runner

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.util.concurrent.TimeUnit
import java.util.Collections
import kotlin.concurrent.thread

/** Reads the renderer from the same OpenCL runtime used for synthesis. */
object KokoroDeviceProbe {
    data class Result(
        val compatible: Boolean,
        val renderer: String? = null,
        val reason: String? = null,
        val conclusive: Boolean = true,
    )

    suspend fun probe(context: Context): Result = withContext(Dispatchers.IO) {
        val binary = File(context.applicationInfo.nativeLibraryDir, "libkokoro.so")
        if (!binary.isFile) return@withContext Result(false, reason = "Kokoro runtime is not packaged")
        runCatching {
            val runtimeDir = KokoroModelManager.runtimeDir(context).apply { mkdirs() }
            val process = ProcessBuilder(binary.absolutePath, "--device-info")
                .directory(runtimeDir)
                .redirectErrorStream(true)
                .apply {
                    environment()["LD_LIBRARY_PATH"] =
                        "${context.applicationInfo.nativeLibraryDir}:/vendor/lib64:/system/lib64"
                }
                .start()
            // Drain while the probe runs, otherwise a verbose driver can fill
            // its pipe and turn a successful probe into an artificial timeout.
            val captured = Collections.synchronizedList(mutableListOf<String>())
            val reader = thread(name = "kokoro-device-probe", isDaemon = true) {
                runCatching {
                    process.inputStream.bufferedReader().useLines { lines ->
                        lines.forEach { line ->
                            if (line.startsWith("KOKORO_") && captured.size < 16) captured.add(line.take(4096))
                        }
                    }
                }
            }
            try {
                if (!process.waitFor(20, TimeUnit.SECONDS)) {
                    return@runCatching Result(false, reason = "OpenCL check timed out; real synthesis can still be tried", conclusive = false)
                }
                reader.join(1000)
            } finally {
                if (process.isAlive) { process.destroyForcibly(); process.waitFor(2, TimeUnit.SECONDS) }
            }
            val lines = synchronized(captured) { captured.toList() }
            val renderer = lines
                .firstOrNull { it.startsWith("KOKORO_DEVICE ") }
                ?.removePrefix("KOKORO_DEVICE ")
                ?.trim()
                ?.takeIf { it.isNotEmpty() }
            val compatibility = lines
                .firstOrNull { it.startsWith("KOKORO_COMPATIBLE ") }
                ?.removePrefix("KOKORO_COMPATIBLE ")
            Result(
                compatible = compatibility?.startsWith("1") == true,
                conclusive = compatibility != null,
                renderer = renderer,
                reason = compatibility
                    ?.takeIf { it.startsWith("0") }
                    ?.removePrefix("0")
                    ?.trim()
                    ?.takeIf { it.isNotEmpty() },
            )
        }.getOrElse {
            Result(false, reason = "OpenCL compatibility check could not complete", conclusive = false)
        }
    }
}
