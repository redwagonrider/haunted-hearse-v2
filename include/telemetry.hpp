// telemetry.hpp
// Haunted Hearse telemetry for Serial Studio v3
// Emits CSV-style frames wrapped with /* and */ delimiters.
// Order: %1 millis, %2 scene, %3 phase, %4 beam, %5 magnet, %6 buzzer, %7 led_r, %8 led_g, %9 led_b

#pragma once
#include <Arduino.h>

namespace HH {

inline void tel_begin(uint32_t baud = 115200) {
  Serial.begin(baud);
  delay(10); // Mega 2560 should not block on Serial
}

inline void tel_emit(const char* scene,
                     const char* phase,
                     uint8_t beam,
                     uint8_t magnet,
                     uint8_t buzzer,
                     uint8_t r,
                     uint8_t g,
                     uint8_t b) {
  Serial.print("/*");
  Serial.print(millis());  Serial.print(',');
  Serial.print(scene);     Serial.print(',');
  Serial.print(phase);     Serial.print(',');
  Serial.print(beam);      Serial.print(',');
  Serial.print(magnet);    Serial.print(',');
  Serial.print(buzzer);    Serial.print(',');
  Serial.print(r);         Serial.print(',');
  Serial.print(g);         Serial.print(',');
  Serial.print(b);
  Serial.println("*/");
}

} // namespace HH