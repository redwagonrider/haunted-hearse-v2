#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_secret() {
  console_log("Scene: Secret Room");
  effects_secretReveal();
}