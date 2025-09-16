# AI Scene Authoring Guide — Haunted Hearse v2

This guide captures the exact interfaces your scenes use today so new scenes can be added without breaking existing behavior.

## Scope
- Use these headers as the source of truth
  - `include/triggers.hpp` — Mega to Pi trigger pulses by index or name
  - `include/techlight.hpp` — Tech booth light override and auto
  - `include/telemetry.hpp` — Serial Studio v3 frame emitter
  - `include/display.hpp` — 4 digit I2C display with ownership arbitration
  - `src/scenes.cpp` — canon list of scenes and numeric codes
- Pins live in `include/pins.hpp` which defines concrete pin numbers and helper macros

> If you add a new scene, prefer the templates in `templates/` then register the function in `src/scenes.cpp` and in any mapping code that routes beams to scenes.

---

## Pins and helpers — expected from `include/pins.hpp`

Your scenes expect these symbols to exist in `include/pins.hpp`.

```cpp
// Beams
extern const uint8_t PIN_BEAM_0; // D2 Frankenphone
extern const uint8_t PIN_BEAM_1; // D3 Intro
extern const uint8_t PIN_BEAM_2; // D4 Blood
extern const uint8_t PIN_BEAM_3; // D5 Graveyard
extern const uint8_t PIN_BEAM_4; // D7 Mirror
extern const uint8_t PIN_BEAM_5; // D9 Exit
extern const uint8_t PIN_BEAM_6; // optional reed techlight

// Props
extern const uint8_t PIN_MAGNET_CTRL; // D6
extern const uint8_t PIN_BUZZER;      // D8

// LEDs
extern const uint8_t LED_ARMED;       // D10
extern const uint8_t LED_HOLD;        // D11
extern const uint8_t LED_COOLDOWN;    // D12

// Work light and camera relay
extern const uint8_t PIN_WORKLIGHT;
extern const uint8_t PIN_RELAY_GOPRO;

// Relay helpers
#define RELAY_ON(p)  digitalWrite((p), HIGH)
#define RELAY_OFF(p) digitalWrite((p), LOW)
```

Do not redefine these in scenes.

---

## Triggers API — from `include/triggers.hpp`

```cpp
void triggers_begin();

bool triggers_pulse(uint8_t idx, uint16_t ms = 100);
// Example: triggers_pulse(0, 150);

bool triggers_pulse_by_name(const String& upname);
// Names are uppercase such as "SHOW" "BLOOD" "GRAVE" "FUR" "FRANKEN"
// Project specific additions used today:
//   "DR40X_REC_START" and "DR40X_REC_STOP" for the Tascam start or stop

void triggers_print_map();
```

Usage pattern
```cpp
triggers_pulse_by_name(F("DR40X_REC_START"));
triggers_pulse_by_name(F("Start_Graveyard")); // optional Pi overlay
```

---

## TechLight API — from `include/techlight.hpp`

```cpp
void techlight_override_on();
void techlight_override_off();
void techlight_override_auto();        // follow reed switch

void techlight_scene_intro_kill(uint16_t ms_holdoff = 5000);

bool        techlight_is_on();
const char* techlight_mode_name();     // "AUTO" or "FORCE_ON" or "FORCE_OFF"
```

Rules in play
- Intro and cue card force TechLight OFF for a short window using `techlight_scene_intro_kill`
- Console commands can force ON or OFF which overrides the reed input

---

## Telemetry API — from `include/telemetry.hpp`

Frames for Serial Studio v3 use a CSV inside `/* ... */`
```
/*millis,scene,phase,beam,magnet,buzzer,led_r,led_g,led_b*/
```

Emit
```cpp
#include "telemetry.hpp"
using HH::tel_emit;

tel_emit("graveyard", "START", 0, 0, 0, 0, 0, 0);
```

There is also `HH::tel_begin(115200)` which is already called in your project bootstrap.

---

## Display API with arbitration — from `include/display.hpp`

