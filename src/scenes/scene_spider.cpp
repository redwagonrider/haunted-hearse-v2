#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_spider() {
  console_log("Scene: Spider Lair");
  effects_spiderWebFlash();
}