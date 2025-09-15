// triggers.cpp
#include "triggers.hpp"

// Hardware mapping: Mega D22..D25
static const uint8_t kOutPins[4] = {22, 23, 24, 25};

// For your ops card: Pi GPIO targets per channel
static const uint8_t kPiGPIO[4] = {17, 27, 22, 23};

// Friendly names for logs or future console prints
static const char* kNames[4] = {
  "Start_BloodRoom",
  "Start_Graveyard",
  "Start_FurRoom",
  "Start_FrankenLab"
};

struct Chan {
  uint8_t  pin;
  uint32_t t_on;           // when pulse started
  uint32_t t_lockout;      // when channel becomes armed again
  bool     active;         // pulse currently high
};

static Chan CH[4];
static TriggerConfig CFG = {100, 300};

void triggers_begin(const TriggerConfig& cfg) {
  CFG = cfg;
  for (uint8_t i = 0; i < 4; i++) {
    CH[i].pin = kOutPins[i];
    CH[i].t_on = 0;
    CH[i].t_lockout = 0;
    CH[i].active = false;
    pinMode(CH[i].pin, OUTPUT);
    digitalWrite(CH[i].pin, LOW); // idle low, pulse drives HIGH
  }
}

bool triggers_fire(TriggerId id) {
  if (id > TRIG_FRANKENLAB) return false;
  Chan& c = CH[(uint8_t)id];
  uint32_t now = millis();

  // lockout
  if (now < c.t_lockout) return false;
  if (c.active) return false;

  c.active = true;
  c.t_on = now;
  c.t_lockout = now + CFG.lockout_ms;

  digitalWrite(c.pin, HIGH); // begin pulse

  return true;
}

void triggers_update() {
  uint32_t now = millis();
  for (uint8_t i = 0; i < 4; i++) {
    Chan& c = CH[i];
    if (c.active && (now - c.t_on >= CFG.pulse_ms)) {
      digitalWrite(c.pin, LOW); // end pulse
      c.active = false;
      // lockout continues until c.t_lockout
    }
  }
}

uint8_t triggers_pin_for(TriggerId id) {
  if (id > TRIG_FRANKENLAB) return 255;
  return kOutPins[(uint8_t)id];
}

uint8_t triggers_pi_gpio_for(TriggerId id) {
  if (id > TRIG_FRANKENLAB) return 255;
  return kPiGPIO[(uint8_t)id];
}

const char* triggers_name_for(TriggerId id) {
  if (id > TRIG_FRANKENLAB) return "";
  return kNames[(uint8_t)id];
}