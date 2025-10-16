#pragma once

enum class DirectorState {
    OPERATOR_IDLE, LOOP_MODE, TRIGGER_MODE, ATTRACT_MODE
};

enum class Scene {
    NONE, INTRO, BLOOD, FRANKENPHONE, MIRROR, EXIT
};

enum class DirectorEvent {
    NONE, START_LOOP, START_TRIGGER, START_ATTRACT, STOP, SKIP
};

namespace Director {
    void init();
    void update();
    void handleEvent(DirectorEvent event);
}