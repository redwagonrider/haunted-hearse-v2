#pragma once
#include <Arduino.h>

// Console override controls
void techlight_override_on();
void techlight_override_off();
void techlight_override_auto();   // follow reed switch again

// One-shot kill used by Intro/Cue scene start
void techlight_scene_intro_kill(uint16_t ms_holdoff = 5000); // default 5 s block

// Introspection for console
bool        techlight_is_on();
const char* techlight_mode_name(); // "AUTO" | "FORCE_ON" | "FORCE_OFF"