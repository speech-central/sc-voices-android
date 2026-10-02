package com.labsii.voices.runner

import org.junit.Assert.*
import org.junit.Test
import java.nio.file.Files
import java.io.File

class VerifiedFileTest {
    @Test fun readinessRequiresVerifiedContentNotJustLength() {
        val dir = Files.createTempDirectory("verified-file-test").toFile()
        val file = File(dir, "model.bin")
        val sha = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
        try {
            file.writeText("abc")
            assertFalse(VerifiedFile.isVerified(file, 3, sha))
            assertTrue(VerifiedFile.verify(file, 3, sha))
            VerifiedFile.record(file, sha)
            assertTrue(VerifiedFile.isVerified(file, 3, sha))
            assertFalse(VerifiedFile.isVerified(file, 3, "new-generation"))
            val modified = file.lastModified()
            file.writeText("xyz")
            file.setLastModified(modified + 2000)
            assertFalse(VerifiedFile.isVerified(file, 3, sha))
            assertFalse(VerifiedFile.verify(file, 3, sha))
        } finally { dir.deleteRecursively() }
    }
}
