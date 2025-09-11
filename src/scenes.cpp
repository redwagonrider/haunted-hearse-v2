#include "scenes.hpp"
#include "display.hpp"
#include "effects.hpp"
// If/when you add Falcon/Pi control headers, include them here

// ------- internal state -------
static Scene g_scene = Scene::Standby;
static unsigned long g_enterMs = 0;
static unsigned long g_timerStart = 0;
static uint32_t g_timerDur = 0;

// Forward decls for per-scene handlers
static void enter_PhoneLoading();
static void update_PhoneLoading();
static void exit_PhoneLoading();

static void enter_IntroCue();
static void update_IntroCue();
static void exit_IntroCue();

static void enter_BloodRoom();
static void update_BloodRoom();
static void exit_BloodRoom();

static void enter_Graveyard();
static void update_Graveyard();
static void exit_Graveyard();

static void enter_FurRoom();
static void update_FurRoom();
static void exit_FurRoom();

static void enter_OrcaDino();
static void update_OrcaDino();
static void exit_OrcaDino();

static void enter_FrankenLab();
static void update_FrankenLab();
static void exit_FrankenLab();

static void enter_MirrorRoom();
static void update_MirrorRoom();
static void exit_MirrorRoom();

static void enter_ExitHole();
static void update_ExitHole();
static void exit_ExitHole();

static void enter_Standby();
static void update_Standby();
static void exit_Standby();

// ------- small utilities -------
void scenes_startTimer(uint32_t ms){ g_timerStart = millis(); g_timerDur = ms; }
bool scenes_timerExpired(){ return (g_timerDur > 0) && (millis() - g_timerStart >= g_timerDur); }
uint32_t scenes_elapsed(){ return millis() - g_enterMs; }

// Stub “notifiers” – later wire to FPP/UDP/HTTP/etc.
void scenes_notify_audio(const __FlashStringHelper* msg){ (void)msg; /* TODO: send to Pi */ }
void scenes_notify_video(const __FlashStringHelper* msg){ (void)msg; /* TODO: send to Pi */ }
void scenes_notify_pixels(const __FlashStringHelper* msg){ (void)msg; /* TODO: send to Falcon */ }

// Scene pretty-names
const __FlashStringHelper* scenes_name(Scene s){
  switch(s){
    case Scene::PhoneLoading: return F("PhoneLoading");
    case Scene::IntroCue:     return F("IntroCue");
    case Scene::BloodRoom:    return F("BloodRoom");
    case Scene::Graveyard:    return F("Graveyard");
    case Scene::FurRoom:      return F("FurRoom");
    case Scene::OrcaDino:     return F("OrcaDino");
    case Scene::FrankenLab:   return F("FrankenLab");
    case Scene::MirrorRoom:   return F("MirrorRoom");
    case Scene::ExitHole:     return F("ExitHole");
    case Scene::Standby:      return F("Standby");
  }
  return F("Unknown");
}

// ------- public API -------
void scenes_begin(){
  g_scene = Scene::Standby;
  g_enterMs = millis();
  enter_Standby();
}

Scene scenes_current(){ return g_scene; }

static void enterScene(Scene s){
  // Call exit of old scene
  switch(g_scene){
    case Scene::PhoneLoading: exit_PhoneLoading(); break;
    case Scene::IntroCue:     exit_IntroCue();     break;
    case Scene::BloodRoom:    exit_BloodRoom();    break;
    case Scene::Graveyard:    exit_Graveyard();    break;
    case Scene::FurRoom:      exit_FurRoom();      break;
    case Scene::OrcaDino:     exit_OrcaDino();     break;
    case Scene::FrankenLab:   exit_FrankenLab();   break;
    case Scene::MirrorRoom:   exit_MirrorRoom();   break;
    case Scene::ExitHole:     exit_ExitHole();     break;
    case Scene::Standby:      exit_Standby();      break;
  }

  g_scene = s;
  g_enterMs = millis();
  g_timerDur = 0;

  switch(s){
    case Scene::PhoneLoading: enter_PhoneLoading(); break;
    case Scene::IntroCue:     enter_IntroCue();     break;
    case Scene::BloodRoom:    enter_BloodRoom();    break;
    case Scene::Graveyard:    enter_Graveyard();    break;
    case Scene::FurRoom:      enter_FurRoom();      break;
    case Scene::OrcaDino:     enter_OrcaDino();     break;
    case Scene::FrankenLab:   enter_FrankenLab();   break;
    case Scene::MirrorRoom:   enter_MirrorRoom();   break;
    case Scene::ExitHole:     enter_ExitHole();     break;
    case Scene::Standby:      enter_Standby();      break;
  }
}

void scenes_set(Scene s){ enterScene(s); }

void scenes_next(){
  Scene s = g_scene;
  switch(s){
    case Scene::PhoneLoading: s = Scene::IntroCue; break;
    case Scene::IntroCue:     s = Scene::BloodRoom; break;
    case Scene::BloodRoom:    s = Scene::Graveyard; break;
    case Scene::Graveyard:    s = Scene::FurRoom; break;
    case Scene::FurRoom:      s = Scene::OrcaDino; break;
    case Scene::OrcaDino:     s = Scene::FrankenLab; break;
    case Scene::FrankenLab:   s = Scene::MirrorRoom; break;
    case Scene::MirrorRoom:   s = Scene::ExitHole; break;
    case Scene::ExitHole:     s = Scene::Standby; break;
    case Scene::Standby:      s = Scene::PhoneLoading; break;
  }
  enterScene(s);
}

