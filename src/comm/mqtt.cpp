#include "mqtt.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "../core/state.h"
#include "../core/control.h"
#include "../config/pins.h"

// ===================== CONFIG =====================
const MQTTConfig MQTT_CONFIG = {
  "broker.mqtt-dashboard.com",
  1883,
  "EWolfTelemetryS3",
  {
    "pb/telemetry/json",
    "pb/cmd/motor",
    "pb/status",
    "pb/cmd/throttle",
    "pb/cmd/config"
  }
};

// ===================== CLIENT =====================
WiFiClient espClient;
PubSubClient mqtt(espClient);

static unsigned long lastMqtt = 0;
static constexpr unsigned long MQTT_IV_MS = 1000;


// ===================== AUX =====================
static void sendCmd(const String& s){
  Pins::SerialArd.println(s);
}

static void handleMotor(const char* msg) {
    if (!strcasecmp(msg, "STOP") || !strcasecmp(msg, "OFF")  || !strcmp(msg, "0")) {
      sendCmd("STOP");
      snprintf(ack.last, sizeof(ack.last), "STOP");
      ack.timestamp = millis();
    }

    else if (!strcasecmp(msg, "START") || !strcasecmp(msg, "ON")    || !strcmp(msg, "1")) {
      sendCmd("START");
      snprintf(ack.last, sizeof(ack.last), "START");
      ack.timestamp = millis();
    }

    else {
        Serial.printf("[MQTT] Unknown motor cmd: %s\n", msg);
    }
}

static void handleThrottle(JsonDocument& doc) {
  if (doc["override_enabled"].is<bool>() && doc["override_enabled"]) {
    float pct = doc["pct"].is<float>() ? doc["pct"].as<float>() : data.override_pct;

    data.override_enabled = true;
    data.override_pct = constrain(pct, 0, 100);

    controlSetSource(MQTT);

    snprintf(ack.last, sizeof(ack.last), "THROTTLE");
    ack.timestamp = millis();
    return;
  }

  if (doc["auto"].is<bool>() && doc["auto"]) {
    data.override_enabled = false;
    controlSetSource(LOCAL);

    snprintf(ack.last, sizeof(ack.last), "AUTO");
    ack.timestamp = millis();
    return;
  }
}

static void handleConfig(JsonDocument& doc) {
  // -------- TELEMETRIA --------
  if (doc["max_pct"].is<float>())
    data.max_pct = constrain(doc["max_pct"], 0.0f, 100.0f);

  if (doc["min_v"].is<float>())
    data.min_v = doc["min_v"];

  if (doc["max_v"].is<float>())
    data.max_v = doc["max_v"];

  if (doc["wheel_cm"].is<float>())
    data.wheel_cm = doc["wheel_cm"];

  if (doc["ppr"].is<int>())
    data.ppr = (uint8_t)doc["ppr"];

  if (doc["poll_ms"].is<int>())
    data.poll_ms = (uint32_t)doc["poll_ms"];

  // -------- LOG --------
  if (doc["log_enabled"].is<bool>())
    logger.enabled = doc["log_enabled"];

  if (doc["log_iv_ms"].is<int>()) {
    uint32_t ms = doc["log_iv_ms"];
    logger.interval_ms = constrain(ms, 100UL, 60000UL);
  }

  if (doc["log_clear"].is<bool>() && doc["log_clear"]) {
    if (LittleFS.exists(logger.PATH))     LittleFS.remove(logger.PATH);
    if (LittleFS.exists(logger.PATH_OLD)) LittleFS.remove(logger.PATH_OLD);
  }

  // -------- CONFIG AVANÇADA --------
  if (doc["pwm_hz"].is<float>())
    config.pwm_hz = constrain(doc["pwm_hz"], 100.0f, 8000.0f);

  if (doc["start_min_pct"].is<float>())
    config.start_min_pct = constrain(doc["start_min_pct"], 0.0f, 40.0f);

  if (doc["rapid_ms"].is<float>())
    config.rapid_ms = constrain(doc["rapid_ms"], 50.0f, 1500.0f);

  if (doc["rapid_up"].is<float>())
    config.rapid_up = constrain(doc["rapid_up"], 10.0f, 400.0f);

  if (doc["slew_up"].is<float>())
    config.slew_up = constrain(doc["slew_up"], 5.0f, 200.0f);

  if (doc["slew_dn"].is<float>())
    config.slew_dn = constrain(doc["slew_dn"], 5.0f, 300.0f);

  if (doc["zero_timeout_ms"].is<float>())
    config.zero_timeout_ms = constrain(doc["zero_timeout_ms"], 50.0f, 2000.0f);

  snprintf(ack.last, sizeof(ack.last), "CONFIG_UPDATED");
  ack.timestamp = millis();
}

// ===================== CALLBACK =====================
static void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char msg[256];
  unsigned int len = min(length, sizeof(msg)-1);

  memcpy(msg, payload, len);
  msg[len] = '\0';

  Serial.printf("[MQTT RX] %s | %s\n", topic, msg);

  // MOTOR
  if (strcmp(topic, MQTT_CONFIG.topics.cmd_motor) == 0) {
    handleMotor(msg);
    return;
  }

  // JSON
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, msg)) {
    Serial.println("[MQTT] JSON error");
    return;
  }

  // THROTTLE
  if (strcmp(topic, MQTT_CONFIG.topics.cmd_throttle) == 0) {
    handleThrottle(doc);
    return;
  }

  // CONFIG (mantive igual, só limpei)
  if (strcmp(topic, MQTT_CONFIG.topics.cmd_config) == 0) {
    handleConfig(doc);
    return;
  }
}

