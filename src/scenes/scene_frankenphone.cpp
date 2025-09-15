// src/scenes/scene_frankenphone.cpp
//
// Frankenphones Lab scene with exact text flow requested:
//
// HOLD 8 s total (magnet ON first 5 s):
//   - Flash "----" two times
//   - Scroll "SYSTEM OVERRIDE"
//   - Scroll 16 random digits
//   - Flash MMYY
//   - Scroll "PIN " + 3 digits
//   - Scroll 5-digit ZIP once
// COOLDOWN 20 s:
//   - Yellow LED flicker
//   - Flash cycle: ACES, GRTD, DONE, OPEN, OHIO
//   - A couple of PIN flashes with 3 digits
//
// Uses display arbitration so HOLD text cannot be stomped.
// Pins and LED labels come from pins.hpp.

#include <Arduino.h>
#include "pins.hpp"
#include "display.hpp"
#include "console.hpp"
#include "scenes/scene_frankenphone.hpp"

// ---------- Constants ----------
static const char*  OWNER            = "FRANK";
static const uint8_t OWNER_PRIO      = 10;

static const unsigned long HOLD_MS     = 8000UL;   // 8 s total hold
static const unsigned long MAG_ON_MS   = 5000UL;   // magnet ON first 5 s
static const unsigned long COOLDOWN_MS = 20000UL;  // 20 s cooldown

// ---------- State ----------
enum PhaseState { IDLE = 0, HOLD, COOLDOWN };
static PhaseState g_state = IDLE;
static unsigned long g_tPhaseStart = 0;

// ---------- Modem mute control ----------
static bool g_muted = false;

// ---------- LED helpers ----------
inline void magnetOn()  { digitalWrite(PIN_MAGNET_CTRL, HIGH); }
inline void magnetOff() { digitalWrite(PIN_MAGNET_CTRL, LOW);  }
inline void buzzerOff() { noTone(PIN_BUZZER); }

static void animateGreenIdle() {
  unsigned long ms = millis();
  const unsigned long period = 2400;
  unsigned long t = ms % period;
  int val = (t < period/2) ? map(t, 0, period/2, 30, 255)
                           : map(t, period/2, period, 255, 30);
  analogWrite(LED_ARMED, val);
  analogWrite(LED_HOLD, 0);
  analogWrite(LED_COOLDOWN, 0);
}
static void animateRedHold(unsigned long holdElapsed) {
  const unsigned int PERIOD_SLOW_MS = 300;
  const unsigned int PERIOD_FAST_MS = 40;
  unsigned long clamped = (holdElapsed > HOLD_MS) ? HOLD_MS : holdElapsed;
  unsigned int period = PERIOD_SLOW_MS
                      - ( (long)(PERIOD_SLOW_MS - PERIOD_FAST_MS) * (long)clamped ) / (long)HOLD_MS;
  int brightness = 60 + (int)((195L * (long)clamped) / (long)HOLD_MS);
  brightness = constrain(brightness, 0, 255);
  unsigned long phase = millis() % period;
  bool on = (phase < (period * 45UL) / 100UL);
  analogWrite(LED_HOLD, on ? brightness : 0);
  analogWrite(LED_ARMED, 0);
  analogWrite(LED_COOLDOWN, 0);
}
static void animateYellowCooldown() {
  static unsigned long ledRandDeadline = 0;
  static int ledYellowPWM = 0;
  unsigned long now = millis();
  if (now >= ledRandDeadline) {
    ledYellowPWM = random(40, 255);
    unsigned long dwell = (unsigned long)random(20, 120);
    ledRandDeadline = now + dwell;
    analogWrite(LED_COOLDOWN, ledYellowPWM);
  }
  analogWrite(LED_ARMED, 0);
  analogWrite(LED_HOLD, 0);
}

// ---------- Modem sound (~8 s, moderate volume) ----------
static void modemSound(unsigned long t){
  if (g_muted) { buzzerOff(); return; }
  if      (t <  300) tone(PIN_BUZZER, 1700);
  else if (t <  700) tone(PIN_BUZZER, ((millis()/20)&1)?1200:2000);
  else if (t < 1400) tone(PIN_BUZZER, 500 + (int)((t-700)*(1600.0/700.0)));
  else if (t < 2200) { if ((millis()&0x03)==0) tone(PIN_BUZZER, random(600,3000)); }
  else if (t < 3000) tone(PIN_BUZZER, ((millis()/35)&1)?1300:1800);
  else if (t < 8000) tone(PIN_BUZZER, 1000);
  else buzzerOff();
}

