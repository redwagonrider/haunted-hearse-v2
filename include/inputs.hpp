#pragma once
#include <Arduino.h>
#include "scenes.hpp"

// Initialize inputs with up to 6 beam receiver pins.
// pins[0..5] = Arduino digital pin numbers. Unused beams set to 255.
void inputs_begin(const uint8_t pins[6], uint16_t debounce_ms, uint32_t rearm_ms);

// Periodic update (non-blocking). Call every loop() tick.
void inputs_update();

// True exactly once when the given beam index fired since last update/read.
bool inputs_triggered(uint8_t idx);

// Runtime tuning
void inputs_setDebounce(uint16_t ms);
void inputs_setRearm(uint32_t ms);

// Optional per-beam inversion (false=LOW means broken [default], true=HIGH means broken)
void inputs_setInvert(uint8_t idx, bool inverted);

// Debug dump
void inputs_printStatus(Stream& s);