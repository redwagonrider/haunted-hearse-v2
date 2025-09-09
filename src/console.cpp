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

      Serial.println(F("ERR ? for help"));
    } else if (c != '\r'){
      line += c;
      // prevent runaway
      if (line.length() > 80) line.remove(0, line.length()-80);
    }
  }
}