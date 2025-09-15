#pragma once
#include <Arduino.h>

// Initialize I2C 4-digit AlphaNum4 display
void display_begin(uint8_t i2c_addr = 0x70, uint8_t brightness = 8);

// Ownership and arbitration
// priority: 0..255, higher can preempt lower. Equal priority cannot preempt.
// hold_ms: 0 means no auto-expire. If >0, ownership auto-releases after that window unless renewed.
bool display_acquire(const char* owner, uint8_t priority, uint32_t hold_ms = 0);
bool display_renew(const char* owner, uint32_t hold_ms);
void display_release(const char* owner);
bool display_is_owner(const char* owner);
bool display_is_free();

// Owned operations. These only act if 'owner' currently owns the display.
void display_print4_owned(const char* owner, const char* s4);
bool display_set_brightness_owned(const char* owner, uint8_t level); // 0..15

// Convenience: write idle text only if the display is free
void display_idle(const char* s4);

// Optional direct print without ownership check (used internally)
void display_print4_unchecked(const char* s4);