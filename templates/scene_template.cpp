#include <Arduino.h>
#include "pins.hpp"
#include "display.hpp"
#include "triggers.hpp"
#include "techlight.hpp"
#include "telemetry.hpp"

using HH::tel_emit;

namespace {
enum Phase { S_IDLE = 0, S_START, S_RUN, S_END, S_DONE };
Phase s = S_IDLE;
uint32_t t0 = 0;

// Display control
bool own = false;
const char* OWNER = "NEWS";   // change per scene
const uint8_t PRIORITY = 50;  // choose priority relative to other scenes
}

// Helper examples
static void start_display_claim(uint16_t hold_ms) {
  if (!own && display_acquire(OWNER, PRIORITY, hold_ms)) {
    own = true;
  }
}
static void release_display() {
  if (own) display_release(OWNER);
  own = false;
}

void scene_newscene() {
  uint32_t now = millis();

  switch (s) {
    case S_IDLE:
      t0 = now;
      s = S_START;
      tel_emit("newscene", "START", 0, 0, 0, 0, 0, 0);
      break;

    case S_START:
      start_display_claim(1500);
      if (own) display_print4_owned(OWNER, "HELO");
      s = S_RUN;
      break;

    case S_RUN:
      // example timed action 2 seconds later
      if (now - t0 > 2000) {
        if (own) display_print4_owned(OWNER, "RUN ");
        tel_emit("newscene", "RUN", 0, 0, 0, 0, 32, 0);
        s = S_END;
      }
      break;

    case S_END:
      release_display();
      tel_emit("newscene", "END", 0, 0, 0, 0, 0, 0);
      s = S_DONE;
      break;

    case S_DONE:
      // remain quiescent
      break;
  }
}
