package com.labsii.voices.runner

import kotlinx.coroutines.*
import org.junit.Assert.*
import org.junit.Test
import java.io.*
import java.nio.file.Files
import java.util.concurrent.*
import java.util.concurrent.atomic.AtomicBoolean

class KokoroSessionTest {
    private class Pipe : InputStream() {
        private val bytes = LinkedBlockingQueue<Int>()
        @Volatile private var closed = false
        fun put(value: ByteArray) { value.forEach { bytes.put(it.toInt() and 255) } }
        override fun read(): Int {
            if (closed && bytes.isEmpty()) return -1
            return bytes.take().also { if (it < 0) closed = true }
        }
        override fun read(buffer: ByteArray, offset: Int, length: Int): Int {
            if (length == 0) return 0
            val first = read()
            if (first < 0) return -1
            buffer[offset] = first.toByte()
            var count = 1
            while (count < length) {
                val next = bytes.poll() ?: break
                if (next < 0) { closed = true; break }
                buffer[offset + count++] = next.toByte()
            }
            return count
        }
        override fun close() { closed = true; bytes.offer(-1) }
    }

    private class FakeProcess(
        private val closeGate: CountDownLatch? = null,
        private val refuseExit: Boolean = false,
        val handle: (FakeProcess, String) -> Unit,
    ) : Process() {
        val audio = Pipe()
        val errors = Pipe()
        val ended = CountDownLatch(1)
        val alive = AtomicBoolean(true)
        val input = object : OutputStream() {
            val line = ByteArrayOutputStream()
            override fun write(value: Int) {
                if (!alive.get()) throw IOException("closed")
                if (value == 10) { val text = line.toString("UTF-8"); line.reset(); handle(this@FakeProcess, text) }
                else line.write(value)
            }
            override fun close() { closeGate?.await() }
        }
        init { event("ready. protocol=2 build=test") }
        fun event(value: String) = errors.put((value + "\n").toByteArray())
        fun success(id: String) {
            event("KOKORO_PCM_BEGIN $id 2 24000")
            audio.put(byteArrayOf(100, 0, -100, -1))
            event("KOKORO_UTT_END $id OK")
        }
        override fun getOutputStream(): OutputStream = input
        override fun getInputStream(): InputStream = audio
        override fun getErrorStream(): InputStream = errors
        override fun isAlive() = alive.get()
        override fun waitFor(): Int { ended.await(); return 0 }
        override fun waitFor(timeout: Long, unit: TimeUnit) = ended.await(timeout, unit)
        override fun exitValue(): Int { if (alive.get()) throw IllegalThreadStateException(); return 0 }
        override fun destroy() {
            if (refuseExit) return
            alive.set(false); audio.close(); errors.close(); ended.countDown()
        }
        override fun destroyForcibly(): Process { destroy(); return this }
    }

    // Deliberately runs a cancelled task too: simulates one already executing
    // when finishRequest() cancels its future, then delayed acquiring the lock.
    private class ManualDeadlines : ScheduledThreadPoolExecutor(1) {
        val tasks = mutableListOf<Runnable>()
        override fun schedule(command: Runnable, delay: Long, unit: TimeUnit): ScheduledFuture<*> {
            tasks.add(command)
            return super.schedule({}, 1, TimeUnit.DAYS)
        }
    }

    private suspend fun fixture(
        factory: () -> FakeProcess,
        timeout: Long = 1000,
        scheduler: ScheduledExecutorService = Executors.newSingleThreadScheduledExecutor(),
        retirementTimeout: Long = 5000,
        idleTimeout: Long = 120000,
        body: suspend (KokoroSession) -> Unit,
    ) {
        val dir = Files.createTempDirectory("kokoro-session-test").toFile()
        File(dir, "libkokoro.so").writeText("fixture")
        val session = KokoroSession(dir.path, dir, { factory() }, timeout, 50, scheduler,
            retirementTimeout, idleTimeout)
        try { body(session) } finally { session.close(); scheduler.shutdownNow(); dir.deleteRecursively() }
    }

