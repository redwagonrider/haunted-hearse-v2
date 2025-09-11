#include "inputs.hpp"

static uint8_t g_pins[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static uint8_t g_count   = 0;

static uint16_t g_debounceMs = 30;
static uint32_t g_rearmMs    = 2000;

struct SensorState {
  bool     stable;        // debounced logical state (true = broken/LOW)
  bool     lastStable;
  bool     event;         // latched “just broke” event
  uint8_t  pin;
  uint32_t tLastChange;
  uint32_t tLastTrigger;  // last time we accepted a break event
};
static SensorState S[6];

void inputs_begin(const uint8_t pins[6], uint16_t debounceMs, uint32_t rearmMs){
  g_debounceMs = debounceMs;
  g_rearmMs    = rearmMs;
  g_count = 0;
  for (uint8_t i=0; i<6; i++){
    g_pins[i] = pins[i];
    if (g_pins[i] != 0xFF){
      pinMode(g_pins[i], INPUT_PULLUP);   // receivers pull LOW when broken
      S[g_count] = { false, false, false, (uint8_t)g_pins[i], (uint32_t)millis(), (uint32_t)0UL };
      g_count++;
    }
  }
}

void inputs_setDebounce(uint16_t ms){ g_debounceMs = ms; }
void inputs_setRearm(uint32_t ms){ g_rearmMs = ms; }

void inputs_update(){
  uint32_t now = (uint32_t)millis();
  for (uint8_t i=0; i<g_count; i++){
    SensorState &x = S[i];
    bool broken = (digitalRead(x.pin) == LOW);  // LOW = beam broken

    // Edge detect into debounce window
    if (broken != x.lastStable){
      x.tLastChange = now;
      x.lastStable  = broken;
    }

    // Commit state after debounce time
    if ((now - x.tLastChange) >= g_debounceMs){
      if (x.stable != x.lastStable){
        x.stable = x.lastStable;
        // Rising into "broken" -> one-shot event, subject to rearm
        if (x.stable && (now - x.tLastTrigger >= g_rearmMs)){
          x.event = true;
          x.tLastTrigger = now;
        }
      }
    }
  }
}

bool inputs_triggered(uint8_t idx){
  if (idx >= g_count) return false;
  if (S[idx].event){ S[idx].event = false; return true; }
  return false;
}

bool inputs_isBroken(uint8_t idx){
  if (idx >= g_count) return false;
  return S[idx].stable;
}

bool inputs_isStable(uint8_t idx){
  if (idx >= g_count) return false;
  return S[idx].stable;   // debounced state
}

void inputs_printStatus(Stream& s){
  s.println(F("== Sensors =="));
  for (uint8_t i=0; i<g_count; i++){
    s.print(F("#")); s.print(i);
    s.print(F(" pin=")); s.print(S[i].pin);
    s.print(F(" state=")); s.print(S[i].stable ? F("BROKEN") : F("OK"));
    s.print(F(" (event=")); s.print(S[i].event ? F("yes") : F("no"));
    s.println(F(")"));
  }
  s.print(F("debounce=")); s.print(g_debounceMs);
  s.print(F("ms, rearm=")); s.print(g_rearmMs); s.println(F("ms"));
}