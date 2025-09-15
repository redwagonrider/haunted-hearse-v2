# Haunted Hearse v2

An experimental **miniature haunted ride** built into a 1992 Cadillac Brougham (Superior Coach conversion). 
The ride is powered by **addressable LEDs, sensors, and props**, all orchestrated by an **Arduino Mega 2560** + **Falcon F16v5** + **Raspberry Pi (FPP)** stack.

---

## 🛠 Project Overview

- **Ride Control**: Arduino Mega 2560 + custom breadboard modules 
- **Lighting**: Falcon F16v5, WS2812B / WS2815 pixels, UV LEDs 
- **Sequencing**: xLights + FPP (running on Raspberry Pi) 
- **Props**: Electromagnets, passive buzzers, IR break-beams, sensors 
- **Persistence**: Settings & mappings stored in EEPROM 

The ride consists of **13 themed scenarios**, each triggered by break-beam sensors along the track. 
Scenes include the **Blood Room, Graveyard, Fur Room, Orca/Dino, Frankenphones Lab, Mirror Room**, and more.

---

## 📂 Repository Layout

haunted-hearse-v2/

├── include/       # Header files (console.hpp, settings.hpp, etc.)

├── src/           # Source code (main.cpp, console.cpp, inputs.cpp, etc.)

├── docs/          # Documentation & references

├── .pio/          # PlatformIO build system

├── platformio.ini # Project configuration

└── README.md      # This file


---

## ⚡ Hardware Setup

### Power
- **12 V Car Battery** → feeds LED rails + converters  
- **DROK DC-DC Converters** → 12 V regulated rail and 5 V rail for logic/props  
- **Common Ground** required across Mega, Pi, Falcon, and power rails  

### Arduino Mega 2560 Connections
- **Break-beam sensors (6x)**: Pins 2, 3, 4, 5, 7, 9 (INPUT_PULLUP)  
- **Magnet MOSFET driver**: Pin 6 (output → gate → electromagnet)  
- **Passive buzzer**: Pin 8  
- **Indicator LEDs**: Pins 10 (green), 11 (red), 12 (yellow)  
- **I²C display**: SDA = 20, SCL = 21  

### Sensors
- IR Break-beams wired **VCC=5V**, **GND=common**, **Signal → Mega pin**  
- Each sensor auto-debounced in code and re-arms after 20s (configurable)
## Beams and Scene Map

All show beams are **active LOW** with `INPUT_PULLUP`. Debounce 30 ms. Auto re-arm 20 s.

| Beam | Arduino Pin | Purpose                        |
|-----:|-------------|--------------------------------|
| B0   | D2          | Frankenphones Lab              |
| B1   | D3          | Intro / Cue Card into Blackout |
| B2   | D4          | Blood Room                     |
| B3   | D5          | Graveyard                      |
| B4   | D7          | Mirror Room                    |
| B5   | D9          | Exit                           |
| B6   | D30         | Tech Booth Light reed switch   |

### Tech Booth Light control

- Hardware: Adafruit 375 reed switch on D30. Output driver on D26 to MOSFET or relay.
- Priority: **Intro/Cue** scene forces lights **OFF** immediately.  
  Console commands override the reed switch:  
  - `LIGHT ON` forces ON  
  - `LIGHT OFF` forces OFF  
  - `LIGHT AUTO` follows the reed again  
  - `LIGHT TOGGLE` flips the current state

### Lighting
- **Falcon F16v5** handles pixel outputs (WS2811/2815 @ 12 V, WS2812B @ 5 V)  
- Falcon and Mega communicate via Pi/FPP for sync sequencing  
- Pi also manages triggers and sequences if extended  

### Notes
- All grounds (12 V, 5 V, Mega, Pi, Falcon) must be tied together  
- Use **fused distribution blocks** for safety (both 12 V and 5 V rails)  
- EEPROM ensures tunables (hold time, cooldown, brightness, mappings) survive power cycles

---

