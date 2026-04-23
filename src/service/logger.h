#pragma once

#include <Arduino.h>
#include <LittleFS.h>

struct Log {
  const char* PATH = "/telemetry.csv";
  const char* PATH_OLD = "/telemetry.csv.1";

  bool enabled = false;
  uint32_t interval_ms = 1000;
  uint32_t last_log = 0;

  File file;
  bool isOpen = false;
  size_t cached_size = 0;
};
extern Log logger;

void loopLogger();
void clearLogs();