// src/scenes/scene_frankenphone.cpp
#include "scenes/scene_frankenphone.hpp"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include "pins.hpp"
#include "effects.hpp"
#include "console.hpp"
#include "display.hpp"
#include "telemetry.hpp"

// ===== Beam lock =====
#ifndef PIN_BEAM_0
#error "PIN_BEAM_0 must be defined in include/pins.hpp"
#endif
#if PIN_BEAM_0 != 2
#error "Frankenphones Lab is locked to Beam 0 on D2. Update include/pins.hpp if you must move it."
#endif

// ===== Timings =====
static unsigned long HOLD_MS         = 5000;    // magnet on window
static const unsigned long COOLDOWN_MS     = 20000;
static const unsigned long TOTAL_SCENE_MS  = 25000;   // HOLD + COOLDOWN
static const unsigned long TEL_HZ_MS       = 1000;
static const unsigned long DEBOUNCE_MS     = 30;

// ===== I2C display =====
static Adafruit_AlphaNum4 alpha4;
static const uint8_t DISP_ADDR = 0x70;
static const char IDLE_TEXT[5] = "OBEY";

// Cooldown frames (cycles for full 20 s)
static const char* GRANT_FRAMES[] = {"ACES","GRTD","OPEN","DONE"};
static const uint8_t N_FRAMES = 4;

// ===== State =====
enum RunState { ST_IDLE, ST_HOLD, ST_COOLDOWN };
static RunState state = ST_IDLE;
static unsigned long tStateStart = 0;  // phase start
static unsigned long tSceneStart = 0;  // overall start

// ===== Display sequence state (during HOLD) =====
enum DispPhase { PH_SYSOVR, PH_DIGITS, PH_DONEFLASH, PH_DATE, PH_PIN, PH_DONE };
static DispPhase dphase = PH_SYSOVR;

static String digits16;     // 16 random digits
static String yyMM;         // "YY/MM"
static String pinStr;       // " PIN" + 3 digits (we right-pad)
static unsigned long tScroll = 0;
static uint16_t scrollDelay = 120;
static int scrollIndex = 0;
static String scrollBuf;

static bool blinkOn = false;
static uint8_t blinkCount = 0;
static unsigned long tBlink = 0;

// ===== Cooldown display state =====
static bool cdFlashOn = false;
static unsigned long cdTick = 0;
static uint8_t cdFrame = 0;
static const uint16_t FLASH_ON_MS = 220;
static const uint16_t FLASH_OFF_MS = 180;

// ===== LED animation helpers =====
static unsigned long ledRandDeadline = 0;
static int ledYellowPWM = 0;

// ===== Magnet and buzzer =====
static inline void magnetOn()  { digitalWrite(PIN_MAGNET_CTRL, HIGH); }
static inline void magnetOff() { digitalWrite(PIN_MAGNET_CTRL, LOW);  }

static bool gBuzzerOn = false;
static inline void buzzerTone(uint16_t f) { tone(PIN_BUZZER, f); gBuzzerOn = true; }
static inline void buzzerOff() { noTone(PIN_BUZZER); gBuzzerOn = false; }

// ===== Beam with debounce (active LOW) =====
static uint8_t beam_state = 0, beam_last = 0;
static unsigned long tBeamChange = 0;
static inline uint8_t beam_raw() { return digitalRead(PIN_BEAM_0) == LOW ? 1 : 0; }

static void beam_update() {
  uint8_t r = beam_raw();
  if (r != beam_last) { beam_last = r; tBeamChange = millis(); }
  if (millis() - tBeamChange >= DEBOUNCE_MS) beam_state = r;
}
static inline uint8_t beam_active() { return beam_state; }

