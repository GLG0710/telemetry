#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ===================== CONFIG =====================
struct MQTTTopics {
  const char* tlm_json;
  const char* cmd_motor;
  const char* status;
  const char* cmd_throttle;
  const char* cmd_config;
};

struct MQTTConfig {
  const char* host;
  uint16_t port;
  const char* id_base;
  MQTTTopics topics;
};

extern const MQTTConfig MQTT_CONFIG;

// ===================== CLIENT =====================
extern WiFiClient espClient;
extern PubSubClient mqtt;

// ===================== API =====================
void setupMqtt();
void loopMqtt();