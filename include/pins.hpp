#pragma once
#include <Arduino.h>

// ================= Sensors =================
// Beams 0..5 (active LOW with INPUT_PULLUP)
#define PIN_BEAM_0 2   // Frankenphones Lab
#define PIN_BEAM_1 3   // Intro / Cue Card -> Blackout
#define PIN_BEAM_2 4   // Blood Room
#define PIN_BEAM_3 5   // Graveyard
#define PIN_BEAM_4 7   // Mirror Room
#define PIN_BEAM_5 9   // Exit

// Beam 6: Adafruit 375 reed switch (active when CLOSED)
// Wire one lead to GND and the other to this pin. Use INPUT_PULLUP.
#define PIN_BEAM_6 30  // Tech booth light switch

// ================= Actuators =================
#define PIN_MAGNET_CTRL 6
#define PIN_BUZZER      8

// Status LEDs
#define LED_ARMED    10  // green
#define LED_HOLD     11  // red
#define LED_COOLDOWN 12  // yellow

// Tech booth light output (drives MOSFET or relay)
// Active HIGH: write HIGH to turn light ON.
#define PIN_TECHLIGHT 26
#define TECHLIGHT_ACTIVE_HIGH 1  // set to 0 if your relay is active LOW

// I2C pins for Mega 2560 (hardware-defined)
#define I2C_SDA_PIN 20
#define I2C_SCL_PIN 21