#pragma once
#include <Arduino.h>

// ============================================================
// Haunted Hearse Console Interface
// ============================================================
//
// Provides a simple serial command parser for runtime control.
// Hooked into main.cpp with callback functions.
//
// Supported commands (case-insensitive):
//   ? or HELP             - show help
//   CFG                   - print current settings/status
//   MAP                   - print beam->scene mapping
//   HOLD <ms>             - set hold duration (FrankenLab magnet/buzzer)
//   COOL <ms>             - set cooldown/lockout duration
//   BRIGHT <0..15>        - set display brightness
//   STATE <code>          - force scene/state by integer code
//   SCENE <name>          - jump to scene by name (STANDBY, BLOODROOM, etc.)
//   SDEB <ms>             - set debounce (0..2000 ms)
//   SREARM <ms>           - set re-arm lockout (0..600000 ms)
//   SAVE                  - save settings to EEPROM
//   LOAD                  - load settings from EEPROM
//
// ============================================================

// Initialize the console at a given baud rate
void console_begin(unsigned long baud = 115200);

// Attach application callbacks (set in main.cpp):
//   setHold(ms)     -> update hold duration
//   setCool(ms)     -> update cooldown duration
//   setBright(b)    -> update display brightness (0..15)
//   forceState(int) -> force a scene/state code
//   printStatus()   -> print current config/status
void console_attach(
  void (*setHold)(uint32_t),
  void (*setCool)(uint32_t),
  void (*setBright)(uint8_t),
  void (*forceState)(int),
  void (*printStatus)()
);

// Call every loop() to process serial input
void console_update();
