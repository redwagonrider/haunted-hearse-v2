#include <Arduino.h>
#include "display.hpp"
#include "effects.hpp"
#include "console.hpp"

// --- Pin assignments (adjust if yours differ) ---
const uint8_t PIN_BEAM        = 3;   // Break-beam input (receiver output)
const uint8_t PIN_MAGNET_CTRL = 6;   // MOSFET signal to electromagnet
const uint8_t PIN_BUZZER      = 8;   // Passive buzzer
const uint8_t LED_ARMED       = 10;  // Green
const uint8_t LED_HOLD        = 11;  // Red
const uint8_t LED_COOLDOWN    = 12;  // Yellow

// --- Timings ---
unsigned long HOLD_MS = 5000;                // magnet + modem
unsigned long COOLDOWN_MS = 20000;           // display flash + lockout

// --- State machine ---
enum RunState { IDLE, HOLD, COOLDOWN };
RunState state = IDLE;
unsigned long tStateStart = 0;

// ---------- Console callback implementations (placed BEFORE setup) ----------
static void setHold(uint32_t ms){
  HOLD_MS = ms;
  effects_setHoldMs(HOLD_MS);
  Serial.print(F("HOLD_MS = ")); Serial.println(HOLD_MS);
}
static void setCool(uint32_t ms){
  COOLDOWN_MS = ms;
  Serial.print(F("COOLDOWN_MS = ")); Serial.println(COOLDOWN_MS);
}
static void setBright(uint8_t b){
  display_set_brightness(b);
  Serial.print(F("BRIGHT = ")); Serial.println((int)b);
}
static void printStatus(){
  Serial.println(F("\n--- STATUS ---"));
  Serial.print(F("State: "));
  Serial.println(state==IDLE?"IDLE":state==HOLD?"HOLD":"COOLDOWN");
  Serial.print(F("HOLD_MS=")); Serial.println(HOLD_MS);
  Serial.print(F("COOLDOWN_MS=")); Serial.println(COOLDOWN_MS);
  Serial.println(F("Pins: beam=3, magnet=6, buzzer=8, green=10, red=11, yellow=12"));
  Serial.println(F("--------------\n"));
}
static void forceState(int st){
  unsigned long now = millis();
  switch(st){
    case 0: // IDLE
      state = IDLE;
      display_idle("OBEY");
      effects_magnet_off();
      effects_sound_stop();
      break;
    case 1: // HOLD
      state = HOLD;
      tStateStart = now;
      effects_magnet_on();
      display_hold_init();
      break;
    case 2: // COOLDOWN
      state = COOLDOWN;
      tStateStart = now;
      effects_magnet_off();
      effects_sound_stop();
      display_cooldown_init();
      break;
  }
}
// ---------------------------------------------------------------------------

void setup() {
  pinMode(PIN_BEAM, INPUT_PULLUP);           // LOW = broken (flip if opposite)
  randomSeed(analogRead(A0));

  display_begin(0x70, 8);
  display_idle("OBEY");

  effects_begin(LED_ARMED, LED_HOLD, LED_COOLDOWN, PIN_BUZZER, PIN_MAGNET_CTRL, HOLD_MS);

  console_begin(115200);
  console_attach(setHold, setCool, setBright, forceState, printStatus);
}

void loop() {
  console_update();   // read serial commands

  bool beamBroken = (digitalRead(PIN_BEAM) == LOW);
  unsigned long now = millis();

  switch (state) {
    case IDLE:
      effects_idle_update();
      if (beamBroken) {
        state = HOLD;
        tStateStart = now;
        effects_magnet_on();
        display_hold_init();
      }
      break;

    case HOLD: {
      unsigned long elapsed = now - tStateStart;
      effects_hold_update(elapsed);
      display_hold_update();
      if (elapsed >= HOLD_MS) {
        state = COOLDOWN;
        tStateStart = now;
        effects_magnet_off();
        effects_sound_stop();
        display_cooldown_init();
      }
    } break;

    case COOLDOWN:
      effects_cooldown_update();
      display_cooldown_update();
      display_hold_update(); // finish PIN/ZIP if still running
      if (now - tStateStart >= COOLDOWN_MS) {
        state = IDLE;
        display_idle("OBEY");
      }
      break;
  }

  delay(2); // small tick
}