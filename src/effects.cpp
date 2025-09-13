#include "effects.hpp"

namespace {
  // ==== Pins ====
  uint8_t PIN_LED_ARMED   = 10; // Green (Standby)
  uint8_t PIN_LED_HOLD    = 11; // Red (Hold)
  uint8_t PIN_LED_COOL    = 12; // Yellow (Cooldown)
  uint8_t PIN_BUZZER      = 8;  // Passive buzzer +
  uint8_t PIN_MAGNET      = 6;  // MOSFET gate (electromagnet)

  // ==== Tunables ====
  uint32_t HOLD_MS = 5000;                 // set by effects_setHoldMs()
  const uint16_t STANDBY_PERIOD_MS = 1200; // green pulse period
  const uint16_t STANDBY_ON_MS     = 240;  // green ON window (20%)

  // ==== State ====
  unsigned long tStartHold     = 0;
  unsigned long tLastPulse     = 0;
  unsigned long tLastStep      = 0;
  unsigned long tLastFlick     = 0;
  unsigned long tRearmCueUntil = 0;        // solid-green cue window end time

  bool gPulseOn = false; // current pulse state
  bool coolOn   = true;  // current yellow state

  inline void ledG(bool on){ digitalWrite(PIN_LED_ARMED,  on ? HIGH : LOW); }
  inline void ledR(bool on){ digitalWrite(PIN_LED_HOLD,   on ? HIGH : LOW); }
  inline void ledY(bool on){ digitalWrite(PIN_LED_COOL,   on ? HIGH : LOW); }
  inline void mag (bool on){ digitalWrite(PIN_MAGNET,     on ? HIGH : LOW); }
  inline void buzzOff(){ noTone(PIN_BUZZER); digitalWrite(PIN_BUZZER, LOW); }

  // Quick ~22ms chirp packet (for modem-ish bursts)
  inline void chirpPacket(uint16_t f){
    tone(PIN_BUZZER, f, 22);
  }
}

void effects_begin(uint8_t pinLED_armed,
                   uint8_t pinLED_hold,
                   uint8_t pinLED_cool,
                   uint8_t pinBuzzer,
                   uint8_t pinMagnet,
                   uint32_t hold_ms_default)
{
  PIN_LED_ARMED = pinLED_armed;
  PIN_LED_HOLD  = pinLED_hold;
  PIN_LED_COOL  = pinLED_cool;
  PIN_BUZZER    = pinBuzzer;
  PIN_MAGNET    = pinMagnet;
  HOLD_MS       = hold_ms_default;

  pinMode(PIN_LED_ARMED, OUTPUT);
  pinMode(PIN_LED_HOLD,  OUTPUT);
  pinMode(PIN_LED_COOL,  OUTPUT);
  pinMode(PIN_BUZZER,    OUTPUT);
  pinMode(PIN_MAGNET,    OUTPUT);

  ledG(false); ledR(false); ledY(false);
  mag(false);  buzzOff();

  tLastPulse     = millis();
  gPulseOn       = false;
  tRearmCueUntil = 0;
}

void effects_setHoldMs(uint32_t ms){ HOLD_MS = ms; }

// NEW: show a solid-green indicator for ~500ms to signal we are re-armed
void effects_rearmCue(){
  tRearmCueUntil = millis() + 500;
  ledG(true); // solid green during cue
}

// === Standby: GREEN slow pulse (unless inside re-arm cue window) ===
void effects_showStandby(){
  buzzOff(); mag(false);
  ledR(false); ledY(false);

  const unsigned long now = millis();

  // During the re-arm cue, hold solid green
  if (now < tRearmCueUntil){
    ledG(true);
    return;
  }

  // Otherwise pulse
  unsigned long phase = (now - tLastPulse) % STANDBY_PERIOD_MS;
  bool on = (phase < STANDBY_ON_MS);
  if (on != gPulseOn){
    gPulseOn = on;
    ledG(on);
  }
}

// === HOLD: RED stutter + chirps that accelerate; last 1s steady scream; MAGNET ON ===
void effects_startHold(){
  ledG(false); ledY(false); ledR(true);
  mag(true);
  buzzOff();
  tStartHold = millis();
  tLastStep  = tStartHold;
}

void effects_updateHold(){
  const unsigned long now     = millis();
  const unsigned long elapsed = now - tStartHold;
  const uint32_t span         = (HOLD_MS == 0) ? 1 : HOLD_MS;

  // Last 1s: continuous scream + solid red
  if (elapsed >= (span > 1000 ? span - 1000 : 0)){
    ledR(true);
    mag(true);
    tone(PIN_BUZZER, 2400);
    return;
  }

  // Stutter interval shrinks from ~220ms → ~60ms as time advances
  uint16_t interval = 220 - (uint16_t)((160.0f * elapsed) / span);
  if (interval < 60) interval = 60;

  if (now - tLastStep >= interval){
    tLastStep = now;

    // Red stutter toggle
    static bool rOn = true; rOn = !rOn;
    digitalWrite(PIN_LED_HOLD, rOn ? HIGH : LOW);

    // Chirp triplet sweeping upward
    uint16_t f0 = 1100 + (uint16_t)((1000.0f * elapsed) / span); // 1100 → ~2100
    uint16_t f1 = f0 + 180;
    uint16_t f2 = f1 + 220;
    chirpPacket(f0);
    chirpPacket(f1);
    chirpPacket(f2);
  }

  mag(true);
}

void effects_endHold(){
  buzzOff();
  mag(false);
  ledR(false);
}

// === COOLDOWN: YELLOW irregular jitter/flicker; MAGNET OFF ===
void effects_startCooldown(){
  buzzOff();
  mag(false);
  ledG(false); ledR(false); ledY(true);
  tLastFlick = millis();
  coolOn = true;
}

void effects_updateCooldown(){
  unsigned long now = millis();
  static uint8_t seed = 0x5A;
  auto rnd = [&seed](){ seed ^= seed<<7; seed ^= seed>>5; seed ^= seed<<3; return seed; };

  static unsigned long nextInt = 120;
  if (now - tLastFlick >= nextInt){
    tLastFlick = now;
    coolOn = !coolOn;
    digitalWrite(PIN_LED_COOL, coolOn ? HIGH : LOW);
    nextInt = 70 + (rnd() % 110); // 70..180ms
  }
}