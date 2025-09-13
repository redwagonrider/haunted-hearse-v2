#include <Arduino.h>
#include "settings.hpp"
#include "scenes.hpp"

// These are defined in src/main.cpp (our current design keeps them there)
extern uint8_t BEAM_PINS[6];
extern Scene   BEAM_SCENE[6];

// Local helper: translate persisted scene code -> our minimal Scene enum
static Scene codeToScene(uint8_t c){
  // We currently implement only Standby (0) and FrankenLab (6).
  // Other codes fallback to Standby to avoid crashes.
  switch(c){
    case 6: return Scene::FrankenLab;
    case 0:
    default: return Scene::Standby;
  }
}

// This function is referenced by other modules (e.g. console) after settings change.
// Re-copy pins + scene mapping from EEPROM-backed settings into the runtime tables.
void apply_mapping_from_settings(){
  auto& S = settings_ref();
  for (uint8_t i = 0; i < 6; ++i){
    BEAM_PINS[i]  = S.beam_pins[i];
    BEAM_SCENE[i] = codeToScene(S.beam_scene[i]);
  }
}