```cpp
bool display_acquire(const char* owner, uint8_t priority, uint32_t hold_ms = 0);
bool display_renew(const char* owner, uint32_t hold_ms);
void display_release(const char* owner);
bool display_is_owner(const char* owner);
bool display_is_free();

void display_print4_owned(const char* owner, const char* s4);
bool display_set_brightness_owned(const char* owner, uint8_t level);

// Convenience for idle state when nobody owns it
void display_idle(const char* s4);
```

Rules
- Owners are strings such as "FRANK" "GRAVE"
- Priority 0 to 255 higher can preempt lower equal cannot preempt
- `hold_ms` zero means no auto expire greater than zero auto releases unless renewed

Example blink with ownership

```cpp
if (display_acquire("GRAVE", 60, 2500)) {
  display_print4_owned("GRAVE", "REC ");
  // after work
  display_release("GRAVE");
}
```

---

## Scene registry and numeric codes — from `src/scenes.cpp`

Mapping today:

```text
0  Standby
1  FrankenLab
2  MirrorRoom
10 PhoneLoading
11 IntroCue
12 BloodRoom
13 Graveyard
14 FurRoom
15 OrcaDino
16 FrankenLab alias
17 MirrorRoom alias
18 ExitHole
```

Entry points are `void scene_name()` functions which are called repeatedly from the main loop. Each scene maintains its own static state machine and must not block for long periods.

To switch scenes programmatically:
```cpp
extern void scenes_set(Scene s); // declared in scenes.hpp
scenes_set(Scene::Graveyard);
```

---

## Pattern for a non blocking scene

Key rules
- Use an enum for phases
- Keep `millis()` timestamps
- No long `delay` calls inside the scene
- If the scene wants the display call `display_acquire` with a priority and a limited `hold_ms`
- Release the display when done or on exit

See `templates/scene_template.cpp` for a ready pattern.

---

## Reference — Graveyard scene behavior today

The current `scene_graveyard()` used in the project performs:

- Turns the work light ON for the entire record window
- Pulses the GoPro relay to start shortly after entry then later to stop
- Asks the Pi to start and stop a DR40X recorder via named triggers
- Blinks "REC " on the 4 digit display three times using display ownership at priority 60 for 2500 ms
- Flashes the red LED exactly six times at the start

All timings are constant at the top of the file:
```cpp
static const uint32_t REC_WINDOW_MS   = 20000;
static const uint16_t PULSE_MS        = 250;
static const uint8_t  REC_FLASHES     = 3;
static const uint8_t  RED_LED_FLASHES = 6;
```

Names relied upon in the trigger layer
- `DR40X_REC_START`
- `DR40X_REC_STOP`

Pins relied upon in `pins.hpp`
- `PIN_RELAY_GOPRO`
- `PIN_WORKLIGHT`
- `LED_HOLD`

---

## Checklist for a new scene

1. Copy `templates/scene_template.hpp` and `templates/scene_template.cpp`
2. Rename function and file names to your scene
3. Register function in `src/scenes.cpp` inside `map_code_to_fn` and `scene_name`
4. If the scene uses display
   - choose an owner string
   - choose a priority lower than scenes that must override you
   - acquire with a finite `hold_ms` then renew as needed
5. If the scene triggers Pi events call `triggers_pulse_by_name(...)`
6. Emit telemetry lines with a stable `scene` and `phase`
7. Avoid blocking delays
8. Build flash test in Serial Monitor and in Serial Studio
9. Commit and push on a feature branch then PR to main

---

## Common problems and fixes

- Undefined references or duplicate globals  
  Make sure shared arrays or globals are defined in exactly one translation unit. Use `extern` in headers.

- Default argument conflicts  
  Do not repeat default parameter values in both header and source. Define them in the header only.

- Display never shows your text  
  Either you did not acquire ownership or another owner has higher priority or longer `hold_ms`.

- Console stopped responding  
  Make sure the scene loop does not block. Avoid long delays.

- Pins are unknown  
  Add the pin symbol to `include/pins.hpp` and wire it in hardware docs. Scenes must include `pins.hpp` to use it.
