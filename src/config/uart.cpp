/*
    Inicializa interfaces UART
    Arduino: comunicação com o Arduino Mega
    Lora: comunicação com o módulo LoRa
*/

#include "uart.h"

namespace Uart {
    HardwareSerial Arduino(1);
    HardwareSerial Lora(2);

    void setup() {
        Arduino.begin(115200, SERIAL_8N1, Pins::ARDUINO_RX, Pins::ARDUINO_TX);
        Lora.begin(9600, SERIAL_8N1, Pins::LORA_RX, Pins::LORA_TX);
    }
}