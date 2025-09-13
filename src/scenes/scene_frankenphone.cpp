#include "scene_frankenphone.hpp"
#include "../display.hpp"
#include "../effects.hpp"
#include "../console.hpp"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <Arduino.h>

// ================== Pins (assumed globally defined) ==================
extern const uint8_t PIN_MAGNET_CTRL;
extern const uint8_t PIN_BUZZER;
extern const uint8_t LED_ARMED;
extern const uint8_t LED_HOLD;
extern const uint8_t LED_COOLDOWN;

// ================== Duration Settings ==================
const unsigned long HOLD_MS     = 5000;
const unsigned long COOLDOWN_MS = 20000;

// ================== State Machine ==================
enum PhaseState { IDLE = 0, HOLD, COOLDOWN };
PhaseState state = IDLE;
unsigned long tPhaseStart = 0;

// ================== Display ==================
Adafruit_AlphaNum4 alpha = Adafruit_AlphaNum4();

enum DispStage {
  DISP_IDLE = 0, DISP_FLASH, DISP_SCROLL_TEXT, DISP_RANDOM16,
  DISP_EXPIRY_DATE, DISP_PIN, DISP_AREA5, DISP_DONE
};
DispStage dispStage = DISP_IDLE;
String dispScrollMsg;
uint16_t dispScrollIndex = 0;
unsigned long dispNextTick = 0;
bool dispFlashOn = false;
uint8_t dispFlashCount = 0;

// ================== LED Flicker ==================
unsigned long ledRandDeadline = 0;
int ledYellowPWM = 0;

// ================== Display Helpers ==================
void alphaShow4(char a, char b, char c, char d) {
  alpha.clear();
  alpha.writeDigitAscii(0, a);
  alpha.writeDigitAscii(1, b);
  alpha.writeDigitAscii(2, c);
  alpha.writeDigitAscii(3, d);
  alpha.writeDisplay();
}
void alphaShowString4(const String& s) {
  alphaShow4(
    (s.length() > 0) ? s[0] : ' ',
    (s.length() > 1) ? s[1] : ' ',
    (s.length() > 2) ? s[2] : ' ',
    (s.length() > 3) ? s[3] : ' '
  );
}
void alphaScrollInit(const String& msg) {
  dispScrollMsg = "    " + msg + "    ";
  dispScrollIndex = 0;
  dispNextTick = millis();
}
bool alphaScrollStep() {
  if (millis() < dispNextTick) return false;
  if (dispScrollIndex + 4 > dispScrollMsg.length()) return true;
  alphaShowString4(dispScrollMsg.substring(dispScrollIndex, dispScrollIndex + 4));
  dispScrollIndex++;
  dispNextTick = millis() + 180;
  return false;
}
String randomDigits(uint8_t n) {
  String s; for (uint8_t i = 0; i < n; i++) s += char('0' + random(10));
  return s;
}
String randomFutureMMYY() {
  char buf[6];
  snprintf(buf, sizeof(buf), "%02u%02u", random(1, 13), random(26, 31));
  return String(buf);
}

// ================== Display Sequence ==================
void dispStartSequence() {
  dispStage = DISP_FLASH;
  dispFlashCount = 0;
  dispFlashOn = false;
  dispNextTick = millis();
}
void dispUpdate() {
  if (dispStage == DISP_IDLE || dispStage == DISP_DONE) return;
  unsigned long now = millis();

  switch (dispStage) {
    case DISP_FLASH:
      if (now >= dispNextTick) {
        dispFlashOn = !dispFlashOn;
        alphaShow4(dispFlashOn ? '-' : ' ', dispFlashOn ? '-' : ' ',
                   dispFlashOn ? '-' : ' ', dispFlashOn ? '-' : ' ');
        dispNextTick = now + 150;
        if (++dispFlashCount >= 12) {
          alphaScrollInit("SECURING ENCRYPTED DATA");
          dispStage = DISP_SCROLL_TEXT;
        }
      }
      break;

    case DISP_SCROLL_TEXT:
      if (alphaScrollStep()) dispStage = DISP_RANDOM16;
      break;

    case DISP_RANDOM16: {
      static String digits = randomDigits(16);
      static uint8_t chunk = 0;
      if (now >= dispNextTick) {
        alphaShowString4(digits.substring(chunk * 4, (chunk + 1) * 4));
        dispNextTick = now + 400;
        if (++chunk >= 4) {
          chunk = 0;
          dispStage = DISP_EXPIRY_DATE;
        }
      }
    } break;

    case DISP_EXPIRY_DATE: {
      static String mmyy = randomFutureMMYY();
      static uint8_t sub = 0;
      if (now >= dispNextTick) {
        if (sub == 0) { alphaShowString4("DATE"); dispNextTick = now + 500; sub = 1; }
        else if (sub == 1) { alphaShowString4(mmyy); dispNextTick = now + 700; sub = 2; }
        else { sub = 0; dispStage = DISP_PIN; }
      }
    } break;

    case DISP_PIN: {
      static String pin3 = randomDigits(3);
      static uint8_t sub = 0;
      if (now >= dispNextTick) {
        if (sub == 0) { alphaShowString4("PIN "); dispNextTick = now + 500; sub = 1; }
        else if (sub == 1) { alphaShowString4(" " + pin3); dispNextTick = now + 700; sub = 2; }
        else { sub = 0; dispStage = DISP_AREA5; }
      }
    } break;

    case DISP_AREA5: {
      static String zip = randomDigits(5);
      static uint8_t sub = 0;
      if (now >= dispNextTick) {
        if (sub == 0) { alphaShowString4("AREA"); dispNextTick = now + 500; sub = 1; }
        else if (sub == 1) { alphaShowString4(zip.substring(0, 4)); dispNextTick = now + 600; sub = 2; }
        else if (sub == 2) { alphaShowString4(String(" ") + zip[4]); dispNextTick = now + 500; sub = 3; }
        else { sub = 0; dispStage = DISP_DONE; }
      }
    } break;

    default: break;
  }
}

