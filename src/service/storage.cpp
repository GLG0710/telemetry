#include <LittleFS.h>
#include <WiFi.h>

#include "storage.h"
#include "controller/control.h"
#include "state/data.h"
#include "state/ack.h"
#include "config/adv_config.h"
#include "logger.h"

// Utilitários
size_t fileSize() {
  File f = LittleFS.open(logger.PATH, "r"); 
  if (!f) return 0;

  size_t size = f.size(); 
  f.close();
  return size;
}

void createCSVHeader() {
  if (LittleFS.exists(logger.PATH) && fileSize() > 0) return;

  File f = LittleFS.open(logger.PATH, "a");
  if (!f) {
    Serial.println("[CSV] Erro ao criar header");
    return;
  }
  
  f.println(F("ts_iso,ms,volts,pct,temp,humi,rpm,speed_kmh,current_bat_a,current_mot_a,min,max,wheel_cm,ppr,override_enabled,override_pct,max_pct,rssi,src,log_enabled,log_iv_ms,pwm_hz,start_min_pct,rapid_ms,rapid_up,slew_up,slew_dn,zero_timeout_ms,last_ack"));
  f.close();
}

static bool headerChecked = false;

// Controle de arquivo
void openLogFile() {
  if (logger.isOpen) return;

  if (!headerChecked) {
    createCSVHeader();
    headerChecked = true;
  }

  logger.file = LittleFS.open(logger.PATH, "a");
  if (logger.file) {
    logger.isOpen = true;
    logger.cached_size = logger.file.size();
  } else {
    Serial.println("[CSV] Falha ao abrir arquivo");
  }
}

void closeLogFile() {
  if (logger.isOpen) {
    logger.file.flush();
    logger.file.close();
    logger.isOpen = false;
  }
}

// Rotação de arquivo
void rotateIfNeeded() {
    const size_t MAX_BYTES = 1024UL * 1024UL;

    static uint32_t lastCheck = 0;
    if (millis() - lastCheck < 10000) return;
    lastCheck = millis();

    static uint32_t lastSync = 0;

    if (millis() - lastSync > 60000) { 
      logger.cached_size = fileSize();
      lastSync = millis();
    }

    size_t size = logger.cached_size;

    if (size >= MAX_BYTES) {
      closeLogFile();

      if (LittleFS.exists(logger.PATH_OLD) && !LittleFS.remove(logger.PATH_OLD)) {
        Serial.println("[CSV] Falha ao remover arquivo antigo"); 
        return;
      }

      if (!LittleFS.rename(logger.PATH, logger.PATH_OLD)) {
        Serial.println("[CSV] Falha ao rotacionar arquivo");
        return;
      }

      headerChecked = false;
      logger.cached_size = 0;
    }
} 

// Formatadores
void formatTimestamp(char* buf, size_t size) {
  time_t now = time(nullptr);
  if (now > 1600000000) { //2020
    struct tm t;
    gmtime_r(&now, &t);
    
    if (strftime(buf, size, "%Y-%m-%dT%H:%M:%SZ", &t) == 0) {
      snprintf(buf, size, "%lu", millis());
    }
  } else {
    snprintf(buf, size, "%lu", millis());
  }
}

static void formatMaybeNan(char* out, size_t outSize, float value, uint8_t digits = 3) {
  if (!out || outSize == 0) return;

  if (!isfinite(value)) {
    out[0] = '\0';
    return;
  }

  int len = snprintf(out, outSize, "%.*f", digits, value);
  if (len < 0 || len >= outSize) {
    out[outSize - 1] = '\0';
  }
}

void sanitize(char* str) {
  for (; *str; str++) {
    if (*str == ',' || *str == '\n' || *str == '\r') *str = ' ';
  }
}

int wifiRSSI() {
    return (WiFi.status()==WL_CONNECTED) ? WiFi.RSSI() : 0;
}

// CSV Append (talvez fragmentar mais)
void appendCsvRow() {
  if (!logger.enabled) return;

  rotateIfNeeded();

  openLogFile();
  if (!logger.isOpen) return;

  char timeStamp[32];
  char tempBuffer[16];
  char humiBuffer[16];

  formatTimestamp(timeStamp, sizeof(timeStamp));
  uint32_t now = millis();
  formatMaybeNan(tempBuffer, sizeof(tempBuffer), data.temp, 1);
  formatMaybeNan(humiBuffer, sizeof(humiBuffer), data.humi, 1);

  char ackSafe[64];
  strncpy(ackSafe, ack.last, sizeof(ackSafe));
  ackSafe[sizeof(ackSafe)-1] = '\0';

  sanitize(ackSafe);

  int written = logger.file.printf(
      "%s,%lu,%.3f,%.1f,%s,%s,%.1f,%.2f,%.3f,%.3f,%.3f,%.3f,%.1f,%u,%u,%.0f,%.0f,%d,%s,%u,%lu,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%s\n",
      timeStamp,
      now,
      data.volts,
      data.pct,
      tempBuffer,
      humiBuffer,
      data.rpm,
      data.speed_kmh,
      data.current_bat_a,
      data.current_mot_a,
      data.min_v,
      data.max_v,
      data.wheel_cm,
      data.ppr,
      data.override_enabled ? 1 : 0,
      data.override_pct,
      data.max_pct,
      wifiRSSI(),
      ctrlGetSource(),
      logger.enabled ? 1 : 0,
      logger.interval_ms,
      config.pwm_hz,
      config.start_min_pct,
      config.rapid_ms,
      config.rapid_up,
      config.slew_up,
      config.slew_dn,
      config.zero_timeout_ms,
      ackSafe  
  );

  //Flush
  static uint32_t lastFlush = 0;
  static uint16_t lines = 0;

  lines++;

  if (lines >= 10 || millis() - lastFlush > 5000) {
      logger.file.flush();
      lastFlush = millis();
      lines = 0;
  }

  if (written > 0 && written < 512) { 
    logger.cached_size += written;
  }
}