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

// Lock stamp
#define FRANKENPHONE_LOCK_VER "frankenphone-locked-2025-09-14-09sA"
#pragma message ("Building " FRANKENPHONE_LOCK_VER)

// Config switches
#define FRANKEN_DATE_MMYY          1
#define FRANKEN_INCLUDE_ZIP_STAGE  1
#define FRANKEN_DISP_BRIGHTNESS    8

// Modem loudness control
#define MODEM_ATTEN_PCT            35
#define MODEM_ATTEN_PERIOD_MS      12

// Pin sanity guards
#ifndef PIN_BEAM_0
#error "PIN_BEAM_0 must be defined in include/pins.hpp"
#endif
#if PIN_BEAM_0 != 2
#error "Frankenphone is locked to Beam 0 on D2. Update include/pins.hpp or remove this guard."
#endif
#ifndef PIN_MAGNET_CTRL
#error "PIN_MAGNET_CTRL must be defined"
#endif
#if PIN_MAGNET_CTRL != 6
#error "Magnet must be on D6 for this locked scene"
#endif
#ifndef PIN_BUZZER
#error "PIN_BUZZER must be defined"
#endif
#if PIN_BUZZER != 8
#error "Buzzer must be on D8 for this locked scene"
#endif

// Timings
static unsigned long HOLD_MS            = 5000;
static const unsigned long COOLDOWN_MS  = 20000;
static const unsigned long TEL_HZ_MS    = 1000;
static const unsigned long DEBOUNCE_MS  = 30;
static const unsigned long MODEM_TOTAL_MS = 9000; // 9 s

// Display
static Adafruit_AlphaNum4 alpha4;
static const uint8_t DISP_ADDR = 0x70;
static const char IDLE_TEXT[5] = "OBEY";
static const char* GRANT_FRAMES[] = {"ACES","GRTD","OPEN","DONE"};
static const uint8_t N_FRAMES = 4;

// State
enum RunState { ST_IDLE, ST_HOLD, ST_COOLDOWN };
static RunState state = ST_IDLE;
static unsigned long tStateStart = 0;
static unsigned long tSceneStart = 0;

// avoid ODR clash with display.cpp
enum FPDispPhase { FP_SYSOVR, FP_DIGITS, FP_DONEFLASH, FP_DATE, FP_PIN, FP_ZIP, FP_DONE };
static FPDispPhase dphase = FP_SYSOVR;

static String digits16, dateStr, pinStr, zip5;
static unsigned long tScroll = 0;
static uint16_t scrollDelay = 120;
static int scrollIndex = 0;
static String scrollBuf;
static bool blinkOn = false;
static uint8_t blinkCount = 0;
static unsigned long tBlink = 0;

static bool cdFlashOn = false;
static unsigned long cdTick = 0;
static uint8_t cdFrame = 0;
static const uint16_t FLASH_ON_MS = 220;
static const uint16_t FLASH_OFF_MS = 180;
static bool cooldownCycleStarted = false;

static bool cdPinInjectActive = false;
static uint8_t cdPinStage = 0;
static unsigned long cdPinUntil = 0;
static unsigned long cdNextPinAt = 0;

// IO helpers
static inline void magnetOn()  { digitalWrite(PIN_MAGNET_CTRL, HIGH); }
static inline void magnetOff() { digitalWrite(PIN_MAGNET_CTRL, LOW);  }

// mute control exposed to console
static bool gMute = false;
extern "C" void frankenphone_set_mute(bool m) { gMute = m; }

static bool gBuzzerOn = false;
static inline void buzzerToneAtten(uint16_t f) {
  if (gMute) { noTone(PIN_BUZZER); gBuzzerOn = false; return; }
  uint8_t on_ms = (MODEM_ATTEN_PERIOD_MS * MODEM_ATTEN_PCT) / 100;
  if (on_ms == 0) on_ms = 1;
  if ((millis() % MODEM_ATTEN_PERIOD_MS) < on_ms) {
    tone(PIN_BUZZER, f);
    gBuzzerOn = true;
  } else {
    noTone(PIN_BUZZER);
    gBuzzerOn = false;
  }
}
static inline void buzzerOff() { noTone(PIN_BUZZER); gBuzzerOn = false; }

