#include <Arduino.h>          // for millis()
#include "scene_blackout.hpp"
#include "console.hpp"        // for console_log()
#include "effects.hpp"        // for effects_showBlackout(), etc.
#include "pins.hpp"

void scene_blackout() {
  console_log("Scene: Blackout");
  effects_showBlackout();  // Add this effect in effects.cpp if needed
}