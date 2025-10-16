#pragma once
#include <Arduino.h>

struct Settings {
    uint16_t magic;
    uint32_t intro_duration;
    uint32_t blood_duration;
    uint32_t frankenphone_duration;
    uint32_t frankenphone_hold;
    uint32_t mirror_duration;
    uint32_t exit_duration;
    uint32_t beam_timeout;
    uint16_t checksum;
};

namespace SettingsManager {
    void init();
    void loadDefaults();
    bool loadFromEEPROM();
    void saveToEEPROM();
    void printSettings();
    bool setDuration(const char* scene, uint32_t value);
    Settings& getSettings();
}

extern Settings g_settings;