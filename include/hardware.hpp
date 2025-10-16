#pragma once
#include <Arduino.h>

constexpr uint8_t KEYPAD_ROW_PINS[] = {40, 41, 42, 43};
constexpr uint8_t KEYPAD_COL_PINS[] = {44, 45, 46, 47};
constexpr uint8_t FPP_START_LOOP = 48;
constexpr uint8_t FPP_START_TRIGGER = 49;
constexpr uint8_t FPP_START_ATTRACT = 50;
constexpr uint8_t FPP_STOP_ALL = 51;
constexpr uint8_t BEAM_INTRO = 22;
constexpr uint8_t BEAM_BLOOD = 23;
constexpr uint8_t BEAM_FRANKENPHONE = 24;
constexpr uint8_t BEAM_MIRROR = 25;
constexpr uint8_t BEAM_EXIT = 26;
constexpr uint8_t OPERATOR_LED_R = 36;
constexpr uint8_t OPERATOR_LED_G = 37;
constexpr uint8_t OPERATOR_LED_B = 38;
constexpr uint8_t OPERATOR_LED_W = 39;
constexpr uint8_t SCENE_LED_1 = 10;
constexpr uint8_t SCENE_LED_2 = 11;
constexpr uint8_t SCENE_LED_3 = 12;
constexpr uint8_t MAGNET_PIN = 8;
constexpr uint8_t BUZZER_PIN = 9;
constexpr uint8_t NUM_SCENE_LEDS = 3;
constexpr uint8_t NUM_BEAMS = 5;