// ---------- Random helpers ----------
static String randomDigits(uint8_t n){
  String s; s.reserve(n);
  for(uint8_t i=0;i<n;i++) s += char('0'+random(10));
  return s;
}
static String nearFutureMMYY(){
  uint8_t addMonths= random(1,18);
  uint8_t nowMonth = 7;
  uint16_t nowYear = 2025;
  uint16_t y = nowYear + (nowMonth+addMonths-1)/12;
  uint8_t  m = ((nowMonth-1+addMonths)%12)+1;
  char buf[5]; // 4 chars + NUL
  snprintf(buf,sizeof(buf),"%02u%02u", m, (uint8_t)(y%100));
  return String(buf); // "MMYY" 4 chars
}

// ---------- HOLD display sequence ----------
enum FPDispStage { FP_IDLE=0, FP_FLASH2, FP_SCROLL_SYS, FP_SCROLL_16, FP_FLASH_MMYY, FP_SCROLL_PIN3, FP_SCROLL_ZIP5, FP_DONE };
static FPDispStage fp_disp = FP_IDLE;

static String fp_scroll;
static uint16_t fp_idx = 0;
static unsigned long fp_next = 0;
static uint8_t fp_flashCount = 0;
static String fp_digits16;
static String fp_mmyy;
static String fp_pin3;
static String fp_zip5;

static void show4_owned(const char* s4) {
  display_print4_owned(OWNER, s4);
}
static void showStr4_owned(const String& s) {
  char b[5] = {' ',' ',' ',' ','\0'};
  for (uint8_t i=0;i<4 && i<s.length(); ++i) b[i]=s[i];
  display_print4_owned(OWNER, b);
}
static void scroll_init(const String& msg, uint16_t leadSpaces = 4, uint16_t tailSpaces = 4) {
  String lead(leadSpaces, ' ');
  String tail(tailSpaces, ' ');
  fp_scroll = lead + msg + tail;
  fp_idx = 0;
  fp_next = millis(); // first tick ASAP
}
static bool scroll_step(uint16_t step_ms = 160) {
  if (millis() < fp_next) return false;
  if (fp_idx + 4 > fp_scroll.length()) return true;
  showStr4_owned(fp_scroll.substring(fp_idx, fp_idx + 4));
  fp_idx++;
  fp_next = millis() + step_ms;
  return false;
}
static void hold_sequence_begin() {
  fp_digits16 = randomDigits(16);
  fp_mmyy     = nearFutureMMYY();
  fp_pin3     = String("PIN ") + randomDigits(3);  // will be scrolled
  fp_zip5     = randomDigits(5);

  fp_disp = FP_FLASH2;
  fp_flashCount = 0;
  fp_next = millis(); // immediate
}

// ---------- COOLDOWN helpers ----------
static const char* CD_FRAMES[] = {"ACES","GRTD","DONE","OPEN","OHIO"};
static const uint8_t CD_NFR = 5;
static uint8_t cd_frame = 0;
static unsigned long cd_nextFrame = 0;
static unsigned long cd_nextPin = 0;
static uint8_t cd_pinBudget = 2; // "a couple"

// Simple cooldown PIN flash mini-state
static bool cd_pinPhase = false; // false = show "PIN ", true = show " ddd"
static String cd_pinDigits;

// ---------- Public API ----------
void frankenphone_set_mute(bool muted) {
  g_muted = muted;
  if (g_muted) buzzerOff();
}

void frankenphone_init() {
  pinMode(PIN_MAGNET_CTRL, OUTPUT); magnetOff();
  pinMode(PIN_BUZZER, OUTPUT);      buzzerOff();
  pinMode(LED_ARMED, OUTPUT);
  pinMode(LED_HOLD, OUTPUT);
  pinMode(LED_COOLDOWN, OUTPUT);
  randomSeed(analogRead(A0));

  display_idle("OBEY");

  g_state = IDLE;
  fp_disp = FP_IDLE;
}

void scene_frankenphone() {
  g_tPhaseStart = millis();
  g_state = HOLD;

  // Actuators
  magnetOn(); // will auto-off at 5 s in update

  // Display: take ownership for hold + a little slack
  display_acquire(OWNER, OWNER_PRIO, HOLD_MS + 1500);
  display_set_brightness_owned(OWNER, 10);
  hold_sequence_begin();

  console_log("Frankenphone: HOLD start");
}

