#pragma once
#include <Arduino.h>

// Initialize effects system and bind pins
void effects_begin(uint8_t pinLED_armed,
                   uint8_t pinLED_hold,
                   uint8_t pinLED_cool,
                   uint8_t pinBuzzer,
                   uint8_t pinMagnet,
                   uint32_t hold_ms_default);

// Change hold duration at runtime (used by settings/console)
void effects_setHoldMs(uint32_t ms);

// Standby visuals (green slow pulse). Call every loop while in Standby.
void effects_showStandby();

// Enter FrankenLab hold (turn on magnet, start red stutter + chirps)
// Call once when the scene changes into FrankenLab.
void effects_startHold();

// Update FrankenLab hold animation/audio. Call every loop while in FrankenLab.
void effects_updateHold();

// Exit FrankenLab hold (stop tone, magnet off, red off). Call once on transition out.
void effects_endHold();

// Enter Cooldown (yellow flicker). Call once when going into cooldown.
void effects_startCooldown();

// Update Cooldown flicker. Call every loop while in Cooldown.
void effects_updateCooldown();

// NEW: brief solid-green flash to indicate the system is re-armed (called on entry to Standby)
void effects_rearmCue();