    @Test fun consecutiveSentencesReuseWorkerAndDoNotLoseTerminalFrames() = runBlocking {
        var starts = 0
        fixture({ starts++; FakeProcess { p, line -> p.success(line.split(' ')[1]) } }) { session ->
            repeat(3) {
                val request = session.beginRequest()
                session.start(request, "heart", "en-us")
                var samples = 0
                repeat(2) { session.speak(request, "Hello.", 1f) { samples += it.samples.size } }
                assertEquals(4, samples)
                session.finishRequest(request)
            }
            assertEquals(1, starts)
        }
    }

    @Test fun delayedOldWatchdogCannotKillNewRequest() = runBlocking {
        val scheduler = ManualDeadlines()
        val process = FakeProcess { p, line -> if (line.startsWith("SAY")) p.success(line.split(' ')[1]) }
        fixture({ process }, scheduler = scheduler) { session ->
            val old = session.beginRequest()
            session.start(old, "heart", "en-us")
            session.requestCancel(old)
            session.finishRequest(old)
            val replacement = session.beginRequest()
            session.start(replacement, "heart", "en-us")
            scheduler.tasks.forEach { it.run() }
            assertTrue(process.isAlive)
            session.speak(replacement, "Still speaking.", 1f) {}
            session.finishRequest(replacement)
        }
    }

    @Test fun partialPcmIsCoveredByDeadline() = runBlocking {
        lateinit var process: FakeProcess
        fixture({ FakeProcess { p, line -> p.event("KOKORO_PCM_BEGIN ${line.split(' ')[1]} 20 24000"); p.audio.put(byteArrayOf(0, 0)) }.also { process = it } }, timeout = 80) { session ->
            val request = session.beginRequest()
            session.start(request, "heart", "en-us")
            try { session.speak(request, "Stalled.", 1f) {}; fail("Expected timeout") }
            catch (_: TimeoutCancellationException) { /* timeout must wake despite blocked readFully */ }
            assertTrue(process.ended.await(1, TimeUnit.SECONDS))
            session.finishRequest(request)
        }
    }