// ================== LED Animations ==================
void animateRedHold(unsigned long t) {
  const unsigned int PERIOD_SLOW = 300;
  const unsigned int PERIOD_FAST = 40;
  unsigned int period = PERIOD_SLOW - (t * (PERIOD_SLOW - PERIOD_FAST)) / HOLD_MS;
  int brightness = 60 + (195L * t) / HOLD_MS;
  brightness = constrain(brightness, 0, 255);
  bool on = (millis() % period) < (period * 45UL) / 100UL;
  analogWrite(LED_HOLD, on ? brightness : 0);
}
void animateYellowCooldown() {
  if (millis() >= ledRandDeadline) {
    ledYellowPWM = random(40, 255);
    ledRandDeadline = millis() + random(20, 120);
    analogWrite(LED_COOLDOWN, ledYellowPWM);
  }
}
void animateGreenArmed() {
  unsigned long t = millis() % 2400;
  int val = (t < 1200) ? map(t, 0, 1200, 30, 255) : map(t, 1200, 2400, 255, 30);
  analogWrite(LED_ARMED, val);
}

// ================== Sound + Control ==================
inline void magnetOn()  { digitalWrite(PIN_MAGNET_CTRL, HIGH); }
inline void magnetOff() { digitalWrite(PIN_MAGNET_CTRL, LOW); }
inline void buzzerOff() { noTone(PIN_BUZZER); }
void playModemSound(unsigned long t) {
  if (t < 800UL) tone(PIN_BUZZER, 400 + (t * 1400) / 800UL);
  else if (t < 1600UL) tone(PIN_BUZZER, 1800 - ((t - 800UL) * 1200) / 800UL);
  else if (t < 3000UL) { if ((millis() & 0x07) == 0) tone(PIN_BUZZER, random(400, 2500)); }
  else if (t < 4200UL) tone(PIN_BUZZER, ((millis() / 40) & 1) ? 1400 : 1800);
  else if (t < HOLD_MS) tone(PIN_BUZZER, 1000);
  else buzzerOff();
}

// ================== Public Scene API ==================
void frankenphone_init() {
  Wire.begin();
  alpha.begin(0x70);
  alpha.setBrightness(10);
  alpha.clear(); alpha.writeDisplay();
  pinMode(PIN_MAGNET_CTRL, OUTPUT); magnetOff();
  pinMode(PIN_BUZZER, OUTPUT); buzzerOff();
  pinMode(LED_ARMED, OUTPUT);
  pinMode(LED_HOLD, OUTPUT);
  pinMode(LED_COOLDOWN, OUTPUT);
  randomSeed(analogRead(A0));
  state = IDLE;
  dispStage = DISP_IDLE;
}

void scene_frankenphone() {
  tPhaseStart = millis();
  state = HOLD;
  magnetOn();
  dispStartSequence();
  console_log("Frankenphone: HOLD start");
}

void frankenphone_update() {
  unsigned long now = millis();

  if (state == HOLD) {
    unsigned long elapsed = now - tPhaseStart;
    playModemSound(elapsed);
    animateRedHold(elapsed);
    dispUpdate();
    if (elapsed >= HOLD_MS) {
      state = COOLDOWN;
      tPhaseStart = now;
      buzzerOff();
      magnetOff();
      console_log("Frankenphone: COOLDOWN start");
    }
  } else if (state == COOLDOWN) {
    animateYellowCooldown();
    dispUpdate();
    if (now - tPhaseStart >= COOLDOWN_MS) {
      state = IDLE;
      dispStage = DISP_IDLE;
      console_log("Frankenphone: rearmed");
    }
  } else if (state == IDLE) {
    animateGreenArmed();
  }
}