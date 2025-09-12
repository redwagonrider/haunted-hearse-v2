#include <Arduino.h>
#include "console.hpp"
#include "settings.hpp"  // SAVE/LOAD, versions, reset
// main.cpp provides this to re-apply mapping after edits:
extern void apply_mapping_from_settings();

// === Callback storage (wired by console_attach in main.cpp) ===
static void (*cbSetHold)(uint32_t)   = nullptr;
static void (*cbSetCool)(uint32_t)   = nullptr;
static void (*cbSetBright)(uint8_t)  = nullptr;
static void (*cbForce)(int)          = nullptr;
static void (*cbPrint)()             = nullptr;

// === Serial config ===
static unsigned long g_baud = 115200;
static String g_line;

// === Log mode flag ===
static bool g_log = false;
bool console_should_log(){ return g_log; }

// --- helpers ---
static String toUpperTrim(const String& in){
  String s = in; s.trim(); s.toUpperCase(); return s;
}

static long parseLong(const String& s, bool& ok){
  char *end=nullptr;
  long v = strtol(s.c_str(), &end, 10);
  ok = (end && *end == '\0');
  return v;
}

static int nameToSceneCode(const String& n){
  String arg = toUpperTrim(n);
  if      (arg == "STANDBY")        return 0;
  else if (arg == "PHONELOADING")   return 10;
  else if (arg == "INTRO" || arg == "INTRO CUE" || arg == "INTRO_CUE") return 11;
  else if (arg == "BLOOD" || arg == "BLOODROOM" || arg == "BLOOD_ROOM") return 12;
  else if (arg == "GRAVE" || arg == "GRAVEYARD") return 13;
  else if (arg == "FUR" || arg == "FURROOM" || arg == "FUR_ROOM") return 14;
  else if (arg == "ORCA" || arg == "ORCADINO" || arg == "ORCA_DINO") return 15;
  else if (arg == "LAB" || arg == "FRANKENLAB" || arg == "FRANKENPHONES" || arg == "FRANKENPHONES LAB") return 16;
  else if (arg == "MIRROR" || arg == "MIRRORROOM" || arg == "MIRROR_ROOM") return 17;
  else if (arg == "EXIT" || arg == "EXITHOLE" || arg == "EXIT_HOLE") return 18;
  return -1;
}

void console_begin(unsigned long baud){
  g_baud = baud;
  Serial.begin(g_baud);
  g_line.reserve(96);
  delay(50);
  Serial.println(F("\n[HH Console Ready] Type ? for help"));
}

void console_attach(void (*setHold)(uint32_t),
                    void (*setCool)(uint32_t),
                    void (*setBright)(uint8_t),
                    void (*forceState)(int),
                    void (*printStatus)()){
  cbSetHold   = setHold;
  cbSetCool   = setCool;
  cbSetBright = setBright;
  cbForce     = forceState;
  cbPrint     = printStatus;
}

static void printHelp(){
  Serial.println(F("Commands:"));
  Serial.println(F("  ? or HELP             - show this help"));
  Serial.println(F("  VER                   - show firmware/EEPROM versions"));
  Serial.println(F("  CFG                   - show status/settings"));
  Serial.println(F("  MAP                   - print beam->scene mapping"));
  Serial.println(F("  HOLD <ms>             - set hold duration"));
  Serial.println(F("  COOL <ms>             - set cooldown/lockout"));
  Serial.println(F("  BRIGHT <0..15>        - set display brightness"));
  Serial.println(F("  SDEB <ms>             - set debounce (0..2000)"));
  Serial.println(F("  SREARM <ms>           - set re-arm (0..600000)"));
  Serial.println(F("  SAVE / LOAD           - EEPROM persist / restore"));
  Serial.println(F("  RESET                 - factory reset (defaults)"));
  Serial.println(F("  STATE <code>          - quick force (0/1/2 or 10..18)"));
  Serial.println(F("  SCENE <name>          - force by name"));
  Serial.println(F("  BMAP <idx> <pin>      - set beam index->Arduino pin"));
  Serial.println(F("  BSCENE <idx> <name|code> - set beam index->scene"));
  Serial.println(F("  LOG ON|OFF            - toggle event logging"));
}

