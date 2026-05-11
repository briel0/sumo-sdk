#include "Receiver.hpp"
#include <Arduino.h>

Receiver receptor;

void setup() {
    Serial.begin(115200);

    // Dá 2 segundos de respiro no boot.
    // É o tempo exato para você clicar no monitor serial do VS Code e não perder os primeiros prints.
    delay(2000);

    Serial.println("\n=========================================");
    Serial.println("   SISTEMA DE RÁDIO INICIADO (MODO TESTE)  ");
    Serial.println("=========================================\n");

    receptor.init();
}

void loop() {
    receptor.update();

    delay(1500);
}