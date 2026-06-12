/*
	Message Queuing Telemetry Transport, protocolo de comunicação em rede
	Envia dados para telemetria.

	Coisas pra fazer:
	Fazer função para setAck
*/

#include "mqtt.h"

// Instância do espClient, Config e mqtt
namespace Mqtt {
	constexpr Config CONFIG = {
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
	
	WiFiClient espClient;
	PubSubClient client(espClient);
}

namespace {
	uint32_t lastMqtt = 0;
	constexpr uint32_t MQTT_IV_MS = 1000;

	// Auxiliares
	void sendCmd(const char* s){
		Uart::Arduino.println(s);
	}

	void handleMotor(const char* msg) {
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

	void handleThrottle(JsonDocument& doc) {
		if (doc["override_enabled"].is<bool>() && doc["override_enabled"]) {
			float pct = doc["pct"].is<float>() ? doc["pct"].as<float>() : Telemetry::data.override_pct;

			Telemetry::data.override_enabled = true;
			Telemetry::data.override_pct = constrain(pct, 0, 100);

			Control::setSource(Control::Source::MQTT);

			snprintf(ack.last, sizeof(ack.last), "THROTTLE");
			ack.timestamp = millis();
			return;
		}

		if (doc["auto"].is<bool>() && doc["auto"]) {
			Telemetry::data.override_enabled = false;
			Control::setSource(Control::Source::LOCAL);

			snprintf(ack.last, sizeof(ack.last), "AUTO");
			ack.timestamp = millis();
			return;
		}
	}

	void handleConfig(JsonDocument& doc) {
		// -------- TELEMETRIA --------
		if (doc["max_pct"].is<float>()) {
			float max_pct = doc["max_pct"].as<float>();
			Telemetry::data.max_pct = constrain(max_pct, 0.0f, 100.0f);
		}

		if (doc["min_v"].is<float>())
			Telemetry::data.min_v = doc["min_v"].as<float>();

		if (doc["max_v"].is<float>())
			Telemetry::data.max_v = doc["max_v"].as<float>();

		if (doc["wheel_cm"].is<float>())
			Telemetry::data.wheel_cm = doc["wheel_cm"].as<float>();

		if (doc["ppr"].is<int>())
			Telemetry::data.ppr = (uint8_t)doc["ppr"].as<int>();

		if (doc["poll_ms"].is<int>())
			Telemetry::data.poll_ms = (uint32_t)doc["poll_ms"].as<int>();

		// -------- LOG --------
		if (doc["log_enabled"].is<bool>())
			Log::logger.enabled = doc["log_enabled"].as<bool>();

		if (doc["log_iv_ms"].is<int>()) {
			uint32_t ms = doc["log_iv_ms"].as<int>();
			Log::logger.interval_ms = constrain(ms, 100UL, 60000UL);
		}

		if (doc["log_clear"].is<bool>() && doc["log_clear"].as<bool>()) {
			Log::clear(); 
		}
		// -------- CONFIG AVANÇADA --------
		if (doc["pwm_hz"].is<float>()) {
			float pwm_hz = doc["pwm_hz"].as<float>();
			Advanced::config.pwm_hz = constrain(pwm_hz, 100.0f, 8000.0f);
		}

		if (doc["start_min_pct"].is<float>()) {
			float start_min_pct = doc["start_min_pct"].as<float>();
			Advanced::config.startMin = constrain(start_min_pct, 0.0f, 40.0f);
		}

		if (doc["rapid_ms"].is<float>()) {
			float rapid_ms = doc["rapid_ms"].as<float>();
			Advanced::config.rapidRamp = constrain(rapid_ms, 50.0f, 1500.0f);
		}

		if (doc["rapid_up"].is<float>()) {
			float rapid_up = doc["rapid_up"].as<float>();
			Advanced::config.rapidUp = constrain(rapid_up, 10.0f, 400.0f);
		}

		if (doc["slew_up"].is<float>()) {
			float slew_up = doc["slew_up"].as<float>();
			Advanced::config.slewUp = constrain(slew_up, 5.0f, 200.0f);
		}

		if (doc["slew_dn"].is<float>()) {
			float slew_dn = doc["slew_dn"].as<float>();
			Advanced::config.slewDown = constrain(slew_dn, 5.0f, 300.0f);
		}

		if (doc["zero_timeout_ms"].is<float>()) {
			float zero_timeout_ms = doc["zero_timeout_ms"].as<float>();
			Advanced::config.zeroTimeoutMs = constrain(zero_timeout_ms, 50.0f, 2000.0f);
		}

		snprintf(ack.last, sizeof(ack.last), "CONFIG_UPDATED");
		ack.timestamp = millis();
	}

	// Callback
	void mqttCallback(char* topic, byte* payload, unsigned int length) {
		char msg[256];

		// Verificar se dá erro aqui
		if (length >= sizeof(msg)) {
			Serial.printf("[MQTT] Payload too large (%u bytes)\n", length);
			return;
		}

		unsigned int len = min(length, sizeof(msg)-1);

		memcpy(msg, payload, len);
		msg[len] = '\0';

		Serial.printf("[MQTT RX] %s | %s\n", topic, msg);

		// MOTOR
		if (strcmp(topic, Mqtt::CONFIG.topics.cmd_motor) == 0) {
			handleMotor(msg);
			return;
		}

		// JSON
		JsonDocument doc;
		if (deserializeJson(doc, msg)) {
			Serial.println("[MQTT] JSON error");
			return;
		}

		// THROTTLE
		if (strcmp(topic, Mqtt::CONFIG.topics.cmd_throttle) == 0) {
			handleThrottle(doc);
			return;
		}

		// CONFIG
		if (strcmp(topic, Mqtt::CONFIG.topics.cmd_config) == 0) {
			handleConfig(doc);
			return;
		}
	}

	// Conexão
	void ensureMqtt() {
		if (Mqtt::client.connected()) return;

		static uint32_t lastTry = 0;
		static int lastErr = -999;
		static bool firstTry = true;
		static uint32_t lastErrLog = 0;

		if (millis() - lastTry < 1500) return;
		lastTry = millis();

		char clientId[64];
		snprintf(clientId, sizeof(clientId), "%s-%llX", Mqtt::CONFIG.id_base, ESP.getEfuseMac());

		Serial.println("======================================");
		if (firstTry) {
			Serial.println("[MQTT] Connecting...");
			firstTry = false;
		}

		if (Mqtt::client.connect(clientId, Mqtt::CONFIG.topics.status, 0, true, "offline")) {
			Serial.println("[MQTT] Connected");

			Mqtt::client.publish(Mqtt::CONFIG.topics.status, "online", true);
			Mqtt::client.subscribe(Mqtt::CONFIG.topics.cmd_motor);
			Mqtt::client.subscribe(Mqtt::CONFIG.topics.cmd_throttle);
			Mqtt::client.subscribe(Mqtt::CONFIG.topics.cmd_config);

			lastErr = -999;
			firstTry = true;
			lastErrLog = 0;
		} else {
			int err = Mqtt::client.state();

			if (err != lastErr || millis() - lastErrLog > 60000) {
				Serial.printf("[MQTT] Connection failed, state=%d\n", err);
				lastErr = err;
				lastErrLog = millis();
			}
		}
		Serial.println("======================================");
	}

	// Publicar
	void mqttPublishTelemetry() {
		if (!Mqtt::client.connected()) return;

		JsonDocument doc;

		// -------- DADOS --------
		doc["volts"]     = Telemetry::data.volts;
		doc["pct"]       = Telemetry::data.pct;
		doc["rpm"]       = Telemetry::data.rpm;
		doc["speed_kmh"] = Telemetry::data.speed_kmh;

		if (isnan(Telemetry::data.temp)) {
			doc["temp"] = nullptr;
		} else {
			doc["temp"] = Telemetry::data.temp;
		}
		if (isnan(Telemetry::data.humi)) {
			doc["humi"] = nullptr;
		} else {
			doc["humi"] = Telemetry::data.humi;
		}

		doc["current_bat_a"] = Telemetry::data.current_bat_a;
		doc["current_mot_a"] = Telemetry::data.current_mot_a;

		doc["min"]      = Telemetry::data.min_v;
		doc["max"]      = Telemetry::data.max_v;
		doc["wheel_cm"] = Telemetry::data.wheel_cm;
		doc["ppr"]      = Telemetry::data.ppr;

		doc["override_enabled"] = Telemetry::data.override_enabled;
		doc["override_pct"]     = Telemetry::data.override_pct;
		doc["max_pct"]          = Telemetry::data.max_pct;

		doc["src"]     = Control::getSource();
		doc["poll_ms"] = Telemetry::data.poll_ms;

		// -------- LOGGER --------
		doc["log_enabled"] = Log::logger.enabled;
		doc["log_iv_ms"]   = Log::logger.interval_ms;
		doc["log_size"]    = Log::logger.cached_size;

		// -------- CONFIG AVANÇADA --------
		doc["pwm_hz"]          = Advanced::config.pwm_hz;
		doc["start_min_pct"]   = Advanced::config.startMin;
		doc["rapid_ms"]        = Advanced::config.rapidRamp;
		doc["rapid_up"]        = Advanced::config.rapidUp;
		doc["slew_up"]         = Advanced::config.slewUp;
		doc["slew_dn"]         = Advanced::config.slewDown;
		doc["zero_timeout_ms"] = Advanced::config.zeroTimeoutMs;

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
		bool ok = Mqtt::client.publish(Mqtt::CONFIG.topics.tlm_json, buffer, len);

		if (!ok) {
		Serial.printf("[MQTT] publish FAIL, len = %u\n", len);
		}
	}
}
// Loop e Setup
namespace Mqtt {  
	// Setup
	void setup() {
		client.setServer(CONFIG.host, CONFIG.port);
		client.setCallback(mqttCallback);
		client.setBufferSize(2048);

		Serial.println("======================================");
		Serial.println("[MQTT] Configuration:");
		Serial.printf("  Broker : %s:%d\n", CONFIG.host, Mqtt::CONFIG.port);
		Serial.printf("  Client : %s\n", CONFIG.id_base);

		Serial.println("  Topics:");
		Serial.printf("    PUB  : %s\n", CONFIG.topics.tlm_json);
		Serial.printf("    SUB  : %s\n", CONFIG.topics.cmd_motor);
		Serial.printf("    SUB  : %s\n", CONFIG.topics.cmd_throttle);
		Serial.printf("    SUB  : %s\n", CONFIG.topics.cmd_config);
		Serial.printf("    STAT : %s\n", CONFIG.topics.status);
		Serial.println("======================================");
	}

	// Loop
	void loop() {
		static bool wasConnected = false;
		bool wifiOk = (WiFi.status() == WL_CONNECTED);

		if (!wifiOk && wasConnected) {
			client.disconnect();
		}

		wasConnected = wifiOk;

		ensureMqtt();
		client.loop();

		uint32_t now = millis();
		if (now - lastMqtt < MQTT_IV_MS) return;
		lastMqtt = now;

		mqttPublishTelemetry();

		if (ack.last[0] != '\0' && (now - ack.timestamp) > 2000) {
			ack.last[0] = '\0';
		}
	}
}