#include "settings.hpp"
#include <EEPROM.h>
#include <string.h> // for memcpy

// -------------------------------------------------------------------
// EEPROM layout: [MAGIC(2)][VERSION(1)][HHSettings struct][CRC16(2)]
// -------------------------------------------------------------------
static const uint16_t MAGIC   = 0x4848; // 'HH'
static const uint8_t  VERSION = 2;      // bump when HHSettings changes

// In-RAM copy
static HHSettings G;

// "Apply" callbacks provided by main.cpp
static ApplyU32 cbHold   = nullptr;
static ApplyU32 cbCool   = nullptr;
static ApplyU16 cbDeb    = nullptr;
static ApplyU32 cbRearm  = nullptr;
static ApplyU8  cbBright = nullptr;

// ---------------- CRC16 (Modbus-ish) ----------------
static uint16_t crc16(const uint8_t* d, size_t n){
  uint16_t c = 0xFFFF;
  for(size_t i=0;i<n;i++){
    c ^= (uint16_t)d[i];
    for(uint8_t b=0;b<8;b++){
      if (c & 1) c = (c >> 1) ^ 0xA001;
      else       c >>= 1;
    }
  }
  return c;
}

static void applyAll(){
  if (cbHold)   cbHold(G.hold_ms);
  if (cbCool)   cbCool(G.cooldown_ms);
  if (cbDeb)    cbDeb(G.debounce_ms);
  if (cbRearm)  cbRearm(G.rearm_ms);
  if (cbBright) cbBright(G.brightness);
}

// ---------------- Public API ----------------
void settings_begin(ApplyU32 applyHold,
                    ApplyU32 applyCooldown,
                    ApplyU16 applyDebounce,
                    ApplyU32 applyRearm,
                    ApplyU8  applyBrightness)
{
  cbHold   = applyHold;
  cbCool   = applyCooldown;
  cbDeb    = applyDebounce;
  cbRearm  = applyRearm;
  cbBright = applyBrightness;

  // Defaults (safe baseline)
  G.hold_ms     = 5000;
  G.cooldown_ms = 20000;
  G.debounce_ms = 30;
  G.rearm_ms    = 20000;
  G.brightness  = 8;

  // Default beam pins and scenes (codes; mapped to enum in main.cpp)
  const uint8_t defaultPins[6]  = {2,3,4,5,7,9};
  const uint8_t defaultScene[6] = {
    1, // IntroCue
    2, // BloodRoom
    3, // Graveyard
    4, // FurRoom
    6, // FrankenLab
    7  // MirrorRoom
  };
  memcpy(G.beam_pins,  defaultPins,  sizeof(G.beam_pins));
  memcpy(G.beam_scene, defaultScene, sizeof(G.beam_scene));

  // Try auto-load from EEPROM; if invalid, keep defaults
  if (!settings_load()){
    applyAll();
  }
}

void settings_save(){
  const int base = 0;
  int addr = base;

  EEPROM.put(addr, MAGIC);   addr += sizeof(MAGIC);
  EEPROM.put(addr, VERSION); addr += sizeof(VERSION);
  EEPROM.put(addr, G);       addr += sizeof(G);

  const uint16_t c = crc16(reinterpret_cast<const uint8_t*>(&G), sizeof(G));
  EEPROM.put(addr, c);
}

bool settings_load(){
  const int base = 0;
  int addr = base;

  uint16_t mg=0; uint8_t ver=0; HHSettings tmp; uint16_t c=0;
  EEPROM.get(addr, mg);   addr += sizeof(mg);
  EEPROM.get(addr, ver);  addr += sizeof(ver);
  EEPROM.get(addr, tmp);  addr += sizeof(tmp);
  EEPROM.get(addr, c);

  if (mg != MAGIC || ver != VERSION) return false;

  const uint16_t expect = crc16(reinterpret_cast<const uint8_t*>(&tmp), sizeof(tmp));
  if (expect != c) return false;

  G = tmp;
  applyAll();
  return true;
}

HHSettings& settings_ref(){ return G; }

// Tunable setters (update RAM + apply)
void settings_set_hold(uint32_t v){        G.hold_ms = v;        if (cbHold)   cbHold(v); }
void settings_set_cool(uint32_t v){        G.cooldown_ms = v;    if (cbCool)   cbCool(v); }
void settings_set_debounce(uint16_t v){    G.debounce_ms = v;    if (cbDeb)    cbDeb(v); }
void settings_set_rearm(uint32_t v){       G.rearm_ms = v;       if (cbRearm)  cbRearm(v); }
void settings_set_brightness(uint8_t v){   if (v>15) v=15; G.brightness = v; if (cbBright) cbBright(v); }

// Beam mapping setters (persisted via SAVE)
void settings_set_beam_pin(uint8_t idx, uint8_t pin){
  if (idx < 6) G.beam_pins[idx] = pin;
}
void settings_set_beam_scene(uint8_t idx, uint8_t scene_code){
  if (idx < 6) G.beam_scene[idx] = scene_code;
}

// Pretty print
void settings_print(Stream& s){
  s.println(F("== Settings =="));
  s.print(F("HOLD_MS="));     s.println(G.hold_ms);
  s.print(F("COOLDOWN_MS=")); s.println(G.cooldown_ms);
  s.print(F("DEBOUNCE_MS=")); s.println(G.debounce_ms);
  s.print(F("REARM_MS="));    s.println(G.rearm_ms);
  s.print(F("BRIGHTNESS="));  s.println(G.brightness);

  s.println(F("Beam map: idx : pin -> sceneCode"));
  for (uint8_t i=0;i<6;i++){
    s.print(F("  ")); s.print(i); s.print(F(" : "));
    s.print(G.beam_pins[i]); s.print(F(" -> "));
    s.println(G.beam_scene[i]);
  }
}