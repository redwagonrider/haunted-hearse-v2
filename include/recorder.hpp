#pragma once
#include <Arduino.h>

// Tascam DR-05 power/record control via a 5V-switched relay/load switch

// Initialize with the Arduino pin that drives the relay's IN pin (e.g., 23).
void recorder_begin(uint8_t pinPower);

// Force power ON/OFF to the recorder (ON -> starts recording if DR-05 is set to auto-record on power).
void recorder_power(bool on);

// Timed record helper: power ON, wait <ms>, power OFF (non-blocking).
void recorder_record_for(uint32_t ms);

// Call from loop() to service the timed OFF.
void recorder_update();