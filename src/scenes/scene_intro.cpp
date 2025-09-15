#include <Arduino.h>
#include "console.hpp"
#include "effects.hpp"
#include "scene_common.hpp"

void scene_intro() {
  console_log("Scene: Intro");
  effects_introFade();
}