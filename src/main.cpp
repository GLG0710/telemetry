#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "config/uart.h"
#include "controller/control.h"
#include "service/logger.h"
#include "protocol/parser.h"
#include "system/ota.h"

#include "comm/ble/ble.h"
#include "comm/lora/lora.h"
#include "comm/mqtt/mqtt.h"


namespace SerialDiagnostic {
  bool haveSerialData = false;
  bool reportedSerialStatus = false;
  unsigned long serialCheckStartMs = 0;
  constexpr unsigned long SERIAL_CHECK_WINDOW_MS = 5000;
}

void setupTime() {
  configTime(0, 0, "pool.ntp.org", "time.google.com");
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  Serial.println("\n[BOOT] Initializing system...");

  Uart::setup();
  Log::setup();

  WiFi.mode(WIFI_AP_STA);
  WiFiManager manager;
  manager.setConfigPortalBlocking(true);
  manager.setTimeout(120);
  manager.setDebugOutput(true);

  Serial.println("======================================");
  Serial.println("[WiFi] Initializing Connection...");
  if (!manager.autoConnect("Telemetry-Setup")) {
    Serial.println("[WiFi] Connection failed, restarting...");
    delay(3000);
    ESP.restart();
  }
  Serial.printf("[WiFi] Connected | IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.println("UART Arduino in Serial1 (GPIO9 RX, GPIO10 TX)");
  Serial.println("======================================");

  Ota::setup();
  Mqtt::setup();  
  setupTime();   // Sincronização de tempo
  Ble::setup();   

  SerialDiagnostic::serialCheckStartMs = millis();

  Serial.println("[BOOT] System Ready!\n");
}

void diagnostic() {
  if (!SerialDiagnostic::reportedSerialStatus) {
    unsigned long now = millis();
    if (now - SerialDiagnostic::serialCheckStartMs > SerialDiagnostic::SERIAL_CHECK_WINDOW_MS) {
        Serial.println(SerialDiagnostic::haveSerialData
            ? "DBG: Receiving telemetry from Arduino (Serial1 OK)"
            : "DBG: No telemetry received from Arduino in the first 5s");
        SerialDiagnostic::reportedSerialStatus = true;
    }
  }
}

void loop(){
  while (Uart::Arduino.available()) {
    protocolFeedByte(Uart::Arduino.read());
  }

  // timeout serial inicial
  diagnostic();

  Control::loop();

  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastTry = 0;
    if (millis() - lastTry > 5000) {
      Serial.println("[WiFi] Reconnecting..."); 
      WiFi.reconnect();
      lastTry = millis();
    }
  } else {
    Ota::loop();
    Mqtt::loop();
  }

  Log::loop();
  Ble::loop();
  Lora::loop();

  static unsigned long lastStatusPrint = 0;
  if (millis() - lastStatusPrint > 2000) {
    Serial.printf("[WiFi] Status: %d | RSSI: %d \n", WiFi.status(), WiFi.RSSI());  // Status 3 = Conectado | Status 6 = Desconectado / RSSI, quanto menor melhor
    lastStatusPrint = millis();
  }
}