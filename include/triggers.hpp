#pragma once
#include <Arduino.h>

// Initialize Mega -> Pi GPIO trigger outputs (optocouplers or relays)
void triggers_begin();

// Pulse by index. Returns true on success.
bool triggers_pulse(uint8_t idx, uint16_t ms = 100);

// Pulse by uppercase name: SHOW, BLOOD, GRAVE, FUR, FRANKEN
bool triggers_pulse_by_name(const String& upname);

// Print mapping to Serial
void triggers_print_map();