# Haunted Hearse — Triggers and Beams
_Last updated: 2025-09-15_

Append new sections at the bottom when you add sensors or change behavior. Do not rewrite old entries.

---

## How to read this
- **Beam**: logical index in firmware (B0..Bn)
- **Pin**: Arduino Mega input pin
- **Type**: electrical behavior and debounce
- **Scene / Action**: what happens when it trips
- **Notes**: wiring, pullups, re-arm timer, special rules
- **Console test**: one line to paste in the serial monitor

---

## Current wiring quick map

| Beam | Pin | Purpose                         | Scene / Action                        |
|-----:|-----|----------------------------------|---------------------------------------|
| B0   | D2  | Break beam                       | Frankenphones Lab                     |
| B1   | D3  | Break beam                       | Intro / Cue Card then Blackout        |
| B2   | D4  | Break beam                       | Blood Room                            |
| B3   | D5  | Break beam                       | Graveyard                             |
| B4   | D7  | Break beam                       | Mirror Room                           |
| B5   | D9  | Break beam                       | Exit                                  |
| B6   | D30 | Reed contact, tech booth light   | Drives TechLight output D26 per rules |

Electrical defaults: Beams B0..B5 use `INPUT_PULLUP`, active LOW, debounce 30 ms, auto re-arm 20 s.  
B6 is a reed switch to GND with `INPUT_PULLUP`. Closed equals LOW.

---

## Beam 0 — Frankenphones Lab
- **Pin**: D2
- **Type**: active LOW with `INPUT_PULLUP`, debounce 30 ms, auto re-arm 20 s
- **Scene / Action**: starts `scene_frankenphone()` immediately
- **Behavior spec**  
  - **Idle**: Green LED slow pulse, 4-digit display shows `OBEY`  
  - **On trip**: state goes to HOLD  
    - Magnet **ON** for 5 s  
    - Red LED ramps faster over the 5 s  
    - Buzzer plays a quieter 9 s modem handshake sequence  
    - 4-digit display sequence:  
      1. Scrolls `SYSTEM OVERRIDE`  
      2. Shows a random 16-digit number in 4-char chunks  
      3. Shows a near-future `MM/YY` date  
  - **Cooldown**: 20 s  
    - Magnet OFF, buzzer OFF after the 9 s sound completes  
    - Yellow LED flickers  
    - Display alternates through `ACES`, `GRTD`, `DONE`, `OPEN`  
    - Occasional `PIN` flashes accompanied by random 3 digits, reduced frequency from baseline  
    - Zip code remains visible as designed for your variant  
  - **Rearm**: returns to Idle. Green pulses and `OBEY` shown
- **Telemetry**  
  Emitted once per second while running and on phase transitions. Frame format matches the Serial Studio v3 project:  
  ```text
  /*<ms>,frankenphone,<PHASE>,<beam>,<magnet>,<buzzer>,<R>,<G>,<B>*/

### Beam 1 — Intro / Cue Card then Blackout
- **Pin**: D3
- **Type**: active LOW with `INPUT_PULLUP`, debounce 30 ms, auto re-arm 20 s
- **Scene / Action**:
  1) Pulses Pi GPIO **SHOW** via Mega D27 to start Falcon FPP **main_show** sequence  
  2) Runs `scene_intro()` locally if needed for any Arduino-side cues
- **Blackout**: handled inside your FPP sequence as part of main_show
- **TechLight rule**: turns TechLight **OFF and latches it OFF**. It remains OFF until:
  - the **reed closes** while in `LIGHT AUTO`, or
  - you issue a **console override** (`LIGHT ON` or `LIGHT AUTO` after closing reed)
- **Console test**: `TRIG SHOW` to exercise the Pi path, or trip the sensor
- **Wiring note**: Arduino **D27 -> Pi GPIO5** through optocoupler, Pi input uses pull-up

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