    @Test fun stopDoesNotWaitBehindBlockedWriter() = runBlocking {
        val entered = CountDownLatch(1)
        lateinit var process: FakeProcess
        fixture({ FakeProcess { _, _ -> entered.countDown(); CountDownLatch(1).await() }.also { process = it } }) { session ->
            val request = session.beginRequest()
            session.start(request, "heart", "en-us")
            supervisorScope {
                val speaking = async { session.speak(request, "Blocked input.", 1f) {} }
                withContext(Dispatchers.IO) { assertTrue(entered.await(1, TimeUnit.SECONDS)) }
                val before = System.nanoTime()
                session.requestCancel(request)
                assertTrue(TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - before) < 100)
                try { speaking.await(); fail("Expected retired worker") } catch (_: IOException) {}
            }
            assertTrue(process.ended.await(1, TimeUnit.SECONDS))
            session.finishRequest(request)
        }
    }

    @Test fun exitBetweenReadyAndSubmissionIsRemembered() = runBlocking {
        val process = FakeProcess { _, _ -> }
        fixture({ process }) { session ->
            val request = session.beginRequest()
            session.start(request, "heart", "en-us")
            process.destroy()
            withTimeout(500) {
                try { session.speak(request, "Hello.", 1f) {}; fail("Expected closed worker") }
                catch (_: IOException) {}
            }
            session.finishRequest(request)
        }
    }

    @Test fun partialAudioFollowedByNativeErrorIsNotSuccess() = runBlocking {
        fixture({ FakeProcess { p, line ->
            val id = line.split(' ')[1]
            p.event("KOKORO_PCM_BEGIN $id 1 24000"); p.audio.put(byteArrayOf(10, 0))
            p.event("KOKORO_UTT_END $id ERROR")
        } }) { session ->
            val request = session.beginRequest()
            session.start(request, "heart", "en-us")
            try { session.speak(request, "Hello.", 1f) {}; fail("Expected inference error") }
            catch (_: IOException) {}
            session.finishRequest(request)
        }
    }

    @Test fun cooperativeCancellationKeepsWarmWorkerForNextSentence() = runBlocking {
        val started = CompletableDeferred<Unit>()
        var starts = 0
        var says = 0
        fixture({ starts++; FakeProcess { p, line ->
            val id = line.split(' ')[1]
            if (line.startsWith("CANCEL")) p.event("KOKORO_UTT_END $id CANCELLED")
            else if (++says == 1) started.complete(Unit)
            else p.success(id)
        } }) { session ->
            val old = session.beginRequest()
            session.start(old, "heart", "en-us")
            coroutineScope {
                val speaking = async { session.speak(old, "Cancel me.", 1f) {} }
                started.await()
                session.requestCancel(old)
                speaking.await()
            }
            session.finishRequest(old)
            val next = session.beginRequest()
            session.start(next, "heart", "en-us")
            session.speak(next, "Replacement.", 1f) {}
            session.finishRequest(next)
            assertEquals(1, starts)
        }
    }

    @Test fun cancellationBeforeStartupLaunchesNothing() = runBlocking {
        var starts = 0
        fixture({ starts++; FakeProcess { _, _ -> } }) { session ->
            val request = session.beginRequest()
            session.requestCancel(request)
            session.start(request, "heart", "en-us")
            session.speak(request, "Cancelled.", 1f) { fail("Unexpected audio") }
            session.finishRequest(request)
            assertEquals(0, starts)
        }
    }

    @Test fun voiceChangeWaitsForPreviousWorkerExit() = runBlocking {
        var previous: FakeProcess? = null
        var starts = 0
        fixture({
            previous?.let { assertFalse("Workers must not overlap", it.isAlive) }
            starts++
            FakeProcess { p, line -> p.success(line.split(' ')[1]) }.also { previous = it }
        }) { session ->
            for (voice in listOf("heart", "bella", "heart")) {
                val request = session.beginRequest()
                session.start(request, voice, "en-us")
                session.speak(request, "Voice change.", 1f) {}
                session.finishRequest(request)
            }
            assertEquals(3, starts)
        }
    }

    @Test fun blockedPipeCloseCannotDelayReplacementAfterWorkerExit() = runBlocking {
        val closeGate = CountDownLatch(1)
        var starts = 0
        try {
            fixture({
                starts++
                FakeProcess(closeGate = if (starts == 1) closeGate else null) { p, line ->
                    p.success(line.split(' ')[1])
                }
            }, retirementTimeout = 500) { session ->
                val first = session.beginRequest()
                session.start(first, "heart", "en-us")
                session.speak(first, "First.", 1f) {}
                session.finishRequest(first)
                val second = session.beginRequest()
                withTimeout(1000) { session.start(second, "bella", "en-us") }
                session.speak(second, "Second.", 1f) {}
                session.finishRequest(second)
                assertEquals(2, starts)
            }
        } finally { closeGate.countDown() }
    }

    @Test fun unresponsiveWorkerRetirementHasDeadlineAndNeverOverlaps() = runBlocking {
        var starts = 0
        fixture({
            starts++
            FakeProcess(refuseExit = true) { p, line -> p.success(line.split(' ')[1]) }
        }, retirementTimeout = 80) { session ->
            val first = session.beginRequest()
            session.start(first, "heart", "en-us")
            session.finishRequest(first)
            val second = session.beginRequest()
            try { session.start(second, "bella", "en-us"); fail("Expected retirement timeout") }
            catch (error: IOException) { assertTrue(error.message!!.contains("did not retire")) }
            session.finishRequest(second)
            assertEquals(1, starts)
        }
    }

    @Test fun idleWorkerReleasesNativeMemory() = runBlocking {
        var starts = 0
        lateinit var process: FakeProcess
        fixture({
            starts++
            FakeProcess { p, line -> p.success(line.split(' ')[1]) }.also { process = it }
        }, idleTimeout = 50) { session ->
            val first = session.beginRequest()
            session.start(first, "heart", "en-us")
            session.finishRequest(first)
            assertTrue(process.ended.await(1, TimeUnit.SECONDS))
            val second = session.beginRequest()
            session.start(second, "heart", "en-us")
            session.speak(second, "Still works.", 1f) {}
            session.finishRequest(second)
            assertEquals(2, starts)
        }
    }
}
