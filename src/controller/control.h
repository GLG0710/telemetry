#pragma once

#include <Arduino.h>

#include "core/telemetry.h"

namespace Control {
	enum class Source {
		LOCAL,
		BLE,
		MQTT
	};

	struct State {
		Source src = LOCAL;          
		uint32_t last_ms = 0;
	};
	extern State state;

	constexpr uint32_t TIMEOUT_MS = 2000;  

	// funções de controle
	void setSource(Source src);
	const char* getSource();
	void loop();
}