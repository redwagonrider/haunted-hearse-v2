#pragma once
#include <Arduino.h>

// Initialize up to 6 break-beam inputs (use 0xFF for unused slots)
void inputs_begin(const uint8_t pins[6], uint16_t debounceMs = 30, uint32_t rearmMs = 2000);

// Call every loop to update internal state
void inputs_update();

// Returns true once per break event (LOW edge) then auto-clears
bool inputs_triggered(uint8_t idx);    // idx = 0..5

// Live state helpers (no latching)
bool inputs_isBroken(uint8_t idx);     // true while beam is broken (LOW)
bool inputs_isStable(uint8_t idx);     // debounced “stable” reading

// Tuning at runtime (optional)
void inputs_setDebounce(uint16_t ms);
void inputs_setRearm(uint32_t ms);

// For console / debugging
void inputs_printStatus(Stream& s);