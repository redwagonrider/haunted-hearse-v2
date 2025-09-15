// src/triggers.cpp
#include <Arduino.h>
#include "triggers.hpp"

// Five trigger outputs (active HIGH to optos)
// Wire these to Pi GPIOs with internal pullups enabled on the Pi.
static const uint8_t PIN_TRIG[]  = {27, 22, 23, 24, 25};  // D27, D22, D23, D24, D25
static const char*   NAME_TRIG[] = {"SHOW","BLOOD","GRAVE","FUR","FRANKEN"};
static const uint8_t N_TRIG = sizeof(PIN_TRIG) / sizeof(PIN_TRIG[0]);

// Simple lockout to avoid double-pulses
static unsigned long last_fire_ms[5] = {0,0,0,0,0};
static const unsigned long MIN_LOCKOUT_MS = 300;

void triggers_begin() {
  for (uint8_t i = 0; i < N_TRIG; ++i) {
    pinMode(PIN_TRIG[i], OUTPUT);
    digitalWrite(PIN_TRIG[i], LOW); // idle OFF
    last_fire_ms[i] = 0;
  }
}

bool triggers_pulse(uint8_t idx, uint16_t ms) {
  if (idx >= N_TRIG) return false;
  const unsigned long now = millis();
  if (now - last_fire_ms[idx] < MIN_LOCKOUT_MS) return false;

  digitalWrite(PIN_TRIG[idx], HIGH);
  delay(ms);
  digitalWrite(PIN_TRIG[idx], LOW);

  last_fire_ms[idx] = millis();
  return true;
}

static int name_to_index(const String& up) {
  for (uint8_t i = 0; i < N_TRIG; ++i) {
    if (up == NAME_TRIG[i]) return i;
  }
  return -1;
}

bool triggers_pulse_by_name(const String& upname) {
  int idx = name_to_index(upname);
  if (idx < 0) return false;
  return triggers_pulse((uint8_t)idx, 100);
}

void triggers_print_map() {
  Serial.println(F("=== GPIO Trigger Map (Mega -> Pi) ==="));
  Serial.println(F("  D27 -> GPIO5  : Start_MainShow  (SHOW)"));
  Serial.println(F("  D22 -> GPIO17 : Start_BloodRoom (BLOOD)"));
  Serial.println(F("  D23 -> GPIO27 : Start_Graveyard (GRAVE)"));
  Serial.println(F("  D24 -> GPIO22 : Start_FurRoom   (FUR)"));
  Serial.println(F("  D25 -> GPIO23 : Start_Franken   (FRANKEN)"));
  Serial.println(F("Pulse: 100 ms active, ~300 ms lockout. Pi should detect FALLING edge."));
}