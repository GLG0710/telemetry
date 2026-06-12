#pragma once
#include <Arduino.h>

namespace Telemetry {
	struct Data {
		float volts = 0.0f;
		float pct = 0.0f;
		
		float temp = NAN;
		float humi = NAN;

		float rpm = 0.0f;
		float speed_kmh = 0.0f;

		float current_bat_a = 0.0f;
		float current_mot_a = 0.0f;

		float min_v = 0.80f;
		float max_v = 4.20f;

		float wheel_cm = 50.8f;
		uint8_t ppr = 1;

		bool override_enabled = false;
		float override_pct = 0;

		float max_pct = 100.0f;
		uint32_t poll_ms = 1000;
	};
	extern Data data;
}

