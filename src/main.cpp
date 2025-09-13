#include <Arduino.h>
#include "settings.hpp"
#include "console.hpp"
#include "effects.hpp"
#include "display.hpp"
#include "scenes/scenes.hpp"
#include "scenes/scene_frankenphone.hpp"

// =======================
// Pin assignments
// =======================
const uint8_t PIN_MAGNET_CTRL = 6;
const uint8_t PIN_BUZZER      = 8;
const uint8_t LED_ARMED       = 10;
const uint8_t LED_HOLD        = 11;
const uint8_t LED_COOLDOWN    = 12;

// =======================
// Break-beam config (6 beams)
// =======================
const uint8_t NUM_BEAMS = 6;

uint8_t BEAM_PINS[NUM_BEAMS] = {
  2, 3, 4, 5, 7, 9   // adjust as needed
};

Scene BEAM_SCENE[NUM_BEAMS] = {
  scene_blackout,     // Beam 0
  scene_spider,       // Beam 1
  scene_graveyard,    // Beam 2
  scene_orca,         // Beam 3
  scene_frankenphone, // Beam 4 — our new FSM scene
  scene_fire          // Beam 5
};

bool beamLastState[NUM_BEAMS] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

// =======================
// Setup
// =======================
void setup() {
  Serial.begin(115200);
  console_log("Haunted Hearse: System booting");

  // Set break-beam inputs
  for (uint8_t i = 0; i < NUM_BEAMS; i++) {
    pinMode(BEAM_PINS[i], INPUT_PULLUP);
  }

  // Init all systems
  effects_init();         // Lighting effects (if used)
  display_init();         // Optional onboard display
  settings_init();        // Holds/cooldowns config
  frankenphone_init();    // Setup FSM for Frankenphone scene
}

// =======================
// Main Loop
// =======================
void loop() {
  // Break-beam check
  for (uint8_t i = 0; i < NUM_BEAMS; i++) {
    bool nowState = digitalRead(BEAM_PINS[i]);
    if (beamLastState[i] == HIGH && nowState == LOW) {  // beam broken
      console_log("Break-beam triggered: zone " + String(i));
      if (BEAM_SCENE[i]) BEAM_SCENE[i]();
    }
    beamLastState[i] = nowState;
  }

  // Update ongoing systems
  effects_update();       // Lighting
  display_update();       // Optional global display
  frankenphone_update();  // FSM stepper

  delay(2); // Keep loop smooth
}