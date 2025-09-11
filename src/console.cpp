#include "console.hpp"

static SetU32       cbSetHold   = nullptr;
static SetU32       cbSetCool   = nullptr;
static SetU8        cbSetBright = nullptr;
static ForceStateFn cbForce     = nullptr;
static PrintFn      cbPrint     = nullptr;

static String line;

static void printHelp(){
  Serial.println(F("\n=== Haunted Hearse Console ==="));
  Serial.println(F("?                 : Help"));
  Serial.println(F("CFG               : Show config"));
  Serial.println(F("HOLD <ms>         : Set hold duration (e.g. HOLD 5000)"));
  Serial.println(F("COOL <ms>         : Set cooldown duration"));
  Serial.println(F("BRIGHT <0..15>    : Set display brightness"));
  Serial.println(F("STATE IDLE|HOLD|COOLDOWN : Force state"));
  Serial.println();
}

void console_begin(uint32_t baud){
  Serial.begin(baud);
  // small delay for USB CDC to enumerate (optional)
  delay(50);
  printHelp();
}

void console_attach(SetU32 setHoldMs,
                    SetU32 setCooldownMs,
                    SetU8  setBrightness,
                    ForceStateFn forceState,
                    PrintFn printStatus)
{
  cbSetHold   = setHoldMs;
  cbSetCool   = setCooldownMs;
  cbSetBright = setBrightness;
  cbForce     = forceState;
  cbPrint     = printStatus;
}

static void trim(String &s){
  s.trim();
  // compress multiple spaces
  String out; out.reserve(s.length());
  bool ws=false;
  for (size_t i=0;i<s.length();++i){
    char c=s[i];
    if (c==' ' || c=='\t' || c=='\r') { if (!ws){ out+=' '; ws=true; } }
    else { out+=c; ws=false; }
  }
  s = out;
}

static long parseLong(const String &s, bool &ok){
  char *end=nullptr;
  long v = strtol(s.c_str(), &end, 10);
  ok = (end && *end=='\0');
  return v;
}

void console_update(){
  while (Serial.available()){
    char c = (char)Serial.read();
    if (c == '\n'){
      String cmd = line;
      line = "";
      trim(cmd);
      if (cmd.length()==0) return;

      // Uppercase copy for command matching
      String uc = cmd; uc.toUpperCase();

      if (uc == "?" || uc == "HELP"){
        printHelp();
        return;
      }
      if (uc == "CFG"){
        if (cbPrint) cbPrint();
        return;
      }
      if (uc.startsWith("HOLD ")){
        String arg = cmd.substring(5); arg.trim();
        bool ok=false; long v = parseLong(arg, ok);
        if (ok && v>=0){ if (cbSetHold) cbSetHold((uint32_t)v); Serial.println(F("OK HOLD")); }
        else Serial.println(F("ERR HOLD <ms>"));
        return;
      }
      if (uc.startsWith("COOL ")){
        String arg = cmd.substring(5); arg.trim();
        bool ok=false; long v = parseLong(arg, ok);
        if (ok && v>=0){ if (cbSetCool) cbSetCool((uint32_t)v); Serial.println(F("OK COOL")); }
        else Serial.println(F("ERR COOL <ms>"));
        return;
      }
      if (uc.startsWith("BRIGHT ")){
        String arg = cmd.substring(7); arg.trim();
        bool ok=false; long v = parseLong(arg, ok);
        if (ok && v>=0 && v<=15){ if (cbSetBright) cbSetBright((uint8_t)v); Serial.println(F("OK BRIGHT")); }
        else Serial.println(F("ERR BRIGHT <0..15>"));
        return;
      }
      if (uc.startsWith("STATE ")){
        String arg = uc.substring(6); arg.trim();
        int st = -1;
        if (arg=="IDLE") st = 0;
        else if (arg=="HOLD") st = 1;
        else if (arg=="COOLDOWN") st = 2;
        if (st>=0){ if (cbForce) cbForce(st); Serial.println(F("OK STATE")); }
        else Serial.println(F("ERR STATE IDLE|HOLD|COOLDOWN"));
        return;
      }
            if (uc == "SAVE"){
        // We can’t include settings.hpp here without a circular dep,
        // so expose thin shims in main OR call a function pointer.
        // Easiest: we’ll have main attach a PrintFn that prints settings,
        // and we just invoke a global weak function. Simpler solution:
        extern void settings_save();  // forward from settings.cpp linkage
        settings_save();
        Serial.println(F("OK SAVE"));
        return;
      }

      if (uc == "LOAD"){
        extern bool settings_load();
        if (settings_load()){
          Serial.println(F("OK LOAD"));
        } else {
          Serial.println(F("ERR LOAD (invalid EEPROM)"));
        }
        return;
      }
            if (uc.startsWith("SDEB ")){ // SDEB <ms>
        String arg = uc.substring(5); arg.trim();
        long v = strtol(arg.c_str(), nullptr, 10);
        if (v >= 0 && v <= 2000){
          extern void settings_set_debounce(uint16_t);
          settings_set_debounce((uint16_t)v);
          Serial.println(F("OK SDEB"));
        } else Serial.println(F("ERR SDEB <0..2000>"));
        return;
      }

      if (uc.startsWith("SREARM ")){ // SREARM <ms>
        String arg = uc.substring(7); arg.trim();
        long v = strtol(arg.c_str(), nullptr, 10);
        if (v >= 0 && v <= 600000){
          extern void settings_set_rearm(uint32_t);
          settings_set_rearm((uint32_t)v);
          Serial.println(F("OK SREARM"));
        } else Serial.println(F("ERR SREARM <0..600000>"));
        return;
      }
            if (uc == "MAP"){
        if (cbPrint) cbPrint();     // reuse your existing printer from main.cpp
        Serial.println(F("OK MAP"));
        return;
      }
            // SCENE <name>  (name is case-insensitive; examples below)
      if (uc.startsWith("SCENE ")){
        String arg = uc.substring(6); arg.trim();  // uppercase already
        int code = -1;

        // Map scene NAME -> integer code (these codes are implemented in main.cpp::forceState)
        if      (arg == "STANDBY")       code = 0;
        else if (arg == "PHONELOADING")  code = 10;
        else if (arg == "INTRO" || arg == "INTRO CUE" || arg == "INTROcue" || arg == "INTRO_CUE" || arg == "INTROCUe" || arg == "INTRO_CUE") code = 11;
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
          Serial.println(F("ERR SCENE name unknown. Try: STANDBY, PHONELOADING, INTRO, BLOODROOM, GRAVEYARD, FURROOM, ORCADINO, FRANKENLAB, MIRRORROOM, EXITHOLE"));
        }
        return;
      }

      Serial.println(F("ERR ? for help"));
    } else if (c != '\r'){
      line += c;
      // prevent runaway
      if (line.length() > 80) line.remove(0, line.length()-80);
    }
  }
}