## 🎛 Console Commands

Connect to the Mega via **Serial Monitor @ 115200 baud**.  
Commands are case-insensitive.

### Help & Status
- `?` or `HELP` — show all commands  
- `CFG` — print current settings + sensor states  
- `MAP` — print beam → scene mapping  

### Timing / Settings
- `HOLD <ms>` — set FrankenLab hold duration  
- `COOL <ms>` — set cooldown/lockout duration  
- `BRIGHT <0..15>` — set display brightness  
- `SDEB <ms>` — set beam debounce (0–2000 ms)  
- `SREARM <ms>` — set beam re-arm (0–600000 ms)  

### EEPROM Persistence
- `SAVE` — save settings to EEPROM  
- `LOAD` — load settings from EEPROM (auto-loads on boot)  

### Forcing States & Scenes
- `STATE <code>` — force by numeric code  
  - 0 = Standby  
  - 1 = FrankenLab  
  - 2 = MirrorRoom  
  - 10 = PhoneLoading  
  - 11 = IntroCue  
  - 12 = BloodRoom  
  - 13 = Graveyard  
  - 14 = FurRoom  
  - 15 = OrcaDino  
  - 16 = FrankenLab  
  - 17 = MirrorRoom  
  - 18 = ExitHole  

- `SCENE <name>` — force by scene name  
  Examples:  
  `SCENE BLOODROOM`, `SCENE FRANKENLAB`, `SCENE EXIT`

---

## 🧪 Workflow Example

```text
CFG                # check current settings
HOLD 6000          # set hold duration to 6s
BRIGHT 10          # adjust display brightness
SAVE               # persist changes to EEPROM
SCENE FRANKENLAB   # force Frankenphones Lab scene
MAP                # confirm beam mapping

## Addendum - Display Arbiter, Beams, Tech Light, and Scene Text (2025-09-15)

### What changed
- Added a display arbiter so scenes can own the 4-digit AlphaNum4. Idle "OBEY" only draws when the display is free.
- Frankenphones Lab uses the arbiter during HOLD and briefly into COOLDOWN.
- Blood Room gained a non-blocking DRIP animation that drips in, flashes, fades, then drips out.
- Introduced a TechLight output with operator overrides and an intro kill rule.

---

### Hardware pins in use
- Break beams B0..B5 on D2, D3, D4, D5, D7, D9
- Reed switch for tech booth B6 on D30
- TechLight output D26  active high unless configured otherwise
- Magnet MOSFET gate D6
- Buzzer D8
- Status LEDs  Green D10, Red D11, Yellow D12
- I2C display SDA D20, SCL D21 at 0x70

---

### Beam map and effects
| Beam | Pin | Scene or Action |
|------|-----|------------------|
| B0   | D2  | Frankenphones Lab. Full sequence with modem sound, magnet, LEDs, and display takeover. |
| B1   | D3  | Intro and Cue Card then blackout. Pulses Pi SHOW trigger and forces TechLight OFF for at least 5 s. |
| B2   | D4  | Blood Room. Pulses Pi BLOOD trigger and runs DRIP text animation on the 4-digit. |
| B3   | D5  | Graveyard. Placeholder, ready for scene text via arbiter. |
| B4   | D7  | Mirror Room. Placeholder, ready for scene text via arbiter. |
| B5   | D9  | Exit. Placeholder, ready for scene text via arbiter. |
| B6   | D30 | TechLight reed input. Closed equals ON when in AUTO mode. |

All beams B0..B5 are active LOW with INPUT_PULLUP, 30 ms debounce, 20 s auto re-arm.

---

### TechLight operator rules
- Modes  AUTO, FORCE ON, FORCE OFF
- Priority  operator overrides trump everything
- Intro and Cue sets TechLight OFF immediately, enforces at least 5 s blackout, and keeps OFF until either
  - you set FORCE ON, or
  - you are in AUTO and the reed closes

Console shortcuts examples
```text
TL ON