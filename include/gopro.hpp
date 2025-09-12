#pragma once
#include <Arduino.h>

// Initialize the GoPro power control (relay/load-switch IN pin)
void gopro_begin(uint8_t pinPower);

// Manually force power ON/OFF to GoPro USB 5V
void gopro_power(bool on);

// Start recording for ms (uses GoPro Labs: auto-record on USB 5V present)
void gopro_record_for(uint32_t ms);

// Call this frequently (in loop) to turn power off after the duration elapses
void gopro_update();