#pragma once
#include <Keypad.h>

namespace KeypadInput {
    void init();
    char getKey();
}

enum class KeyEvent {
    NONE, STOP, ATTRACT, SKIP, LOOP_MODE, TRIGGER_MODE
};

KeyEvent mapKeyToEvent(char key);