// Beam with debounce (active LOW)
static uint8_t beam_state = 0, beam_last = 0;
static unsigned long tBeamChange = 0;
static inline uint8_t beam_raw() { return digitalRead(PIN_BEAM_0) == LOW ? 1 : 0; }
static void beam_update() {
  uint8_t r = beam_raw();
  if (r != beam_last) { beam_last = r; tBeamChange = millis(); }
  if (millis() - tBeamChange >= DEBOUNCE_MS) beam_state = r;
}
static inline uint8_t beam_active() { return beam_state; }

// Telemetry
static const char* phase_name() {
  switch (state) { case ST_HOLD: return "HOLD"; case ST_COOLDOWN: return "COOLDOWN"; default: return "IDLE"; }
}
static void tel_emit_now() {
  uint8_t r=0,g=0,b=0; effects_getRGB(r,g,b);
  HH::tel_emit("frankenphone", phase_name(), beam_active(),
               (digitalRead(PIN_MAGNET_CTRL)==HIGH)?1:0, gBuzzerOn?1:0, r,g,b);
}
static void tel_emit_transition(const char* newPhase) {
  uint8_t r=0,g=0,b=0; effects_getRGB(r,g,b);
  HH::tel_emit("frankenphone", newPhase ? newPhase : phase_name(), beam_active(),
               (digitalRead(PIN_MAGNET_CTRL)==HIGH)?1:0, gBuzzerOn?1:0, r,g,b);
}

// Display helpers
static void dispShow4(const char* s4){ for(int i=0;i<4;i++) alpha4.writeDigitAscii(i,s4[i]); alpha4.writeDisplay(); }
static void dispShowStr4(const String& s){ char buf[5]={' ',' ',' ',' ','\0'}; for(int i=0;i<4 && i<(int)s.length(); ++i) buf[i]=s[i]; dispShow4(buf); }
static void dispClear(){ alpha4.clear(); alpha4.writeDisplay(); }

static String randomDigits(uint8_t n){ String s; s.reserve(n); for(uint8_t i=0;i<n;i++) s += char('0'+random(10)); return s; }
static String nearFutureDateStr(){
  uint8_t addMonths = random(1, 19), nowMonth = 7; uint16_t nowYear = 2025;
  uint16_t y = nowYear + (nowMonth + addMonths - 1) / 12; uint8_t m = ((nowMonth - 1 + addMonths) % 12) + 1;
  char buf[6];
#if FRANKEN_DATE_MMYY
  snprintf(buf,sizeof(buf),"%02u/%02u", m, (uint8_t)(y%100));
#else
  snprintf(buf,sizeof(buf),"%02u/%02u", (uint8_t)(y%100), m);
#endif
  return String(buf);
}

// LEDs with RGB telemetry feed
static void animateGreenArmed(){
  unsigned long ms=millis(), period=2400, t=ms%period;
  int val=(t<period/2)? map(t,0,period/2,30,255) : map(t,period/2,period,255,30);
  analogWrite(LED_ARMED,val); analogWrite(LED_HOLD,0); analogWrite(LED_COOLDOWN,0);
  uint8_t g = map(val,30,255,16,96);
  effects_setRGB(0, g, 0); // restore RGB feed
}
static void animateRedHold(unsigned long holdElapsed){
  const unsigned int SLOW=300, FAST=40;
  unsigned long clamped = (holdElapsed>HOLD_MS)?HOLD_MS:holdElapsed;
  unsigned int period = SLOW - (unsigned int)(((long)(SLOW-FAST)*(long)clamped)/(long)HOLD_MS);
  int brightness = constrain(60 + (int)((195L*(long)clamped)/(long)HOLD_MS), 0, 255);
  bool on = (millis()%period) < (period*45UL)/100UL;
  analogWrite(LED_HOLD, on?brightness:0); analogWrite(LED_ARMED,0); analogWrite(LED_COOLDOWN,0);
  uint8_t r = on ? map(brightness,60,255,64,255) : 32;
  effects_setRGB(r, 32, 0); // restore RGB feed
}
static void animateYellowCooldown(){
  static unsigned long nextChange=0; static uint8_t yPWM=0; unsigned long now=millis();
  if(now>=nextChange){ yPWM=(uint8_t)random(40,255); nextChange=now+(unsigned long)random(20,120); analogWrite(LED_COOLDOWN,yPWM); }
  analogWrite(LED_ARMED,0); analogWrite(LED_HOLD,0);
  uint8_t y = map(yPWM,40,255,64,200);
  effects_setRGB(y, y*2/3, 0); // restore RGB feed
}

