#include <Arduino.h>
#include "console.hpp"
#include "settings.hpp"  // for SAVE/LOAD direct calls

// === Callback storage (wired by console_attach in main.cpp) ===
static void (*cbSetHold)(uint32_t)   = nullptr;
static void (*cbSetCool)(uint32_t)   = nullptr;
static void (*cbSetBright)(uint8_t)  = nullptr;
static void (*cbForce)(int)          = nullptr;
static void (*cbPrint)()             = nullptr;

// === Serial config ===
static unsigned long g_baud = 115200;
static String g_line;

// --- helpers ---
static String toUpperTrim(const String& in){
  String s = in;
  s.trim();
  s.toUpperCase();
  return s;
}

static long parseLong(const String& s, bool& ok){
  char *end=nullptr;
  long v = strtol(s.c_str(), &end, 10);
  ok = (end && *end == '\0');
  return v;
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
  Serial.println(F("  CFG                   - show status/settings"));
  Serial.println(F("  MAP                   - print beam->scene mapping"));
  Serial.println(F("  HOLD <ms>             - set hold duration"));
  Serial.println(F("  COOL <ms>             - set cooldown/lockout"));
  Serial.println(F("  BRIGHT <0..15>        - set display brightness"));
  Serial.println(F("  STATE <code>          - force state (0/1/2 or 10..18)"));
  Serial.println(F("  SCENE <name>          - jump to scene by name"));
  Serial.println(F("     names: STANDBY, PHONELOADING, INTRO, BLOODROOM,"));
  Serial.println(F("            GRAVEYARD, FURROOM, ORCADINO, FRANKENLAB, MIRRORROOM, EXITHOLE"));
  Serial.println(F("  SDEB <ms>             - set debounce ms (0..2000)"));
  Serial.println(F("  SREARM <ms>           - set re-arm ms (0..600000)"));
  Serial.println(F("  SAVE                  - save settings to EEPROM"));
  Serial.println(F("  LOAD                  - load settings from EEPROM"));
}

void console_update(){
  while (Serial.available()){
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c != '\n'){
      g_line += c;
      // basic line overflow protection
      if (g_line.length() > 200) g_line = "";
      continue;
    }

    // full line received
    String line = g_line;
    g_line = "";

    String uc = toUpperTrim(line);
    if (uc.length() == 0) continue;

    // HELP / ?
    if (uc == "?" || uc == "HELP"){
      printHelp();
      continue;
    }

    // CFG -> print status
    if (uc == "CFG"){
      if (cbPrint) cbPrint();
      else Serial.println(F("No printer attached"));
      continue;
    }

    // MAP -> reuse printer (it includes beam map now)
    if (uc == "MAP"){
      if (cbPrint) cbPrint();
      Serial.println(F("OK MAP"));
      continue;
    }

    // SAVE / LOAD (directly call settings module)
    if (uc == "SAVE"){
      settings_save();
      Serial.println(F("OK SAVE"));
      continue;
    }
    if (uc == "LOAD"){
      if (settings_load()) Serial.println(F("OK LOAD"));
      else                 Serial.println(F("ERR LOAD (invalid EEPROM)"));
      continue;
    }

    // HOLD <ms>
    if (uc.startsWith("HOLD ")){
      String a = toUpperTrim(line.substring(5));
      bool ok=false; long v = parseLong(a, ok);
      if (ok && v >= 0){
        if (cbSetHold) cbSetHold((uint32_t)v);
        Serial.println(F("OK HOLD"));
      } else Serial.println(F("ERR HOLD <ms>"));
      continue;
    }

    // COOL <ms>
    if (uc.startsWith("COOL ")){
      String a = toUpperTrim(line.substring(5));
      bool ok=false; long v = parseLong(a, ok);
      if (ok && v >= 0){
        if (cbSetCool) cbSetCool((uint32_t)v);
        Serial.println(F("OK COOL"));
      } else Serial.println(F("ERR COOL <ms>"));
      continue;
    }

    // BRIGHT <0..15>
    if (uc.startsWith("BRIGHT ")){
      String a = toUpperTrim(line.substring(7));
      bool ok=false; long v = parseLong(a, ok);
      if (ok && v >= 0 && v <= 15){
        if (cbSetBright) cbSetBright((uint8_t)v);
        Serial.println(F("OK BRIGHT"));
      } else Serial.println(F("ERR BRIGHT 0..15"));
      continue;
    }

    // SDEB <ms>
    if (uc.startsWith("SDEB ")){
      String a = toUpperTrim(line.substring(5));
      bool ok=false; long v = parseLong(a, ok);
      if (ok && v >= 0 && v <= 2000){
        settings_set_debounce((uint16_t)v); // applies via callback set in settings_begin
        Serial.println(F("OK SDEB"));
      } else Serial.println(F("ERR SDEB 0..2000"));
      continue;
    }

    // SREARM <ms>
    if (uc.startsWith("SREARM ")){
      String a = toUpperTrim(line.substring(7));
      bool ok=false; long v = parseLong(a, ok);
      if (ok && v >= 0 && v <= 600000){
        settings_set_rearm((uint32_t)v);
        Serial.println(F("OK SREARM"));
      } else Serial.println(F("ERR SREARM 0..600000"));
      continue;
    }

    // STATE <code>  (0/1/2 quick; 10..18 full list)
    if (uc.startsWith("STATE ")){
      String a = toUpperTrim(line.substring(6));
      bool ok=false; long v = parseLong(a, ok);
      if (ok){
        if (cbForce) cbForce((int)v);
        Serial.println(F("OK STATE"));
      } else Serial.println(F("ERR STATE <int>"));
      continue;
    }

    // SCENE <name>
    if (uc.startsWith("SCENE ")){
      String arg = toUpperTrim(line.substring(6));
      int code = -1;

      if      (arg == "STANDBY")        code = 0;
      else if (arg == "PHONELOADING")   code = 10;
      else if (arg == "INTRO" || arg == "INTRO CUE" || arg == "INTRO_CUE") code = 11;
      else if (arg == "BLOOD" || arg == "BLOODROOM" || arg == "BLOOD_ROOM") code = 12;
      else if (arg == "GRAVE" || arg == "GRAVEYARD") code = 13;
      else if (arg == "FUR" || arg == "FURROOM" || arg == "FUR_ROOM") code = 14;
      else if (arg == "ORCA" || arg == "ORCADINO" || arg == "ORCA_DINO") code = 15;
      else if (arg == "LAB" || arg == "FRANKENLAB" || arg == "FRANKENPHONES" || arg == "FRANKENPHONES LAB") code = 16;
      else if (arg == "MIRROR" || arg == "MIRRORROOM" || arg == "MIRROR_ROOM") code = 17;
      else if (arg == "EXIT" || arg == "EXITHOLE" || arg == "EXIT_HOLE") code = 18;

      if (code >= 0){
        if (cbForce) cbForce(code);
        Serial.println(F("OK SCENE"));
      } else {
        Serial.println(F("ERR SCENE name unknown"));
      }
      continue;
    }

    // Unknown command
    Serial.println(F("ERR ? for help"));
  }
}
