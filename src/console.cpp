// src/console.cpp
#include <Arduino.h>
#include "console.hpp"
#include "pins.hpp"
#include "scenes/scene_frankenphone.hpp"

// scene-side mute bridge
extern "C" void frankenphone_set_mute(bool m);

static String inbuf;

// helpers
static void print_kv(const __FlashStringHelper* k, int v) {
  Serial.print(k); Serial.println(v);
}

// commands
static void cmd_help() {
  Serial.println(F("Commands:"));
  Serial.println(F("  ? | HELP         show this help"));
  Serial.println(F("  VER              print firmware version"));
  Serial.println(F("  CFG              print pin map and live states"));
  Serial.println(F("  STATE 16         force Frankenphones Lab now"));
  Serial.println(F("  QUIET ON|OFF     mute or unmute buzzer"));
}

static void cmd_ver() {
  Serial.println(F("Haunted Hearse build:"));
  Serial.println(F("  frankenphone-locked-2025-09-14-09sA"));
  Serial.println(F("OK VER"));
}

static void cmd_cfg() {
  Serial.println(F("=== CFG ==="));
  Serial.println(F("Pins"));
  Serial.println(F("  Beam0: D2 (INPUT_PULLUP, active LOW)"));
  print_kv(F("  Magnet D"), PIN_MAGNET_CTRL);
  print_kv(F("  Buzzer D"), PIN_BUZZER);
  print_kv(F("  LED Green D"), LED_ARMED);
  print_kv(F("  LED Red   D"), LED_HOLD);
  print_kv(F("  LED Yell  D"), LED_COOLDOWN);
  Serial.println(F("  I2C display 0x70 on SDA=20 SCL=21"));
  Serial.println(F("States"));
  Serial.print(F("  Beam0: ")); Serial.println(digitalRead(PIN_BEAM_0) == LOW ? F("BROKEN") : F("CLEAR"));
  Serial.print(F("  Magnet: ")); Serial.println(digitalRead(PIN_MAGNET_CTRL) ? F("ON") : F("OFF"));
  Serial.println(F("  Note: buzzer is tonal and not readable via digitalRead"));
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

static void handle_line(String line) {
  line.trim();
  if (line.length() == 0) return;
  Serial.print(F("> ")); Serial.println(line);  // echo

  String up = line; up.toUpperCase();

  if (up == "?" || up == "HELP") { cmd_help(); return; }
  if (up == "VER")               { cmd_ver();  return; }
  if (up == "CFG")               { cmd_cfg();  return; }
  if (up.startsWith("STATE"))    { cmd_state(up); return; }
  if (up.startsWith("QUIET"))    { cmd_quiet(up); return; }

  Serial.println(F("ERR unknown"));
}

// public API
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