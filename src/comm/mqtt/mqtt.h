#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

namespace Mqtt {
  // ===================== CONFIG =====================
  struct Topics {
    const char* tlm_json;
    const char* cmd_motor;
    const char* status;
    const char* cmd_throttle;
    const char* cmd_config;
  };

  struct Config {
    const char* host;
    uint16_t port;
    const char* id_base;
    Topics topics;
  };

  extern const Config CONFIG;

  // ===================== CLIENT =====================
  extern WiFiClient espClient;
  extern PubSubClient client;

  // ===================== API =====================
  void setup();
  void loop();
}