#pragma once

#include <Arduino.h>

enum ControlSource {
  LOCAL,
  BLE,
  MQTT
};

struct Control {
  ControlSource src = LOCAL;          
  unsigned long last_ms = 0;
  
  static constexpr unsigned long TIMEOUT_MS = 2000;  
};
extern Control ctrl;

// funções de controle
void controlSetSource(ControlSource src);
const char* ctrlGetSource();
void controlUpdateTimeout();
