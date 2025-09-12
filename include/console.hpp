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
//   VER                     (NEW) print versions
//   RESET                   (NEW) factory reset settings + mapping
//   STATE <code>, SCENE <name>
//   BMAP <idx> <pin>        set beam index -> Arduino pin
//   BSCENE <idx> <name|code> set beam index -> scene
//   LOG ON|OFF              toggle event logging
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

// NEW: attach recorder/gopro hooks (call in main.cpp)
void console_attach_devices(
  void (*gopro_on)(),
  void (*gopro_off)(),
  void (*gopro_ms)(uint32_t),
  void (*audio_on)(),
  void (*audio_off)(),
  void (*audio_ms)(uint32_t)
);

// Call every loop() to process serial input
void console_update();

// Logging flag getter for main.cpp
bool console_should_log();