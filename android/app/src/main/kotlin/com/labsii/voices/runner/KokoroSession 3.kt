/*
 * Derived from a8nova/adreno-llms, copyright 2026 a8nova.
 * Modified by Labsii Ltd. in 2026: request-scoped worker ownership, protocol v2,
 * asynchronous cancellation and bounded retirement. SPDX-License-Identifier: Apache-2.0
 */
package com.labsii.voices.runner

import android.content.Context
import android.util.Log
import android.os.SystemClock
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.withTimeout
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.TimeoutCancellationException
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
import java.util.concurrent.atomic.AtomicReference
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
    private val retirementTimeoutMs: Long = RETIREMENT_TIMEOUT_MS,
    private val idleTimeoutMs: Long = IDLE_TIMEOUT_MS,
    private val diagnosticsEnabled: Boolean = false,
    private val startupTimeoutMs: Long = START_TIMEOUT_MS,
) {
    enum class GpuPriority(val setting: String) {
        LOW("low"),
    }
    class StartupDeadlineException(message: String) : IOException(message)
    class InferenceDeadlineException(message: String) : IOException(message)

    constructor(context: Context, diagnosticsEnabled: Boolean = false) : this(
        context.applicationInfo.nativeLibraryDir, KokoroModelManager.runtimeDir(context),
        diagnosticsEnabled = diagnosticsEnabled,
    )
    private class Startup(val request: Request) {
        val result = CompletableDeferred<Unit>()
        val finished = AtomicBoolean()
        val stage = AtomicReference("queued")
    }
    private val startupExecutor = Executors.newSingleThreadExecutor { r ->
        Thread(r, "kokoro-startup").apply { isDaemon = true }
    }
    private var pendingStartup: Startup? = null // protected by lock
    private val lock = Any()
    private var worker: Worker? = null
    private var active: Request? = null
    private var bound: Worker? = null
    private var closed = false
    private var idleRetirement: ScheduledFuture<*>? = null
    private val ids = AtomicLong()

    class Request internal constructor(val id: Long) {
        val cancelled = AtomicBoolean()
        internal var fallback: ScheduledFuture<*>? = null
    }
    /** Debug-only counters; no native timing or per-buffer clocks in release. */
    class DebugTiming {
        val phonemizeNs = AtomicLong()
        val inferenceNs = AtomicLong()
        val nativeWriteNs = AtomicLong()
        val pcmReadNs = AtomicLong()
        val nativeSamples = AtomicLong()

        internal fun acceptNative(line: String, commandId: Long?) {
            val fields = line.split(' ')
            if (fields.size != 6 || fields[1].toLongOrNull() != commandId) return
            val values = fields.drop(2).map { it.toLongOrNull() ?: return }
            if (values.any { it < 0 }) return
            phonemizeNs.addAndGet(values[0])
            inferenceNs.addAndGet(values[1])
            nativeWriteNs.addAndGet(values[2])
            nativeSamples.addAndGet(values[3])
        }
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
        val lastNativeError = AtomicReference<String?>(null)
        val lastNativeStage = AtomicReference<String?>(null)
        val debugTiming = AtomicReference<DebugTiming?>(null)
        val debugCommandId = AtomicLong()
        var commandId: Long? = null // protected by the session lock
    }

    fun beginRequest(): Request = synchronized(lock) {
        check(!closed) { "Kokoro session is closed" }
        check(active == null) { "Concurrent Kokoro requests are not supported" }
        idleRetirement?.cancel(false)
        idleRetirement = null
        Request(ids.incrementAndGet()).also { active = it; bound = null }
    }

    fun finishRequest(request: Request) = synchronized(lock) {
        if (active === request) {
            request.fallback?.cancel(false)
            active = null
            bound = null
            worker?.takeIf { it.process.isAlive && !it.retiring.get() }?.let { idleWorker ->
                idleRetirement = deadlines.schedule({
                    synchronized(lock) {
                        if (!closed && active == null && worker === idleWorker) {
                            retire(idleWorker, "idle memory reclamation")
                        }
                    }
                }, idleTimeoutMs, TimeUnit.MILLISECONDS)
            }
        }
    }

    /** Free the warm model when Android reports pressure and no request owns it. */
    fun releaseIdleWorker(reason: String) = synchronized(lock) {
        if (!closed && active == null) worker?.let { retire(it, reason) }
        Unit
    }

    suspend fun start(
        request: Request, voicePackPath: String, phonemizerVoice: String,
        priority: GpuPriority = GpuPriority.LOW,
    ) {
        if (request.cancelled.get()) return
        val attempt = synchronized(lock) {
            check(active === request && !closed)
            if (request.cancelled.get()) return
            if (pendingStartup != null) throw IOException(
                "Previous native startup is still retiring; refusing overlapping launch",
            )
            Startup(request).also { pendingStartup = it }
        }
        val deadline = try {
            deadlines.schedule({
                val error = StartupDeadlineException(
                    "Native startup exceeded ${startupTimeoutMs}ms at ${attempt.stage.get()}",
                )
                if (abortStartup(attempt, error)) {
                    Log.w(TAG, "KOKORO_STARTUP_DEADLINE request=${request.id} " +
                        "timeoutMs=$startupTimeoutMs stage=${attempt.stage.get()}")
                }
            }, startupTimeoutMs, TimeUnit.MILLISECONDS)
        } catch (failure: Exception) {
            synchronized(lock) { if (pendingStartup === attempt) pendingStartup = null }
            throw failure
        }
        try {
            startupExecutor.execute {
                var failure: Throwable? = null
                try { runBlocking { startWorker(attempt, voicePackPath, phonemizerVoice, priority) } }
                catch (error: Throwable) { failure = error }
                finally {
                    synchronized(lock) {
                        if (pendingStartup === attempt) pendingStartup = null
                        if (attempt.finished.compareAndSet(false, true)) {
                            if (failure == null) attempt.result.complete(Unit)
                            else attempt.result.completeExceptionally(failure)
                        }
                    }
                }
            }
            attempt.result.await()
        } catch (failure: Throwable) {
            abortStartup(attempt, failure)
            if (failure is java.util.concurrent.RejectedExecutionException) {
                synchronized(lock) { if (pendingStartup === attempt) pendingStartup = null }
            }
            throw failure
        } finally { deadline.cancel(false) }
    }

    private fun abortStartup(attempt: Startup, error: Throwable): Boolean = synchronized(lock) {
        if (!attempt.finished.compareAndSet(false, true)) return@synchronized false
        if (active === attempt.request) bound?.let { retire(it, "startup aborted: ${attempt.stage.get()}") }
        attempt.result.completeExceptionally(error)
        true
    }

    private fun startupStage(attempt: Startup, stage: String) {
        attempt.stage.set(stage)
        if (diagnosticsEnabled) Log.i(TAG, "KOKORO_STARTUP request=${attempt.request.id} stage=$stage " +
            "elapsedRealtimeMs=${SystemClock.elapsedRealtime()}")
    }

    private suspend fun startWorker(
        attempt: Startup, voicePackPath: String, phonemizerVoice: String, priority: GpuPriority,
    ) {
        val request = attempt.request
        fun aborted() = request.cancelled.get() || attempt.finished.get()
        if (aborted()) return
        startupStage(attempt, "voice_files")
        // A repaired file must not reuse a worker holding the old contents.
        val voiceKey = "$voicePackPath|$phonemizerVoice|" +
            "${File(cwd, voicePackPath).lastModified()}|" +
            "${File(cwd, "weights/model.fp16.bin").lastModified()}|${priority.setting}"
        val previous = synchronized(lock) {
            check(active === request && !closed)
            worker
        }
        if (aborted()) return
        if (previous != null && previous.voice == voiceKey &&
            previous.process.isAlive && !previous.retiring.get()) {
            synchronized(lock) {
                if (active === request && !closed && !aborted()) bound = previous
            }
            if (aborted()) return
            startupStage(attempt, "reuse_ready")
            previous.ready.await()
            return
        }
        // Failed retirement prevents spawning overlapping heavy workers.
        if (previous != null) {
            startupStage(attempt, "previous_worker_exit")
            val retirement = retire(previous, "voice change or dead worker")
            try {
                // A past failed retirement is not permanent session state:
                // the OS may have completed the exit since that failure.
                // Actual process exit, not pipe cleanup or a cached future,
                // is the safety gate for a replacement.
                if (previous.process.isAlive) {
                    withTimeout(retirementTimeoutMs.milliseconds) { retirement.await() }
                }
            } catch (timeout: TimeoutCancellationException) {
                if (previous.process.isAlive)
                    throw IOException("Native worker did not retire within ${retirementTimeoutMs}ms", timeout)
            } catch (failure: Exception) {
                if (failure is CancellationException || previous.process.isAlive) throw failure
            }
            if (previous.process.isAlive) throw IOException("Native worker is still alive after retirement")
        }
        if (aborted()) return
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
            put("NNOPT_QCOM_PRIORITY", priority.setting)
            // Whole-graph replay cannot observe cancellation between blocks.
            put("NNOPT_RECORD", "0")
            put("NNOPT_COOPERATIVE", "1")
            if (diagnosticsEnabled) put("NNOPT_DIAGNOSTICS", "1")
        }
        if (aborted()) return
        startupStage(attempt, "process_create")
        val started = Worker(processFactory(builder), voiceKey)
        val accepted = synchronized(lock) {
            // Track even a late launch, so a replacement cannot overlap it.
            worker = started
            if (active !== request || closed || aborted()) false
            else { bound = started; true }
        }
        if (!accepted) {
            retire(started, "cancelled or expired during process creation")
            return
        }
        startupStage(attempt, "native_ready")
        startReader(started)
        try {
            started.ready.await()
        } catch (t: Throwable) {
            retire(started, "startup failed")
            throw t
        }
        startupStage(attempt, "ready")
        if (diagnosticsEnabled) Log.i(TAG, "Kokoro ready: protocol=3 priority=${priority.setting}")
    }

    /** Pipe reads live on the reader thread, so timeout also covers partial PCM. */
    suspend fun speak(
        request: Request, text: String, speechRate: Float,
        timing: DebugTiming? = null,
        timeoutMs: Long = synthesisTimeoutMs,
        pitch: Float = 1f,
        onAudio: (PcmAudio) -> Unit,
    ) {
        if (request.cancelled.get()) return
        val commandId = ids.incrementAndGet()
        val target = synchronized(lock) {
            check(active === request && !closed)
            if (request.cancelled.get()) return
            val current = bound ?: throw IOException("Kokoro worker is unavailable")
            if (current.retiring.get()) throw IOException("Kokoro worker is retiring")
            current.commandId = commandId
            current.lastNativeError.set(null)
            current.lastNativeStage.set(null)
            current.debugTiming.set(timing)
            current.debugCommandId.set(commandId)
            // Same lock + FIFO writer: CANCEL cannot overtake its SAY.
            send(current, KokoroProtocol.say(commandId, speechRate, text, pitch))
            current
        }
        var totalSamples = 0
        val deadlineAt = SystemClock.elapsedRealtime() + timeoutMs
        val deadlineUptime = if (diagnosticsEnabled) SystemClock.uptimeMillis() + timeoutMs else 0L
        val completed = AtomicBoolean(false)
        val expired = AtomicBoolean(false)
        if (diagnosticsEnabled) {
            Log.i(TAG, "KOKORO_DEADLINE_ARM request=${request.id} command=$commandId " +
                "worker=${System.identityHashCode(target.process)} timeoutMs=$timeoutMs " +
                "dueElapsedMs=$deadlineAt dueUptimeMs=$deadlineUptime")
        }
        val deadline = deadlines.schedule({
            if (completed.compareAndSet(false, true)) {
                expired.set(true)
                Log.w(TAG, "KOKORO_DEADLINE request=${request.id} command=$commandId " +
                    "lateMs=${SystemClock.elapsedRealtime() - deadlineAt} " +
                    "cancelled=${request.cancelled.get()}")
                if (diagnosticsEnabled) {
                    Log.w(TAG, "KOKORO_DEADLINE_STATE request=${request.id} command=$commandId " +
                        "worker=${System.identityHashCode(target.process)} " +
                        "lateUptimeMs=${SystemClock.uptimeMillis() - deadlineUptime} " +
                        "lastReceivedStage=${target.lastNativeStage.get() ?: "none"}")
                }
                retire(target, "independent synthesis deadline")
            }
        }, timeoutMs, TimeUnit.MILLISECONDS)
        try {
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
                                else -> throw IOException(
                                    "Kokoro inference failed (command=$commandId, samples=$totalSamples): " +
                                        (target.lastNativeError.get() ?: "native worker reported ERROR without detail"),
                                )
                            }
                            break
                        }
                    }
                }
            if (expired.get() && !request.cancelled.get())
                throw InferenceDeadlineException("Native synthesis exceeded ${timeoutMs}ms")
        } catch (t: Throwable) {
            retire(target, "synthesis failed or timed out")
            if (expired.get() && !request.cancelled.get())
                throw InferenceDeadlineException("Native synthesis exceeded ${timeoutMs}ms")
            throw t
        } finally {
            completed.set(true)
            deadline.cancel(false)
            target.debugTiming.compareAndSet(timing, null)
            target.debugCommandId.compareAndSet(commandId, 0L)
            synchronized(lock) { if (target.commandId == commandId) target.commandId = null }
        }
    }

    /** Never writes pipes or waits for inference/exit on Android's stop thread. */
    fun requestCancel(request: Request) = synchronized(lock) {
        request.cancelled.set(true)
        if (active !== request || closed) return@synchronized
        pendingStartup?.takeIf { it.request === request }?.let {
            abortStartup(it, IOException("Startup cancelled by Android onStop"))
        }
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
        pendingStartup?.let { abortStartup(it, IOException("Session closed during startup")) }
        startupExecutor.shutdownNow()
        active?.cancelled?.set(true)
        active?.fallback?.cancel(false)
        worker?.let { retire(it, "service destroyed") }
        idleRetirement?.cancel(false)
        deadlines.shutdownNow()
        Unit
    }

    private fun send(target: Worker, command: String) {
        if (target.retiring.get()) return
        try {
            target.commands.execute {
                try {
                    if (diagnosticsEnabled) debugIo(target, "stdin_write_begin", command.substringBefore(' '))
                    target.writer.write(command)
                    target.writer.newLine()
                    target.writer.flush() // Unlike PrintWriter, reports broken pipes.
                    if (diagnosticsEnabled) debugIo(target, "stdin_write_end", command.substringBefore(' '))
                } catch (_: Exception) {
                    retire(target, "native input closed")
                }
            }
        } catch (_: java.util.concurrent.RejectedExecutionException) {
            retire(target, "native command queue closed")
        }
    }

    private fun debugIo(target: Worker, event: String, kind: String = "PCM") {
        if (!diagnosticsEnabled) return
        Log.i(TAG, "KOKORO_IO event=$event kind=$kind command=${target.debugCommandId.get()} " +
            "worker=${System.identityHashCode(target.process)} " +
            "elapsedRealtimeMs=${SystemClock.elapsedRealtime()} uptimeMs=${SystemClock.uptimeMillis()}")
    }

    private fun startReader(target: Worker) {
        thread(name = "kokoro-protocol", isDaemon = true) {
            var failure: Throwable = IOException("Kokoro process closed")
            try {
                target.process.errorStream.bufferedReader(Charsets.UTF_8).useLines { lines ->
                    for (line in lines) {
                        if (line.startsWith("ready.")) {
                            if (line.split(' ').none { it == "protocol=3" }) throw IOException("Stale native runtime: protocol v3 required")
                            if (diagnosticsEnabled) {
                                // Report the binary's own build marker, not just
                                // the Kotlin app version, to expose stale staging.
                                Log.i(TAG, "KOKORO_NATIVE_READY worker=${System.identityHashCode(target.process)} $line")
                            }
                            target.ready.complete(Unit)
                        } else if (line.startsWith("KOKORO_PCM_BEGIN ")) {
                            val header = KokoroProtocol.audioHeader(line)
                            val raw = ByteArray(header.samples * 2)
                            val timing = target.debugTiming.get()
                            val readStarted = if (timing != null) System.nanoTime() else 0L
                            debugIo(target, "pcm_read_begin")
                            target.pcm.readFully(raw)
                            debugIo(target, "pcm_read_end")
                            if (timing != null) timing.pcmReadNs.addAndGet(System.nanoTime() - readStarted)
                            val samples = ShortArray(header.samples)
                            ByteBuffer.wrap(raw).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().get(samples)
                            if (target.events.trySend(KokoroProtocol.Audio(header.id, samples, header.rate)).isFailure) break
                        } else if (line.startsWith("KOKORO_UTT_END ")) {
                            debugIo(target, "terminal_received", "END")
                            if (target.events.trySend(KokoroProtocol.end(line)).isFailure) break
                        } else if (line.startsWith("KOKORO_TIMING ")) {
                            // Internal debug protocol record, never log text or per-chunk data.
                            target.debugTiming.get()?.acceptNative(line, target.debugCommandId.get())
                        } else if (line.startsWith("KOKORO_STARTUP ") && diagnosticsEnabled) {
                            target.lastNativeStage.set(line)
                            Log.i(TAG, "$line worker=${System.identityHashCode(target.process)}")
                        } else if (line.startsWith("KOKORO_STAGE ") && diagnosticsEnabled) {
                            val stage = "$line worker=${System.identityHashCode(target.process)} " +
                                "receivedElapsedMs=${SystemClock.elapsedRealtime()}"
                            target.lastNativeStage.set(stage)
                            Log.i(TAG, stage)
                        } else if (line.startsWith("KOKORO_MEMORY ")) {
                            if (diagnosticsEnabled) Log.i(TAG, line)
                        } else if (line.startsWith("KOKORO_NO_SPEECH ")) {
                            if (diagnosticsEnabled) Log.i(TAG, line)
                        } else if (line.startsWith("ERROR:") || line.startsWith("FATAL")) {
                            target.lastNativeError.set(line.take(MAX_NATIVE_ERROR_LENGTH))
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
        val workerId = System.identityHashCode(target.process)
        if (diagnosticsEnabled) Log.i(TAG, "Retiring native worker id=$workerId: $reason")
        terminate(target, force = false)
        thread(name = "kokoro-retire", isDaemon = true) {
            try {
                // Process.destroy() itself closes Java pipes. It can block
                // behind a reader/writer even after the OS killed the child.
                // Observe exit and escalate independently of that call.
                if (!target.process.waitFor(1, TimeUnit.SECONDS)) {
                    terminate(target, force = true)
                    if (!target.process.waitFor(2, TimeUnit.SECONDS)) throw IOException("Native worker did not exit")
                }
                target.retired.complete(Unit)
                if (diagnosticsEnabled) Log.i(TAG, "Native worker exited id=$workerId")
            } catch (t: Throwable) {
                target.retired.completeExceptionally(t)
                Log.e(TAG, "Worker retirement failed id=$workerId", t)
            } finally {
                runCatching { target.writer.close() }
                runCatching { target.pcm.close() }
                runCatching { target.process.errorStream.close() }
            }
        }
        return target.retired
    }

    private fun terminate(target: Worker, force: Boolean) {
        thread(name = if (force) "kokoro-kill" else "kokoro-terminate", isDaemon = true) {
            val event = if (force) "force_destroy" else "destroy"
            try {
                debugIo(target, "${event}_begin", "RETIRE")
                if (force) target.process.destroyForcibly() else target.process.destroy()
                debugIo(target, "${event}_returned", "RETIRE")
            } catch (failure: Exception) {
                // The exit observer still checks the process and escalates.
                Log.w(TAG, "Native $event failed worker=${System.identityHashCode(target.process)}: " +
                    failure.javaClass.simpleName)
            }
        }
    }

    companion object {
        private const val TAG = "KokoroSession"
        private const val START_TIMEOUT_MS = 30_000L
        private const val SYNTHESIS_TIMEOUT_MS = 120_000L
        private const val CANCEL_FALLBACK_MS = 2_500L
        private const val RETIREMENT_TIMEOUT_MS = 5_000L
        private const val IDLE_TIMEOUT_MS = 120_000L
        private const val MAX_NATIVE_ERROR_LENGTH = 512
    }
}
