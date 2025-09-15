// src/inputs.cpp
#include <Arduino.h>
#include "pins.hpp"
#include "console.hpp"
#include "techlight.hpp"
#include "inputs.hpp"
#include "triggers.hpp"
#include "display.hpp"   // for any idle writers you already use

// Scene entry points
#include "scenes/scene_frankenphone.hpp"
void scene_intro();
void scene_blackout();
void scene_graveyard();
void scene_mirror();
void scene_exit();

// Blood scene API: entry + per-loop tick
void scene_blood();
void scene_blood_tick();

static const uint8_t N_SCENE_BEAMS = 6; // beams 0..5 launch scenes

// Map: which Arduino pins are beams 0..6
static const uint8_t BEAM_PINS[7] = {
  PIN_BEAM_0, PIN_BEAM_1, PIN_BEAM_2, PIN_BEAM_3, PIN_BEAM_4, PIN_BEAM_5, PIN_BEAM_6
};
static const char*   BEAM_NAMES[7] = { "B0","B1","B2","B3","B4","B5","B6" };

// Debounce and rearm timing
static const unsigned long DEBOUNCE_MS = 30;
static const unsigned long REARM_MS    = 20000; // 20 s

// Per-beam state machine (for beams 0..5)
static uint8_t stable_state[6];     // 0 clear, 1 broken
static uint8_t last_raw[6];
static unsigned long t_change[6];
static unsigned long t_last_fire[6];

// Beam 6 (reed) raw tracking
static uint8_t  reed_last_raw = 1;  // 1 = open due to pullup
static uint8_t  reed_stable   = 1;
static unsigned long reed_t_change = 0;

// Tech light control state
// -1 = AUTO (follow reed), 0 = FORCE_OFF, 1 = FORCE_ON
static int8_t  gLightOverride     = -1;
static bool    gLightIsOn         = false;
// Intro behavior: minimum blackout window, then latch OFF until reed closes or override ON
static bool    gLightIntroLatch   = false;
static unsigned long gLightBlockUntil = 0;

static inline uint8_t raw_active_low(uint8_t pin) { return digitalRead(pin) == LOW ? 1 : 0; }

static inline void techlight_write_hw(bool on) {
#if TECHLIGHT_ACTIVE_HIGH
  digitalWrite(PIN_TECHLIGHT, on ? HIGH : LOW);
#else
  digitalWrite(PIN_TECHLIGHT, on ? LOW : HIGH);
#endif
  gLightIsOn = on;
}

// ====== Techlight API (implementation) ======
void techlight_override_on()  { gLightOverride = 1;  gLightIntroLatch = false; techlight_write_hw(true);  console_log("TechLight OVERRIDE ON"); }
void techlight_override_off() { gLightOverride = 0;  techlight_write_hw(false); console_log("TechLight OVERRIDE OFF"); }
void techlight_override_auto(){ gLightOverride = -1; console_log("TechLight AUTO (follow reed)"); }

// Header declares default 5000 ms. Definition here enforces OFF now,
// minimum blackout window, then latch until reed closes in AUTO or override ON.
void techlight_scene_intro_kill(uint16_t ms_holdoff) {
  techlight_write_hw(false);
  gLightIntroLatch   = true;
  gLightBlockUntil   = millis() + ms_holdoff;
  console_log("TechLight OFF by Intro/Cue");
}

bool techlight_is_on() { return gLightIsOn; }
const char* techlight_mode_name() {
  switch (gLightOverride) {
    case 1:  return "FORCE_ON";
    case 0:  return "FORCE_OFF";
    default: return "AUTO";
  }
}

// ====== Scene mapper ======
static void scene_for_beam(uint8_t idx) {
  switch (idx) {
    case 0:
      scene_frankenphone();
      break;

    case 1:
      // Fire SHOW cue to Pi to start main show, then kill tech lights
      triggers_pulse_by_name("SHOW");
      scene_intro();
      techlight_scene_intro_kill(); // default 5000 ms min blackout
      break;

    case 2:
      // Start Blood animation. scene_blood_tick() will be called every loop.
      scene_blood();
      break;

    case 3: scene_graveyard(); break;
    case 4: scene_mirror();    break;
    case 5: scene_exit();      break;
    default: break;
  }
}

