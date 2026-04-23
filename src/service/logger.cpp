#include <Arduino.h>

#include "logger.h"
#include "storage.h"

Log logger; 

void loopLogger() {
  if (!logger.enabled || logger.interval_ms == 0) return;

  uint32_t now = millis();

  if (now - logger.last_log < logger.interval_ms) return;

  logger.last_log = now;

  appendCsvRow();
}   

void clearLogs() {
  if (LittleFS.exists(logger.PATH))     LittleFS.remove(logger.PATH);
  if (LittleFS.exists(logger.PATH_OLD)) LittleFS.remove(logger.PATH_OLD);
}