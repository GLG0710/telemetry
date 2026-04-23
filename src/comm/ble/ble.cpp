#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJSON.h>

#include "ble.h"
#include "controller/control.h"
#include "state/data.h"

BLEState ble;

class MyServerCallbacks : public NimBLEServerCallbacks {
public: 
    void onConnect(NimBLEServer* server, NimBLEConnInfo& connInfo) override {
      ble.clientConnected = true;
      ble.mtu = connInfo.getMTU();
      Serial.printf("[BLE] Client connected (MTU=%d)\n", ble.mtu);
    }

    void onDisconnect(NimBLEServer* server, NimBLEConnInfo& connInfo, int reason) override {
      ble.clientConnected = false;
      ble.mtu = 23;
      Serial.printf("[BLE] Client desconnected (reason=%d)\n", reason);
      NimBLEDevice::startAdvertising();
    }
};

class MyRxCallbacks : public NimBLECharacteristicCallbacks {
private:
  bool tryParseManual(const char* data_str, size_t len) {
    if (len < 5) return false;
    
    const char* modePos = strstr(data_str, "\"mode\":");
    if (modePos) {
        int mode = atoi(modePos + 7);
        ble.mode = mode;
        
        if (mode == 0) {
          data.override_enabled = false;
          controlSetSource(LOCAL);
        } else {
          ctrl.last_ms = millis();
        }
        return true;
    }
    
    const char* pctPos = strstr(data_str, "\"pct\":");
    if (pctPos) {
      int pct = atoi(pctPos + 6);
      pct = constrain(pct, 0, 100);
      
      ble.pct = pct;
      data.override_enabled = true;
      data.override_pct = pct;
      controlSetSource(BLE);
      return true;
    }
    
    return false;
  }

public:
  void onWrite(NimBLECharacteristic* c, NimBLEConnInfo& connInfo) override {
    std::string s = c->getValue();
    if (s.empty()) return;
  
    const size_t len = s.length();
    if (len > 128) {
      Serial.printf("[BLE] Payload too large: %u bytes\n", len);
      return;
    }

    Serial.printf("[BLE RX] %s\n", s.c_str());

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, s);

    if (err) {
      Serial.printf("[BLE] JSON error: %s\n", err.c_str());

      if (s[0] == '{' && tryParseManual(s.c_str(), len)) {
        Serial.println("[BLE] Fallback parsing succeeded");
      } else {
        Serial.println("[BLE] Invalid format, ignored");
      }
      return;
    }
    
    // MODO (0=local, 1+=BLE)
    if (doc["mode"].is<int>()) {
      int newMode = doc["mode"].as<int>();
      ble.mode = newMode;

      if (newMode == 0) {
        data.override_enabled = false;
        controlSetSource(LOCAL);
        Serial.println("[BLE] Local Mode activated");
      } else {
        Serial.printf("[BLE] BLE Mode %d activated\n", newMode);
      }
    }

    // PCT override
    if (doc["pct"].is<int>()) {
      int pct = constrain(doc["pct"].as<int>(), 0, 100);

      ble.pct               = pct;
      data.override_enabled = true;
      data.override_pct     = ble.pct;
      controlSetSource(BLE);
      Serial.printf("[BLE] Override PCT: %d%%\n", pct);
    }
  }
};

bool sendSpeedo() {
  if (!ble.clientConnected || !ble.txChar) return false;
  
  JsonDocument doc;
  
  doc["mode"] = ble.mode;
  doc["speed_kmh"] = data.speed_kmh;
  doc["rpm"] = data.rpm;
  doc["pct"] = data.pct;
  
  doc["temp"] = isnan(data.temp) ? 0.0f : data.temp;
  doc["current"] = isnan(data.current_bat_a) ? 0.0f : data.current_bat_a;
  
  doc["override"] = data.override_enabled;
  if (data.override_enabled) {
    doc["override_pct"] = data.override_pct;
  }

  char buffer[160];
  size_t len = serializeJson(doc, buffer, sizeof(buffer));

  if (len <= 0 || len >= sizeof(buffer)) {
    Serial.println("[BLE] Serializing Json Error");
    return false;
  }

  ble.txChar->setValue((uint8_t*)buffer, len);
  ble.txChar->notify();
  return true;
}

void setupBLE() {
  Serial.println("[BLE] Initializing...");

  NimBLEDevice::init("EWolf-Telemetry");
  NimBLEDevice::setDeviceName("EWolf-Telemetry");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setMTU(128);

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks()); // NimBLE gerencia a memória

  static NimBLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
  static NimBLEUUID charRxUUID ("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
  static NimBLEUUID charTxUUID ("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

  NimBLEService* pService = pServer->createService(serviceUUID);

  // RX: comandos do app
  ble.rxChar = pService->createCharacteristic(
    charRxUUID,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );
  ble.rxChar->setCallbacks(new MyRxCallbacks());

  // TX: telemetria para app
  ble.txChar = pService->createCharacteristic(
    charTxUUID,
    NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
  );

  ble.txChar->createDescriptor(
    NimBLEUUID((uint16_t)0x2902),
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
  );

  //pService->start();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(serviceUUID);
  adv->setName("EWolf-Telemetry");
  adv->setMinInterval(32);
  adv->setMaxInterval(48);

  if (adv->start()) {
    Serial.println("[BLE] Advertising inicialized as 'EWolf-Telemetry'");
  } else {
    Serial.println("[BLE] ERROR: Error of inicializing advertising!");
  }
}

void loopBLE() {
  static uint32_t last = 0;
  uint32_t now = millis();

  if (now - last < 100) return;
  last = now;

  bool sent = sendSpeedo();

  static uint32_t lastDebug = 0;
  if (sent && now - lastDebug > 5000) {
    Serial.printf("[BLE TX] Packed Sent\n");
    lastDebug = now;
  }
}