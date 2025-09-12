#include <Arduino.h>
#include "display.hpp"
#include "effects.hpp"
#include "console.hpp"
#include "scenes.hpp"
#include "inputs.hpp"
#include "settings.hpp"
#include "gopro.hpp"
#include "gopro.hpp"
#include "recorder.hpp"


// =======================
// Pin assignments (outputs)
// =======================
const uint8_t PIN_MAGNET_CTRL = 6;   // MOSFET gate -> electromagnet
const uint8_t PIN_BUZZER      = 8;   // Passive buzzer (+); (-) to GND
const uint8_t LED_ARMED       = 10;  // Green indicator
const uint8_t LED_HOLD        = 11;  // Red indicator
const uint8_t LED_COOLDOWN    = 12;  // Yellow indicator
const uint8_t PIN_GOPRO_PWR = 22;   // Relay IN pin for GoPro USB 5V
const uint32_t GOPRO_REC_MS = 20000; // 20 seconds (adjust as needed)
const uint8_t PIN_GOPRO_PWR   = 22;
const uint8_t PIN_TASCAM_PWR  = 23;
const uint32_t REC_DURATION   = 20000; // 20 secnds


// =======================
// Break-beam runtime tables (filled from EEPROM/defaults on boot)
// =======================
uint8_t BEAM_PINS[6];   // digital pins for 6 receivers (LOW=broken)
Scene   BEAM_SCENE[6];  // scene per beam index

// ------------------------------------------------------------------
// Helper: convert persisted scene code (uint8_t) -> enum Scene
// (Placed BEFORE any use so we don't need extern)
// ------------------------------------------------------------------
static Scene codeToScene(uint8_t c){
  switch(c){
    case 0:  return Scene::Standby;
    case 1:  return Scene::IntroCue;
    case 2:  return Scene::BloodRoom;
    case 3:  return Scene::Graveyard;
    case 4:  return Scene::FurRoom;
    case 5:  return Scene::OrcaDino;
    case 6:  return Scene::FrankenLab;
    case 7:  return Scene::MirrorRoom;
    case 8:  return Scene::ExitHole;
    case 9:  return Scene::PhoneLoading;
    default: return Scene::Standby;
  }
}

// ------------------------------------------------------------------
// Re-apply mapping from EEPROM-backed settings and re-init inputs
// (Console calls this after BMAP/BSCENE edits.)
// ------------------------------------------------------------------
void apply_mapping_from_settings(){
  auto& S = settings_ref();
  for (uint8_t i=0;i<6;i++){
    BEAM_PINS[i]  = S.beam_pins[i];
    BEAM_SCENE[i] = codeToScene(S.beam_scene[i]);
  }
  inputs_begin(BEAM_PINS, S.debounce_ms, S.rearm_ms);
}

// =======================
// Console callback helpers
// (route changes into settings so EEPROM stays in sync)
// =======================
static void setHold(uint32_t ms){ settings_set_hold(ms); }
static void setCool(uint32_t ms){ settings_set_cool(ms); }
static void setBright(uint8_t b){ settings_set_brightness(b); }
// Device control helpers
static void gopro_on()  { gopro_power(true); }
static void gopro_off() { gopro_power(false); }
static void gopro_ms(uint32_t ms) { gopro_record_for(ms); }

static void audio_on()  { recorder_power(true); }
static void audio_off() { recorder_power(false); }
static void audio_ms(uint32_t ms) { recorder_record_for(ms); }

static void printStatus(){
  Serial.println(F("\n--- STATUS ---"));
  Serial.print(F("Scene: ")); Serial.println(scenes_name(scenes_current()));

  auto& S = settings_ref();
  Serial.print(F("HOLD_MS="));     Serial.println(S.hold_ms);
  Serial.print(F("COOLDOWN_MS=")); Serial.println(S.cooldown_ms);
  Serial.print(F("DEBOUNCE_MS=")); Serial.println(S.debounce_ms);
  Serial.print(F("REARM_MS="));    Serial.println(S.rearm_ms);
  Serial.print(F("BRIGHT="));      Serial.println(S.brightness);

  Serial.println(F("Beam map (idx : pin -> scene):"));
  for (uint8_t i=0;i<6;i++){
    Serial.print(F("  ")); Serial.print(i);
    Serial.print(F(" : ")); Serial.print(BEAM_PINS[i]);
    Serial.print(F(" -> ")); Serial.println(scenes_name(BEAM_SCENE[i]));
  }

  inputs_printStatus(Serial);
  Serial.println(F("--------------\n"));
}

