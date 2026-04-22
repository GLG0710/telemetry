#include <Arduino.h>
#include <config/pins.h>

namespace Pins {
    HardwareSerial SerialArd(1);
    HardwareSerial SerialLora(2);

    void init() {
        SerialArd.begin(ARDUINO_BAUD, SERIAL_8N1, ARDUINO_RX, ARDUINO_TX);
        SerialLora.begin(LORA_BAUD, SERIAL_8N1, LORA_RX, LORA_TX);
    }
}