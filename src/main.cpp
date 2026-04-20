#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("Sistema iniciado");
}

void loop() {
    Serial.println("Rodando...");
    delay(1000);
}