void scenes_update(){
  switch(g_scene){
    case Scene::PhoneLoading: update_PhoneLoading(); break;
    case Scene::IntroCue:     update_IntroCue();     break;
    case Scene::BloodRoom:    update_BloodRoom();    break;
    case Scene::Graveyard:    update_Graveyard();    break;
    case Scene::FurRoom:      update_FurRoom();      break;
    case Scene::OrcaDino:     update_OrcaDino();     break;
    case Scene::FrankenLab:   update_FrankenLab();   break;
    case Scene::MirrorRoom:   update_MirrorRoom();   break;
    case Scene::ExitHole:     update_ExitHole();     break;
    case Scene::Standby:      update_Standby();      break;
  }
}

// ====== TEMPLATE IMPLEMENTATIONS ======
// Keep these short; call into effects/display. Replace TODOs incrementally.

// Phone Loading
static void enter_PhoneLoading(){
  display_idle("LOAD");                // Placeholder
  effects_magnet_off();
  effects_sound_stop();
  scenes_notify_audio(F("CUE: intro-music"));
}
static void update_PhoneLoading(){
  effects_idle_update();
  // TODO: if operator presses start (GPIO or console), advance:
  // if (/*start condition*/) scenes_next();
}
static void exit_PhoneLoading(){}

// Intro & Tunnel Blackout
static void enter_IntroCue(){
  display_idle("CARD");
  scenes_notify_pixels(F("ALL_BLACKOUT"));   // Falcon cue later
  scenes_startTimer(4000);                   // 4s blackout
}
static void update_IntroCue(){
  // Bloody hand flicker could be a dedicated effect later
  if (scenes_timerExpired()) scenes_next();
}
static void exit_IntroCue(){}

// Blood Room
static void enter_BloodRoom(){
  display_idle("BLOD");
  scenes_notify_audio(F("MUSIC: start"));
  scenes_notify_pixels(F("BLOODROOM_ON"));
  scenes_startTimer(6000); // keep simple; refine later
}
static void update_BloodRoom(){
  // TODO: blood drip pulses; screams (Pi)
  if (scenes_timerExpired()) scenes_next();
}
static void exit_BloodRoom(){}

// Graveyard
static void enter_Graveyard(){
  display_idle("GRVE");
  scenes_notify_pixels(F("GRAVES_ON"));
  scenes_startTimer(7000);
}
static void update_Graveyard(){
  // Fire sky flicker via effects engine or Falcon sequence
  effects_cooldown_update(); // reuse yellow flicker as placeholder
  if (scenes_timerExpired()) scenes_next();
}
static void exit_Graveyard(){}

// Fur Room
static void enter_FurRoom(){
  display_idle("FUR ");
  scenes_notify_pixels(F("GRAVE_DIM"));
  scenes_startTimer(6000);
}
static void update_FurRoom(){
  effects_idle_update(); // gentle pulse
  // TODO: sensor trigger -> blast
  if (scenes_timerExpired()) scenes_next();
}
static void exit_FurRoom(){}

// Orca & Dino (placeholder)
static void enter_OrcaDino(){
  display_idle("ORCA");
  scenes_startTimer(5000);
}
static void update_OrcaDino(){
  if (scenes_timerExpired()) scenes_next();
}
static void exit_OrcaDino(){}

// Frankenphone's Lab (your working scene)
static void enter_FrankenLab(){
  display_hold_init();                 // start SYSTEM OVERRIDE sequence
  effects_magnet_on();                 // lock
  scenes_startTimer(5000);             // re-using your HOLD
  scenes_notify_audio(F("MODEM: chirp"));
}
static void update_FrankenLab(){
  uint32_t el = scenes_elapsed();
  effects_hold_update(el);
  display_hold_update();
  if (scenes_timerExpired()){
    effects_magnet_off();
    effects_sound_stop();
    display_cooldown_init();           // ACES/GRTD flash
    scenes_startTimer(20000);          // cooldown/flash before next
    // Move to COOLDOWN-like behavior by staying in this scene or jump next after timer
  }
  // Optional: after cooldown timer, advance automatically:
  if (g_timerDur == 20000 && scenes_timerExpired()){
    scenes_next();
  }
}
static void exit_FrankenLab(){}

// Mirror Room
static void enter_MirrorRoom(){
  display_idle("MIRR");
  scenes_notify_pixels(F("MATRIX_SEQUENCE"));
  scenes_notify_audio(F("ZAPS+SCREAM"));
  scenes_startTimer(7000);
}
static void update_MirrorRoom(){
  // TODO: strobe bursts; servo mirrors
  if (scenes_timerExpired()) scenes_next();
}
static void exit_MirrorRoom(){}

// Exit Hole
static void enter_ExitHole(){
  display_idle("EXIT");
  scenes_notify_audio(F("PLEASE WAIT"));
  scenes_startTimer(5000);
}
static void update_ExitHole(){
  if (scenes_timerExpired()) scenes_next();
}
static void exit_ExitHole(){}

// Standby
static void enter_Standby(){
  display_idle("OBEY");
  effects_magnet_off();
  effects_sound_stop();
}
static void update_Standby(){
  effects_idle_update();
  // Wait here for operator start or trigger:
  // e.g., if (digitalRead(START_PIN)==LOW) scenes_set(Scene::PhoneLoading);
}
static void exit_Standby(){}