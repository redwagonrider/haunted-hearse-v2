#pragma once
#include <Arduino.h>

struct HHSettings {
  uint32_t hold_ms;         // FrankenLab hold
  uint32_t cooldown_ms;     // general cooldown/lockout
  uint16_t debounce_ms;     // sensor debounce
  uint32_t rearm_ms;        // sensor re-arm/lockout
  uint8_t  brightness;      // 0..15 display brightness

  uint8_t  beam_pins[6];    // digital pins for 6 break-beams (0xFF = unused)
  uint8_t  beam_scene[6];   // scene code per beam (enum value as uint8_t)
};

// Apply callbacks
typedef void (*ApplyU32)(uint32_t);
typedef void (*ApplyU16)(uint16_t);
typedef void (*ApplyU8)(uint8_t);

void settings_begin(ApplyU32 applyHold,
                    ApplyU32 applyCooldown,
                    ApplyU16 applyDebounce,
                    ApplyU32 applyRearm,
                    ApplyU8  applyBrightness);

void settings_save();
bool settings_load();

HHSettings& settings_ref();

// Individual setters (update RAM + call apply cb if relevant)
void settings_set_hold(uint32_t v);
void settings_set_cool(uint32_t v);
void settings_set_debounce(uint16_t v);
void settings_set_rearm(uint32_t v);
void settings_set_brightness(uint8_t v);

// Beam mapping setters
void settings_set_beam_pin(uint8_t idx, uint8_t pin);
void settings_set_beam_scene(uint8_t idx, uint8_t scene_code);

// Pretty print
void settings_print(Stream& s);