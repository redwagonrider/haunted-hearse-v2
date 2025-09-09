#include "effects.hpp"

// --- stored pins & timing ---
static uint8_t P_LED_ARMED = 10;
static uint8_t P_LED_HOLD = 11;
static uint8_t P_LED_COOLDOWN = 12;
static uint8_t P_BUZZER = 8;
static uint8_t P_MAGNET = 6;
static unsigned long HOLD_MS_CFG = 5000;

// --- public API ---
void effects_begin(uint8_t pinLedArmed,
                   uint8_t pinLedHold,
                   uint8_t pinLedCooldown,
                   uint8_t pinBuzzer,
                   uint8_t pinMagnet,
                   unsigned long holdMs)
{
  P_LED_ARMED    = pinLedArmed;
  P_LED_HOLD     = pinLedHold;
  P_LED_COOLDOWN = pinLedCooldown;
  P_BUZZER       = pinBuzzer;
  P_MAGNET       = pinMagnet;
  HOLD_MS_CFG    = holdMs;

  pinMode(P_LED_ARMED, OUTPUT);
  pinMode(P_LED_HOLD, OUTPUT);
  pinMode(P_LED_COOLDOWN, OUTPUT);
  pinMode(P_BUZZER, OUTPUT);
  pinMode(P_MAGNET, OUTPUT);

  digitalWrite(P_MAGNET, LOW);
  noTone(P_BUZZER);
}

void effects_setHoldMs(unsigned long holdMs){ HOLD_MS_CFG = holdMs; }

// --- magnet ---
void effects_magnet_on()  { digitalWrite(P_MAGNET, HIGH); }
void effects_magnet_off() { digitalWrite(P_MAGNET, LOW);  }

// --- sound ---
void effects_sound_stop(){ noTone(P_BUZZER); }

// --- LED effects ---
void effects_idle_update(){
  unsigned long ms = millis();
  const unsigned long period = 2400;
  unsigned long t = ms % period;
  int val = (t < period/2) ? map(t, 0, period/2, 30, 255)
                           : map(t, period/2, period, 255, 30);
  analogWrite(P_LED_ARMED, val);
  analogWrite(P_LED_HOLD, 0);
  analogWrite(P_LED_COOLDOWN, 0);
}

void effects_cooldown_update(){
  static unsigned long deadline = 0;
  static int yellow = 0;
  unsigned long now = millis();
  if (now >= deadline){
    yellow = random(40, 255);
    deadline = now + (unsigned long)random(20, 120);
    analogWrite(P_LED_COOLDOWN, yellow);
  }
  analogWrite(P_LED_ARMED, 0);
  analogWrite(P_LED_HOLD, 0);
}

static void modem_sound(unsigned long t){
  // Stronger, varied “modem-ish” behavior
  if (t < 300) tone(P_BUZZER, 2400);
  else if (t < 700) tone(P_BUZZER, ((millis()/20)&1)?1200:2200);
  else if (t < 1400){
    int f = 500 + (int)((t-700)*(1600.0/700.0));
    tone(P_BUZZER, f);
  } else if (t < 2200){
    if ((millis() & 0x03)==0) tone(P_BUZZER, random(600,3000));
  } else if (t < 3200){
    tone(P_BUZZER, ((millis()/40)&1)?1300:1800);
  } else if (t < HOLD_MS_CFG){
    tone(P_BUZZER, 1000);
  } else {
    noTone(P_BUZZER);
  }
}

void effects_hold_update(unsigned long elapsedMs){
  // Red “stutter” grows faster/brighter while hold runs
  const unsigned int PERIOD_SLOW_MS = 300;
  const unsigned int PERIOD_FAST_MS = 40;
  unsigned long clamped = (elapsedMs > HOLD_MS_CFG) ? HOLD_MS_CFG : elapsedMs;
  unsigned int period = PERIOD_SLOW_MS
                      - ( (long)(PERIOD_SLOW_MS - PERIOD_FAST_MS) * (long)clamped ) / (long)HOLD_MS_CFG;
  int brightness = 60 + (int)((195L * (long)clamped) / (long)HOLD_MS_CFG);
  brightness = constrain(brightness, 0, 255);
  bool on = (millis() % period) < (period * 45UL / 100UL);

  analogWrite(P_LED_HOLD, on ? brightness : 0);
  analogWrite(P_LED_ARMED, 0);
  analogWrite(P_LED_COOLDOWN, 0);

  // Audio
  modem_sound(elapsedMs);
}
