#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_spiders() {
  console_log("Scene: Spiders Lair");
  effects_spiderWebFlash();
}