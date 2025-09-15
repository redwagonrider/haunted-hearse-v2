#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_graveyard() {
  console_log("Scene: Graveyard");
  effects_mistyGraveyard();
}