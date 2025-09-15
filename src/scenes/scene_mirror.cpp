#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_mirror() {
  console_log("Scene: Mirror Room");
  effects_mirrorFlash();
}