// ===================== SETUP =====================
void setupMqtt() {
    mqtt.setServer(MQTT_CONFIG.host, MQTT_CONFIG.port);
    mqtt.setCallback(mqttCallback);
    mqtt.setBufferSize(2048);

    Serial.println("[MQTT] Configuração:");
    Serial.printf("  Broker : %s:%d\n", MQTT_CONFIG.host, MQTT_CONFIG.port);
    Serial.printf("  Client : %s\n", MQTT_CONFIG.id_base);

    Serial.println("  Tópicos:");
    Serial.printf("    PUB  : %s\n", MQTT_CONFIG.topics.tlm_json);
    Serial.printf("    SUB  : %s\n", MQTT_CONFIG.topics.cmd_motor);
    Serial.printf("    SUB  : %s\n", MQTT_CONFIG.topics.cmd_throttle);
    Serial.printf("    SUB  : %s\n", MQTT_CONFIG.topics.cmd_config);
    Serial.printf("    STAT : %s\n", MQTT_CONFIG.topics.status);
}

// ===================== CONNECTION =====================
void ensureMqtt() {
    if (mqtt.connected()) return;

    static unsigned long lastTry = 0;
    static int lastErr = -999;
    static bool firstTry = true;
    static unsigned long lastErrLog = 0;

    if (millis() - lastTry < 1500) return;
    lastTry = millis();

    char clientId[64];
    snprintf(clientId, sizeof(clientId), "%s-%llX", MQTT_CONFIG.id_base, ESP.getEfuseMac());

    if (firstTry) {
      Serial.println("[MQTT] Connecting...");
      firstTry = false;
    }

    if (mqtt.connect(clientId, MQTT_CONFIG.topics.status, 0, true, "offline")) {
      Serial.println("[MQTT] Connected");

      mqtt.publish(MQTT_CONFIG.topics.status, "online", true);
      mqtt.subscribe(MQTT_CONFIG.topics.cmd_motor);
      mqtt.subscribe(MQTT_CONFIG.topics.cmd_throttle);
      mqtt.subscribe(MQTT_CONFIG.topics.cmd_config);

      lastErr = -999;
      firstTry = true;
      lastErrLog = 0;
    } else {
      int err = mqtt.state();

    if (err != lastErr || millis() - lastErrLog > 60000) {
        Serial.printf("[MQTT] Connection failed, state=%d\n", err);
        lastErr = err;
        lastErrLog = millis();
    }
  }
}

// ===================== LOOP =====================
void mqttLoop() {
    static bool wasConnected = false;
    bool wifiOk = (WiFi.status() == WL_CONNECTED);

    if (!wifiOk && wasConnected) {
        mqtt.disconnect();
    }

    wasConnected = wifiOk;

    ensureMqtt();
    mqtt.loop();
    controlUpdateTimeout();

    uint32_t now = millis();

    if (now - lastMqtt < MQTT_IV_MS) return;

    lastMqtt = now;

    mqttPublishTelemetry();

    if (ack.last[0] != '\0' && (now - ack.timestamp) > 2000) {
        ack.last[0] = '\0';
    }
}


// ===================== PUBLISH =====================
void mqttPublishTelemetry() {
    if (!mqtt.connected()) return;

    StaticJsonDocument<768> doc;

    // -------- DADOS --------
    doc["volts"]     = data.volts;
    doc["pct"]       = data.pct;
    doc["rpm"]       = data.rpm;
    doc["speed_kmh"] = data.speed_kmh;

    if (isnan(data.temp)) {
        doc["temp"] = nullptr;
    } else {
        doc["temp"] = data.temp;
    }
    if (isnan(data.humi)) {
        doc["humi"] = nullptr;
    } else {
        doc["humi"] = data.humi;
    }

    doc["current_bat_a"] = data.current_bat_a;
    doc["current_mot_a"] = data.current_mot_a;

    doc["min"]      = data.min_v;
    doc["max"]      = data.max_v;
    doc["wheel_cm"] = data.wheel_cm;
    doc["ppr"]      = data.ppr;

    doc["override_enabled"] = data.override_enabled;
    doc["override_pct"]     = data.override_pct;
    doc["max_pct"]          = data.max_pct;

    doc["src"]     = ctrlGetSource(); // já que você criou isso
    doc["poll_ms"] = data.poll_ms;

    // -------- LOGGER --------
    doc["log_enabled"] = logger.enabled;
    doc["log_iv_ms"]   = logger.interval_ms;
    doc["log_size"]    = logger.cached_size;

    // -------- CONFIG AVANÇADA --------
    doc["pwm_hz"]          = config.pwm_hz;
    doc["start_min_pct"]   = config.start_min_pct;
    doc["rapid_ms"]        = config.rapid_ms;
    doc["rapid_up"]        = config.rapid_up;
    doc["slew_up"]         = config.slew_up;
    doc["slew_dn"]         = config.slew_dn;
    doc["zero_timeout_ms"] = config.zero_timeout_ms;

    // -------- ACK --------
    doc["ack"] = (ack.last[0] != '\0') ? ack.last : nullptr;

    // -------- SERIALIZAÇÃO --------
    char buffer[768];
    size_t len = serializeJson(doc, buffer, sizeof(buffer));

    if (len == 0 || len >= sizeof(buffer)) {
      Serial.println("[MQTT] JSON serialize error");
      return;
    }

    // -------- PUBLISH --------
    bool ok = mqtt.publish(MQTT_CONFIG.topics.tlm_json, buffer, len);

    if (!ok) {
      Serial.printf("[MQTT] publish FAIL, len = %u\n", len);
    }
}