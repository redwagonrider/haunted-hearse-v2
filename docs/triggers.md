# Haunted Hearse — Triggers and Beams

Append new sections at the bottom as you add sensors or change behavior.

---

## How to read this
- **Beam**: logical index used in firmware (B0..Bn)
- **Pin**: Arduino Mega input pin
- **Type**: electrical behavior and debounce
- **Scene / Action**: what happens when it trips
- **Notes**: wiring, pullups, re-arm timer, special rules
- **Console test**: one line you can paste in the serial monitor

---

## Template to copy for a new trigger

### Beam X — <Name>
- **Pin**: DX
- **Type**: active LOW with INPUT_PULLUP, debounce 30 ms, re-arm 20 s
- **Scene / Action**: <what to run>
- **Notes**: <wiring, special cases>
- **Console test**:
  - `MAP` to verify mapping
  - If a scene is implemented, use its `STATE` or trip the physical sensor

---

## Current triggers

### Beam 0 — Frankenphones Lab
- **Pin**: D2
- **Type**: active LOW with INPUT_PULLUP, debounce 30 ms, re-arm 20 s
- **Scene / Action**: `scene_frankenphone()`
- **Notes**: aligns with Serial Studio telemetry and LED patterns
- **Console test**: `STATE 16`

### Beam 1 — Intro / Cue Card then Blackout
- **Pin**: D3
- **Type**: active LOW with INPUT_PULLUP, debounce 30 ms, re-arm 20 s
- **Scene / Action**: `scene_intro()`; forces TechLight OFF for 5 s
- **Notes**: TechLight override still works, but intro does an immediate kill first
- **Console test**: trip sensor or use your scene’s STATE once implemented

### Beam 2 — Blood Room
- **Pin**: D4
- **Type**: active LOW with INPUT_PULLUP, debounce 30 ms, re-arm 20 s
- **Scene / Action**: `scene_blood()`
- **Notes**: —
- **Console test**: trip sensor

### Beam 3 — Graveyard
- **Pin**: D5
- **Type**: active LOW with INPUT_PULLUP, debounce 30 ms, re-arm 20 s
- **Scene / Action**: `scene_graveyard()`
- **Notes**: —
- **Console test**: trip sensor

### Beam 4 — Mirror Room
- **Pin**: D7
- **Type**: active LOW with INPUT_PULLUP, debounce 30 ms, re-arm 20 s
- **Scene / Action**: `scene_mirror()`
- **Notes**: —
- **Console test**: trip sensor

### Beam 5 — Exit
- **Pin**: D9
- **Type**: active LOW with INPUT_PULLUP, debounce 30 ms, re-arm 20 s
- **Scene / Action**: `scene_exit()`
- **Notes**: —
- **Console test**: trip sensor

### Beam 6 — Tech Booth Light (Reed)
- **Pin**: D30 (input), D26 (output driver)
- **Type**: Reed contact to GND, INPUT_PULLUP. Closed = LOW
- **Scene / Action**: Not a scene. While closed, TechLight follows rules:
  1) Intro kill window forces OFF
  2) Console override ON/OFF beats reed
  3) Otherwise reed controls (closed = ON)
- **Notes**: Set `TECHLIGHT_ACTIVE_HIGH` in pins.hpp to match relay or MOSFET
- **Console test**:
  - `LIGHT ON`  `LIGHT OFF`  `LIGHT AUTO`
  - Close reed to see ON in AUTO