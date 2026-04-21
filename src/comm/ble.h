#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>

struct BLEState {
  NimBLECharacteristic* rxChar = nullptr;
  NimBLECharacteristic* txChar = nullptr;

  bool clientConnected = false;
  uint16_t mtu = 23;
  
  int pct = 0;
  int mode = 0;
};
extern BLEState ble;

void sendSpeedo();
void setupBLE();