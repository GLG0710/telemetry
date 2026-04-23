#pragma once
#include <Arduino.h>	
		
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