// Extendable mapping for STATE/SCENE console quick-jumps
static void forceState(int st){
  switch(st){
    case 0:  scenes_set(Scene::Standby);      break;
    case 1:  scenes_set(Scene::FrankenLab);   break;
    case 2:  scenes_set(Scene::MirrorRoom);   break;

    case 10: scenes_set(Scene::PhoneLoading); break;
    case 11: scenes_set(Scene::IntroCue);     break;
    case 12: scenes_set(Scene::BloodRoom);    break;
    case 13: scenes_set(Scene::Graveyard);    break;
    case 14: scenes_set(Scene::FurRoom);      break;
    case 15: scenes_set(Scene::OrcaDino);     break;
    case 16: scenes_set(Scene::FrankenLab);   break;
    case 17: scenes_set(Scene::MirrorRoom);   break;
    case 18: scenes_set(Scene::ExitHole);     break;
    default: /* ignore */                     break;
  }
}

void setup(){
  // Seed RNG (harmless without sensor on A0)
  randomSeed(analogRead(A0));

void setup(){
  // ... existing setup code ...
  gopro_begin(PIN_GOPRO_PWR);
  // ... rest of setup ...
}

void setup(){
  // ... existing setup
  gopro_begin(PIN_GOPRO_PWR);
  recorder_begin(PIN_TASCAM_PWR);
}

  // Display first (so brightness applies cleanly)
  display_begin(0x70, 8);     // I2C addr, brightness 0..15
  display_idle("OBEY");

  // Settings: load from EEPROM (or defaults) and "apply" to subsystems
  settings_begin(
    /*applyHold*/      [](uint32_t v){ effects_setHoldMs(v); },
    /*applyCooldown*/  [](uint32_t v){ (void)v; /* scenes can read settings_ref() */ },
    /*applyDebounce*/  [](uint16_t v){ inputs_setDebounce(v); },
    /*applyRearm*/     [](uint32_t v){ inputs_setRearm(v); },
    /*applyBrightness*/[](uint8_t b){ display_set_brightness(b); }
  );

  // Copy persisted mapping into runtime arrays and init inputs
  apply_mapping_from_settings();

  // Effects after settings have been applied
  auto& S = settings_ref();
  effects_begin(LED_ARMED, LED_HOLD, LED_COOLDOWN, PIN_BUZZER, PIN_MAGNET_CTRL, S.hold_ms);

  // Start state machine
  scenes_begin();

  // Serial console
  console_begin(115200);
  console_attach(setHold, setCool, setBright, forceState, printStatus);
console_attach_devices(gopro_on, gopro_off, gopro_ms,
                       audio_on, audio_off, audio_ms);
}

void loop(){
  // Always keep console responsive
  console_update();

void loop(){
  console_update();
  inputs_update();
  scenes_update();

  // Beam->scene jump (your existing block)
  for (uint8_t i=0; i<6; i++){
    if (inputs_triggered(i)){
      scenes_set(BEAM_SCENE[i]);
      break;
    }
  }

  // --- GoPro trigger on scene transition to Graveyard ---
  static Scene last = Scene::Standby;
  Scene now = scenes_current();
  if (now != last) {
    // Entering a new scene
    if (now == Scene::Graveyard) {
      // Start recording for GOPRO_REC_MS, will auto-stop
      gopro_record_for(GOPRO_REC_MS);
    }
    // Optional: stop instantly when leaving Graveyard (uncomment if desired)
    // if (last == Scene::Graveyard) {
    //   gopro_power(false);
    // }
    last = now;
  }

  // Service GoPro timers (powers OFF when time is up)
  gopro_update();

  delay(2);
}

void loop(){
  console_update();
  inputs_update();
  scenes_update();

  // beam->scene jump ...
  
  static Scene last = Scene::Standby;
  Scene now = scenes_current();
  if (now != last) {
    if (now == Scene::Graveyard) {
      gopro_record_for(REC_DURATION);
      recorder_record_for(REC_DURATION);
    }
    last = now;
  }

  gopro_update();
  recorder_update();
  delay(2);
}

  // Sensors + scene logic
  inputs_update();
  scenes_update();

  // If any beam fires, jump to its mapped scene (one per tick)
  for (uint8_t i=0; i<6; i++){
    if (inputs_triggered(i)){
      if (console_should_log()){
        Serial.print(F("[LOG] beam=")); Serial.print(i);
        Serial.print(F(" pin=")); Serial.print(BEAM_PINS[i]);
        Serial.print(F(" -> scene=")); Serial.println(scenes_name(BEAM_SCENE[i]));
      }
      scenes_set(BEAM_SCENE[i]);
      break;
    }
  }

  delay(2); // light scheduler tick
}