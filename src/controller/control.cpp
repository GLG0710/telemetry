#include "control.h"
#include "state/data.h"

namespace Control {
  State state;

  void setSource(Source src) {
    state.src = src;
    state.last_ms = millis();
  }

const char* getSource(){
  switch(state.src){
    case BLE:  return "BLE";
    case MQTT: return "MQTT";
    default:   return "LOCAL";
  }
}

void loop() {
  unsigned long now = millis();

  if (state.src != LOCAL &&
      (now - state.last_ms) > TIMEOUT_MS) {

    state.src = LOCAL;
    data.override_enabled = false;
  } 
} 
} 