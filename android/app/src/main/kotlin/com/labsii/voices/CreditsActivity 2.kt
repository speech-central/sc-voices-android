/*
 * Adreno System TTS integration.
 * SPDX-License-Identifier: GPL-3.0-or-later
 * See NOTICE for upstream copyright and attribution information.
 */
package com.labsii.voices

import android.app.Activity
import android.os.Bundle
import android.text.method.LinkMovementMethod
import android.text.util.Linkify
import android.view.View
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView

/** Displays attribution notices and complete license texts shipped in the APK. */
class CreditsActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(20), dp(16), dp(20), dp(16))
        }
        root.addView(Button(this).apply {
            text = "Back"
            setOnClickListener { finish() }
        })
        root.addView(TextView(this).apply {
            text = "Credits and licenses"
            textSize = 26f
            setPadding(0, dp(12), 0, dp(8))
        })

        val legalAssets = assets.list("legal").orEmpty().sortedWith(compareBy<String> {
            LEGAL_ASSET_ORDER.indexOf(it).takeIf { index -> index >= 0 } ?: Int.MAX_VALUE
        }.thenBy { it })
        val legalText = legalAssets.joinToString("\n\n" + "—".repeat(72) + "\n\n") { asset ->
            assets.open("legal/$asset").bufferedReader().use { it.readText() }
        }
        val notice = TextView(this).apply {
            text = legalText
            textSize = 13f
            setTextIsSelectable(true)
            Linkify.addLinks(this, Linkify.WEB_URLS)
            movementMethod = LinkMovementMethod.getInstance()
        }
        root.addView(ScrollView(this).apply { addView(notice) },
            LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, 0, 1f))
        setContentView(root)
    }

    private fun dp(value: Int): Int = (value * resources.displayMetrics.density).toInt()

    companion object {
        private val LEGAL_ASSET_ORDER = listOf(
            "NOTICE.txt",
            "Branding.txt",
            "GPL-3.0-or-later.txt",
            "Apache-2.0.txt",
            "eSpeak-NG-Apache.txt",
            "eSpeak-NG-BSD-2-Clause.txt",
            "eSpeak-NG-Unicode.txt",
        )
    }
}
