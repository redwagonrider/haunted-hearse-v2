#include "../include/settings.hpp"
#include "../include/config.hpp"
#include <EEPROM.h>

Settings g_settings;

namespace SettingsManager {
    void init() { if (!loadFromEEPROM()) { loadDefaults(); saveToEEPROM(); } }
    void loadDefaults() {
        g_settings.magic = EEPROM_MAGIC_NUMBER;
        g_settings.intro_duration = INTRO_DURATION_MS;
        g_settings.blood_duration = BLOOD_DURATION_MS;
        g_settings.frankenphone_duration = FRANKENPHONE_DURATION_MS;
        g_settings.frankenphone_hold = FRANKENPHONE_HOLD_MS;
        g_settings.mirror_duration = MIRROR_DURATION_MS;
        g_settings.exit_duration = EXIT_DURATION_MS;
        g_settings.beam_timeout = BEAM_TIMEOUT_MS;
    }
    bool loadFromEEPROM() {
        EEPROM.get(EEPROM_START_ADDRESS, g_settings);
        return g_settings.magic == EEPROM_MAGIC_NUMBER;
    }
    void saveToEEPROM() {
        EEPROM.put(EEPROM_START_ADDRESS, g_settings);
    }
    void printSettings() {
        Serial.println(F("Settings loaded"));
    }
    bool setDuration(const char* scene, uint32_t value) {
        if (value < MIN_DURATION_MS || value > MAX_DURATION_MS) return false;
        if (strcmp(scene, "intro") == 0) g_settings.intro_duration = value;
        return true;
    }
    Settings& getSettings() { return g_settings; }
}