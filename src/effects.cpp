// effects.cpp
#include "effects.hpp"
#include "pins.hpp"
#include <Arduino.h>

// ===== Internal RGB state for telemetry =====
static uint8_t s_currR = 0;
static uint8_t s_currG = 0;
static uint8_t s_currB = 0;

void effects_setRGB(uint8_t r, uint8_t g, uint8_t b) {
  s_currR = r; s_currG = g; s_currB = b;
}

void effects_getRGB(uint8_t& r, uint8_t& g, uint8_t& b) {
  r = s_currR; g = s_currG; b = s_currB;
}

// ===== Hardware status LEDs on the Mega (not pixels) =====
static inline void leds_all_off() {
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_HOLD, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
}

void effects_begin() {
  pinMode(LED_ARMED, OUTPUT);
  pinMode(LED_HOLD, OUTPUT);
  pinMode(LED_COOLDOWN, OUTPUT);
  leds_all_off();
  effects_setRGB(0, 16, 0);
  digitalWrite(LED_ARMED, HIGH);
}

// ===== Small helpers =====
static uint8_t tri8(uint32_t t, uint32_t period_ms) {
  if (period_ms == 0) return 0;
  uint32_t p = t % period_ms;
  if (p < period_ms / 2) {
    return (uint8_t)((p * 510UL) / period_ms);
  } else {
    uint32_t d = p - period_ms / 2;
    return (uint8_t)(255UL - (d * 510UL) / period_ms);
  }
}

static uint8_t sin8(uint32_t t, uint32_t period_ms, uint8_t minv, uint8_t maxv) {
  uint8_t v = tri8(t, period_ms);
  uint16_t span = (uint16_t)maxv - (uint16_t)minv;
  return (uint8_t)(minv + ((uint16_t)v * span) / 255U);
}

static uint8_t rand_between(uint8_t a, uint8_t b) {
  if (a > b) { uint8_t t = a; a = b; b = t; }
  return (uint8_t)(a + (uint8_t)random(b - a + 1));
}

// ===== Core scene helpers =====
void effects_holdPulseRed(unsigned long elapsed) {
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
  digitalWrite(LED_HOLD, HIGH);

  uint8_t r = tri8(elapsed, 1200);
  uint8_t g = 32;
  uint8_t b = 0;
  effects_setRGB(r, g, b);
}

void effects_updateCooldown() {
  uint32_t now = millis();
  digitalWrite(LED_COOLDOWN, ((now / 500) & 1) ? HIGH : LOW);
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_HOLD, LOW);
  effects_setRGB(8, 0, 0);
}

void effects_updateArmed() {
  digitalWrite(LED_ARMED, HIGH);
  digitalWrite(LED_HOLD, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
  effects_setRGB(0, 16, 0);
}

// ===== Universal effects =====
void effects_showBlackout() {
  leds_all_off();
  effects_setRGB(0, 0, 0);
}

void effects_exitStrobe() {
  static uint32_t last = 0;
  static bool on = false;
  uint32_t now = millis();
  const uint16_t period = 100;

  if (now - last >= period) { on = !on; last = now; }
  digitalWrite(LED_HOLD, on ? HIGH : LOW);
  effects_setRGB(on ? 255 : 0, on ? 255 : 0, on ? 255 : 0);
}

// wrapper used by scene_standby
void effects_showStandby() {
  effects_standbyIdle();
}

// ===== Fire room =====
void effects_fireFlicker() {
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last >= 15) {
    last = now;
    uint8_t r = rand_between(180, 255);
    uint8_t g = rand_between(40, 90);
    effects_setRGB(r, g, 0);
    digitalWrite(LED_HOLD, (r > 220) ? HIGH : LOW);
  }
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
}

// ===== Fur room =====
void effects_furPulse() {
  uint32_t now = millis();
  uint8_t v = sin8(now, 1600, 10, 180);
  effects_setRGB(v, 0, v);
  digitalWrite(LED_HOLD, (v > 120) ? HIGH : LOW);
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
}

// ===== Graveyard =====
void effects_mistyGraveyard() {
  uint32_t now = millis();
  uint8_t g = sin8(now, 2800, 10, 120);
  uint8_t b = sin8(now + 700, 3200, 20, 180);
  effects_setRGB(0, g, b);
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_HOLD, LOW);
  digitalWrite(LED_COOLDOWN, (g + b) > 200 ? HIGH : LOW);
}

// ===== Blood room =====
void effects_bloodPulse() {
  uint32_t now = millis();
  uint8_t r = sin8(now, 1400, 40, 255);
  effects_setRGB(r, 8, 8);
  digitalWrite(LED_HOLD, (r > 180) ? HIGH : LOW);
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
}

