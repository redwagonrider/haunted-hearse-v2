#include "../include/keypad_input.hpp"
#include "../include/hardware.hpp"

constexpr uint8_t ROWS = 4;
constexpr uint8_t COLS = 4;
char keys[ROWS][COLS] = {{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
uint8_t rowPins[ROWS] = {40,41,42,43};
uint8_t colPins[COLS] = {44,45,46,47};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

namespace KeypadInput {
    void init() {}
    char getKey() { return keypad.getKey(); }
}
    KeyEvent mapKeyToEvent(char key) {
    if (key == '1') return KeyEvent::LOOP_MODE;      // 1 = Loop Mode
    if (key == '2') return KeyEvent::STOP;           // 2 = Stop
    if (key == '3') return KeyEvent::TRIGGER_MODE;   // 3 = Trigger Mode
    if (key == '4') return KeyEvent::ATTRACT;        // 4 = Attract Mode
    // A and B reserved for future RF remote engine triggers
    return KeyEvent::NONE;
}
