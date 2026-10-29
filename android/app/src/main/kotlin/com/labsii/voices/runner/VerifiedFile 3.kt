/* Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0 */
package com.labsii.voices.runner

import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.security.MessageDigest

/** Integrity receipts for app-private, atomically installed immutable files. */
internal object VerifiedFile {
    private fun receipt(file: File) = File(file.parentFile, file.name + ".verified")
    private fun stamp(file: File, sha256: String) = "$sha256:${file.length()}:${file.lastModified()}"

    // Cheap readiness checks: hashing never runs on Android TTS Binder threads.
    fun isVerified(file: File, bytes: Long, sha256: String): Boolean =
        file.isFile && file.length() == bytes && runCatching {
            receipt(file).readText() == stamp(file, sha256)
        }.getOrDefault(false)

    fun verify(file: File, bytes: Long, sha256: String): Boolean {
        if (!file.isFile || file.length() != bytes) return false
        val before = stamp(file, sha256)
        val digest = MessageDigest.getInstance("SHA-256")
        file.inputStream().buffered(256 * 1024).use { input ->
            val buffer = ByteArray(256 * 1024)
            while (true) {
                val count = input.read(buffer)
                if (count < 0) break
                digest.update(buffer, 0, count)
            }
        }
        val actual = digest.digest().joinToString("") { "%02x".format(it.toInt() and 255) }
        return before == stamp(file, sha256) && actual.equals(sha256, ignoreCase = true)
    }

    // Call only after successful hash verification, after the final rename.
    fun record(file: File, sha256: String) = atomicWrite(receipt(file), stamp(file, sha256))

    fun atomicWrite(destination: File, text: String) {
        val temp = File(destination.parentFile, destination.name + ".tmp")
        FileOutputStream(temp).use { it.write(text.toByteArray(Charsets.UTF_8)); it.fd.sync() }
        if (!temp.renameTo(destination)) throw IOException("Could not install ${destination.name}")
    }
}
