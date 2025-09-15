// src/console.cpp
#include <Arduino.h>
#include "console.hpp"
#include "pins.hpp"
#include "triggers.hpp"
#include "scenes/scene_frankenphone.hpp"
#include "techlight.hpp"
#include "inputs.hpp"

// scene-side mute bridge
extern "C" void frankenphone_set_mute(bool m);

static String inbuf;

static void print_kv(const __FlashStringHelper* k, int v) {
  Serial.print(k); Serial.println(v);
}

static void cmd_help() {
  Serial.println(F("Commands:"));
  Serial.println(F("  ? | HELP           show this help"));
  Serial.println(F("  VER                print firmware version"));
  Serial.println(F("  CFG                print pins and live states"));
  Serial.println(F("  MAP                print beam -> scene map"));
  Serial.println(F("  STATE 16           force Frankenphones Lab now"));
  Serial.println(F("  QUIET ON|OFF       mute or unmute buzzer"));
  Serial.println(F("  TRIG LIST          show GPIO trigger mapping"));
  Serial.println(F("  TRIG <ROOM>        pulse GPIO for BLOOD|GRAVE|FUR|FRANKEN"));
  Serial.println(F("  TRIG ALL           pulse all in sequence"));
  Serial.println(F("  LIGHT ON|OFF|AUTO|TOGGLE   tech booth light override"));
}

static void cmd_ver() {
  Serial.println(F("Haunted Hearse build:"));
  Serial.println(F("  frankenphone-locked-2025-09-14-09sA"));
  Serial.println(F("OK VER"));
}

static void cmd_cfg() {
  Serial.println(F("=== CFG ==="));
  Serial.println(F("Pins"));
  Serial.println(F("  Beam0..5: D2 D3 D4 D5 D7 D9 (INPUT_PULLUP, active LOW)"));
  print_kv(F("  Beam6 (reed) D"), PIN_BEAM_6);
  print_kv(F("  Magnet D"), PIN_MAGNET_CTRL);
  print_kv(F("  Buzzer D"), PIN_BUZZER);
  print_kv(F("  LED Green D"), LED_ARMED);
  print_kv(F("  LED Red   D"), LED_HOLD);
  print_kv(F("  LED Yell  D"), LED_COOLDOWN);
  print_kv(F("  TechLight OUT D"), PIN_TECHLIGHT);
  Serial.println(F("  I2C display 0x70 on SDA=20 SCL=21"));

  Serial.println(F("States"));
  Serial.print(F("  Beam0: ")); Serial.println(digitalRead(PIN_BEAM_0) == LOW ? F("BROKEN") : F("CLEAR"));
  Serial.print(F("  Beam1: ")); Serial.println(digitalRead(PIN_BEAM_1) == LOW ? F("BROKEN") : F("CLEAR"));
  Serial.print(F("  Beam2: ")); Serial.println(digitalRead(PIN_BEAM_2) == LOW ? F("BROKEN") : F("CLEAR"));
  Serial.print(F("  Beam3: ")); Serial.println(digitalRead(PIN_BEAM_3) == LOW ? F("BROKEN") : F("CLEAR"));
  Serial.print(F("  Beam4: ")); Serial.println(digitalRead(PIN_BEAM_4) == LOW ? F("BROKEN") : F("CLEAR"));
  Serial.print(F("  Beam5: ")); Serial.println(digitalRead(PIN_BEAM_5) == LOW ? F("BROKEN") : F("CLEAR"));
  Serial.print(F("  Reed(B6): ")); Serial.println(digitalRead(PIN_BEAM_6) == LOW ? F("CLOSED") : F("OPEN"));
  Serial.print(F("  Magnet: ")); Serial.println(digitalRead(PIN_MAGNET_CTRL) ? F("ON") : F("OFF"));
  Serial.print(F("  TechLight: ")); Serial.println(techlight_is_on() ? F("ON") : F("OFF"));
  Serial.print(F("  TechLight mode: ")); Serial.println(techlight_mode_name());
  Serial.println(F("OK CFG"));
}

static void cmd_state(const String& s) {
  int sp = s.indexOf(' ');
  if (sp < 0) { Serial.println(F("ERR STATE")); return; }
  int code = s.substring(sp + 1).toInt();
  if (code == 16) {
    scene_frankenphone();
    Serial.println(F("OK STATE 16"));
  } else {
    Serial.println(F("ERR STATE unsupported"));
  }
}

static void cmd_quiet(const String& s) {
  if (s.endsWith(" ON"))  { frankenphone_set_mute(true);  Serial.println(F("OK QUIET ON"));  return; }
  if (s.endsWith(" OFF")) { frankenphone_set_mute(false); Serial.println(F("OK QUIET OFF")); return; }
  Serial.println(F("ERR QUIET use ON or OFF"));
}

static void cmd_trig(const String& s) {
  if (s.endsWith(" LIST")) { triggers_print_map(); Serial.println(F("OK TRIG LIST")); return; }

  if (s.endsWith(" ALL")) {
    const char* names[4] = {"BLOOD","GRAVE","FUR","FRANKEN"};
    for (uint8_t i=0;i<4;i++) {
      triggers_pulse_by_name(String(names[i]));
      Serial.print(F("TRIG ")); Serial.println(names[i]);
      delay(500);
    }
    Serial.println(F("OK TRIG ALL"));
    return;
  }

  int sp = s.indexOf(' ');
  if (sp < 0 || sp+1 >= (int)s.length()) { Serial.println(F("ERR TRIG")); return; }
  String name = s.substring(sp + 1);
  name.trim();
  name.toUpperCase();
  if (triggers_pulse_by_name(name)) {
    Serial.print(F("OK TRIG ")); Serial.println(name);
  } else {
    Serial.println(F("ERR TRIG name (use BLOOD|GRAVE|FUR|FRANKEN)"));
  }
}

static void handle_line(String line) {
  line.trim();
  if (line.length() == 0) return;
  Serial.print(F("> ")); Serial.println(line);

  String up = line; up.toUpperCase();

  if (up == "?" || up == "HELP") { cmd_help(); return; }
  if (up == "VER")               { cmd_ver();  return; }
  if (up == "CFG")               { cmd_cfg();  return; }
  if (up == "MAP")               { inputs_print_map(); Serial.println(F("OK MAP")); return; }
  if (up.startsWith("STATE"))    { cmd_state(up); return; }
  if (up.startsWith("QUIET"))    { cmd_quiet(up); return; }
  if (up.startsWith("TRIG"))     { cmd_trig(up); return; }

  Serial.println(F("ERR unknown"));
}

void console_update() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') { handle_line(inbuf); inbuf = ""; }
    else if (inbuf.length() < 96) inbuf += c;
  }
}

void console_log(const char* msg)   { Serial.println(msg); }
void console_log(const String& msg) { Serial.println(msg); }