// ===== Telemetry helpers =====
static const char* phase_name() {
  switch (state) {
    case ST_HOLD:      return "HOLD";
    case ST_COOLDOWN:  return "COOLDOWN";
    default:           return "IDLE";
  }
}
static void tel_emit_now() {
  uint8_t r=0,g=0,b=0; effects_getRGB(r,g,b);
  HH::tel_emit("frankenphone", phase_name(),
               beam_active(),
               (digitalRead(PIN_MAGNET_CTRL)==HIGH)?1:0,
               gBuzzerOn?1:0,
               r,g,b);
}
static void tel_emit_transition(const char* newPhase) {
  uint8_t r=0,g=0,b=0; effects_getRGB(r,g,b);
  HH::tel_emit("frankenphone", newPhase ? newPhase : phase_name(),
               beam_active(),
               (digitalRead(PIN_MAGNET_CTRL)==HIGH)?1:0,
               gBuzzerOn?1:0,
               r,g,b);
}

// ===== Alpha display helpers =====
static void dispShow4(const char* s4) {
  for (int i=0;i<4;i++) alpha4.writeDigitAscii(i, s4[i]);
  alpha4.writeDisplay();
}
static void dispShowStr4(const String& s) {
  char buf[5] = {' ',' ',' ',' ','\0'};
  for (int i=0;i<4 && i<(int)s.length(); ++i) buf[i] = s[i];
  dispShow4(buf);
}
static void dispClear() { alpha4.clear(); alpha4.writeDisplay(); }

static String randomDigits(uint8_t n) {
  String s; s.reserve(n);
  for (uint8_t i=0;i<n;i++) s += char('0' + random(10));
  return s;
}
static String nearFutureYYslashMM() {
  // random 1..18 months from a nominal "now" to create a near-future YY/MM
  uint8_t addMonths = random(1, 19);
  uint8_t nowMonth = 7;
  uint16_t nowYear = 2025;
  uint16_t y = nowYear + (nowMonth + addMonths - 1) / 12;
  uint8_t  m = ((nowMonth - 1 + addMonths) % 12) + 1;
  char buf[6];
  snprintf(buf, sizeof(buf), "%02u/%02u", (uint8_t)(y % 100), m);
  return String(buf);
}

// ===== LED animations (physical) + RGB for telemetry =====
static void animateGreenArmed() {
  unsigned long ms = millis();
  const unsigned long period = 2400;
  unsigned long t = ms % period;
  int val = (t < period/2) ? map(t, 0, period/2, 30, 255)
                           : map(t, period/2, period, 255, 30);
  analogWrite(LED_ARMED, val);
  analogWrite(LED_HOLD, 0);
  analogWrite(LED_COOLDOWN, 0);
  // Telemetry RGB
  uint8_t g = map(val, 30, 255, 16, 96);
  effects_setRGB(0, g, 0);
}

static void animateRedHold(unsigned long holdElapsed) {
  const unsigned int PERIOD_SLOW_MS = 300;
  const unsigned int PERIOD_FAST_MS = 40;
  unsigned long clamped = (holdElapsed > HOLD_MS) ? HOLD_MS : holdElapsed;
  unsigned int period = PERIOD_SLOW_MS
      - (unsigned int)(((long)(PERIOD_SLOW_MS - PERIOD_FAST_MS) * (long)clamped) / (long)HOLD_MS);
  int brightness = 60 + (int)((195L * (long)clamped) / (long)HOLD_MS);
  brightness = constrain(brightness, 0, 255);
  unsigned long phase = millis() % period;
  bool on = (phase < (period * 45UL) / 100UL);
  analogWrite(LED_HOLD, on ? brightness : 0);
  analogWrite(LED_ARMED, 0);
  analogWrite(LED_COOLDOWN, 0);
  // Telemetry RGB
  uint8_t r = on ? map(brightness, 60, 255, 64, 255) : 32;
  effects_setRGB(r, 32, 0);
}

