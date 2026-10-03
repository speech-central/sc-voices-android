/*
 * Derived from a8nova/adreno-llms, copyright 2026 a8nova.
 * Modified in 2026 for automatic Kokoro installation in an Android TTS engine.
 * SPDX-License-Identifier: Apache-2.0
 */
package com.labsii.voices.runner

import android.content.Context
import android.content.Intent
import android.speech.tts.TextToSpeech
import android.util.Log
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.net.HttpURLConnection
import java.net.URL

/**
 * Installs Kokoro-82M, its English phonemizer data, and all published US/UK
 * English voices. Downloads are resumable and every file is promoted
 * atomically only after its expected size and checksum have been verified.
 */
class KokoroModelManager private constructor(private val context: Context) {

    sealed interface State {
        data object NotInstalled : State
        data class Downloading(
            val currentFile: String,
            val bytesDone: Long,
            val bytesTotal: Long,
        ) : State {
            val progress: Int
                get() = if (bytesTotal <= 0L) 0 else ((bytesDone * 100L) / bytesTotal).toInt()
        }
        data object Ready : State
        data class Failed(val message: String) : State
    }

    private val root = runtimeDir(context)
    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.IO)
    private val _state = MutableStateFlow<State>(
        if (isReady(context)) State.Ready else State.NotInstalled,
    )
    val state: StateFlow<State> = _state.asStateFlow()

    @Volatile private var installJob: Job? = null

    /** Starts or resumes installation. Repeated calls are single-flight. */
    @Synchronized
    fun ensureInstalled() {
        if (installJob?.isActive == true) return
        if (isReady(context)) {
            _state.value = State.Ready
            context.sendBroadcast(Intent(TextToSpeech.Engine.ACTION_TTS_DATA_INSTALLED))
            return
        }
        installJob = scope.launch {
            try {
                verifyPackagedRuntime()
                extractBundledRuntimeFiles()
                downloadMissingFiles()
                if (!isReady(context)) throw IOException("Kokoro installation did not pass validation")
                _state.value = State.Ready
                context.sendBroadcast(Intent(TextToSpeech.Engine.ACTION_TTS_DATA_INSTALLED))
            } catch (_: CancellationException) {
                _state.value = State.NotInstalled
            } catch (t: Throwable) {
                Log.e(TAG, "Kokoro installation failed", t)
                _state.value = State.Failed(t.message ?: t.javaClass.simpleName)
            }
        }
    }

    fun retry() = ensureInstalled()

    private fun verifyPackagedRuntime() {
        val binary = File(context.applicationInfo.nativeLibraryDir, NATIVE_BINARY)
        if (!binary.isFile) {
            throw IOException(
                "$NATIVE_BINARY is not packaged. Run android/scripts/prepare_assets.sh before building the APK.",
            )
        }
        for (relative in BUNDLED_FILES) {
            try {
                context.assets.open("$ASSET_ROOT/$relative").close()
            } catch (_: IOException) {
                throw IOException(
                    "Kokoro runtime asset '$relative' is not packaged. Run android/scripts/prepare_assets.sh.",
                )
            }
        }
    }

    private fun extractBundledRuntimeFiles() {
        root.mkdirs()
        if (bundledRuntimeReady(context)) return
        File(root, RUNTIME_MARKER).delete()
        for (relative in BUNDLED_FILES) {
            val destination = File(root, relative)
            destination.parentFile?.mkdirs()
            val temporary = File(destination.parentFile, destination.name + ".tmp")
            context.assets.open("$ASSET_ROOT/$relative").use { input ->
                FileOutputStream(temporary, false).use { output ->
                    input.copyTo(output, BUFFER_BYTES)
                    output.fd.sync()
                }
            }
            if (!temporary.renameTo(destination)) {
                temporary.delete()
                throw IOException("Could not install $relative")
            }
        }
        VerifiedFile.atomicWrite(File(root, RUNTIME_MARKER), runtimeGeneration(context))
    }

    private fun downloadMissingFiles() {
        root.mkdirs()
        var completed = DOWNLOADS.sumOf { spec ->
            File(root, spec.localPath).takeIf { it.isFile && it.length() == spec.sizeBytes }?.length() ?: 0L
        }
        val remaining = TOTAL_DOWNLOAD_BYTES - completed
        if (remaining > 0L && root.usableSpace in 1L until (remaining + MIN_FREE_SPACE_BYTES)) {
            throw IOException("Not enough storage for Kokoro (need about ${mib(remaining + MIN_FREE_SPACE_BYTES)} MiB free)")
        }

        for (spec in DOWNLOADS) {
            val destination = File(root, spec.localPath)
            if (destination.isFile && destination.length() == spec.sizeBytes) {
                _state.value = State.Downloading("Verifying ${spec.label}", completed, TOTAL_DOWNLOAD_BYTES)
                if (VerifiedFile.isVerified(destination, spec.sizeBytes, spec.sha256) ||
                    VerifiedFile.verify(destination, spec.sizeBytes, spec.sha256)) {
                    VerifiedFile.record(destination, spec.sha256)
                    notifyVoiceDataChanged(spec)
                    continue
                }
                if (!destination.delete()) {
                    throw IOException("Could not remove corrupt ${spec.label}")
                }
                completed -= spec.sizeBytes
                ensureFreeSpace(spec.sizeBytes)
            }
            if (destination.exists()) destination.delete()
            completed = downloadOne(spec, completed)
        }
    }

    private fun downloadOne(spec: DownloadSpec, completedBefore: Long): Long {
        val destination = File(root, spec.localPath)
        destination.parentFile?.mkdirs()
        val partial = File(destination.parentFile, destination.name + ".part")
        if (partial.length() > spec.sizeBytes) partial.delete()
        if (partial.length() == spec.sizeBytes) {
            verifyChecksum(partial, spec)
            promote(partial, destination)
            VerifiedFile.record(destination, spec.sha256)
            notifyVoiceDataChanged(spec)
            return completedBefore + spec.sizeBytes
        }

        var existing = partial.length()
        val connection = (URL("$HF_BASE/${spec.remotePath}").openConnection() as HttpURLConnection).apply {
            connectTimeout = CONNECT_TIMEOUT_MS
            readTimeout = READ_TIMEOUT_MS
            instanceFollowRedirects = true
            requestMethod = "GET"
            setRequestProperty("Accept-Encoding", "identity")
            setRequestProperty("User-Agent", "Labsii-Voices/0.6.11")
            if (existing > 0L) setRequestProperty("Range", "bytes=$existing-")
        }

        try {
            connection.connect()
            val response = connection.responseCode
            if (response !in 200..299) throw IOException("HTTP $response while downloading ${spec.label}")
            val append = existing > 0L && response == HttpURLConnection.HTTP_PARTIAL
            if (!append) {
                existing = 0L
                partial.delete()
            }

            var fileBytes = existing
            var lastReported = fileBytes
            _state.value = State.Downloading(spec.label, completedBefore + fileBytes, TOTAL_DOWNLOAD_BYTES)
            connection.inputStream.use { input ->
                FileOutputStream(partial, append).use { output ->
                    val buffer = ByteArray(BUFFER_BYTES)
                    while (true) {
                        val count = input.read(buffer)
                        if (count < 0) break
                        if (count == 0) continue
                        if (fileBytes + count > spec.sizeBytes) throw IOException("Oversized download: ${spec.label}")
                        output.write(buffer, 0, count)
                        fileBytes += count
                        if (fileBytes - lastReported >= PROGRESS_STEP_BYTES || fileBytes == spec.sizeBytes) {
                            lastReported = fileBytes
                            _state.value = State.Downloading(
                                spec.label,
                                completedBefore + minOf(fileBytes, spec.sizeBytes),
                                TOTAL_DOWNLOAD_BYTES,
                            )
                        }
                    }
                    output.fd.sync()
                }
            }
            if (partial.length() != spec.sizeBytes) {
                throw IOException(
                    "${spec.label} is incomplete: received ${partial.length()} of ${spec.sizeBytes} bytes",
                )
            }
            verifyChecksum(partial, spec)
            promote(partial, destination)
            VerifiedFile.record(destination, spec.sha256)
            notifyVoiceDataChanged(spec)
            return completedBefore + spec.sizeBytes
        } finally {
            connection.disconnect()
        }
    }

    private fun promote(partial: File, destination: File) {
        if (!partial.renameTo(destination)) throw IOException("Could not finish ${destination.name}")
    }

    private fun notifyVoiceDataChanged(spec: DownloadSpec) {
        if (spec.localPath.startsWith(VOICE_DIRECTORY)) {
            context.sendBroadcast(Intent(TextToSpeech.Engine.ACTION_TTS_DATA_INSTALLED))
        }
    }

    private fun ensureFreeSpace(bytesNeeded: Long) {
        if (root.usableSpace in 1L until (bytesNeeded + MIN_FREE_SPACE_BYTES)) {
            throw IOException(
                "Not enough storage to replace a corrupt Kokoro file " +
                    "(need about ${mib(bytesNeeded + MIN_FREE_SPACE_BYTES)} MiB free)",
            )
        }
    }

    private fun verifyChecksum(file: File, spec: DownloadSpec) {
        if (VerifiedFile.verify(file, spec.sizeBytes, spec.sha256)) return
        file.delete()
        throw IOException("Checksum verification failed for ${spec.label}; retrying will download it again")
    }

    private data class DownloadSpec(
        val label: String,
        val remotePath: String,
        val localPath: String,
        val sizeBytes: Long,
        val sha256: String,
    )

    companion object {
        private const val TAG = "KokoroModelManager"
        private const val NATIVE_BINARY = "libkokoro.so"
        private const val ASSET_ROOT = "kokoro"
        private const val VOICE_DIRECTORY = "assets/voices/"
        private const val RUNTIME_MARKER = ".bundled_runtime_v1"
        private const val RUNTIME_VERSION = "0.6.11-protocol2"
        private const val BUFFER_BYTES = 256 * 1024
        private const val PROGRESS_STEP_BYTES = 1024 * 1024
        private const val MIN_FREE_SPACE_BYTES = 32L * 1024L * 1024L
        private const val CONNECT_TIMEOUT_MS = 30_000
        private const val READ_TIMEOUT_MS = 120_000
        private const val HF_REVISION = "7a5f7a1d4fe10589d4b7984b6161591065902707"
        private const val HF_BASE =
            "https://huggingface.co/a8nova/adreno-llms-weights/resolve/$HF_REVISION/kokoro-82m"

        private val BUNDLED_FILES = listOf(
            "assets/phoneme_vocab.tsv",
            "kernels/conv_1d.cl",
            "kernels/conv_transpose_1d.cl",
            "kernels/hifigan_residual_block.cl",
            "kernels/istft.cl",
            "kernels/length_regulator.cl",
            "kernels/text_encoder.cl",
            "kernels/utils.cl",
        )

        private val BASE_DOWNLOADS = listOf(
            DownloadSpec("Kokoro model", "model.fp16.bin", "weights/model.fp16.bin", 163_526_820, "94725a9a8f70300e5fc600523bd664960f0fd0126b8ad9349ac28bdb04b77f7c"),
            DownloadSpec("Kokoro metadata", "model.fp16.meta.json", "weights/model.fp16.meta.json", 120_787, "355b6522bf36c14fc40e618e97f075b3c8c397cd5e6bbca18a2d1529468d39f9"),
            DownloadSpec("English dictionary", "espeak-lang/en/en_dict", "assets/espeak-ng-data/en_dict", 168_304, "7a559766d239f7642632350d421e2b6f2330cc8f56ec4953e1daf37e5dde34e2"),
            DownloadSpec("English intonation", "espeak-lang/en/intonations", "assets/espeak-ng-data/intonations", 2_584, "c4ff28c76c30cf7d9f07643e08ec39e68769ef648098269b5c0ab7731218e219"),
            DownloadSpec("English voice", "espeak-lang/en/lang/gmw/en", "assets/espeak-ng-data/lang/gmw/en", 148, "4c8f40ba654a18e2e5d7837f55db5df294eba3a731cd861d421ea52516c16677"),
            DownloadSpec("English 029 voice", "espeak-lang/en/lang/gmw/en-029", "assets/espeak-ng-data/lang/gmw/en-029", 355, "d450cc687501aa1c68466958373f2b3acea7eb7b4be0c4f5d04b02ccaed72b95"),
            DownloadSpec("Scottish voice", "espeak-lang/en/lang/gmw/en-GB-scotland", "assets/espeak-ng-data/lang/gmw/en-GB-scotland", 312, "adbd70bed8594f2ed878121f49a4bd49a41a6564cd1102fc749f92c72c434617"),
            DownloadSpec("Lancashire voice", "espeak-lang/en/lang/gmw/en-GB-x-gbclan", "assets/espeak-ng-data/lang/gmw/en-GB-x-gbclan", 252, "3d3caf12aa3abeb3e13836dee250e98acff3cd7f08d252559332a6d91bf500c5"),
            DownloadSpec("Welsh English voice", "espeak-lang/en/lang/gmw/en-GB-x-gbcwmd", "assets/espeak-ng-data/lang/gmw/en-GB-x-gbcwmd", 200, "99136e9688eef0ba1ff874f366ee8dff7090e58bd424aa93d22ee2775f16b8c4"),
            DownloadSpec("Received Pronunciation voice", "espeak-lang/en/lang/gmw/en-GB-x-rp", "assets/espeak-ng-data/lang/gmw/en-GB-x-rp", 264, "a305a95506b5deef64faf93428690ddfd14da5f3c3f6a23dd0353ccc563e538a"),
            DownloadSpec("Shavian voice", "espeak-lang/en/lang/gmw/en-Shaw", "assets/espeak-ng-data/lang/gmw/en-Shaw", 124, "11b887abe5b3fa11c2bb760fcf545bdbbd053666869f18c67e6ea9f10facec44"),
            DownloadSpec("US English voice", "espeak-lang/en/lang/gmw/en-US", "assets/espeak-ng-data/lang/gmw/en-US", 272, "4c5f247313a4206d2d5b2b411b973a007b0617f0633add363a74717aba8d421c"),
            DownloadSpec("New York voice", "espeak-lang/en/lang/gmw/en-US-nyc", "assets/espeak-ng-data/lang/gmw/en-US-nyc", 285, "62b13f9a239fee09b8ddc230a171ebdcd0cd2f14c4b198ebe0b6f9682ba2f372"),
            DownloadSpec("Phoneme data", "espeak-lang/en/phondata", "assets/espeak-ng-data/phondata", 600_104, "fbf85d4b2d5ed7f0da20e22a3063cfb2ad0bb36cfb322d5e21a2e9710ad509b7"),
            DownloadSpec("Phoneme manifest", "espeak-lang/en/phondata-manifest", "assets/espeak-ng-data/phondata-manifest", 22_936, "e9dd3bee1b8453a45d2fa08684f850bc156f2021b45e4cc5d1919409cb1e0647"),
            DownloadSpec("Phoneme index", "espeak-lang/en/phonindex", "assets/espeak-ng-data/phonindex", 48_270, "e38b0e293efa022f0569280576d15fcef0d1e339471e6d4a314d9b5e2b6afa0f"),
            DownloadSpec("Phoneme table", "espeak-lang/en/phontab", "assets/espeak-ng-data/phontab", 63_412, "f3158807605f48fe01f5a75ae6d178d1d906a2687166455f458bf1a199ac172d"),
        )

        private val DOWNLOADS = BASE_DOWNLOADS + KokoroVoices.all.map { voice ->
            DownloadSpec(
                label = "${voice.displayName} voice",
                remotePath = voice.remotePath,
                localPath = voice.localPath,
                sizeBytes = KokoroVoices.PACK_BYTES,
                sha256 = voice.sha256,
            )
        }
        val totalDownloadBytes: Long = DOWNLOADS.sumOf { it.sizeBytes }
        private val TOTAL_DOWNLOAD_BYTES = totalDownloadBytes

        @Volatile private var instance: KokoroModelManager? = null

        fun get(context: Context): KokoroModelManager = instance ?: synchronized(this) {
            instance ?: KokoroModelManager(context.applicationContext).also { instance = it }
        }

        fun runtimeDir(context: Context): File = File(context.filesDir, "kokoro")

        fun isRuntimeReady(context: Context): Boolean {
            val root = runtimeDir(context)
            val binary = File(context.applicationInfo.nativeLibraryDir, NATIVE_BINARY)
            return binary.isFile &&
                bundledRuntimeReady(context) &&
                BASE_DOWNLOADS.all { spec ->
                    VerifiedFile.isVerified(File(root, spec.localPath), spec.sizeBytes, spec.sha256)
                }
        }

        fun isVoiceInstalled(context: Context, voice: KokoroVoiceSpec): Boolean =
            VerifiedFile.isVerified(File(runtimeDir(context), voice.localPath), KokoroVoices.PACK_BYTES, voice.sha256)

        fun isReady(context: Context): Boolean {
            val root = runtimeDir(context)
            return isRuntimeReady(context) &&
                DOWNLOADS.all { spec -> VerifiedFile.isVerified(File(root, spec.localPath), spec.sizeBytes, spec.sha256) }
        }

        @Volatile private var generation: String? = null

        private fun runtimeGeneration(context: Context): String = generation ?: synchronized(this) {
            generation ?: "$RUNTIME_VERSION:${context.packageManager.getPackageInfo(context.packageName, 0).lastUpdateTime}"
                .also { generation = it }
        }

        private fun bundledRuntimeReady(context: Context): Boolean {
            val root = runtimeDir(context)
            return BUNDLED_FILES.all { File(root, it).isFile } && runCatching {
                File(root, RUNTIME_MARKER).readText() == runtimeGeneration(context)
            }.getOrDefault(false)
        }

        private fun mib(bytes: Long): Long = (bytes + 1024L * 1024L - 1L) / (1024L * 1024L)
    }
}
