// src/triggers.cpp
#include <Arduino.h>
#include "triggers.hpp"

// Mega output pins that drive the optocoupler LEDs or relay inputs (active HIGH)
static const uint8_t PIN_TRIG[4] = {22, 23, 24, 25};
static const char*   NAME_TRIG[4] = {"BLOOD", "GRAVE", "FUR", "FRANKEN"};

// Simple lockout to avoid double-pulses
static unsigned long last_fire_ms[4] = {0,0,0,0};
static const unsigned long MIN_LOCKOUT_MS = 300;

void triggers_begin() {
  for (uint8_t i = 0; i < 4; ++i) {
    pinMode(PIN_TRIG[i], OUTPUT);
    digitalWrite(PIN_TRIG[i], LOW); // idle OFF
    last_fire_ms[i] = 0;
  }
}

bool triggers_pulse(uint8_t idx, uint16_t ms) {
  if (idx >= 4) return false;
  unsigned long now = millis();
  if (now - last_fire_ms[idx] < MIN_LOCKOUT_MS) return false;

  digitalWrite(PIN_TRIG[idx], HIGH);
  delay(ms);
  digitalWrite(PIN_TRIG[idx], LOW);

  last_fire_ms[idx] = millis();
  return true;
}

static int name_to_index(const String& up) {
  for (uint8_t i = 0; i < 4; ++i) {
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
  Serial.println(F("  D22 -> GPIO17 : Start_BloodRoom"));
  Serial.println(F("  D23 -> GPIO27 : Start_Graveyard"));
  Serial.println(F("  D24 -> GPIO22 : Start_FurRoom"));
  Serial.println(F("  D25 -> GPIO23 : Start_FrankenLab"));
  Serial.println(F("Pulse: 100 ms active, ~300 ms lockout, Pi triggers on FALLING edge"));
}