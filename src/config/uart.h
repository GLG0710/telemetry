#include <Arduino.h>
#pragma once

namespace Uart {
    extern HardwareSerial Arduino;
    extern HardwareSerial Lora;

    void setup();
}