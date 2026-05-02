#include <Arduino.h>

#include <state/data.h>
#include <comm/ble/ble.h>
#include <config/uart.h>
#include <comm/lora/lora.h>
#include <comm/mqtt/mqtt.h>
#include <service/logger.h>

// Long Range, responsável pela comunicação a longa distânica

// Detecta erros
uint8_t lora_crc8(const uint8_t* data, uint8_t len) {
  uint8_t crc = 0x00;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80) crc = (crc << 1) ^ 0x07;
      else crc <<= 1;
    }
  }
  return crc;
}

bool rxTelemetry() {
  if (Uart::Lora.available() < sizeof(LoraTelemetryFrame)) 
    return false;
  
  LoraTelemetryFrame frame;

  uint8_t received = Uart::Lora.readBytes((uint8_t*)&frame, sizeof(LoraTelemetryFrame));
  
  if (received != sizeof(LoraTelemetryFrame)) 
    return false;
  
  if (frame.sync != 0xAA) return false;
  
  if (lora_crc8((uint8_t*)&frame, sizeof(frame) - 1) != frame.crc) {
    Serial.println("[LoRa] CRC fail");
    return false;
  }
  
  data.rpm = frame.rpm;
  data.volts = frame.vbatt_mv / 1000.0f;
  data.current_bat_a = frame.ibatt_ma / 1000.0f;
  
  data.override_enabled = frame.flags & (1 << 0);
  Ble::state.mode = (frame.flags & (1 << 1)) ? 1 : 0;
  Ble::state.clientConnected = frame.flags & (1 << 2);
  
  Serial.printf("[LoRa RX] RPM=%u V=%.1fA I=%.1fA seq=%u\n",
    frame.rpm, data.volts, data.current_bat_a, frame.seq);
    
  return true;
}

static uint8_t loraSeq = 0;

void txTelemetry() {
  LoraTelemetryFrame frame;

  frame.sync = 0xAA;
  frame.seq  = loraSeq++;
  frame.rpm       = constrain((uint16_t)data.rpm, 0, 65535);
  frame.vbatt_mv  = constrain((uint16_t)(data.volts * 1000.0f + 0.5f), 0, 65535);
  frame.ibatt_ma  = constrain((int16_t)(data.current_bat_a * 1000.0f + 0.5f), -32768, 32767);

  frame.flags = 0;
  if (data.override_enabled)  frame.flags |= (1 << 0);
  if (Ble::state.mode)               frame.flags |= (1 << 1);
  if (Ble::state.clientConnected)    frame.flags |= (1 << 2);
  if (Mqtt::mqtt.connected())       frame.flags |= (1 << 3);
  if (Log::logger.enabled)         frame.flags |= (1 << 4);

  frame.crc = lora_crc8((uint8_t*)&frame, sizeof(frame) - 1);

  Uart::Lora.write((uint8_t*)&frame, sizeof(frame));
}

namespace Lora {
  void loop() {
  static uint32_t last = 0;
  uint32_t now = millis();

  if (now - last < 100) return;
  last = now;

  txTelemetry();
}
}