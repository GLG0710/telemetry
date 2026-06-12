#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "config/uart.h"
#include "controller/control.h"
#include "service/logger.h"
#include "protocol/parser.h"
#include "system/ota.h"

#include "communication/ble/ble.h"
#include "communication/lora/lora.h"
#include "communication/mqtt/mqtt.h"

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

    Serial.println("[BOOT] System Ready!\n");
}

void loop(){
    while (Uart::Arduino.available()) {
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