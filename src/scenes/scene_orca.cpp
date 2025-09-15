#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_orca() {
  console_log("Scene: Orca Whale");
  effects_orcaSplash();
}