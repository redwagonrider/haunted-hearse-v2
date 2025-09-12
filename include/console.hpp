#pragma once
#include <Arduino.h>

// ============================================================
// Haunted Hearse Console Interface
// ============================================================
//
// Commands (case-insensitive):
//   HELP / ?
//   CFG, MAP
//   HOLD <ms>, COOL <ms>, BRIGHT <0..15>
//   SDEB <ms>, SREARM <ms>
//   SAVE, LOAD
//   STATE <code>, SCENE <name>
//   BMAP <idx> <pin>         (NEW) beam index -> Arduino pin
//   BSCENE <idx> <name|code> (NEW) beam index -> scene
//   LOG ON|OFF               (NEW) enable event logging
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

// NEW: logging flag getter for main.cpp
bool console_should_log();