#pragma once
#include <Arduino.h>

/* ===== Status LEDs =====
   Active HIGH. Wire each LED with a series resistor to these pins.
   Green = ARMED, Red = HOLD, Yellow = COOLDOWN
*/
#define LED_ARMED       10   // green
#define LED_HOLD        11   // red
#define LED_COOLDOWN    12   // yellow

/* ===== Actuators =====
   PIN_MAGNET_CTRL drives the MOSFET gate for the electromagnet
   PIN_BUZZER drives the passive piezo buzzer
*/
#define PIN_MAGNET_CTRL 6
#define PIN_BUZZER      8

/* ===== Break-beam sensors =====
   INPUT_PULLUP. Active LOW when the beam is broken.
   You have six beams available on these pins.
*/
#define PIN_BEAM_0      2
#define PIN_BEAM_1      3
#define PIN_BEAM_2      4
#define PIN_BEAM_3      5
#define PIN_BEAM_4      7
#define PIN_BEAM_5      9

/* ===== I2C display =====
   Adafruit 4 digit alphanumeric backpack at 0x70
   Mega SDA = 20, SCL = 21 (hardware I2C)
*/

/* ===== Optional Pi trigger outputs (present but not used by Frankenphone) ===== */
#define PIN_TRIG0       22   // Pi GPIO17
#define PIN_TRIG1       23   // Pi GPIO27
#define PIN_TRIG2       24   // Pi GPIO22
#define PIN_TRIG3       25   // Pi GPIO23