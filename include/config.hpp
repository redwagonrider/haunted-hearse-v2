#pragma once
#include <Arduino.h>

constexpr uint32_t INTRO_DURATION_MS = 10000;
constexpr uint32_t BLOOD_DURATION_MS = 15000;
constexpr uint32_t FRANKENPHONE_DURATION_MS = 28000;
constexpr uint32_t FRANKENPHONE_HOLD_MS = 8000;
constexpr uint32_t MIRROR_DURATION_MS = 12000;
constexpr uint32_t EXIT_DURATION_MS = 8000;
constexpr uint32_t BEAM_TIMEOUT_MS = 60000;
constexpr uint32_t FPP_PULSE_MS = 100;
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint8_t DISPLAY_COLS = 16;
constexpr uint8_t DISPLAY_ROWS = 2;
constexpr uint8_t I2C_ADDRESS = 0x27;
constexpr uint16_t EEPROM_MAGIC_NUMBER = 0xBEEF;
constexpr uint16_t EEPROM_START_ADDRESS = 0;
constexpr uint32_t MIN_DURATION_MS = 1000;
constexpr uint32_t MAX_DURATION_MS = 120000;