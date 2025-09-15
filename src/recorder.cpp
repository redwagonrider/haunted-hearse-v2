#include "recorder.hpp"

namespace {
  uint8_t PIN = 255;
  bool powered = false;
  unsigned long until = 0;
}

void recorder_begin(uint8_t relayPin){
  PIN = relayPin;
  if (PIN != 255){
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN, LOW);
  }
  powered = false;
  until = 0;
}

void recorder_power(bool on){
  powered = on;
  if (PIN != 255) digitalWrite(PIN, on ? HIGH : LOW);
  if (!on) until = 0;
}

void recorder_record_for(uint32_t ms){
  if (PIN == 255) return;
  powered = true;
  digitalWrite(PIN, HIGH);
  until = millis() + ms;
}

void recorder_update(){
  if (until && millis() >= until){
    until = 0;
    powered = false;
    if (PIN != 255) digitalWrite(PIN, LOW);
  }
}