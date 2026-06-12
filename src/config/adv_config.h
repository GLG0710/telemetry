#pragma once

#include <Arduino.h>

namespace Advanced {
	// Alguns dos tipos estão diferentes do mega (pwm, startmin, rapidRamp), conversões ocorrem entre esp e mega
	// Para corrigir isso tem que mudar aqui e o accelerator_config_mqtt.html
	
	struct Config {
		// PWM
		float pwm_hz = 1000.0f; // Valor default

		// RAMPA
		float rapidRamp = 250;
		float rapidUp = 150.0f;
		float slewUp = 40.0f;
		float slewDown = 60.0f;
		float startMin = 8;

		// Rpm
		float zeroTimeoutMs = 600; // ms (No mega é em us, por isso precisa ser float)
	};
	extern Config config;
}