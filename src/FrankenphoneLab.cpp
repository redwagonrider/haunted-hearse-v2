// Frankenphone Lab — Break-beam → Magnet (Adafruit STEMMA MOSFET) + Passive Buzzer (Dial-up Modem) + State LEDs
// Board: Arduino Mega 2560
#include <stdio.h>
#include "stdint.h"
#include <Arduino.h>

// Pins
const uint8_t PIN_BEAM        = 3;   // Break-beam: intact=HIGH, broken=LOW (INPUT_PULLUP)
const uint8_t PIN_MAGNET_CTRL = 9;   // Adafruit STEMMA MOSFET SIG (ACTIVE-HIGH)
const uint8_t PIN_BUZZER      = 8;   // PASSIVE buzzer (+ to D8, - to GND)

// Status LEDs (PWM)
const uint8_t LED_ARMED    = 10;  // Green  : pulse while ARMED
const uint8_t LED_HOLD     = 11;  // Red    : stutter & brighten during HOLD
const uint8_t LED_COOLDOWN = 12;  // Yellow : flicker during COOLDOWN

// Durations
const unsigned long HOLD_MS     = 5000;   // Magnet + modem sound duration
const unsigned long COOLDOWN_MS = 20000;  // Re-arm delay after each run

// State
bool armed      = true;
bool activeHold = false;
bool cooldown   = false;
unsigned long tPhaseStart = 0;

// Modem sound internals
unsigned long modemLastChange = 0;
int           modemCurrentHz  = 0;

// LED animation internals
unsigned long ledRandDeadline = 0;
int           ledYellowPWM    = 0;

// Helpers
inline void magnetOn()  { digitalWrite(PIN_MAGNET_CTRL, HIGH); } // active-HIGH board
inline void magnetOff() { digitalWrite(PIN_MAGNET_CTRL, LOW);  }
inline void buzzerOff() { noTone(PIN_BUZZER); }

// Green ARMED: smooth breathing pulse (triangle) with ~2.4s period
void animateGreenArmed() {
  unsigned long ms = millis();
  const unsigned long period = 2400;
  unsigned long t = ms % period;
  int val = (t < period/2) ? map(t, 0, period/2, 30, 255)
                           : map(t, period/2, period, 255, 30);
  analogWrite(LED_ARMED, val);
  analogWrite(LED_HOLD, 0);
  analogWrite(LED_COOLDOWN, 0);
}

// Yellow COOLDOWN: random flicker (brightness & dwell)
void animateYellowCooldown() {
  unsigned long now = millis();
  if (now >= ledRandDeadline) {
    ledYellowPWM = random(40, 255);
    unsigned long dwell = (unsigned long)random(20, 120);
    ledRandDeadline = now + dwell;
    analogWrite(LED_COOLDOWN, ledYellowPWM);
  }
  analogWrite(LED_ARMED, 0);
  analogWrite(LED_HOLD, 0);
}

// Red HOLD: stutter that ramps faster & brighter over HOLD_MS
void animateRedHold(unsigned long holdElapsed) {
  const unsigned int PERIOD_SLOW_MS = 300;
  const unsigned int PERIOD_FAST_MS = 40;
  unsigned long clamped = (holdElapsed > HOLD_MS) ? HOLD_MS : holdElapsed;
  unsigned int period = PERIOD_SLOW_MS
                      - ( (long)(PERIOD_SLOW_MS - PERIOD_FAST_MS) * (long)clamped ) / (long)HOLD_MS;
  int brightness = 60 + (int)((195L * (long)clamped) / (long)HOLD_MS); // 60→255
  brightness = constrain(brightness, 0, 255);
  unsigned long phase = millis() % period;
  bool on = (phase < (period * 45UL) / 100UL);
  analogWrite(LED_HOLD, on ? brightness : 0);
  analogWrite(LED_ARMED, 0);
  analogWrite(LED_COOLDOWN, 0);
}

// Modem sound fills entire 5s window
void updateModemSound(unsigned long holdElapsed) {
  unsigned long now = millis();

  if (holdElapsed < 800UL) {
    int f = 400 + (int)((holdElapsed * (1800 - 400)) / 800UL);
    if (f != modemCurrentHz) { tone(PIN_BUZZER, f); modemCurrentHz = f; }

  } else if (holdElapsed < 1600UL) {
    unsigned long e = holdElapsed - 800UL;
    int f = 1800 - (int)((e * (1800 - 600)) / 800UL);
    if (f != modemCurrentHz) { tone(PIN_BUZZER, f); modemCurrentHz = f; }

  } else if (holdElapsed < 3000UL) {
    if (now - modemLastChange >= 6) {
      modemCurrentHz = random(400, 2500);
      tone(PIN_BUZZER, modemCurrentHz);
      modemLastChange = now;
    }

  } else if (holdElapsed < 4200UL) {
    if (now - modemLastChange >= 40) {
      modemCurrentHz = (modemCurrentHz == 1400 ? 1800 : 1400);
      tone(PIN_BUZZER, modemCurrentHz);
      modemLastChange = now;
    }

  } else if (holdElapsed < HOLD_MS) {
    if (modemCurrentHz != 1000) { tone(PIN_BUZZER, 1000); modemCurrentHz = 1000; }

  } else {
    buzzerOff();
    modemCurrentHz = 0;
  }
}

// Phases
void startHold() {
  activeHold   = true;
  cooldown     = false;
  armed        = false;
  tPhaseStart  = millis();
  modemLastChange = tPhaseStart;
  modemCurrentHz  = 0;
  magnetOn();
}

void stopHoldAndStartCooldown() {
  activeHold  = false;
  cooldown    = true;
  tPhaseStart = millis();
  magnetOff();
  buzzerOff();
  modemCurrentHz = 0;
}

void endCooldownAndArm() {
  cooldown   = false;
  armed      = true;
}

// Setup/loop
void setup() {
  pinMode(PIN_BEAM, INPUT_PULLUP);     // intact=HIGH, broken=LOW
  pinMode(PIN_MAGNET_CTRL, OUTPUT);
  digitalWrite(PIN_MAGNET_CTRL, LOW);  // ensure OFF at boot (active-HIGH board)
  pinMode(PIN_BUZZER, OUTPUT);
  buzzerOff();
  pinMode(LED_ARMED,    OUTPUT);
  pinMode(LED_HOLD,     OUTPUT);
  pinMode(LED_COOLDOWN, OUTPUT);
  randomSeed(analogRead(A0));          // for flicker & jitter
}

void loop() {
  bool beamBroken = (digitalRead(PIN_BEAM) == LOW);
  unsigned long now = millis();

  if (armed && !activeHold && !cooldown && beamBroken) {
    startHold();
  }

  if (activeHold) {
    unsigned long elapsed = now - tPhaseStart;
    updateModemSound(elapsed);
    animateRedHold(elapsed);
    if (elapsed >= HOLD_MS) {
      stopHoldAndStartCooldown();
    }
  } else if (cooldown) {
    animateYellowCooldown();
    if (now - tPhaseStart >= COOLDOWN_MS) {
      endCooldownAndArm();
    }
  } else if (armed) {
    animateGreenArmed();
  }

  delay(2);
}
