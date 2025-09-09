#pragma once
#include <Arduino.h>

// Initialize display (I2C addr and brightness 0..15)
void display_begin(uint8_t i2c_addr = 0x70, uint8_t brightness = 8);
void display_set_brightness(uint8_t b0_15);

// Immediate helpers
void display_show4(const char* s4);   // exactly 4 chars
void display_clear();
void display_idle(const char* four);  // e.g. "OBEY"

// Hold (trigger) sequence
void display_hold_init();    // start "SYSTEM OVERRIDE" sequence
void display_hold_update();  // call every loop tick (non-blocking)

// Cooldown sequence (ACCESS GRANTED variants)
void display_cooldown_init();
void display_cooldown_update();