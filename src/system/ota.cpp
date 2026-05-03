#include <ArduinoOTA.h>
#include <WiFi.h>
#include <ESPmDNS.h>

namespace Ota {
    void setup() {
        if (MDNS.begin("telemetry")) {
            Serial.println("[OTA] mDNS started: telemetry.local");
        } else {
            Serial.println("[OTA] mDNS failed - OTA will still work using IP address");
        }

        ArduinoOTA.setHostname("telemetry-s3");
        ArduinoOTA.setPassword("espota");

        ArduinoOTA.onStart([]() {
            Serial.println("[OTA] Update started...");
        });
        
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
        });

        ArduinoOTA.onEnd([]() {
            Serial.println("\n[OTA] Update completed! Rebooting...");
        });
        
        ArduinoOTA.onError([](ota_error_t error) {
            Serial.printf("[OTA] Error[%u]: ", error);
            switch(error) {
            case OTA_AUTH_ERROR:    Serial.println("Auth failed"); break;
            case OTA_BEGIN_ERROR:   Serial.println("Begin failed"); break;
            case OTA_CONNECT_ERROR: Serial.println("Connect failed"); break;
            case OTA_RECEIVE_ERROR: Serial.println("Receive failed"); break;
            case OTA_END_ERROR:     Serial.println("End failed"); break;
            }
        });
        
        ArduinoOTA.begin();
        Serial.println("[OTA] Ready - Use network port in Arduino IDE");
    }

    void loop() {
        ArduinoOTA.handle();
    }
}