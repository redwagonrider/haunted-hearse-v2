# Haunted Hearse

A unified repo for the Haunted Hearse project: Arduino triggers + FPP + Falcon F16v5 + xLights + wiring docs.

## Layout
```
.
├── arduino/               # Arduino sketches
│   └── FrankenphoneLab/   # Break-beam → magnet + buzzer (dial-up), 20s cooldown, status LEDs
├── fpp/                   # FPP exports (playlists, outputs, backups)
├── xlights/               # xLights sequences, models, network setup exports
├── wiring/                # Diagrams, pinouts, PDFs
├── docs/                  # Setup guides (Pi/FPP, Falcon, power)
└── .github/               # CI, issue templates, project board
```

## Quick Start
### Arduino (Arduino IDE)
1. Open `arduino/FrankenphoneLab/FrankenphoneLab.ino` (Board: **Arduino/Genuino Mega or Mega 2560**).
2. Wire as shown in `wiring/Frankenphone_Lab_Wiring_Sketch.pdf`.
3. Upload. Break the beam to trigger the 5 s lock + modem sound; 20 s cooldown.

### PlatformIO (VS Code)
```bash
pio run -e mega2560
pio run -e mega2560 -t upload
```

### Arduino CLI (terminal)
```bash
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli compile --fqbn arduino:avr:mega arduino/FrankenphoneLab
# Upload example (check your serial port):
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:mega arduino/FrankenphoneLab
```

### xLights & FPP
- Keep `.xsq` and `.fseq` in `/xlights/` (large files tracked by Git LFS).
- Upload sequences to FPP via xLights “Upload to FPP”.
- Save FPP backups + JSON configs into `/fpp/`.

## Safety / Power
- Magnet is switched with **Adafruit STEMMA MOSFET + diode** (active‑HIGH).
- All grounds common. Fuse the 5 V rail feeding the module panel.
- Avoid committing Wi‑Fi passwords or private keys (use local `.env`, not in git).
