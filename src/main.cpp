#include <Arduino.h>
#include <Drive.hpp>
#include <Config.hpp>

Drive drive(RIGHT_POS_PIN, RIGHT_NEG_PIN, LEFT_POS_PIN, LEFT_NEG_PIN);

void setup(){
    Serial.begin(115200);
    Serial.println("Teste do MCPWM");
    Serial.println("Iniciando em 3 segundos...");
    delay(3000);
}

void loop(){
    // 1. TESTE: FRENTE (60% de potência para não sair voando da bancada)
    Serial.println("Movimento: FRENTE");
    drive.setSpeed(60, 60);
    delay(2000);

    // 2. TESTE: FREIO ATIVO (Deve travar as rodas instantaneamente)
    Serial.println("Estado: FREIO");
    drive.setSpeed(0, 0); 
    delay(1000);

    // 3. TESTE: TRÁS (A função setSpeed deve aplicar o dead-time automaticamente)
    Serial.println("Movimento: TRÁS");
    drive.setSpeed(-60, -60);
    delay(2000);

    // 4. TESTE: PARADA EM COAST (Rodas soltas)
    Serial.println("Estado: RELEASE (Coast)");
    drive.release();
    delay(2000);
}