static void animateYellowCooldown() {
  unsigned long now = millis();
  if (now >= ledRandDeadline) {
    ledYellowPWM = random(40, 255);
    unsigned long dwell = (unsigned long)random(20, 120);
    ledRandDeadline = now + dwell;
    analogWrite(LED_COOLDOWN, ledYellowPWM);
  }
  analogWrite(LED_ARMED, 0);
  analogWrite(LED_HOLD, 0);
  // Telemetry RGB
  uint8_t y = map(ledYellowPWM, 40, 255, 64, 200);
  effects_setRGB(y, y * 2 / 3, 0);
}

// ===== Modem sound over full 25 s =====
static void modemSound25s(unsigned long tSinceSceneStart) {
  if (tSinceSceneStart < 300UL) buzzerTone(2400);
  else if (tSinceSceneStart < 700UL) buzzerTone(((millis()/20)&1)?1200:2200);
  else if (tSinceSceneStart < 1400UL) {
    int f = 500 + (int)((tSinceSceneStart - 700) * (1600.0 / 700.0));
    buzzerTone((uint16_t)f);
  } else if (tSinceSceneStart < 2200UL) {
    if ((millis() & 0x03) == 0) buzzerTone((uint16_t)random(600, 3000));
  } else if (tSinceSceneStart < 3200UL) {
    buzzerTone(((millis()/40)&1) ? 1300 : 1800);
  } else if (tSinceSceneStart < HOLD_MS) {
    buzzerTone(1000);
  } else if (tSinceSceneStart < HOLD_MS + 15000UL) {
    if ((millis() & 0x03) == 0) buzzerTone((uint16_t)random(800, 2200));
  } else if (tSinceSceneStart < TOTAL_SCENE_MS - 3000UL) {
    buzzerTone(((millis() / 80) & 1) ? 1200 : 2000);
  } else if (tSinceSceneStart < TOTAL_SCENE_MS) {
    buzzerTone(1000);
  } else {
    buzzerOff();
  }
}

// ===== HOLD display program =====
static void holdDisplayInit() {
  dphase = PH_SYSOVR;
  digits16 = randomDigits(16);
  yyMM     = nearFutureYYslashMM();        // "YY/MM"
  pinStr   = String(" ") + randomDigits(3); // display as " PIN" then " 123"

  // Start scroll buffer with SYSTEM OVERRIDE
  scrollBuf = "    SYSTEM OVERRIDE    ";
  scrollDelay = 140;
  scrollIndex = 0;
  tScroll = 0;

  blinkOn = false;
  blinkCount = 0;
  tBlink = 0;
}

static bool stepScroll() {
  if (scrollIndex + 4 <= (int)scrollBuf.length()) {
    dispShowStr4(scrollBuf.substring(scrollIndex, scrollIndex + 4));
    scrollIndex++;
    return false;
  }
  return true;
}

static void holdDisplayUpdate() {
  unsigned long now = millis();
  if (now - tScroll < scrollDelay) return;
  tScroll = now;

  switch (dphase) {
    case PH_SYSOVR:
      if (stepScroll()) {
        // Show 16 digits by scrolling across
        scrollBuf = String("    ") + digits16 + String("    ");
        scrollDelay = 180;
        scrollIndex = 0;
        dphase = PH_DIGITS;
      }
      break;

    case PH_DIGITS:
      if (stepScroll()) {
        dphase = PH_DONEFLASH;
        blinkOn = false;
        blinkCount = 0;
        tBlink = now;
      }
      break;

    case PH_DONEFLASH:
      if (now - tBlink >= (blinkOn ? 150U : 150U)) {
        tBlink = now;
        blinkOn = !blinkOn;
        dispShowStr4(blinkOn ? "DONE" : "    ");
        if (++blinkCount >= 6) {
          // Show YY/MM as a short scroll
          scrollBuf = String("    ") + yyMM + String("    ");
          scrollDelay = 140;
          scrollIndex = 0;
          dphase = PH_DATE;
        }
      }
      break;

    case PH_DATE:
      if (stepScroll()) {
        dphase = PH_PIN;
        // Brief "PIN " then " 123"
        dispShowStr4("PIN ");
        tBlink = now + 240;
        blinkCount = 0;
      }
      break;

    case PH_PIN:
      if (blinkCount == 0 && now >= tBlink) {
        dispShowStr4(pinStr);  // " 123"
        tBlink = now + 420;
        blinkCount = 1;
      } else if (blinkCount == 1 && now >= tBlink) {
        dphase = PH_DONE;
      }
      break;

    case PH_DONE:
    default:
      break;
  }
}

