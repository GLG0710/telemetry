#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "config/pins.h"
#include "comm/ble/ble.h"
#include "comm/lora/lora.h"
#include "comm/mqtt/mqtt.h"
#include "state/data.h"
#include "state/ack.h"
#include "controller/control.h"
#include "service/logger.h"
#include "protocol/parser.h"

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
  Serial.println("\n[BOOT] Inicializando sistema...");

  Pins::setupUart();

  if (LittleFS.begin(true)) {
    Serial.println("[FS] LittleFS OK");
  } else {
    Serial.println("[FS] ERRO ao iniciar LittleFS");
  }

  WiFi.mode(WIFI_STA);
  WiFiManager manager;
  manager.setConfigPortalBlocking(true);
  manager.setTimeout(120);
  manager.setDebugOutput(false);

  Serial.println("[WiFi] Iniciando conexão...");
  manager.autoConnect("Telemetry-Setup");
  Serial.printf("[WiFi] Conectado | IP: %s\n", WiFi.localIP().toString().c_str());

  Serial.println("======================================");
  Serial.print (" WiFi IP:  "); Serial.println(WiFi.localIP()); 
  Serial.printf(" MQTT pub: %s \n", MQTT_CONFIG.topics.tlm_json);
  Serial.printf(" MQTT sub: %s \n", MQTT_CONFIG.topics.cmd_motor);
  Serial.printf(" MQTT sub: %s \n", MQTT_CONFIG.topics.cmd_throttle);
  Serial.printf(" MQTT sub: %s \n", MQTT_CONFIG.topics.cmd_config);
  Serial.println(" UART Arduino em Serial1 (GPIO9 RX, GPIO10 TX)");
  Serial.println("======================================");

  mqtt.setServer(MQTT_CONFIG.host, MQTT_CONFIG.port);
  //mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(2048);

  setupMqtt();  
  //setupOTA();    // Atualização over-the-air
  setupTime();   // Sincronização de tempo
  setupBLE();    // Bluetooth Low Energy

  // ==================== FINALIZAÇÃO ====================
  SerialDiagnostic::serialCheckStartMs = millis();

  Serial.println("[BOOT] Sistema pronto!\n");
  Serial.println("================================================");
  Serial.println("Sistema operacional. Aguardando comandos...");
  Serial.println("================================================\n");
}

void diagnostic() {
  if (!SerialDiagnostic::reportedSerialStatus) {
    unsigned long nowMs = millis();
    if (nowMs - SerialDiagnostic::serialCheckStartMs > SerialDiagnostic::SERIAL_CHECK_WINDOW_MS) {
        Serial.println(SerialDiagnostic::haveSerialData
            ? "DBG: Recebendo telemetria do Arduino (Serial1 OK)"
            : "DBG: Nenhuma telemetria recebida do Arduino nos primeiros 5s.");
        SerialDiagnostic::reportedSerialStatus = true;
    }
  }
}

void loop(){
  //ArduinoOTA.handle();

  while (Pins::SerialArd.available()) {
    protocolFeedByte(Pins::SerialArd.read());
  }

  // timeout serial inicial
  diagnostic();

  // Control
  controlUpdateTimeout();

  loopMqtt();
  loopLogger();
  loopBLE();
  loopLora();
}