// src/display.cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include "display.hpp"

static Adafruit_AlphaNum4 g_alpha;
static bool     g_inited = false;

static char     g_owner[16] = "";      // current owner tag
static uint8_t  g_prio      = 0;       // current owner priority
static uint32_t g_hold_until= 0;       // 0 = indefinite
static char     g_last4[5]  = "    ";
static uint8_t  g_bright    = 8;       // 0..15

static inline uint32_t now_ms() { return millis(); }

static void maybe_expire() {
  if (g_owner[0] && g_hold_until && now_ms() > g_hold_until) {
    g_owner[0] = '\0';
    g_prio     = 0;
    g_hold_until = 0;
  }
}

void display_begin(uint8_t i2c_addr, uint8_t brightness) {
  if (!g_inited) {
    Wire.begin();
    g_alpha.begin(i2c_addr);
    g_inited = true;
  }
  g_bright = constrain(brightness, 0, 15);
  g_alpha.setBrightness(g_bright);
  g_alpha.clear();
  g_alpha.writeDisplay();
  strcpy(g_last4, "    ");
  g_owner[0] = '\0';
  g_prio     = 0;
  g_hold_until = 0;
}

bool display_is_free() { maybe_expire(); return g_owner[0] == '\0'; }

bool display_is_owner(const char* owner) {
  maybe_expire();
  return owner && g_owner[0] && strcmp(owner, g_owner) == 0;
}

bool display_acquire(const char* owner, uint8_t priority, uint32_t hold_ms) {
  if (!owner || !owner[0]) return false;
  maybe_expire();
  if (!g_owner[0] || strcmp(owner, g_owner) == 0 || priority > g_prio) {
    strncpy(g_owner, owner, sizeof(g_owner)-1);
    g_owner[sizeof(g_owner)-1] = '\0';
    g_prio = priority;
    g_hold_until = hold_ms ? (now_ms() + hold_ms) : 0;
    return true;
  }
  return false;
}

bool display_renew(const char* owner, uint32_t hold_ms) {
  if (!display_is_owner(owner)) return false;
  g_hold_until = hold_ms ? (now_ms() + hold_ms) : 0;
  return true;
}

void display_release(const char* owner) {
  if (!display_is_owner(owner)) return;
  g_owner[0] = '\0';
  g_prio = 0;
  g_hold_until = 0;
}

static void hw_show4(const char* s4) {
  char buf[5] = {' ',' ',' ',' ','\0'};
  for (uint8_t i=0;i<4;i++) buf[i] = s4 && s4[i] ? s4[i] : ' ';
  if (strncmp(buf, g_last4, 4) == 0) return; // avoid redundant I2C writes
  g_alpha.clear();
  g_alpha.writeDigitAscii(0, buf[0]);
  g_alpha.writeDigitAscii(1, buf[1]);
  g_alpha.writeDigitAscii(2, buf[2]);
  g_alpha.writeDigitAscii(3, buf[3]);
  g_alpha.writeDisplay();
  memcpy(g_last4, buf, 4);
  g_last4[4] = '\0';
}

void display_print4_unchecked(const char* s4) { hw_show4(s4); }

void display_print4_owned(const char* owner, const char* s4) {
  if (!display_is_owner(owner)) return;
  hw_show4(s4);
}

bool display_set_brightness_owned(const char* owner, uint8_t level) {
  if (!display_is_owner(owner)) return false;
  uint8_t l = constrain(level, 0, 15);
  if (l == g_bright) return true;
  g_bright = l;
  g_alpha.setBrightness(g_bright);
  return true;
}

void display_idle(const char* s4) {
  maybe_expire();
  if (g_owner[0]) return; // someone else owns it
  hw_show4(s4);
}