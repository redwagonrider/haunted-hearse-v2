// triggers.hpp
#pragma once
#include <Arduino.h>

// Four trigger channels from Mega to Pi via optocouplers
// Mega pins 22..25 drive opto LEDs through current-limit resistors.
// Pi side should use pull-ups on GPIO inputs and detect edges in FPP.

// Logical trigger IDs
enum TriggerId : uint8_t {
  TRIG_BLOODROOM = 0,   // Pi GPIO17
  TRIG_GRAVEYARD = 1,   // Pi GPIO27
  TRIG_FURROOM   = 2,   // Pi GPIO22
  TRIG_FRANKENLAB= 3    // Pi GPIO23
};

// Configuration
struct TriggerConfig {
  uint16_t pulse_ms;    // duration of HIGH pulse on Mega pin
  uint16_t lockout_ms;  // minimum time after a fire where channel cannot retrigger
};

// Initialize pins and timing
void triggers_begin(const TriggerConfig& cfg = {100, 300});

// Fire a trigger. Returns true if accepted, false if locked out or already active.
bool triggers_fire(TriggerId id);

// Call often in loop to time out pulses and enforce lockouts
void triggers_update();

// Optional: map helpers
uint8_t triggers_pin_for(TriggerId id);    // Arduino pin number (22..25)
uint8_t triggers_pi_gpio_for(TriggerId id); // Pi GPIO for your docs
const char* triggers_name_for(TriggerId id);