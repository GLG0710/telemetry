#pragma once

#include <Arduino.h>

struct __attribute__((packed)) LoraTelemetryFrame {
  uint8_t  sync;       // 0xAA
  uint8_t  seq;
  uint16_t rpm;        // da roda (já existe)
  uint16_t vbatt_mv;   // data.volts * 1000 + 0.5
  int16_t  ibatt_ma;   // data.current_bat_a * 1000 + 0.5
  uint8_t  flags;      // estados reais
  uint8_t  crc;
};

bool rxTelemetry();
void txTelemetry();