#pragma once
#include <Arduino.h>

// If your project already defines this type in scenes.hpp, keeping a duplicate typedef is harmless.
typedef void (*SceneFn)();

// Global beam maps that every module can see
extern uint8_t BEAM_PINS[6];
extern SceneFn BEAM_SCENE[6];

// Apply mapping from settings if you have a settings store.
// Safe to call even if you do not use settings yet.
void apply_mapping_from_settings();