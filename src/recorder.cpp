#include "recorder.hpp"

static uint8_t  R_PIN = 255;
static bool     pending = false;
static uint32_t t_start = 0, dur = 0;

void recorder_begin(uint8_t pinPower){
  R_PIN = pinPower;
  pinMode(R_PIN, OUTPUT);
  digitalWrite(R_PIN, LOW);
}

void recorder_power(bool on){
  if (R_PIN == 255) return;
  digitalWrite(R_PIN, on ? HIGH : LOW);
  if (!on) pending = false;
}

void recorder_record_for(uint32_t ms){
  if (R_PIN == 255) return;
  recorder_power(true);
  pending = true;
  dur = ms;
  t_start = millis();
}

void recorder_update(){
  if (pending && (millis() - t_start >= dur)){
    recorder_power(false);
    pending = false;
  }
}