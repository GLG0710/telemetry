#pragma once
#include <Arduino.h>

namespace Pins {
    extern HardwareSerial SerialArd; // UART 1
    // Arduino UART
    constexpr int ARDUINO_RX = 9;
    constexpr int ARDUINO_TX = 10;
    constexpr uint32_t ARDUINO_BAUD = 115200;
    
    extern HardwareSerial SerialLora; // UART 2
    // LoRa UART
    constexpr int LORA_RX = 18;
    constexpr int LORA_TX = 17;
    constexpr uint32_t LORA_BAUD = 9600;

    void init();
}
