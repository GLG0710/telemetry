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

void setupTime() {
    configTime(0, 0, "pool.ntp.org", "time.google.com");
}

// Ambas as variáveis e a função são para log enquanto estivermos com erro de recepção dos dados do mega, retirar tudo quando erro for resolvido (ou deixar como comentário)
static uint32_t lastDiagnostic = 0;
static uint32_t lastMegaData = 0;

void diagnostic() {
    uint32_t now = millis();

    if (now - lastDiagnostic >= 5000) {
        lastDiagnostic = now;

        Serial.printf(
            "[WiFi] Status: %s | RSSI: %d\n",
            WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected",
            WiFi.RSSI()
        );

        uint32_t elapsed = now - lastMegaData;

        if (elapsed < 5000) {
            Serial.printf(
                "[UART] Mega OK - last data received %lu ms ago\n",
                elapsed
            );
        } else {
            Serial.printf(
                "[UART] WARNING - no data from Mega for %lu ms\n",
                elapsed
            );
        }
    }
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

    Serial.println("[BOOT] System Ready!\n");
    lastMegaData = millis();
}

void loop(){
    diagnostic();

    while (Uart::Arduino.available()) {
        lastMegaData = millis();
        protocolFeedByte(Uart::Arduino.read());
    }

    Control::loop();

    // Funções dependentes de WiFi só são executadas se tiver WiFi
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
}