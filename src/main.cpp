#include <Arduino.h>
#include "display.hpp"
#include "effects.hpp"
#include "console.hpp"
#include "scenes.hpp"
#include "inputs.hpp"
#include "settings.hpp"
#include "gopro.hpp"
#include "recorder.hpp"

// =======================
// Pin assignments (outputs)
// =======================
static const uint8_t PIN_MAGNET_CTRL = 6;   // MOSFET gate -> electromagnet
static const uint8_t PIN_BUZZER      = 8;   // Passive buzzer (+); (-) to GND
static const uint8_t LED_ARMED       = 10;  // Green indicator
static const uint8_t LED_HOLD        = 11;  // Red indicator
static const uint8_t LED_COOLDOWN    = 12;  // Yellow indicator

// Devices (relay IN pins)
static const uint8_t PIN_GOPRO_PWR   = 22;  // GoPro USB 5V relay IN
static const uint8_t PIN_TASCAM_PWR  = 23;  // DR-05 USB 5V relay IN

// Default timed record durations (adjust as needed)
static const uint32_t GOPRO_REC_MS   = 120000; // 2 min
static const uint32_t AUDIO_REC_MS   = 120000; // 2 min

// =======================
// Break-beam runtime tables (populated from EEPROM on boot)
// =======================
uint8_t BEAM_PINS[6];   // Arduino pin for each beam (LOW = broken)
Scene   BEAM_SCENE[6];  // Scene to fire for each beam index

// Forward: console.cpp references this to re-apply mapping after edits
extern void apply_mapping_from_settings();

// =======================
// Console callback helpers
// (route changes into settings so EEPROM stays in sync)
// =======================
static void setHold(uint32_t ms){ settings_set_hold(ms); }
static void setCool(uint32_t ms){ settings_set_cool(ms); }
static void setBright(uint8_t b){ settings_set_brightness(b); }

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

// Helper: convert persisted scene code (uint8_t) -> enum Scene
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

// Re-pull mapping from settings into BEAM_* arrays
void apply_mapping_from_settings(){
  auto& S = settings_ref();
  for (uint8_t i=0;i<6;i++){
    BEAM_PINS[i]  = S.beam_pins[i];
    BEAM_SCENE[i] = codeToScene(S.beam_scene[i]);
  }
}

// ---- console device adapters (hooked via console_attach_devices) ----
static void gopro_on()              { gopro_power(true); }
static void gopro_off()             { gopro_power(false); }
static void gopro_ms(uint32_t ms)   { gopro_record_for(ms); }
static void audio_on()              { recorder_power(true); }
static void audio_off()             { recorder_power(false); }
static void audio_ms(uint32_t ms)   { recorder_record_for(ms); }

// =======================
// setup / loop
// =======================
void setup(){
  // Seed RNG (harmless without sensor on A0)
  randomSeed(analogRead(A0));

  // Display first (so brightness applies cleanly)
  display_begin(0x70, 8);     // I2C addr, brightness 0..15
  display_idle("OBEY");

  // Settings: load from EEPROM (or defaults) and "apply" to subsystems
  settings_begin(
    /*applyHold*/      [](uint32_t v){ effects_setHoldMs(v); },
    /*applyCooldown*/  [](uint32_t){ /* scenes can read settings_ref() */ },
    /*applyDebounce*/  [](uint16_t v){ inputs_setDebounce(v); },
    /*applyRearm*/     [](uint32_t v){ inputs_setRearm(v); },
    /*applyBrightness*/[](uint8_t b){ display_set_brightness(b); }
  );

  // Copy persisted mapping into runtime arrays
  apply_mapping_from_settings();

  // Effects/Inputs after settings have been applied
  auto& S = settings_ref();
  effects_begin(LED_ARMED, LED_HOLD, LED_COOLDOWN, PIN_BUZZER, PIN_MAGNET_CTRL, S.hold_ms);
  inputs_begin(BEAM_PINS, S.debounce_ms, S.rearm_ms);

  // Start state machine
  scenes_begin();

  // Devices
  gopro_begin(PIN_GOPRO_PWR);
  recorder_begin(PIN_TASCAM_PWR);

  // Serial console
  console_begin(115200);
  console_attach(setHold, setCool, setBright, forceState, printStatus);
  console_attach_devices(gopro_on, gopro_off, gopro_ms,
                         audio_on, audio_off, audio_ms);
}

void loop(){
  // Always keep console responsive
  console_update();

  // Sensors + scene logic
  inputs_update();
  scenes_update();

  // If any beam fires, jump to its mapped scene (one per tick)
  for (uint8_t i=0; i<6; i++){
    if (inputs_triggered(i)){
      scenes_set(BEAM_SCENE[i]);
      break;
    }
  }

  // GoPro/Audio trigger on transition to Graveyard
  static Scene last = Scene::Standby;
  Scene now = scenes_current();
  if (now != last) {
    if (now == Scene::Graveyard) {
      gopro_record_for(GOPRO_REC_MS);
      recorder_record_for(AUDIO_REC_MS);
    }
    last = now;
  }

  // Service timed power-offs
  gopro_update();
  recorder_update();

  delay(2); // light scheduler tick
}