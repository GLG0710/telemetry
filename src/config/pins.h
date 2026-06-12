/*
    Pinos utilizados
    9 e 10 representam, respectivamente, Rx (GPI09) e Tx (GPI10) do esp 
*/

#pragma once

namespace Pins {
    // Arduino UART
    constexpr int ARDUINO_RX = 9;  // Pino D18 (TX1) do mega ligado ao Pino GPIO9 (Rx) Mega envia telemetria
    constexpr int ARDUINO_TX = 10; // Pino D19 (RX1) do mega ligado ao Pino GPIO10 (Tx) Esp envia comandos
    
    // LoRa UART
    // No momento não temos lora
    constexpr int LORA_RX = 18;
    constexpr int LORA_TX = 17;
}
