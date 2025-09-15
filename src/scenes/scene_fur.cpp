#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_fur() {
  console_log("Scene: Fur Room");
  effects_furPulse();
}