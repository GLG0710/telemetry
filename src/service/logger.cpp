#include <Arduino.h>
#include <LittleFS.h>

#include "logger.h"
#include "storage.h"

namespace Log {
  State logger; 
  
  void setup() {
    if (LittleFS.begin(true)) {
      Serial.println("[LOG] LittleFS Initialized");
    } else {
      Serial.println("[LOG] LittleFS initialization failed");
    }
  }

  void loop() {
    if (!logger.enabled || logger.interval_ms == 0) return;

    uint32_t now = millis();

    if (now - logger.last_log < logger.interval_ms) return;

    logger.last_log = now;

    appendCsvRow();
  }   

  void clear() {
    if (LittleFS.exists(logger.PATH))     LittleFS.remove(logger.PATH);
    if (LittleFS.exists(logger.PATH_OLD)) LittleFS.remove(logger.PATH_OLD);
  }
}