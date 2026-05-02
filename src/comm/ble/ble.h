#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>

namespace Ble {
  struct State {
    NimBLECharacteristic* rxChar = nullptr;
    NimBLECharacteristic* txChar = nullptr;

    bool clientConnected = false;
    uint16_t mtu = 23; // Maximum Transmission Unit, Valor padrão 23 bytes (20 bytes úteis)
    
    int pct = 0; //Powercast Tags, monitoramento de sensorers em tempo real
    int mode = 0;
  };
  extern State state;

  void setup();
  void loop();
}