// Modem 9 s (Option A)
static void modemSound9s_profileA(unsigned long t){
  const unsigned long T_ANS_END=2160UL, T_V8_END=2760UL, T_TRAIN_END=4440UL, T_WARBLE_END=7920UL;
  if(t>=MODEM_TOTAL_MS){ buzzerOff(); return; }
  if(t<T_ANS_END){ unsigned long u=t%450UL; if(u<3UL) buzzerOff(); else buzzerToneAtten(2100); return; }
  if(t<T_V8_END){ unsigned long u=(t-T_ANS_END)/12UL; buzzerToneAtten((u&1)?1800:1200); return; }
  if(t<T_TRAIN_END){ static unsigned long next=0; static uint16_t f=1000; unsigned long now=millis();
    if(now>=next){ f=(uint16_t)random(700,2601); next=now+(unsigned long)random(8,13);} buzzerToneAtten(f); return; }
  if(t<T_WARBLE_END){ unsigned long u=(t-T_TRAIN_END)/37UL; buzzerToneAtten((u&1)?1200:2400); return; }
  buzzerToneAtten(1000);
}

// HOLD display
static bool stepScroll(){ if(scrollIndex+4 <= (int)scrollBuf.length()){ dispShowStr4(scrollBuf.substring(scrollIndex,scrollIndex+4)); scrollIndex++; return false; } return true; }
static void holdDisplayInit(){
  dphase=FP_SYSOVR; digits16=randomDigits(16); dateStr=nearFutureDateStr(); pinStr=String(" ")+randomDigits(3); zip5=randomDigits(5);
  scrollBuf="    SYSTEM OVERRIDE    "; scrollDelay=140; scrollIndex=0; tScroll=0; blinkOn=false; blinkCount=0; tBlink=0;
}
static void holdDisplayUpdate(){
  unsigned long now=millis(); if(now - tScroll < scrollDelay) return; tScroll = now;
  switch(dphase){
    case FP_SYSOVR: if(stepScroll()){ scrollBuf=String("    ")+digits16+String("    "); scrollDelay=180; scrollIndex=0; dphase=FP_DIGITS; } break;
    case FP_DIGITS: if(stepScroll()){ dphase=FP_DONEFLASH; blinkOn=false; blinkCount=0; tBlink=now; } break;
    case FP_DONEFLASH:
      if(now - tBlink >= 150U){ tBlink=now; blinkOn=!blinkOn; dispShowStr4(blinkOn?"DONE":"    ");
        if(++blinkCount>=6){ scrollBuf=String("    ")+dateStr+String("    "); scrollDelay=140; scrollIndex=0; dphase=FP_DATE; } }
      break;
    case FP_DATE: if(stepScroll()){ dphase=FP_PIN; dispShowStr4("PIN "); tBlink=now+240; blinkCount=0; } break;
    case FP_PIN:
      if(blinkCount==0 && now>=tBlink){ dispShowStr4(pinStr); tBlink=now+420; blinkCount=1; }
      else if(blinkCount==1 && now>=tBlink){
#if FRANKEN_INCLUDE_ZIP_STAGE
        scrollBuf=String("    ")+zip5+String("    "); scrollDelay=120; scrollIndex=0; dphase=FP_ZIP;
#else
        dphase=FP_DONE;
#endif
      }
      break;
    case FP_ZIP: if(stepScroll()) dphase=FP_DONE; break;
    case FP_DONE: default: break;
  }
}

