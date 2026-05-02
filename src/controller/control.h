#pragma once

#include <Arduino.h>

namespace Control {
  enum Source {
    LOCAL,
    BLE,
    MQTT
  };

  struct State {
    Source src = LOCAL;          
    unsigned long last_ms = 0;
  };
  extern State state;

  static constexpr unsigned long TIMEOUT_MS = 2000;  

  // funções de controle
  void setSource(Source src);
  const char* getSource();
  void loop();
}