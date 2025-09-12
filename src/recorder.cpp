#include <Arduino.h>
#include "recorder.hpp"

namespace {
  uint8_t  R_PIN     = 255;     // relay IN pin (255 = uninitialized)
  bool     pending   = false;   // currently running a timed session
  uint32_t start_ms  = 0;       // when the timed session began
  uint32_t span_ms   = 0;       // how long to keep power ON
}

void recorder_begin(uint8_t pinPower){
  R_PIN = pinPower;
  pinMode(R_PIN, OUTPUT);
  digitalWrite(R_PIN, LOW);     // power OFF by default
  pending  = false;
  start_ms = 0;
  span_ms  = 0;
}

void recorder_power(bool on){
  if (R_PIN == 255) return;     // ignore if not initialized
  digitalWrite(R_PIN, on ? HIGH : LOW);
  if (!on) {
    // cancel any pending timer if we turned it off manually
    pending = false;
  }
}

void recorder_record_for(uint32_t ms){
  if (R_PIN == 255) return;
  recorder_power(true);         // apply 5V -> DR-05 should auto-record
  pending  = true;
  span_ms  = ms;
  start_ms = millis();
}

void recorder_update(){
  if (!pending) return;
  const uint32_t now = millis();
  if ((uint32_t)(now - start_ms) >= span_ms){
    recorder_power(false);      // cut 5V -> DR-05 stops & saves file
    pending = false;
  }
}