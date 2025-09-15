#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_exit() {
  console_log("Scene: Exit or Die");
  effects_exitStrobe();
}