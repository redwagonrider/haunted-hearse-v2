#pragma once
#include <Arduino.h>

// Initialize effects module with pin map and current HOLD duration
void effects_begin(uint8_t pinLedArmed,
                   uint8_t pinLedHold,
                   uint8_t pinLedCooldown,
                   uint8_t pinBuzzer,
                   uint8_t pinMagnet,
                   unsigned long holdMs);

// If you adjust the hold length later
void effects_setHoldMs(unsigned long holdMs);

// Magnet helpers
void effects_magnet_on();
void effects_magnet_off();

// Sound helpers
void effects_sound_stop();            // stop any buzzer tone

// Per-state visual/audio updates (call each loop)
void effects_idle_update();           // green breathe
void effects_hold_update(unsigned long elapsedMs);   // red stutter + modem sound
void effects_cooldown_update();       // yellow flicker