package com.labsii.voices.runner

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.util.concurrent.TimeUnit
import android.util.Log
import kotlin.concurrent.thread

/** Reads the renderer from the same OpenCL runtime used for synthesis. */
object KokoroDeviceProbe {
    data class Result(
        val compatible: Boolean,
        val renderer: String? = null,
        val reason: String? = null,
        val diagnostics: String = "",
    )

    suspend fun probe(context: Context): Result = withContext(Dispatchers.IO) {
        val binary = File(context.applicationInfo.nativeLibraryDir, "libkokoro.so")
        if (!binary.isFile) return@withContext Result(false, reason = "Kokoro runtime is not packaged")
        val captured = KokoroProbeOutput()
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
            val reader = thread(name = "kokoro-device-probe", isDaemon = true) {
                runCatching {
                    process.inputStream.bufferedReader().useLines { lines ->
                        lines.forEach { line ->
                            captured.accept(line)
                        }
                    }
                }
            }
            var timedOut = false
            try {
                timedOut = !process.waitFor(20, TimeUnit.SECONDS)
            } finally {
                if (process.isAlive) { process.destroyForcibly(); process.waitFor(2, TimeUnit.SECONDS) }
                reader.join(1000)
            }
            if (process.isAlive) throw java.io.IOException("OpenCL probe did not stop after its deadline")
            captured.result(process.exitValue(), if (timedOut) "OpenCL check timed out; real synthesis will still be tried." else null)

        }.getOrElse {
            captured.result(null, "OpenCL compatibility check could not complete: ${it.message ?: it.javaClass.simpleName}")
        }.also { result ->
            if (!result.compatible) Log.w("KokoroDeviceProbe", "${result.reason}\n${result.diagnostics}")
        }
    }
}
