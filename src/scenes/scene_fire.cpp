#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_fire() {
  console_log("Scene: Fire Room");
  effects_fireFlicker();
}