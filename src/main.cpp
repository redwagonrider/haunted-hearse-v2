// src/main.cpp
#include <Arduino.h>

#include "display.hpp"             // display_begin(uint8_t addr=0x70, uint8_t brightness=8)
#include "effects.hpp"             // effects_begin()
#include "console.hpp"             // console_update(), console_log()
#include "scenes.hpp"
#include "inputs.hpp"
#include "settings.hpp"
#include "pins.hpp"
#include "mapping.hpp"
#include "scene_common.hpp"
#include "scenes/scene_frankenphone.hpp"
#include "telemetry.hpp"

// These are DEFINED in mapping.cpp. Only declare them here.
extern uint8_t  BEAM_PINS[6];
extern SceneFn  BEAM_SCENE[6];

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait for native USB if present */ }

  console_log("Haunted Hearse Booting...");
  console_log("Pins: beam D2, magnet D6, buzzer D8, LEDs D10 D11 D12, I2C 0x70");

  effects_begin();
  display_begin(0x70, 8);
  settings_init();
  inputs_init();
  // apply_mapping_from_settings(); // leave off unless you use beam→scene mapping

  frankenphone_init();

  console_log("Setup complete. Type '?' or 'CFG' then Enter.");
}

void loop() {
  console_update();        // handle CFG, STATE 16, etc.
  frankenphone_update();   // run Frankenphones Lab FSM
  delay(1);
}