void frankenphone_update() {
  unsigned long now = millis();

  if (g_state == HOLD) {
    unsigned long elapsed = now - g_tPhaseStart;

    // Magnet window
    if (elapsed >= MAG_ON_MS) magnetOff();

    // Sound and lights
    modemSound(elapsed);
    animateRedHold(elapsed);

    // Keep display ownership alive during HOLD
    display_renew(OWNER, 1200);

    // Sequence steps
    switch (fp_disp) {
      case FP_FLASH2:
        if (now >= fp_next) {
          // Alternate "----" and blanks, total of 2 visible flashes
          if ((fp_flashCount % 2) == 0) show4_owned("----"); else show4_owned("    ");
          fp_flashCount++;
          fp_next = now + ((fp_flashCount % 2) ? 160 : 200); // on 200 ms, off 160 ms
          if (fp_flashCount >= 4) { // on, off, on, off
            scroll_init("SYSTEM OVERRIDE");
            fp_disp = FP_SCROLL_SYS;
          }
        }
        break;

      case FP_SCROLL_SYS:
        if (scroll_step(150)) fp_disp = FP_SCROLL_16;
        break;

      case FP_SCROLL_16: {
        // Scroll the full 16 digits, not just 4-chunks
        static bool seeded = false;
        if (!seeded) { scroll_init(fp_digits16); seeded = true; }
        if (scroll_step(180)) { seeded = false; fp_disp = FP_FLASH_MMYY; }
      } break;

      case FP_FLASH_MMYY: {
        static uint8_t flashes = 0;
        if (now >= fp_next) {
          if ((flashes % 2) == 0) show4_owned("DATE");
          else                    showStr4_owned(fp_mmyy);
          flashes++;
          fp_next = now + 500;
          if (flashes >= 4) { // DATE, MMYY, DATE, MMYY
            fp_disp = FP_SCROLL_PIN3;
          }
        }
      } break;

      case FP_SCROLL_PIN3: {
        static bool seeded = false;
        if (!seeded) { scroll_init(fp_pin3); seeded = true; }
        if (scroll_step(140)) { seeded = false; fp_disp = FP_SCROLL_ZIP5; }
      } break;

      case FP_SCROLL_ZIP5: {
        static bool seeded = false;
        if (!seeded) { scroll_init(fp_zip5); seeded = true; }
        if (scroll_step(140)) { seeded = false; fp_disp = FP_DONE; }
      } break;

      case FP_DONE:
      default:
        // Nothing; wait for HOLD to end
        break;
    }

    // End of HOLD -> COOLDOWN
    if (elapsed >= HOLD_MS) {
      g_state = COOLDOWN;
      g_tPhaseStart = now;
      buzzerOff();
      // release or let the lease expire shortly
      display_release(OWNER);

      // Prep cooldown text
      cd_frame = 0;
      cd_nextFrame = now;   // draw immediately
      cd_nextPin   = now + 1500;
      cd_pinBudget = 2;
      cd_pinPhase  = false;

      console_log("Frankenphone: COOLDOWN start");
    }

  } else if (g_state == COOLDOWN) {
    animateYellowCooldown();

    // Base frame flashing cycle via display_idle
    if (now >= cd_nextFrame) {
      // Flash: word then blank short, so it "flashes"
      static bool showWord = true;
      if (showWord) {
        display_idle(CD_FRAMES[cd_frame]);
        cd_nextFrame = now + 420;
      } else {
        display_idle("    ");
        cd_frame = (cd_frame + 1) % CD_NFR;
        cd_nextFrame = now + 180;
      }
      showWord = !showWord;
    }

    // Occasional PIN flashes during cooldown, only a couple
    if (cd_pinBudget > 0 && now >= cd_nextPin) {
      if (!cd_pinPhase) {
        display_idle("PIN ");
        cd_pinDigits = String(" ") + randomDigits(3); // e.g. " 123"
        cd_pinPhase = true;
        cd_nextPin = now + 450;
      } else {
        display_idle(cd_pinDigits.c_str());
        cd_pinPhase = false;
        cd_pinBudget--;
        cd_nextPin = now + 3000 + random(0, 1500); // next window later
      }
    }

    // Done with cooldown
    if (now - g_tPhaseStart >= COOLDOWN_MS) {
      g_state = IDLE;
      fp_disp = FP_IDLE;
      console_log("Frankenphone: rearmed");
    }

  } else { // IDLE
    animateGreenIdle();
    display_idle("OBEY");
  }
}