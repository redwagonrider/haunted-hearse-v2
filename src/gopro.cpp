#include <Arduino.h>
#include "gopro.hpp"

namespace {
  uint8_t  GP_PIN   = 255;
  bool     pending  = false;
  uint32_t start_ms = 0;
  uint32_t span_ms  = 0;
}

void gopro_begin(uint8_t pinPower){
  GP_PIN = pinPower;
  pinMode(GP_PIN, OUTPUT);
  digitalWrite(GP_PIN, LOW);    // OFF by default
  pending  = false;
  start_ms = 0;
  span_ms  = 0;
}

void gopro_power(bool on){
  if (GP_PIN == 255) return;
  digitalWrite(GP_PIN, on ? HIGH : LOW);
  if (!on) pending = false;
}

void gopro_record_for(uint32_t ms){
  if (GP_PIN == 255) return;
  gopro_power(true);
  pending  = true;
  span_ms  = ms;
  start_ms = millis();
}

void gopro_update(){
  if (!pending) return;
  const uint32_t now = millis();
  if ((uint32_t)(now - start_ms) >= span_ms){
    gopro_power(false);
    pending = false;
  }
}