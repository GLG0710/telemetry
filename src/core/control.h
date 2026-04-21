#pragma once

#include <Arduino.h>

enum ControlSource {
  CTRL_LOCAL,
  CTRL_BLE,
  CTRL_MQTT
};

struct Control {
  ControlSource src = CTRL_LOCAL;          
  unsigned long last_ms = 0;
  const unsigned long TIMEOUT_MS = 2000;  
};
extern Control ctrl;

// funções de controle
void controlSetSource(ControlSource src);
const char* ctrlGetSource(ControlSource src);
void controlUpdateTimeout();