void effects_bloodDrip() {
  static uint32_t nextSpike = 0;
  uint32_t now = millis();
  uint8_t r = 24;

  if (now >= nextSpike) {
    r = 255;
    nextSpike = now + 600 + random(800);
  }
  effects_setRGB(r, 12, 12);
  digitalWrite(LED_HOLD, (r == 255) ? HIGH : LOW);
}

// ===== Spider lair =====
void effects_spiderCrawl() {
  static uint32_t nextBlink = 0;
  uint32_t now = millis();
  if (now >= nextBlink) {
    effects_setRGB(255, 255, 255);
    nextBlink = now + 50 + random(250);
  } else {
    effects_setRGB(4, 4, 4);
  }
  digitalWrite(LED_HOLD, HIGH);
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
}

void effects_spiderEyes() {
  effects_setRGB(8, 0, 0);
  digitalWrite(LED_HOLD, LOW);
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
}

void effects_spiderWebFlash() {
  uint32_t now = millis();
  uint8_t v = tri8(now, 200);
  uint8_t on = v > 220 ? 255 : 0;
  effects_setRGB(on, on, on);
  digitalWrite(LED_HOLD, on ? HIGH : LOW);
}

// ===== Mirror room =====
void effects_mirrorStrobe() {
  static uint32_t last = 0;
  static bool on = false;
  uint32_t now = millis();
  const uint16_t period = 60;
  if (now - last >= period) { on = !on; last = now; }
  effects_setRGB(on ? 255 : 0, on ? 255 : 0, on ? 255 : 0);
  digitalWrite(LED_HOLD, on ? HIGH : LOW);
}

void effects_mirrorSweep() {
  uint32_t now = millis();
  uint8_t v = tri8(now, 1000);
  effects_setRGB(v, v, v);
  digitalWrite(LED_HOLD, (v > 128) ? HIGH : LOW);
}

void effects_mirrorFlash() {
  uint32_t now = millis();
  uint8_t v = (tri8(now, 300) > 200) ? 255 : 0;
  effects_setRGB(v, v, v);
  digitalWrite(LED_HOLD, v ? HIGH : LOW);
}

// ===== Orca scene =====
void effects_orcaBlueFade() {
  uint32_t now = millis();
  uint8_t b = sin8(now, 2000, 30, 200);
  uint8_t g = sin8(now + 400, 2400, 10, 80);
  effects_setRGB(0, g, b);
  digitalWrite(LED_HOLD, LOW);
  digitalWrite(LED_ARMED, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
}

void effects_orcaWave() {
  uint32_t now = millis();
  uint8_t mix = tri8(now, 1600);
  uint8_t b = (uint8_t)(120 + (mix * 120) / 255U);
  uint8_t g = (uint8_t)((mix * 80) / 255U);
  effects_setRGB(0, g, b);
}

void effects_orcaSplash() {
  uint32_t now = millis();
  uint8_t phase = tri8(now, 500);
  uint8_t b = phase;
  uint8_t g = phase / 3;
  effects_setRGB(0, g, b);
  digitalWrite(LED_HOLD, (b > 200) ? HIGH : LOW);
}

// ===== Intro =====
void effects_introFade() {
  uint32_t now = millis();
  uint8_t v = sin8(now, 3000, 0, 255);
  effects_setRGB(v, v, v);
  digitalWrite(LED_ARMED, (v > 128) ? HIGH : LOW);
  digitalWrite(LED_HOLD, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
}

// ===== Secret =====
void effects_secretGlow() {
  uint32_t now = millis();
  uint8_t r = sin8(now, 1800, 20, 120);
  uint8_t b = sin8(now + 600, 1800, 40, 180);
  effects_setRGB(r, 0, b);
  digitalWrite(LED_HOLD, (r + b) > 160 ? HIGH : LOW);
}

void effects_secretReveal() {
  uint32_t now = millis();
  uint8_t pulse = tri8(now, 400);
  uint8_t flash = pulse > 230 ? 255 : 0;
  if (flash) {
    effects_setRGB(255, 255, 255);
    digitalWrite(LED_HOLD, HIGH);
  } else {
    uint8_t r = sin8(now, 2200, 40, 120);
    uint8_t b = sin8(now + 700, 2200, 80, 200);
    effects_setRGB(r, 0, b);
    digitalWrite(LED_HOLD, (r + b) > 160 ? HIGH : LOW);
  }
}

// ===== Standby =====
void effects_standbyIdle() {
  uint32_t now = millis();
  uint8_t g = sin8(now, 2400, 4, 30);
  uint8_t r = sin8(now + 900, 2400, 2, 16);
  effects_setRGB(r, g, 0);
  digitalWrite(LED_ARMED, (g > 20) ? HIGH : LOW);
  digitalWrite(LED_HOLD, LOW);
  digitalWrite(LED_COOLDOWN, LOW);
}