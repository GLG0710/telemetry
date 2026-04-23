#include <Arduino.h>
#include <config/pins.h>
#include <config/adv_config.h>

namespace Pins {
    HardwareSerial SerialArd(1);
    HardwareSerial SerialLora(2);

    void setupUart() {
        SerialArd.begin(ARDUINO_BAUD, SERIAL_8N1, ARDUINO_RX, ARDUINO_TX);
        SerialLora.begin(LORA_BAUD, SERIAL_8N1, LORA_RX, LORA_TX);

        Serial.printf("[UART] Arduino -> RX:%d TX:%d @ %d baud\n", 
                ARDUINO_RX, ARDUINO_TX, ARDUINO_BAUD);
        SerialArd.setTimeout(50);
        Serial.printf("[UART] LoRa    -> RX:%d TX:%d @ %d baud\n", 
                LORA_RX, LORA_TX, LORA_BAUD);
    }
}

AdvancedConfig config;