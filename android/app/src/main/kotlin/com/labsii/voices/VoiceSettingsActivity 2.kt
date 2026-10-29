/*
 * Adreno System TTS integration.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
package com.labsii.voices

import android.app.Activity
import android.graphics.Typeface
import android.os.Bundle
import android.view.View
import android.widget.LinearLayout
import android.widget.RadioButton
import android.widget.RadioGroup
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast
import com.google.android.material.button.MaterialButton
import com.labsii.voices.settings.AppSettings
import com.labsii.voices.system.InstalledKokoroVoice
import com.labsii.voices.system.InstalledKokoroVoiceCatalog
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch
import java.util.Locale

/** Settings activity advertised to Android for clients without a voice picker. */
class VoiceSettingsActivity : Activity() {
    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Main.immediate)
    private lateinit var settings: AppSettings

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        settings = AppSettings(applicationContext)
        setContentView(buildLoadingScreen())
        scope.launch { showVoices() }
    }

    override fun onDestroy() {
        scope.cancel()
        super.onDestroy()
    }

    private fun buildLoadingScreen(): View = ScrollView(this).apply {
        addView(content().apply {
            addView(title("Default Kokoro voice"))
            addView(body("Loading installed voices…"))
        })
    }

    private suspend fun showVoices() {
        val voices = InstalledKokoroVoiceCatalog(applicationContext).installed()
        val stored = settings.snapshot().defaultVoiceId
        val selected = voices.firstOrNull { it.id == stored }?.id
            ?: voices.firstOrNull()?.id
            ?: AppSettings.DEFAULT_VOICE_ID
        val root = content().apply {
            addView(title("Default Kokoro voice"))
            addView(body(
                "This voice is used by apps that do not choose a Kokoro voice themselves. " +
                    "A voice explicitly selected by an app still takes priority.",
            ))
        }
        if (voices.isEmpty()) {
            root.addView(body("Install and verify Kokoro voices in SC Kokoro before choosing a default."), params(20))
            root.addView(MaterialButton(this).apply {
                text = "Open SC Kokoro"
                setOnClickListener {
                    packageManager.getLaunchIntentForPackage(packageName)?.let(::startActivity)
                }
            }, params(20))
        } else {
            val group = RadioGroup(this).apply { orientation = RadioGroup.VERTICAL }
            addVoiceGroup(group, "English (United States)", voices.filter { it.locale == Locale.US }, selected)
            addVoiceGroup(group, "English (United Kingdom)", voices.filter { it.locale == Locale.UK }, selected)
            root.addView(group, params(16))
            group.setOnCheckedChangeListener { _, checkedId ->
                val voiceId = group.findViewById<RadioButton>(checkedId)?.tag as? String
                    ?: return@setOnCheckedChangeListener
                scope.launch {
                    settings.setDefaultVoice(voiceId)
                    Toast.makeText(this@VoiceSettingsActivity, "Default voice saved", Toast.LENGTH_SHORT).show()
                }
            }
        }
        setContentView(ScrollView(this).apply { addView(root) })
    }

    private fun addVoiceGroup(
        group: RadioGroup,
        heading: String,
        voices: List<InstalledKokoroVoice>,
        selected: String,
    ) {
        if (voices.isEmpty()) return
        group.addView(TextView(this).apply {
            text = heading
            textSize = 17f
            setTypeface(typeface, Typeface.BOLD)
        }, params(24))
        voices.forEach { voice ->
            group.addView(RadioButton(this).apply {
                id = View.generateViewId()
                tag = voice.id
                text = voice.displayName
                isChecked = voice.id == selected
            })
        }
    }

    private fun content(): LinearLayout = LinearLayout(this).apply {
        orientation = LinearLayout.VERTICAL
        setPadding(dp(20), dp(28), dp(20), dp(28))
    }

    private fun title(value: String): TextView = TextView(this).apply {
        text = value
        textSize = 28f
        setTypeface(typeface, Typeface.BOLD)
    }

    private fun body(value: String): TextView = TextView(this).apply {
        text = value
        textSize = 15f
        setPadding(0, dp(8), 0, 0)
    }

    private fun params(top: Int): LinearLayout.LayoutParams =
        LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT).apply {
            topMargin = dp(top)
        }

    private fun dp(value: Int): Int = (value * resources.displayMetrics.density).toInt()
}
