/*
	Define o controle atual do sistema.
	LOCAL: operação local, sem comandos remotos, dados armazenados localmente.
    BLE: controle via Bluetooth.
    MQTT: controle via MQTT.
*/

#include "control.h"

namespace Control {
	State state;

	void setSource(Source src) {
		state.src = src;
		state.last_ms = millis();
	}

	const char* getSource(){
		switch(state.src){
			case Source::BLE:  
				return "BLE";
			case Source::MQTT:
				return "MQTT";
			default:   
				return "LOCAL";
		}
	}

	void loop() {
		uint32_t now = millis();

		if (state.src != Source::LOCAL &&
			(now - state.last_ms) > TIMEOUT_MS) {

			state.src = Source::LOCAL;
			Telemetry::data.override_enabled = false;
		} 
	} 
} 