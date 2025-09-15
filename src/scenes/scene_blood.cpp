#include <Arduino.h>          // for millis()
#include "scene_blood.hpp"
#include "console.hpp"        // for console_log()
#include "effects.hpp"        // for effects_* helpers
#include "pins.hpp"           // for pin constants

void scene_blood() {
  console_log("Scene: Blood");

  effects_holdPulseRed(200);   // red pulse for 200ms
  delay(200);
}