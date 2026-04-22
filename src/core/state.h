#pragma once
#include <Arduino.h>	
#include <LittleFS.h>
		
struct TelemetryData {
	float volts = 0.0f; // Talvez mudar para int dependendo do desempenho
	float pct = 0.0f;
  	float temp = NAN;
  	float humi = NAN;

  	float rpm = 0.0f;
  	float speed_kmh = 0.0f;

  	float current_bat_a = 0.0f; // Talvez mudar para int dependendo do desempenho
  	float current_mot_a = 0.0f; // Talvez mudar para int dependendo do desempenho

  	float min_v = 0.80f;
  	float max_v = 4.20f;
  	float wheel_cm = 50.8f;
  	uint8_t ppr = 1;

  	bool override_enabled = false;
  	float override_pct = 0;

  	float max_pct = 100.0f;
  	uint32_t poll_ms = 1000;
};
extern TelemetryData data;

struct AckState {
	char senseLine[256];
	char last[64];
	unsigned long timestamp = 0;
};
extern AckState ack;

struct AdvancedConfig {
	float pwm_hz = 1000.0f;
  	float start_min_pct = 8.0f;
  	float rapid_ms = 250.0f;
  	float rapid_up = 150.0f;
  	float slew_up = 40.0f;
  	float slew_dn = 60.0f;
  	float zero_timeout_ms = 600.0f;
};
extern AdvancedConfig config;

struct Log {
  const char* PATH = "/telemetry.csv";
  const char* PATH_OLD = "/telemetry.csv.1";

  bool enabled = false;
  uint32_t interval_ms = 1000;
  unsigned long last_log = 0;

  File file;
  bool isOpen = false;
  size_t cached_size = 0;
};
extern Log logger;