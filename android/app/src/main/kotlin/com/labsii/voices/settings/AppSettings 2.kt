/*
 * Adreno System TTS integration.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
package com.labsii.voices.settings

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.SharedPreferencesMigration
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.emptyPreferences
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.map
import java.io.IOException

private const val DATASTORE_NAME = "settings"
private const val LEGACY_ONBOARDING = "onboarding"

private val Context.settingsDataStore: DataStore<Preferences> by preferencesDataStore(
    name = DATASTORE_NAME,
    produceMigrations = { context ->
        listOf(
            SharedPreferencesMigration(
                context = context,
                sharedPreferencesName = LEGACY_ONBOARDING,
                keysToMigrate = setOf("complete", "tested_renderer", "tested_generation"),
            ),
        )
    },
)

internal data class SettingsSnapshot(
    val onboardingComplete: Boolean = false,
    val testedRenderer: String? = null,
    val testedGeneration: String? = null,
    val defaultVoiceId: String = AppSettings.DEFAULT_VOICE_ID,
)

internal class AppSettings(context: Context) {
    private val dataStore = context.applicationContext.settingsDataStore

    val snapshots: Flow<SettingsSnapshot> = dataStore.data
        .catch { failure ->
            if (failure is IOException) emit(emptyPreferences()) else throw failure
        }
        .map(::toSnapshot)

    suspend fun snapshot(): SettingsSnapshot = snapshots.first()

    suspend fun completeOnboarding() {
        dataStore.edit { it[Keys.ONBOARDING_COMPLETE] = true }
    }

    suspend fun saveDeviceTest(generation: String, renderer: String?) {
        dataStore.edit { preferences ->
            preferences[Keys.TESTED_GENERATION] = generation
            if (renderer == null) {
                preferences.remove(Keys.TESTED_RENDERER)
            } else {
                preferences[Keys.TESTED_RENDERER] = renderer
            }
        }
    }

    suspend fun setDefaultVoice(voiceId: String) {
        dataStore.edit { it[Keys.DEFAULT_VOICE_ID] = voiceId }
    }

    private fun toSnapshot(preferences: Preferences): SettingsSnapshot = SettingsSnapshot(
        onboardingComplete = preferences[Keys.ONBOARDING_COMPLETE] ?: false,
        testedRenderer = preferences[Keys.TESTED_RENDERER],
        testedGeneration = preferences[Keys.TESTED_GENERATION],
        defaultVoiceId = preferences[Keys.DEFAULT_VOICE_ID] ?: DEFAULT_VOICE_ID,
    )

    private object Keys {
        val ONBOARDING_COMPLETE = booleanPreferencesKey("complete")
        val TESTED_RENDERER = stringPreferencesKey("tested_renderer")
        val TESTED_GENERATION = stringPreferencesKey("tested_generation")
        val DEFAULT_VOICE_ID = stringPreferencesKey("default_voice_id")
    }

    companion object {
        const val DEFAULT_VOICE_ID = "af_heart"
    }
}
