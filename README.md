# Haunted Hearse v2

An experimental miniature haunted ride built into a 1992 Cadillac Brougham (Superior Coach conversion).  
The ride uses addressable LEDs, sensors, and props, orchestrated by an Arduino Mega 2560 + Falcon F16v5 + Raspberry Pi (FPP).

---

## Project overview

- Ride control: Arduino Mega 2560 with custom modules  
- Lighting: Falcon F16v5, WS2812B and WS2815 pixels, UV LEDs  
- Sequencing: xLights + FPP on Raspberry Pi  
- Props: electromagnets, passive buzzers, IR break beams, reed switch  
- Persistence: settings and mappings stored in EEPROM

The ride has multiple themed scenes triggered by sensors along the track. Examples: Blood Room, Graveyard, Fur Room, Orca or Dino, Frankenphones Lab, Mirror Room, Exit.

---

## Repository layout
haunted-hearse-v2/
├── include/            # headers (console.hpp, settings.hpp, pins.hpp, display.hpp, …)
├── src/                # sources (main.cpp, console.cpp, inputs.cpp, scenes/*, …)
├── docs/               # reference docs
├── platformio.ini      # PlatformIO config
└── README.md

---

## Hardware setup

### Power
- 12 V car battery feeds LED rails and DC-DC converters  
- Converters provide a regulated 12 V rail and a 5 V rail for logic and props  
- Common ground required across Mega, Pi, Falcon, and both rails

### Arduino Mega 2560 pins
- Break beams  
  - Beam 0: D2  Frankenphones Lab  
  - Beam 1: D3  Intro and cue card then blackout  
  - Beam 2: D4  Blood Room  
  - Beam 3: D5  Graveyard  
  - Beam 4: D7  Mirror Room  
  - Beam 5: D9  Exit  
  - Beam 6: D30 Tech booth reed switch (Adafruit 375 type)
- Outputs and devices  
  - Magnet MOSFET gate: D6  
  - Passive buzzer: D8  
  - Indicator LEDs: D10 green, D11 red, D12 yellow  
  - TechLight output: D26 active high  
  - I2C 4 digit display: SDA pin 20, SCL pin 21, addr 0x70

### Sensors
- IR break beams: VCC 5 V, GND, signal to Mega pin with INPUT_PULLUP  
- Each sensor is debounced in code and auto re-arms after a cooldown

### Lighting
- Falcon F16v5 drives pixels  
- Pi runs FPP for sequences and events

---

## Display and telemetry

- A display arbiter prevents idle text from stomping scene text  
- Idle uses `display_idle("OBEY")` and yields to any scene that acquires the display  
- Serial Studio v3 project supported  
  - Frame format: `/*ms,scene,phase,beam,magnet,buzzer,R,G,B*/`  
  - Example: `/*5000,frankenphone,HOLD,0,1,0,176,32,0*/`

---

## TechLight behavior

- Modes: AUTO, FORCE ON, FORCE OFF  
- Operator command always wins  
- Intro and cue card force TechLight OFF for at least 5 seconds before returning to the chosen mode

---

## Console commands

Connect at 115200 baud. Commands are case insensitive.

- `?` or `HELP`  show commands  
- `CFG`  print pin map and sensor states  
- `MAP`  print beam to scene mapping

Timing and display
- `HOLD <ms>`  set Frankenphones hold duration  
- `COOL <ms>`  set cooldown or re arm time  
- `BRIGHT <0..15>`  set 4 digit display brightness

Debounce and rearm
- `SDEB <ms>`  set beam debounce  
- `SREARM <ms>`  set beam re arm

EEPROM
- `SAVE`  save settings  
- `LOAD`  load settings

Tech light
- `TL ON`  force on  
- `TL OFF`  force off  
- `TL AUTO`  return to automatic with scene rules

Scenes
- `SCENE <name>`  force a scene by name, for example `SCENE FRANKENLAB` or `SCENE BLOODROOM`  
- `STATE <code>`  developer shortcut when numeric codes are enabled

---

## Workflow example
CFG                 # verify pins and sensor levels
BRIGHT 10           # set display brightness
HOLD 8000           # Frankenphones hold 8 seconds
COOL 20000          # cooldown 20 seconds
SAVE                # persist to EEPROM
SCENE FRANKENLAB    # test the Frankenphones Lab scene
MAP                 # check beam to scene mapping

