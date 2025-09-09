#pragma once
#include <Arduino.h>

// Callback types
typedef void (*SetU32)(uint32_t);
typedef void (*SetU8)(uint8_t);
typedef void (*ForceStateFn)(int);     // 0=IDLE,1=HOLD,2=COOLDOWN
typedef void (*PrintFn)();

void console_begin(uint32_t baud = 115200);

// Attach app hooks (call once in setup)
void console_attach(SetU32 setHoldMs,
                    SetU32 setCooldownMs,
                    SetU8  setBrightness,
                    ForceStateFn forceState,
                    PrintFn printStatus);

// Call every loop()
void console_update();