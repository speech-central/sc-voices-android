/*
 * Derived from a8nova/adreno-llms, copyright 2026 a8nova.
 * Modified by Labsii Ltd. in 2026: request-scoped worker ownership, protocol v2,
 * asynchronous cancellation and bounded retirement. SPDX-License-Identifier: Apache-2.0
 */
package com.labsii.voices.runner

import android.content.Context
import android.util.Log
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.withContext
import kotlinx.coroutines.withTimeout
import java.io.DataInputStream
import java.io.File
import java.io.IOException
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.concurrent.Executors
import java.util.concurrent.ScheduledFuture
import java.util.concurrent.ScheduledExecutorService
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicLong
import kotlin.concurrent.thread
import kotlin.time.Duration.Companion.milliseconds

/** One warm native worker. No process I/O or wait holds [lock]. */
class KokoroSession internal constructor(
    private val nativeLibDir: String,
    private val cwd: File,
    private val processFactory: (ProcessBuilder) -> Process = { it.start() },
    private val synthesisTimeoutMs: Long = SYNTHESIS_TIMEOUT_MS,
    private val cancelTimeoutMs: Long = CANCEL_FALLBACK_MS,
    private val deadlines: ScheduledExecutorService = Executors.newSingleThreadScheduledExecutor { r ->
        Thread(r, "kokoro-deadlines").apply { isDaemon = true }
    },
) {
    constructor(context: Context) : this(
        context.applicationInfo.nativeLibraryDir, KokoroModelManager.runtimeDir(context),
    )
    private val lock = Any()
    private var worker: Worker? = null
    private var active: Request? = null
    private var bound: Worker? = null
    private var closed = false
    private val ids = AtomicLong()

    class Request internal constructor(val id: Long) {
        val cancelled = AtomicBoolean()
        internal var fallback: ScheduledFuture<*>? = null
    }
    data class PcmAudio(val samples: ShortArray, val sampleRate: Int) {
        override fun equals(other: Any?): Boolean {
            if (this === other) return true
            if (javaClass != other?.javaClass) return false

            other as PcmAudio

            if (sampleRate != other.sampleRate) return false
            if (!samples.contentEquals(other.samples)) return false

            return true
        }

        override fun hashCode(): Int {
            var result = sampleRate
            result = 31 * result + samples.contentHashCode()
            return result
        }
    }

    private class Worker(val process: Process, val voice: String) {
        val ready = CompletableDeferred<Unit>()
        // Only protocol events enter here. Bounded trySend lost terminal markers.
        val events = Channel<KokoroProtocol.Event>(Channel.UNLIMITED)
        val commands = Executors.newSingleThreadExecutor { r ->
            Thread(r, "kokoro-control").apply { isDaemon = true }
        }
        val writer = process.outputStream.bufferedWriter(Charsets.UTF_8)
        val pcm = DataInputStream(process.inputStream)
        val retiring = AtomicBoolean()
        val retired = CompletableDeferred<Unit>()
        var commandId: Long? = null // protected by the session lock
    }

    fun beginRequest(): Request = synchronized(lock) {
        check(!closed) { "Kokoro session is closed" }
        check(active == null) { "Concurrent Kokoro requests are not supported" }
        Request(ids.incrementAndGet()).also { active = it; bound = null }
    }

    fun finishRequest(request: Request) = synchronized(lock) {
        if (active === request) {
            request.fallback?.cancel(false)
            active = null
            bound = null
        }
    }

    suspend fun start(request: Request, voicePackPath: String, phonemizerVoice: String) =
        withContext(Dispatchers.IO) {
            // A repaired file must not reuse a worker holding the old contents.
            val voiceKey = "$voicePackPath|$phonemizerVoice|" +
                "${File(cwd, voicePackPath).lastModified()}|" +
                "${File(cwd, "weights/model.fp16.bin").lastModified()}"
            val previous = synchronized(lock) {
                check(active === request && !closed)
                worker
            }
            if (request.cancelled.get()) return@withContext
            if (previous != null && previous.voice == voiceKey &&
                previous.process.isAlive && !previous.retiring.get()) {
                synchronized(lock) {
                    if (active === request && !closed && !request.cancelled.get()) bound = previous
                }
                return@withContext
            }
            // Failed retirement prevents spawning overlapping heavy workers.
            if (previous != null) retire(previous, "voice change or dead worker").await()
            if (request.cancelled.get()) return@withContext
            require(cwd.isDirectory) { "Kokoro runtime is not installed: $cwd" }
            val binary = File(nativeLibDir, "libkokoro.so")
            require(binary.isFile) { "Kokoro native runtime is not packaged: $binary" }
            val builder = ProcessBuilder(
                binary.absolutePath, "--serve-stream", "--voice", phonemizerVoice,
                "--voicepack", voicePackPath,
            ).directory(cwd)
            builder.environment().apply {
                put("LD_LIBRARY_PATH", "$nativeLibDir:/vendor/lib64:/system/lib64")
                put("NNOPT_GPU_FP32_GENERATOR", "1")
                put("NNOPT_PRE_ALLOC_WEIGHTS", "1")
                put("NNOPT_VERIFY_WEIGHTS", "0")
                put("NNOPT_QCOM_PRIORITY", "low")
                // Whole-graph replay cannot observe cancellation between blocks.
                put("NNOPT_RECORD", "0")
                put("NNOPT_COOPERATIVE", "1")
            }
            val started = Worker(processFactory(builder), voiceKey)
            val accepted = synchronized(lock) {
                if (active !== request || closed || request.cancelled.get()) false
                else { worker = started; bound = started; true }
            }
            if (!accepted) {
                retire(started, "cancelled during startup").await()
                return@withContext
            }
            startReader(started)
            try {
                withTimeout(START_TIMEOUT_MS.milliseconds) { started.ready.await() }
            } catch (t: Throwable) {
                retire(started, "startup failed")
                throw t
            }
            Log.i(TAG, "Kokoro ready: protocol=2 build=0.6.6")
        }

    /** Pipe reads live on the reader thread, so timeout also covers partial PCM. */
    suspend fun speak(request: Request, text: String, speechRate: Float, onAudio: (PcmAudio) -> Unit) {
        if (request.cancelled.get()) return
        val commandId = ids.incrementAndGet()
        val target = synchronized(lock) {
            check(active === request && !closed)
            if (request.cancelled.get()) return
            val current = bound ?: throw IOException("Kokoro worker is unavailable")
            if (current.retiring.get()) throw IOException("Kokoro worker is retiring")
            current.commandId = commandId
            // Same lock + FIFO writer: CANCEL cannot overtake its SAY.
            send(current, KokoroProtocol.say(commandId, speechRate, text))
            current
        }
        var totalSamples = 0
        try {
            withTimeout(synthesisTimeoutMs.milliseconds) {
                while (true) {
                    val event = target.events.receive()
                    if (event.id != commandId) throw IOException("Out-of-order Kokoro response ${event.id}, expected $commandId")
                    when (event) {
                        is KokoroProtocol.Audio -> {
                            totalSamples += event.samples.size
                            if (!request.cancelled.get()) onAudio(PcmAudio(event.samples, event.sampleRate))
                        }
                        is KokoroProtocol.End -> {
                            when (event.status) {
                                "OK" -> if (totalSamples == 0) throw IOException("Kokoro produced no audio")
                                "CANCELLED" -> if (!request.cancelled.get()) throw IOException("Unexpected native cancellation")
                                else -> throw IOException("Kokoro inference failed")
                            }
                            break
                        }
                    }
                }
            }
        } catch (t: Throwable) {
            retire(target, "synthesis failed or timed out")
            throw t
        } finally {
            synchronized(lock) { if (target.commandId == commandId) target.commandId = null }
        }
    }

    /** Never writes pipes or waits for inference/exit on Android's stop thread. */
    fun requestCancel(request: Request) = synchronized(lock) {
        request.cancelled.set(true)
        if (active !== request || closed) return@synchronized
        val target = bound ?: return@synchronized
        if (request.fallback == null) {
            request.fallback = deadlines.schedule({
                synchronized(lock) {
                    // Ownership check + retirement claim are atomic.
                    if (active === request && bound === target && worker === target) {
                        retire(target, "cooperative cancellation deadline")
                    }
                }
            }, cancelTimeoutMs, TimeUnit.MILLISECONDS)
        }
        target.commandId?.let { send(target, "CANCEL $it") }
        if (!target.ready.isCompleted) retire(target, "cancelled during startup")
        Unit
    }

    fun failRequest(request: Request) = synchronized(lock) {
        if (active === request) bound?.let { retire(it, "request failed") }
        Unit
    }

    /** Teardown is non-blocking; retirement owns the captured process. */
    fun close() = synchronized(lock) {
        closed = true
        active?.cancelled?.set(true)
        active?.fallback?.cancel(false)
        worker?.let { retire(it, "service destroyed") }
        deadlines.shutdownNow()
        Unit
    }

    private fun send(target: Worker, command: String) {
        if (target.retiring.get()) return
        try {
            target.commands.execute {
                try {
                    target.writer.write(command)
                    target.writer.newLine()
                    target.writer.flush() // Unlike PrintWriter, reports broken pipes.
                } catch (_: Exception) {
                    retire(target, "native input closed")
                }
            }
        } catch (_: java.util.concurrent.RejectedExecutionException) {
            retire(target, "native command queue closed")
        }
    }

    private fun startReader(target: Worker) {
        thread(name = "kokoro-protocol", isDaemon = true) {
            var failure: Throwable = IOException("Kokoro process closed")
            try {
                target.process.errorStream.bufferedReader(Charsets.UTF_8).useLines { lines ->
                    for (line in lines) {
                        if (line.startsWith("ready.")) {
                            if (!line.contains("protocol=2")) throw IOException("Stale native runtime: protocol v2 required")
                            target.ready.complete(Unit)
                        } else if (line.startsWith("KOKORO_PCM_BEGIN ")) {
                            val header = KokoroProtocol.audioHeader(line)
                            val raw = ByteArray(header.samples * 2)
                            target.pcm.readFully(raw)
                            val samples = ShortArray(header.samples)
                            ByteBuffer.wrap(raw).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().get(samples)
                            if (target.events.trySend(KokoroProtocol.Audio(header.id, samples, header.rate)).isFailure) break
                        } else if (line.startsWith("KOKORO_UTT_END ")) {
                            if (target.events.trySend(KokoroProtocol.end(line)).isFailure) break
                        } else if (line.startsWith("ERROR:") || line.startsWith("FATAL")) {
                            Log.w(TAG, line)
                        }
                    }
                }
            } catch (t: Exception) {
                failure = t
            } finally {
                target.ready.completeExceptionally(failure)
                // Per-worker lifetime channel remembers exits before speak().
                target.events.close(failure)
            }
        }
    }

    private fun retire(target: Worker, reason: String): CompletableDeferred<Unit> {
        if (!target.retiring.compareAndSet(false, true)) return target.retired
        val error = IOException("Kokoro worker retired: $reason")
        target.ready.completeExceptionally(error)
        target.events.close(error)
        target.commands.shutdownNow()
        thread(name = "kokoro-retire", isDaemon = true) {
            try {
                // Kill before close: stream close may wait behind readFully/flush.
                target.process.destroy()
                if (!target.process.waitFor(1, TimeUnit.SECONDS)) {
                    target.process.destroyForcibly()
                    if (!target.process.waitFor(2, TimeUnit.SECONDS)) throw IOException("Native worker did not exit")
                }
                runCatching { target.writer.close() }
                runCatching { target.pcm.close() }
                runCatching { target.process.errorStream.close() }
                target.retired.complete(Unit)
            } catch (t: Exception) {
                target.retired.completeExceptionally(t)
                Log.e(TAG, "Worker retirement failed", t)
            }
        }
        return target.retired
    }

    companion object {
        private const val TAG = "KokoroSession"
        private const val START_TIMEOUT_MS = 120_000L
        private const val SYNTHESIS_TIMEOUT_MS = 120_000L
        private const val CANCEL_FALLBACK_MS = 2_500L
    }
}
