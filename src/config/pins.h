#pragma once

namespace Pins {
    // Arduino UART
    constexpr int ARDUINO_RX = 9;  // Pino D18 (TX1) do mega ligado ao Pino GPIO9 (Rx) Mega envia telemetria
    constexpr int ARDUINO_TX = 10; // Pino D19 (RX1) do mega ligado ao Pino GPIO10 (Tx) Esp envia comandos
    
    // LoRa UART
    constexpr int LORA_RX = 18;
    constexpr int LORA_TX = 17;
}
