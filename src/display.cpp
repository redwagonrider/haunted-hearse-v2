#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include "display.hpp"

// --- Private state ---
static Adafruit_AlphaNum4 alpha;
static uint8_t DISP_ADDR = 0x70;
static uint8_t BRIGHT = 8;

// Cooldown frames (4 chars each)
static const char* GRANT_FRAMES[] = {"ACES","GRTD","DONE","OPEN"};
static const uint8_t N_FRAMES = 4;

// Sequence data
static String digits16, mmYY, pinStr, zip5;

// Scroller state
enum DispPhase { PH_SYSOVR, PH_DIGITS, PH_MMYY, PH_PINFLASH, PH_PINNUM, PH_ZIP, PH_DONE };
static DispPhase dphase = PH_SYSOVR;

static String scrollBuf;
static uint16_t scrollDelay = 120;
static unsigned long tScroll = 0, tPinFlash = 0, tFlash = 0;
static int scrollIndex = 0;
static bool flashOn = false, pinOn = false;
static uint8_t flashes = 0, frameIdx = 0;

// --- Private helpers ---
static inline void show4(const char* s4){
  for(int i=0;i<4;i++) alpha.writeDigitAscii(i, s4[i]);
  alpha.writeDisplay();
}
static inline void clear(){ alpha.clear(); alpha.writeDisplay(); }

static String rndDigits(uint8_t n){
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
  char buf[6]; snprintf(buf,sizeof(buf),"%02u/%02u", m, (uint8_t)(y%100));
  return String(buf);
}

// --- Public API ---
void display_begin(uint8_t i2c_addr, uint8_t brightness){
  DISP_ADDR = i2c_addr; BRIGHT = brightness;
  Wire.begin();
  alpha.begin(DISP_ADDR);
  alpha.setBrightness(BRIGHT);
  clear();
}
void display_show4(const char* s4){ show4(s4); }
void display_clear(){ clear(); }
void display_idle(const char* four){ show4(four); }
void display_set_brightness(uint8_t b){BRIGHT = (b > 15) ? 15 : b;
alpha.setBrightness(BRIGHT);}

// Hold (trigger) sequence
void display_hold_init(){
  dphase = PH_SYSOVR;
  digits16 = rndDigits(16);
  mmYY     = nearFutureMMYY();
  pinStr   = "PIN " + rndDigits(3);
  zip5     = rndDigits(5);
  scrollBuf   = "    SYSTEM OVERRIDE    ";
  scrollDelay = 140;
  scrollIndex = 0;
  tScroll     = 0;
}
void display_hold_update(){
  unsigned long now = millis();
  if (now - tScroll < scrollDelay) return;
  tScroll = now;

  auto stepScroll = [&](String& buf)->bool {
    if (scrollIndex + 4 <= (int)buf.length()){
      show4(buf.substring(scrollIndex, scrollIndex+4).c_str());
      scrollIndex++;
      return false;
    }
    return true;
  };

  static String buf;
  switch (dphase){
    case PH_SYSOVR:
      buf = "    SYSTEM OVERRIDE    ";
      if (stepScroll(buf)){ dphase = PH_DIGITS; scrollDelay = 180; scrollIndex = 0; }
      break;
    case PH_DIGITS:
      buf = "    " + digits16 + "    ";
      if (stepScroll(buf)){ dphase = PH_MMYY; scrollDelay = 120; scrollIndex = 0; }
      break;
    case PH_MMYY:
      buf = "    " + mmYY + "    ";
      if (stepScroll(buf)){ dphase = PH_PINFLASH; flashes = 0; pinOn = false; tPinFlash = now; }
      break;
    case PH_PINFLASH: {
      const uint16_t onMs=180, offMs=140;
      uint16_t dur = pinOn ? onMs : offMs;
      if (now - tPinFlash >= dur){
        tPinFlash = now; pinOn = !pinOn;
        if (pinOn) show4("PIN ");
        else { clear(); flashes++; }
        if (flashes >= 3 && !pinOn){
          dphase = PH_PINNUM; scrollDelay = 120; scrollIndex = 0;
        }
      }
    } break;
    case PH_PINNUM:
      buf = "    PIN " + digits16.substring(0,3) + "    "; // using 3 of earlier digits for display width
      if (stepScroll(buf)){ dphase = PH_ZIP; scrollDelay = 120; scrollIndex = 0; }
      break;
    case PH_ZIP:
      buf = "    " + zip5 + "    ";
      if (stepScroll(buf)){ dphase = PH_DONE; }
      break;
    case PH_DONE: break;
  }
}

// Cooldown sequence
void display_cooldown_init(){
  flashOn = false; frameIdx = 0; tFlash = 0;
  clear();
}
void display_cooldown_update(){
  unsigned long now = millis();
  const uint16_t FLASH_ON_MS = 220, FLASH_OFF_MS = 180;
  uint16_t dur = flashOn ? FLASH_ON_MS : FLASH_OFF_MS;
  if (now - tFlash >= dur){
    tFlash = now; flashOn = !flashOn;
    if (flashOn){ show4(GRANT_FRAMES[frameIdx]); frameIdx = (frameIdx+1) % N_FRAMES; }
    else clear();
  }
}