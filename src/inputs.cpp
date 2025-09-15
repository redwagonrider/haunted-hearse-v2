#include "inputs.hpp"

namespace {
  struct Beam {
    uint8_t  pin = 255;          // 255 = unused
    bool     invert = false;     // if true, HIGH = broken
    bool     lastStable = false; // debounced logical state (true=broken)
    bool     curRaw    = false;  // immediate read
    bool     event     = false;  // one-shot latched event
    bool     armed     = true;   // can fire again?
    unsigned long lastChangeMs = 0;
    unsigned long lastFireMs   = 0;
  };

  Beam G[6];
  uint16_t DEBOUNCE_MS = 30;
  uint32_t REARM_MS    = 20000;

  inline bool isUsed(const Beam& b){ return b.pin != 255; }

  // Convert digitalRead -> logical "broken?"
  inline bool rawToBroken(bool raw, bool invert){
    // default: active LOW sensors (LOW=broken) -> invert=false
    // if invert=true: HIGH=broken
    return invert ? raw : !raw;
  }
}

void inputs_begin(const uint8_t pins[6], uint16_t debounce_ms, uint32_t rearm_ms){
  DEBOUNCE_MS = debounce_ms;
  REARM_MS    = rearm_ms;
  for (uint8_t i=0;i<6;i++){
    G[i] = Beam{}; // reset
    if (pins[i] != 255){
      G[i].pin = pins[i];
      pinMode(G[i].pin, INPUT_PULLUP); // typical beam receiver
      // First sample establishes initial stable state
      bool raw = digitalRead(G[i].pin);
      G[i].curRaw = raw;
      G[i].lastStable = rawToBroken(raw, G[i].invert);
      G[i].armed = true;
      G[i].lastChangeMs = millis();
      G[i].lastFireMs   = 0;
    }
  }
}

void inputs_setDebounce(uint16_t ms){ DEBOUNCE_MS = ms; }
void inputs_setRearm(uint32_t ms){ REARM_MS = ms; }

void inputs_setInvert(uint8_t idx, bool inverted){
  if (idx >= 6 || !isUsed(G[idx])) return;
  G[idx].invert = inverted;
  // Re-evaluate stable state immediately
  bool raw = digitalRead(G[idx].pin);
  G[idx].curRaw = raw;
  G[idx].lastStable = rawToBroken(raw, G[idx].invert);
  G[idx].event = false;
  G[idx].armed = true;
  G[idx].lastChangeMs = millis();
}

bool inputs_triggered(uint8_t idx){
  if (idx >= 6 || !isUsed(G[idx])) return false;
  bool e = G[idx].event;
  G[idx].event = false; // consume one-shot
  return e;
}

void inputs_update(){
  const unsigned long now = millis();
  for (uint8_t i=0;i<6;i++){
    if (!isUsed(G[i])) continue;

    bool raw = digitalRead(G[i].pin);
    if (raw != G[i].curRaw){
      G[i].curRaw = raw;
      G[i].lastChangeMs = now; // start debounce
    }

    // Debounce
    if ((now - G[i].lastChangeMs) >= DEBOUNCE_MS){
      bool broken = rawToBroken(G[i].curRaw, G[i].invert);
      if (broken != G[i].lastStable){
        // stable edge occurred
        G[i].lastStable = broken;
        if (broken){
          // Rising into "broken" -> potential fire
          if (G[i].armed){
            G[i].event = true;
            G[i].armed = false;
            G[i].lastFireMs = now;
          }
        } else {
          // Returned to OK; start re-arm timer from this moment
          // (or keep lastFireMs timing—either works; we choose OK moment)
          if (!G[i].armed){
            // wait for REARM_MS after returning OK
          }
        }
      }
    }

    // Re-arm logic: require OK for a while, then re-arm
    if (!G[i].armed){
      // Only re-arm if currently OK, and enough time elapsed since lastFire or since OK
      if (!G[i].lastStable){ // OK state
        if ((now - max(G[i].lastFireMs, G[i].lastChangeMs)) >= REARM_MS){
          G[i].armed = true;
        }
      }
    }
  }
}

void inputs_printStatus(Stream& s){
  s.println(F("== Sensors =="));
  for (uint8_t i=0;i<6;i++){
    if (!isUsed(G[i])) {
      s.print('#'); s.print(i); s.println(F(" unused"));
      continue;
    }
    bool raw = digitalRead(G[i].pin);
    bool broken = rawToBroken(raw, G[i].invert);
    s.print('#'); s.print(i);
    s.print(F(" pin=")); s.print(G[i].pin);
    s.print(F(" inv=")); s.print(G[i].invert ? F("Y") : F("N"));
    s.print(F(" state=")); s.print(broken ? F("BROKEN") : F("OK"));
    s.print(F(" armed=")); s.print(G[i].armed ? F("Y") : F("N"));
    s.print(F(" (event=")); s.print(G[i].event ? F("YES" ) : F("no" ));
    s.println(F(")"));
  }
  s.print(F("debounce=")); s.print(DEBOUNCE_MS); s.print(F("ms, rearm=")); s.print(REARM_MS); s.println(F("ms"));
}

void inputs_init() {
  // Optional: pinMode or setup logic here
}