void inputs_init() {
  // Beams 0..5
  for (uint8_t i = 0; i < N_SCENE_BEAMS; ++i) {
    pinMode(BEAM_PINS[i], INPUT_PULLUP);
    uint8_t r = raw_active_low(BEAM_PINS[i]);
    last_raw[i]   = r;
    stable_state[i]= r;
    t_change[i]   = millis();
    t_last_fire[i]= 0;
  }

  // Beam 6: Reed switch (Adafruit 375). INPUT_PULLUP, active when CLOSED.
  pinMode(PIN_BEAM_6, INPUT_PULLUP);
  reed_last_raw = digitalRead(PIN_BEAM_6); // 0 when closed, 1 when open
  reed_stable   = reed_last_raw;
  reed_t_change = millis();

  // Tech booth light output
  pinMode(PIN_TECHLIGHT, OUTPUT);
  techlight_write_hw(false); // start OFF

  // Triggers to Pi (SHOW, BLOOD, GRAVE, FUR, FRANKEN)
  triggers_begin();

  console_log("Inputs: beams B0..B5 debounced + rearm, B6 reed drives tech light");
  console_log("Map: B0=Franken, B1=Intro+SHOW, B2=Blood, B3=Graveyard, B4=Mirror, B5=Exit, B6=TechLight");
}

void inputs_update() {
  const unsigned long now = millis();

  // Beams 0..5: edge detect break with rearm
  for (uint8_t i = 0; i < N_SCENE_BEAMS; ++i) {
    uint8_t r = raw_active_low(BEAM_PINS[i]);
    if (r != last_raw[i]) {
      last_raw[i] = r;
      t_change[i] = now;
    }
    if (now - t_change[i] >= DEBOUNCE_MS) {
      if (stable_state[i] != r) {
        stable_state[i] = r;
        if (r == 1) { // newly broken
          if (now - t_last_fire[i] >= REARM_MS) {
            t_last_fire[i] = now;
            console_log(String("TRIP ") + BEAM_NAMES[i]);
            scene_for_beam(i);
          }
        }
      }
    }
  }

  // Beam 6: reed switch debounced
  uint8_t reed_raw = digitalRead(PIN_BEAM_6); // 0 = closed, 1 = open (pullup)
  if (reed_raw != reed_last_raw) {
    reed_last_raw = reed_raw;
    reed_t_change = now;
  }
  if (now - reed_t_change >= DEBOUNCE_MS) {
    if (reed_stable != reed_raw) {
      reed_stable = reed_raw;
    }
  }

  // If intro latch is set, it cannot clear before the minimum blackout ends.
  if (gLightIntroLatch && now >= gLightBlockUntil) {
    if (gLightOverride == 1) {
      gLightIntroLatch = false;
    } else if (gLightOverride == -1 && reed_stable == LOW) {
      gLightIntroLatch = false;
    }
  }

  // Final tech light drive with priority rules
  bool want_on = false;
  if (gLightIntroLatch) {
    want_on = false;
  } else if (gLightOverride == 1) {
    want_on = true;
  } else if (gLightOverride == 0) {
    want_on = false;
  } else {
    want_on = (reed_stable == LOW);    // AUTO: reed closed = ON
  }

  if (want_on != gLightIsOn) {
    techlight_write_hw(want_on);
    if (gLightOverride == -1 && !gLightIntroLatch) {
      console_log(want_on ? "TechLight ON (reed)" : "TechLight OFF (reed)");
    } else if (gLightOverride != -1) {
      console_log(want_on ? "TechLight ON (override)" : "TechLight OFF (override)");
    }
  }

  // Tick Blood animation each loop so it progresses after the one-shot trip
  scene_blood_tick();
}

// ====== Mapping printer for console ======
void inputs_print_map() {
  Serial.println(F("=== Beam -> Scene Map ==="));
  Serial.println(F("B0 D2  -> Frankenphones Lab"));
  Serial.println(F("B1 D3  -> Intro / Cue Card  + Pi SHOW trigger  + TechLight kill"));
  Serial.println(F("B2 D4  -> Blood Room (DRIP in/flash/fade/out animation)"));
  Serial.println(F("B3 D5  -> Graveyard"));
  Serial.println(F("B4 D7  -> Mirror Room"));
  Serial.println(F("B5 D9  -> Exit"));
  Serial.println(F("B6 D30 -> TechLight reed  | Output D26"));
  Serial.println(F("Debounce 30 ms, Re-arm 20 s for B0..B5"));
}