#pragma once

#include <Arduino.h>
#include <LittleFS.h>

namespace Log {
  struct State {
    const char* PATH = "/telemetry.csv";
    const char* PATH_OLD = "/telemetry.csv.1";

    bool enabled = false;
    uint32_t interval_ms = 1000;
    uint32_t last_log = 0;

    File file;
    bool isOpen = false;
    size_t cached_size = 0;
  };
  extern State logger;

  void setup();
  void loop();
  void clear();
}