void console_update(){
  while (Serial.available()){
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c != '\n'){
      g_line += c;
      if (g_line.length() > 200) g_line = "";
      continue;
    }

    // full line received
    String line = g_line; g_line = "";
    String uc = toUpperTrim(line);
    if (uc.length() == 0) continue;

    // HELP / ?
    if (uc == "?" || uc == "HELP"){ printHelp(); continue; }

    // VER
    if (uc == "VER"){
      uint16_t magic; uint8_t eep, fw;
      settings_versions(magic, eep, fw);
      Serial.print(F("FW=")); Serial.print(fw);
      Serial.print(F(" EEP_VER=")); Serial.print(eep);
      Serial.print(F(" MAGIC=0x")); Serial.println(magic, HEX);
      continue;
    }

    // CFG / MAP
    if (uc == "CFG"){ if (cbPrint) cbPrint(); else Serial.println(F("No printer")); continue; }
    if (uc == "MAP"){ if (cbPrint) cbPrint(); Serial.println(F("OK MAP")); continue; }

    // SAVE / LOAD / RESET
    if (uc == "SAVE"){ settings_save(); Serial.println(F("OK SAVE")); continue; }
    if (uc == "LOAD"){ if (settings_load()) Serial.println(F("OK LOAD")); else Serial.println(F("ERR LOAD")); continue; }
    if (uc == "RESET"){
      settings_reset_defaults(true); // reset & SAVE
      apply_mapping_from_settings();
      Serial.println(F("OK RESET (defaults applied + saved)"));
      continue;
    }

    // LOG ON|OFF
    if (uc.startsWith("LOG ")){
      String a = toUpperTrim(line.substring(4));
      if (a == "ON"){ g_log = true; Serial.println(F("OK LOG ON")); }
      else if (a == "OFF"){ g_log = false; Serial.println(F("OK LOG OFF")); }
      else Serial.println(F("ERR LOG ON|OFF"));
      continue;
    }

    // HOLD / COOL / BRIGHT
    if (uc.startsWith("HOLD ")){
      bool ok=false; long v = parseLong(line.substring(5), ok);
      if (ok && v >= 0){ if (cbSetHold) cbSetHold((uint32_t)v); Serial.println(F("OK HOLD")); }
      else Serial.println(F("ERR HOLD <ms>"));
      continue;
    }
    if (uc.startsWith("COOL ")){
      bool ok=false; long v = parseLong(line.substring(5), ok);
      if (ok && v >= 0){ if (cbSetCool) cbSetCool((uint32_t)v); Serial.println(F("OK COOL")); }
      else Serial.println(F("ERR COOL <ms>"));
      continue;
    }
    if (uc.startsWith("BRIGHT ")){
      bool ok=false; long v = parseLong(line.substring(7), ok);
      if (ok && v >= 0 && v <= 15){ if (cbSetBright) cbSetBright((uint8_t)v); Serial.println(F("OK BRIGHT")); }
      else Serial.println(F("ERR BRIGHT 0..15"));
      continue;
    }

    // SDEB / SREARM
    if (uc.startsWith("SDEB ")){
      bool ok=false; long v = parseLong(line.substring(5), ok);
      if (ok && v >= 0 && v <= 2000){ settings_set_debounce((uint16_t)v); Serial.println(F("OK SDEB")); }
      else Serial.println(F("ERR SDEB 0..2000"));
      continue;
    }
    if (uc.startsWith("SREARM ")){
      bool ok=false; long v = parseLong(line.substring(7), ok);
      if (ok && v >= 0 && v <= 600000){ settings_set_rearm((uint32_t)v); Serial.println(F("OK SREARM")); }
      else Serial.println(F("ERR SREARM 0..600000"));
      continue;
    }

    // STATE <code>
    if (uc.startsWith("STATE ")){
      bool ok=false; long v = parseLong(line.substring(6), ok);
      if (ok){ if (cbForce) cbForce((int)v); Serial.println(F("OK STATE")); }
      else Serial.println(F("ERR STATE <int>"));
      continue;
    }

    // SCENE <name>
    if (uc.startsWith("SCENE ")){
      int code = nameToSceneCode(line.substring(6));
      if (code >= 0){ if (cbForce) cbForce(code); Serial.println(F("OK SCENE")); }
      else Serial.println(F("ERR SCENE name unknown"));
      continue;
    }

    // BMAP <idx> <pin>
    if (uc.startsWith("BMAP ")){
      String rest = line.substring(5); rest.trim();
      int sp = rest.indexOf(' ');
      if (sp < 0){ Serial.println(F("ERR BMAP <idx> <pin>")); continue; }
      String sIdx = rest.substring(0, sp);
      String sPin = rest.substring(sp+1);
      bool ok1=false, ok2=false;
      long idx = parseLong(sIdx, ok1);
      long pin = parseLong(sPin, ok2);
      if (!ok1 || !ok2 || idx < 0 || idx > 5 || pin < 0 || pin > 69){
        Serial.println(F("ERR BMAP idx=0..5 pin=0..69"));
        continue;
      }
      settings_set_beam_pin((uint8_t)idx, (uint8_t)pin);
      apply_mapping_from_settings();
      Serial.println(F("OK BMAP"));
      continue;
    }

    // BSCENE <idx> <name|code>
    if (uc.startsWith("BSCENE ")){
      String rest = line.substring(7); rest.trim();
      int sp = rest.indexOf(' ');
      if (sp < 0){ Serial.println(F("ERR BSCENE <idx> <name|code>")); continue; }
      String sIdx = rest.substring(0, sp);
      String sArg = rest.substring(sp+1);
      bool ok=false; long idx = parseLong(sIdx, ok);
      if (!ok || idx < 0 || idx > 5){ Serial.println(F("ERR BSCENE idx=0..5")); continue; }

      int code;
      bool numOK=false; long asNum = parseLong(sArg, numOK);
      if (numOK) code = (int)asNum;
      else       code = nameToSceneCode(sArg);

      if (code < 0 || code > 255){ Serial.println(F("ERR BSCENE name/code invalid")); continue; }
      settings_set_beam_scene((uint8_t)idx, (uint8_t)code);
      apply_mapping_from_settings();
      Serial.println(F("OK BSCENE"));
      continue;
    }

    // Unknown
    Serial.println(F("ERR ? for help"));
  }
}