#include "control.h"
#include "core/state.h"

void controlSetSource(ControlSource src) {
  ctrl.src = src;
  ctrl.last_ms = millis();
}

const char* ctrlGetSource(ControlSource src){
  switch(src){
    case CTRL_BLE:  return "BLE";
    case CTRL_MQTT: return "MQTT";
    default:        return "LOCAL";
  }
}

void controlUpdateTimeout() {
  unsigned long now = millis();

  if (ctrl.src != CTRL_LOCAL &&
      (now - ctrl.last_ms) > ctrl.TIMEOUT_MS) {

    ctrl.src = CTRL_LOCAL;
    data.override_enabled = false;
  }
}