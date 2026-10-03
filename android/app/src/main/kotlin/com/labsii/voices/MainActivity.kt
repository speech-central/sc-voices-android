/*
 * Adreno System TTS integration.
 * SPDX-License-Identifier: GPL-3.0-or-later
 * See NOTICE for upstream copyright and attribution information.
 */
package com.labsii.voices

import android.app.Activity
import android.content.Intent
import android.graphics.Typeface
import android.net.Uri
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import com.google.android.material.button.MaterialButton
import com.google.android.material.card.MaterialCardView
import com.google.android.material.progressindicator.LinearProgressIndicator
import com.labsii.voices.runner.KokoroDeviceProbe
import com.labsii.voices.runner.KokoroModelManager
import com.labsii.voices.runner.KokoroSelfTest
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.TimeoutCancellationException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.launch
import java.util.Locale
import androidx.core.content.edit

/** First-run Kokoro installer and entry point to the installed system TTS engine. */
class MainActivity : Activity() {
    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Main.immediate)
    private lateinit var manager: KokoroModelManager

    private lateinit var downloadScreen: LinearLayout
    private lateinit var experienceScreen: LinearLayout
    private lateinit var readyScreen: LinearLayout
    private lateinit var downloadTitleView: TextView
    private lateinit var downloadDescriptionView: TextView
    private lateinit var statusView: TextView
    private lateinit var progressPercentView: TextView
    private lateinit var progressBar: LinearProgressIndicator
    private lateinit var retryButton: MaterialButton
    private lateinit var experienceTitleView: TextView
    private lateinit var experienceRows: LinearLayout
    private lateinit var experienceDetailView: TextView

    private var detectedRenderer: String? = null
    private val preferences by lazy { getSharedPreferences("onboarding", MODE_PRIVATE) }
    private var onboardingComplete = false
    private var selfTestPassed = false
    private var needsSelfTest = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        onboardingComplete = preferences.getBoolean("complete", false)
        manager = KokoroModelManager.get(applicationContext)
        setContentView(buildUi())
        scope.launch {
            val probe = KokoroDeviceProbe.probe(applicationContext)
            if (!probe.compatible && probe.conclusive) {
                renderUnsupportedDevice(probe)
                return@launch
            }
            detectedRenderer = probe.renderer
            needsSelfTest = !probe.compatible || probe.renderer?.contains("Adreno", ignoreCase = true) != true
            selfTestPassed = preferences.getString("tested_generation", null) == testGeneration()
            if (needsSelfTest) {
                downloadDescriptionView.text = "This OpenCL FP16 device is experimental. We will download the model and try real synthesis; compatibility and speed are not guaranteed."
            }
            launch { manager.state.collect { renderState(it) } }
            manager.ensureInstalled()
        }
    }

    override fun onDestroy() {
        scope.cancel()
        super.onDestroy()
    }

    private fun buildUi(): View {
        val content = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(20), dp(28), dp(20), dp(24))
        }

        content.addView(TextView(this).apply {
            text = getString(R.string.product_name)
            textSize = 32f
            setTypeface(typeface, Typeface.BOLD)
        })
        content.addView(TextView(this).apply {
            text = "Private, on-device text-to-speech powered by Kokoro and OpenCL."
            textSize = 16f
            setPadding(0, dp(8), 0, dp(24))
        })

        downloadScreen = buildDownloadScreen()
        experienceScreen = buildExperienceScreen()
        readyScreen = buildReadyScreen()
        content.addView(downloadScreen, matchParams())
        content.addView(experienceScreen, matchParams())
        content.addView(readyScreen, matchParams())
        showScreen(downloadScreen)

        return ScrollView(this).apply {
            isFillViewport = true
            addView(content)
        }
    }

    private fun buildDownloadScreen(): LinearLayout = screen().apply {
        addView(card(matchParams()) { cardContent ->
            downloadTitleView = sectionTitle("Checking compatibility")
            cardContent.addView(downloadTitleView)
            downloadDescriptionView = bodyText(
                "Kokoro must be downloaded from the internet once before it can speak. " +
                    "The model stays in this app's private storage and all synthesis remains on device.",
            )
            cardContent.addView(downloadDescriptionView)
            progressBar = LinearProgressIndicator(this@MainActivity).apply {
                max = 100
                progress = 0
                isIndeterminate = true
            }
            cardContent.addView(progressBar, matchParams(top = 20))
            val progressTextRow = LinearLayout(this@MainActivity).apply {
                orientation = LinearLayout.HORIZONTAL
                gravity = Gravity.TOP
                setPadding(0, dp(14), 0, 0)
            }
            statusView = bodyText("Checking OpenCL FP16 support…").apply {
                setPadding(0, 0, dp(12), 0)
            }
            progressTextRow.addView(statusView, weightedParams())
            progressPercentView = bodyText("").apply {
                setPadding(0, 0, 0, 0)
                gravity = Gravity.END
                maxLines = 1
                setTypeface(typeface, Typeface.BOLD)
            }
            progressTextRow.addView(
                progressPercentView,
                LinearLayout.LayoutParams(dp(56), LinearLayout.LayoutParams.WRAP_CONTENT),
            )
            cardContent.addView(progressTextRow, matchParams())
            retryButton = outlinedButton("Retry download").apply {
                visibility = View.GONE
                setOnClickListener { manager.retry() }
            }
            cardContent.addView(retryButton, matchParams(top = 20))
        })
    }

    private fun buildExperienceScreen(): LinearLayout = screen().apply {
        addView(card(matchParams()) { cardContent ->
            experienceTitleView = sectionTitle("Checking this device's expected experience…")
            cardContent.addView(experienceTitleView)
            cardContent.addView(ratingHeader(), matchParams(top = 16))
            experienceRows = LinearLayout(this@MainActivity).apply { orientation = LinearLayout.VERTICAL }
            cardContent.addView(experienceRows, matchParams())
            experienceDetailView = bodyText("").apply { visibility = View.GONE }
            cardContent.addView(experienceDetailView, matchParams(top = 12))
        })

        addView(speechCentralCard(), matchParams(top = 16))
        addView(primaryButton("Continue").apply {
            setOnClickListener {
                onboardingComplete = true
                preferences.edit { putBoolean("complete", true) }
                showScreen(readyScreen)
            }
        }, matchParams(top = 24))
    }

    private fun buildReadyScreen(): LinearLayout = screen().apply {
        addView(card(matchParams()) { cardContent ->
            cardContent.addView(sectionTitle("Kokoro is ready"))
            cardContent.addView(bodyText(
                "All 28 English voices are installed. SC Kokoro is ready to use as an Android text-to-speech engine.",
            ))
        })
        addView(speechCentralCard(), matchParams(top = 16))
        addView(outlinedButton("Credits and licenses").apply {
            setOnClickListener { startActivity(Intent(this@MainActivity, CreditsActivity::class.java)) }
        }, matchParams(top = 24))
    }

    private fun speechCentralCard(): MaterialCardView = card { cardContent ->
        cardContent.addView(sectionTitle("A better reading experience"))
        cardContent.addView(bodyText(
            "For the best reading experience, we recommend Speech Central. Apps with less advanced buffering may significantly downgrade your experience.",
        ))
        cardContent.addView(primaryButton("Get Speech Central").apply {
            setOnClickListener { openSpeechCentral() }
        }, matchParams(top = 20))
    }

    private fun renderDeviceExperience(renderer: String?) {
        val profile = when {
            renderer?.contains("Adreno", ignoreCase = true) == true && renderer.contains("619") ->
                DeviceExperience("Expected experience for $renderer", listOf(
                    Rating("Linear reading", 6.5),
                    Rating("Non-linear reading", 4.5),
                    Rating("Instant accessibility feedback", 1.5),
                ))
            renderer?.contains("Adreno", ignoreCase = true) == true && renderer.contains("620") ->
                DeviceExperience("Expected experience for $renderer", listOf(
                    Rating("Linear reading", 9.0),
                    Rating("Non-linear reading", 6.5),
                    Rating("Instant accessibility feedback", 2.5),
                ))
            renderer != null && listOf(" 730", " 740", " 750", " 830").any(renderer::contains) ->
                DeviceExperience("Expected experience for $renderer", listOf(
                    Rating("Linear reading", 10.0),
                    Rating("Non-linear reading", 7.5),
                    Rating("Instant accessibility feedback", 3.5),
                ), "10/10 for linear reading requires verified synthesis at 3× real time or faster.")
            renderer != null -> DeviceExperience(
                "Expected experience for $renderer",
                emptyList(),
                "This device has not yet been rated.",
            )
            else -> DeviceExperience(
                "Expected experience unavailable",
                emptyList(),
                "The OpenCL runtime must be ready before this device can be rated.",
            )
        }

        experienceTitleView.text = profile.title
        experienceRows.removeAllViews()
        profile.ratings.forEach { experienceRows.addView(ratingRow(it), matchParams(top = 12)) }
        experienceDetailView.text = profile.detail
        experienceDetailView.visibility = if (profile.detail.isBlank()) View.GONE else View.VISIBLE
    }

    private fun ratingHeader(): LinearLayout = LinearLayout(this).apply {
        orientation = LinearLayout.HORIZONTAL
        addView(TextView(this@MainActivity).apply {
            text = "USE CASE"
            textSize = 12f
            setTypeface(typeface, Typeface.BOLD)
        }, weightedParams())
        addView(TextView(this@MainActivity).apply {
            text = "RATING"
            textSize = 12f
            gravity = Gravity.START
            setTypeface(typeface, Typeface.BOLD)
        }, weightedParams())
    }

    private fun ratingRow(rating: Rating): LinearLayout = LinearLayout(this).apply {
        orientation = LinearLayout.HORIZONTAL
        gravity = Gravity.CENTER_VERTICAL
        addView(TextView(this@MainActivity).apply {
            text = rating.label
            textSize = 15f
            gravity = Gravity.CENTER_VERTICAL
            setPadding(0, 0, dp(12), 0)
        }, weightedParams())
        addView(LinearLayout(this@MainActivity).apply {
            orientation = LinearLayout.VERTICAL
            addView(TextView(this@MainActivity).apply {
                text = "${rating.value.toDisplay()}/10"
                textSize = 16f
                setTypeface(typeface, Typeface.BOLD)
            })
            addView(LinearProgressIndicator(this@MainActivity).apply {
                max = 100
                progress = (rating.value * 10).toInt()
            }, matchParams(top = 6))
        }, weightedParams())
    }

    private suspend fun renderState(state: KokoroModelManager.State) {
        when (state) {
            KokoroModelManager.State.NotInstalled -> {
                showScreen(downloadScreen)
                downloadTitleView.text = "Download Kokoro"
                progressBar.visibility = View.VISIBLE
                progressBar.isIndeterminate = true
                retryButton.visibility = View.GONE
                statusView.text = "Preparing the secure download…"
                progressPercentView.text = ""
            }
            is KokoroModelManager.State.Downloading -> {
                showScreen(downloadScreen)
                downloadTitleView.text = "Download Kokoro"
                progressBar.visibility = View.VISIBLE
                progressBar.isIndeterminate = false
                progressBar.progress = state.progress
                retryButton.visibility = View.GONE
                statusView.text = "${state.currentFile}\n" +
                    "${formatBytes(state.bytesDone)} of ${formatBytes(state.bytesTotal)}"
                progressPercentView.text = "${state.progress}%"
            }
            KokoroModelManager.State.Ready -> {
                if (!selfTestPassed && needsSelfTest) {
                    showScreen(downloadScreen)
                    downloadTitleView.text = "Testing this OpenCL device"
                    statusView.text = "Running a short, silent synthesis test. The first run may take a little longer."
                    progressPercentView.text = ""
                    progressBar.visibility = View.VISIBLE
                    progressBar.isIndeterminate = true
                    retryButton.visibility = View.GONE
                    try {
                        KokoroSelfTest.run(applicationContext)
                        selfTestPassed = true
                        preferences.edit().putString("tested_generation", testGeneration()).apply()
                    } catch (failure: Exception) {
                        if (failure is CancellationException && failure !is TimeoutCancellationException) throw failure
                        downloadTitleView.text = "Device test did not complete"
                        statusView.text = "Models are installed. ${failure.message}. You may retry; experimental hardware is not blocked from trying synthesis."
                        progressBar.visibility = View.GONE
                        retryButton.text = "Retry device test"
                        retryButton.visibility = View.VISIBLE
                        retryButton.setOnClickListener { scope.launch { renderState(KokoroModelManager.State.Ready) } }
                        return
                    }
                }
                showScreen(if (onboardingComplete) readyScreen else experienceScreen)
                renderDeviceExperience(detectedRenderer)
            }
            is KokoroModelManager.State.Failed -> {
                showScreen(downloadScreen)
                progressBar.isIndeterminate = false
                progressBar.visibility = View.GONE
                retryButton.visibility = View.VISIBLE
                statusView.text = "Kokoro setup failed: ${state.message}"
                progressPercentView.text = ""
            }
        }
    }

    private fun renderUnsupportedDevice(probe: KokoroDeviceProbe.Result) {
        showScreen(downloadScreen)
        downloadTitleView.text = "Unsupported device"
        downloadDescriptionView.text =
            "SC Kokoro requires a compatible OpenCL device with working fp16 support. " +
                "The model will not be downloaded on this device."
        progressBar.isIndeterminate = false
        progressBar.visibility = View.GONE
        progressPercentView.text = ""
        retryButton.visibility = View.GONE
        statusView.text = listOfNotNull(
            probe.renderer?.let { "Detected OpenCL device: $it" },
            probe.reason,
        ).joinToString("\n").ifBlank { "No compatible OpenCL device was detected." }
    }

    private fun showScreen(screen: View) {
        downloadScreen.visibility = if (screen === downloadScreen) View.VISIBLE else View.GONE
        experienceScreen.visibility = if (screen === experienceScreen) View.VISIBLE else View.GONE
        readyScreen.visibility = if (screen === readyScreen) View.VISIBLE else View.GONE
    }

    private fun testGeneration(): String =
        "0.6.9:${packageManager.getPackageInfo(packageName, 0).lastUpdateTime}:$detectedRenderer"

    private fun openSpeechCentral() {
        startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(SPEECH_CENTRAL_URL)))
    }

    private fun screen(): LinearLayout = LinearLayout(this).apply {
        orientation = LinearLayout.VERTICAL
    }

    private fun card(
        params: LinearLayout.LayoutParams? = null,
        content: (LinearLayout) -> Unit,
    ): MaterialCardView = MaterialCardView(this).apply {
        val cardContent = LinearLayout(this@MainActivity).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(20), dp(20), dp(20), dp(20))
        }
        content(cardContent)
        addView(cardContent)
        if (params != null) layoutParams = params
    }

    private fun sectionTitle(text: String): TextView = TextView(this).apply {
        this.text = text
        textSize = 21f
        setTypeface(typeface, Typeface.BOLD)
    }

    private fun bodyText(text: String): TextView = TextView(this).apply {
        this.text = text
        textSize = 15f
        setPadding(0, dp(8), 0, 0)
    }

    private fun primaryButton(text: String): MaterialButton = MaterialButton(this).apply {
        this.text = text
    }

    private fun outlinedButton(text: String): MaterialButton = MaterialButton(
        this,
        null,
        com.google.android.material.R.attr.materialButtonOutlinedStyle,
    ).apply {
        this.text = text
    }

    private fun matchParams(top: Int = 0): LinearLayout.LayoutParams =
        LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT).apply {
            topMargin = dp(top)
        }

    private fun weightedParams(): LinearLayout.LayoutParams =
        LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)

    private fun formatBytes(bytes: Long): String =
        String.format(Locale.US, "%.1f MiB", bytes / (1024.0 * 1024.0))

    private fun dp(value: Int): Int = (value * resources.displayMetrics.density).toInt()

    private data class Rating(val label: String, val value: Double)
    private data class DeviceExperience(
        val title: String,
        val ratings: List<Rating>,
        val detail: String = "",
    )

    private fun Double.toDisplay(): String =
        if (this % 1.0 == 0.0) toInt().toString() else toString()

    private companion object {
        const val SPEECH_CENTRAL_URL =
            "https://play.google.com/store/apps/details?id=com.labsiisoftware.speechcentral"
    }
}
