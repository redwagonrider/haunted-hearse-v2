#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_standby() {
  console_log("Scene: Standby");
  effects_showStandby();
}