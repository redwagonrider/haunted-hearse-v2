// src/main.cpp
#include <Arduino.h>

#include "display.hpp"
#include "effects.hpp"
#include "console.hpp"
#include "inputs.hpp"
#include "pins.hpp"
#include "scenes/scene_frankenphone.hpp"

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  console_log("Haunted Hearse Booting...");
  console_log("Pins: Beams D2 D3 D4 D5 D7 D9, Magnet D6, Buzzer D8, LEDs D10 D11 D12, I2C 0x70");

  effects_begin();
  display_begin(0x70, 8);
  inputs_init();
  frankenphone_init();

  console_log("Setup complete. Type '?' for help.");
}

void loop() {
  console_update();     // console commands
  inputs_update();      // beam manager
  frankenphone_update();// scene runtime
  delay(1);
}