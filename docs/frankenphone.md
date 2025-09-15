# Frankenphones Lab scene

Version: frankenphone-locked-2025-09-14-09sA

Pins
- Beam 0 D2 input pullup
- Magnet D6 output high active
- Buzzer D8 output
- LEDs: green D10 red D11 yellow D12
- I2C alphanumeric display 0x70 SDA 20 SCL 21

Timing
- Hold 5000 ms
- Cooldown 20000 ms
- Modem sound 9000 ms Option A quieter

Display program
- HOLD: SYSTEM OVERRIDE scroll then 16 digits then DONE flash then date then PIN then ZIP
- COOLDOWN: finish any remaining PIN ZIP then cycle ACES GRTD OPEN DONE with occasional PIN flashes at reduced rate

Notes
- Beam debounce 30 ms active low
- Telemetry unchanged for Serial Studio v3
- Version string prints at boot and on scene init