// ===== COOLDOWN display =====
static void cooldownDisplayStart() {
  cdFlashOn = false;
  cdTick = millis();
  cdFrame = 0;
  dispClear();
}
static void cooldownDisplayUpdate() {
  unsigned long now = millis();
  uint16_t dur = cdFlashOn ? FLASH_ON_MS : FLASH_OFF_MS;
  if (now - cdTick >= dur) {
    cdTick = now;
    cdFlashOn = !cdFlashOn;
    if (cdFlashOn) {
      dispShow4(GRANT_FRAMES[cdFrame]);
      cdFrame = (cdFrame + 1) % N_FRAMES;
    } else {
      dispClear();
    }
  }
}

// ===== Public API =====
void frankenphone_init() {
  // Hardware
  pinMode(PIN_BEAM_0, INPUT_PULLUP);
  pinMode(PIN_MAGNET_CTRL, OUTPUT); digitalWrite(PIN_MAGNET_CTRL, LOW);
  pinMode(PIN_BUZZER, OUTPUT);      buzzerOff();
  pinMode(LED_ARMED, OUTPUT);
  pinMode(LED_HOLD, OUTPUT);
  pinMode(LED_COOLDOWN, OUTPUT);

  randomSeed(analogRead(A0));

  // Display
  Wire.begin();
  alpha4.begin(DISP_ADDR);
  alpha4.setBrightness(8);
  dispShow4(IDLE_TEXT);

  // Idle
  state = ST_IDLE;
  tStateStart = millis();
  tSceneStart = 0;

  console_log("Frankenphone: init idle");
}

void scene_frankenphone() {
  // Start from IDLE on beam
  tSceneStart = millis();
  tStateStart = tSceneStart;
  state = ST_HOLD;

  magnetOn();
  holdDisplayInit();

  console_log("Frankenphone: HOLD start");
  tel_emit_transition("HOLD");
}

void frankenphone_update() {
  unsigned long now = millis();

  // Debounce beam
  beam_update();

  // 1 Hz telemetry
  static unsigned long lastTel = 0;
  if (now - lastTel >= TEL_HZ_MS) {
    tel_emit_now();
    lastTel = now;
  }

  // Run modem sound across the whole 25 s when scene is active
  if (state == ST_HOLD || state == ST_COOLDOWN) {
    unsigned long tSinceScene = now - tSceneStart;
    modemSound25s(tSinceScene);
  } else {
    buzzerOff();
  }

  switch (state) {
    case ST_IDLE:
      animateGreenArmed();
      dispShow4(IDLE_TEXT);
      if (beam_active()) {
        scene_frankenphone();
      }
      break;

    case ST_HOLD: {
      unsigned long el = now - tStateStart;
      animateRedHold(el);
      holdDisplayUpdate();

      if (el >= HOLD_MS) {
        // Transition
        magnetOff();
        state = ST_COOLDOWN;
        tStateStart = now;
        cooldownDisplayStart();
        console_log("Frankenphone: COOLDOWN start");
        tel_emit_transition("COOLDOWN");
      }
    } break;

    case ST_COOLDOWN:
      animateYellowCooldown();
      cooldownDisplayUpdate();

      if (now - tStateStart >= COOLDOWN_MS) {
        state = ST_IDLE;
        tStateStart = now;
        dispShow4(IDLE_TEXT);
        buzzerOff();
        console_log("Frankenphone: rearmed");
        tel_emit_transition("IDLE");
      }
      break;
  }

  // small breather
  delay(2);
}