// COOLDOWN display
static void cooldownDisplayStart(){
  cdFlashOn=false; cdTick=millis(); cdFrame=0; dispClear();
  cdNextPinAt = millis() + (unsigned long)random(2000,4200); // reduced rate
  cdPinInjectActive=false; cdPinStage=0; cdPinUntil=0;
}
static void scheduleNextCooldownPin(unsigned long now){ cdNextPinAt = now + (unsigned long)random(2000,4200); }
static bool cooldownPinInjection(unsigned long now){
  if(!cooldownCycleStarted) return false;
  if(!cdPinInjectActive){
    if(now < cdNextPinAt) return false;
    cdPinInjectActive=true; cdPinStage=0; pinStr=String(" ")+randomDigits(3); cdPinUntil=now+200;
  }
  if(now >= cdPinUntil){
    cdPinStage++;
    if(cdPinStage==1){ dispShowStr4(pinStr); cdPinUntil=now+380; }
    else { cdPinInjectActive=false; scheduleNextCooldownPin(now); return false; }
  } else {
    if(cdPinStage==0) dispShowStr4("PIN "); else dispShowStr4(pinStr);
  }
  return true;
}
static void cooldownGrantCycleUpdate(){
  unsigned long now=millis(); uint16_t dur = cdFlashOn?FLASH_ON_MS:FLASH_OFF_MS;
  if(now - cdTick >= dur){ cdTick=now; cdFlashOn=!cdFlashOn;
    if(cdFlashOn){ dispShow4(GRANT_FRAMES[cdFrame]); cdFrame=(cdFrame+1)%N_FRAMES; } else dispClear();
  }
}

// Public API
void frankenphone_init(){
  pinMode(PIN_BEAM_0,INPUT_PULLUP);
  pinMode(PIN_MAGNET_CTRL,OUTPUT); digitalWrite(PIN_MAGNET_CTRL,LOW);
  pinMode(PIN_BUZZER,OUTPUT); buzzerOff();
  pinMode(LED_ARMED,OUTPUT); pinMode(LED_HOLD,OUTPUT); pinMode(LED_COOLDOWN,OUTPUT);
  randomSeed(analogRead(A0));

  Wire.begin();
  alpha4.begin(DISP_ADDR);
  alpha4.setBrightness(FRANKEN_DISP_BRIGHTNESS);
  dispShow4(IDLE_TEXT);

  state = ST_IDLE;
  tStateStart = millis();
  tSceneStart = 0;
  cooldownCycleStarted = false;

  console_log("Frankenphone: init idle");
  console_log(FRANKENPHONE_LOCK_VER);
}

void scene_frankenphone(){
  tSceneStart = millis();
  tStateStart = tSceneStart;
  state = ST_HOLD;

  magnetOn();
  holdDisplayInit();

  console_log("Frankenphone: HOLD start");
  tel_emit_transition("HOLD");
}

void frankenphone_update(){
  unsigned long now = millis();

  beam_update();

  // Telemetry at 1 Hz
  static unsigned long lastTel = 0;
  if (now - lastTel >= TEL_HZ_MS) {
    tel_emit_now();
    lastTel = now;
  }

  // Modem sound within first 9 s of scene
  if (state == ST_HOLD || state == ST_COOLDOWN) {
    modemSound9s_profileA(now - tSceneStart);
  } else {
    buzzerOff();
  }

  switch (state) {
    case ST_IDLE:
      animateGreenArmed();
      dispShow4(IDLE_TEXT);
      if (beam_active()) scene_frankenphone();
      break;

    case ST_HOLD: {
      unsigned long el = now - tStateStart;
      animateRedHold(el);
      holdDisplayUpdate();

      if (el >= HOLD_MS) {
        magnetOff();
        state = ST_COOLDOWN;
        tStateStart = now;
        cooldownCycleStarted = false;
        console_log("Frankenphone: COOLDOWN start");
        tel_emit_transition("COOLDOWN");
      }
    } break;

    case ST_COOLDOWN:
      animateYellowCooldown();

      // Finish any remaining scripted stages then switch to grant cycle
      if (dphase != FP_DONE) {
        holdDisplayUpdate();
      } else {
        if (!cooldownCycleStarted) {
          cooldownDisplayStart();
          cooldownCycleStarted = true;
        }
        if (!cooldownPinInjection(now)) {
          cooldownGrantCycleUpdate();
        }
      }

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

  delay(2);
}