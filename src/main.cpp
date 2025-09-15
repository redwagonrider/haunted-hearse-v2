#include <Arduino.h>
#include "display.hpp"
#include "effects.hpp"
#include "console.hpp"
#include "inputs.hpp"
#include "settings.hpp"
#include "pins.hpp"
#include "mapping.hpp"
#include "scene_common.hpp"
#include "scenes/scene_frankenphone.hpp"
#include "telemetry.hpp"

void setup() {
  // Make sure USB serial is up before anything tries to print
  Serial.begin(115200);
  delay(50);
  Serial.println("== Haunted Hearse v2 boot ==");

  pinMode(PIN_MAGNET_CTRL, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(LED_ARMED, OUTPUT);
  pinMode(LED_HOLD, OUTPUT);
  pinMode(LED_COOLDOWN, OUTPUT);

  effects_begin();
  display_begin();
  settings_init();
  inputs_init();

  // Optional, when you wire settings to mapping:
  // apply_mapping_from_settings();

  frankenphone_init();   // starts in IDLE, waits for beam break

  Serial.println("Setup complete");
}

void loop() {
  frankenphone_update();
  // If you add background services, call them here
  // triggers_update();
  // small breather
  delay(1);
}