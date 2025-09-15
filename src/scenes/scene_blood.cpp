// src/scenes/scene_blood.cpp
//
// Blood Room display animation with arbitration:
// DRIP drips in -> flashes -> fades -> drips out.
// Takes ownership "BLOOD" with priority 8 so it can preempt idle OBEY
// but will not preempt Frankenphone during HOLD.
//
// Call scene_blood() once on Beam 2 trip, then call scene_blood_tick() each loop.

#include <Arduino.h>
#include "display.hpp"
#include "triggers.hpp"
#include "console.hpp"
#include "pins.hpp"

static const char* OWNER = "BLOOD";
static const uint8_t OWNER_PRIO = 8; // below FRANK 10, above idle

#ifndef LED_HOLD
#define LED_HOLD 11
#endif

enum Stage { BD_IDLE=0, BD_IN, BD_FLASH, BD_FADE, BD_OUT, BD_DONE };
static Stage s_stage = BD_IDLE;
static bool  s_active = false;
static uint32_t s_next = 0;
static uint8_t s_in_idx = 0;
static bool  s_in_dot = true;
static uint8_t s_flash = 0;
static int8_t s_fade = 12;
static int8_t s_out_idx = 3;
static bool  s_out_dot = true;

static const char WORD[5] = "DRIP";
static const uint16_t T_IN_DOT   = 120;
static const uint16_t T_IN_LET   = 140;
static const uint16_t T_FLASH_ON = 120;
static const uint16_t T_FLASH_OFF= 120;
static const uint8_t  N_FLASH    = 6;
static const uint16_t T_FADE     = 120;
static const int8_t   FADE_MIN   = 3;
static const uint16_t T_OUT_DOT  = 100;
static const uint16_t T_OUT_BLK  = 120;

static inline void red_blink_soft(uint32_t now_ms) {
  uint16_t p = now_ms % 600;
  analogWrite(LED_HOLD, p < 120 ? 150 : 0);
}

static void show4(const char* s4){ display_print4_owned(OWNER, s4); }
static void showDotAt(uint8_t idx, const char* base) {
  char b[5] = {' ',' ',' ',' ','\0'};
  for (uint8_t i=0;i<4;i++) b[i] = base && base[i] ? base[i] : ' ';
  b[idx] = '.';
  show4(b);
}

void scene_blood() {
  if (s_active) return;
  // Acquire for a few seconds to survive idle writers
  display_acquire(OWNER, OWNER_PRIO, 3000);
  display_set_brightness_owned(OWNER, 10);
  show4("    ");

  // initial state
  s_stage = BD_IN;
  s_active= true;
  s_next  = millis() + T_IN_DOT;
  s_in_idx= 0; s_in_dot = true;
  s_flash = 0;
  s_fade  = 12;
  s_out_idx = 3; s_out_dot = true;

  // Pulse Pi BLOOD cue
  triggers_pulse_by_name(String("BLOOD"));

  console_log("Blood: DRIP animation start");
}

void scene_blood_tick() {
  if (!s_active) return;
  uint32_t now = millis();
  red_blink_soft(now);

  switch (s_stage) {
    case BD_IN:
      if (now >= s_next) {
        char base[5] = {' ',' ',' ',' ','\0'};
        for (uint8_t i=0;i<s_in_idx;i++) base[i] = WORD[i];
        if (s_in_dot) {
          showDotAt(s_in_idx, base);
          s_in_dot = false; s_next = now + T_IN_LET;
        } else {
          base[s_in_idx] = WORD[s_in_idx];
          show4(base);
          s_in_dot = true; s_in_idx++;
          s_next = now + T_IN_DOT;
          if (s_in_idx >= 4) { s_stage = BD_FLASH; s_next = now + T_FLASH_ON; show4(WORD); }
        }
      }
      break;

    case BD_FLASH:
      if (now >= s_next) {
        if ((s_flash & 1) == 0) { show4("    "); s_next = now + T_FLASH_OFF; }
        else                    { show4(WORD);   s_next = now + T_FLASH_ON;  }
        s_flash++;
        if (s_flash >= N_FLASH) { s_stage = BD_FADE; s_next = now + T_FADE; display_set_brightness_owned(OWNER, s_fade); show4(WORD); }
      }
      break;

    case BD_FADE:
      if (now >= s_next) {
        if (s_fade > FADE_MIN) {
          s_fade--;
          display_set_brightness_owned(OWNER, s_fade);
          s_next = now + T_FADE;
        } else {
          s_stage = BD_OUT; s_next = now + T_OUT_DOT;
        }
      }
      break;

    case BD_OUT:
      if (now >= s_next) {
        char base[5] = {' ',' ',' ',' ','\0'};
        for (int i=0;i<s_out_idx;i++) base[i] = WORD[i];
        if (s_out_dot) {
          showDotAt(s_out_idx, base);
          s_out_dot = false; s_next = now + T_OUT_BLK;
        } else {
          base[s_out_idx] = ' ';
          show4(base);
          s_out_dot = true; s_out_idx--;
          s_next = now + T_OUT_DOT;
          if (s_out_idx < 0) { s_stage = BD_DONE; s_next = now + 80; show4("    "); }
        }
      }
      break;

    case BD_DONE:
    default:
      analogWrite(LED_HOLD, 0);
      s_active = false;
      display_release(OWNER);
      console_log("Blood: DRIP animation end");
      break;
  }

  // Keep ownership fresh during animation
  display_renew(OWNER, 400);
}