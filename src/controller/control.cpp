#include "control.h"
#include "state/data.h"

Control ctrl;

void controlSetSource(ControlSource src) {
  ctrl.src = src;
  ctrl.last_ms = millis();
}

const char* ctrlGetSource(){
  switch(ctrl.src){
    case BLE:  return "BLE";
    case MQTT: return "MQTT";
    default:   return "LOCAL";
  }
}

void controlUpdateTimeout() {
  unsigned long now = millis();

  if (ctrl.src != LOCAL &&
      (now - ctrl.last_ms) > Control::TIMEOUT_MS) {

    ctrl.src = LOCAL;
    data.override_enabled = false;
  } 
} 