#pragma once
#include <Arduino.h>

// Callback types for applying settings at runtime
typedef void (*ApplyU32)(uint32_t);
typedef void (*ApplyU16)(uint16_t);
typedef void (*ApplyU8)(uint8_t);

// Persistent settings structure (EEPROM-backed)
struct HHSettings {
  uint32_t hold_ms;       // FrankenLab hold
  uint32_t cooldown_ms;   // re-arm lockout/cooldown
  uint16_t debounce_ms;   // input debounce
  uint32_t rearm_ms;      // re-arm delay
  uint8_t  brightness;    // 0..15 for 7-seg display

  uint8_t  beam_pins[6];  // Arduino digital pins for 6 beams
  uint8_t  beam_scene[6]; // Scene codes for each beam
};

// Access the in-RAM copy
HHSettings& settings_ref();

// Initialize settings and attempt EEPROM load; apply callbacks on success or defaults
void settings_begin(ApplyU32 applyHold,
                    ApplyU32 applyCooldown,
                    ApplyU16 applyDebounce,
                    ApplyU32 applyRearm,
                    ApplyU8  applyBrightness);

// Save/load from EEPROM
void settings_save();
bool settings_load();

// NEW: factory reset to defaults (and optionally SAVE immediately)
void settings_reset_defaults(bool save);

// NEW: expose versions for the VER console command
// magic: EEPROM magic (0x4848), eepromVer: layout version, fwVer: firmware code version
void settings_versions(uint16_t &magic, uint8_t &eepromVer, uint8_t &fwVer);

// Setters that update RAM and invoke apply-callbacks
void settings_set_hold(uint32_t v);
void settings_set_cool(uint32_t v);
void settings_set_debounce(uint16_t v);
void settings_set_rearm(uint32_t v);
void settings_set_brightness(uint8_t v);

// Beam mapping setters (persisted by SAVE)
void settings_set_beam_pin(uint8_t idx, uint8_t pin);
void settings_set_beam_scene(uint8_t idx, uint8_t scene_code);

// Pretty print
void settings_print(Stream& s);