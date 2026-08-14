// ============================================================================
// Fumacinha RC — main enxuto para a placa ESP32-C3 (supermini).
//
// Só a pilha de RC: Bluepad32 (Receiver) -> RCMode -> Drive (LEDC) + servos.
// Sem IR, sem tira de LED, sem WiFi/AUTO — nada disso existe nesta placa. Os
// arquivos do modo AUTO nem são compilados aqui (ver build_src_filter do env
// fumacinha_rc no platformio.ini).
//
// O corpo inteiro fica sob ROBOT_FUMACINHA_RC porque nos builds do ESP32 clássico
// este arquivo também é compilado (eles não filtram o src/) — e aí ele precisa
// virar vazio pra não colidir com o setup()/loop() do main.cpp.
// ============================================================================
#if defined(ROBOT_FUMACINHA_RC)

#include "Config.hpp"
#include "Drive.hpp"
#include "RCMode.hpp"
#include "ServoMechanism.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

Drive        motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
WeaponSystem sistemaDeArmas;
RCMode       modoRC;

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("[C3-RC] Fumacinha RC iniciando (ESP32-C3).");

    // Não chamamos ESP32PWM::allocateTimer aqui de propósito: no C3 os 6 canais
    // LEDC são poucos e o Drive prende os dele de forma preguiçosa no loop. Deixar
    // a ESP32PWM em modo de auto-alocação coordena os 4 canais de motor com os 2
    // de servo sem a gente ter que casar timer a timer na mão.
    for(int i = 0; i < Config::NUM_SERVOS; i++) {
        ServoMechanism *s =
            new ServoMechanism(Config::SERVOS[i].pin, Config::SERVOS[i].retractAngle, Config::SERVOS[i].deployAngle);
        s->init();
        sistemaDeArmas.addServo(s);
    }

    // Sobe o Bluepad32 e trava no MAC do perfil (ou entra em modo descoberta se
    // CONTROLLER_MAC for tudo-zero). Lembrando: no C3 o controle tem que ser BLE.
    modoRC.init();
    Serial.println("[C3-RC] Pronto. Aguardando controle BLE...");
}

void loop() {
    modoRC.run(motores, sistemaDeArmas);
    yield();
}

#endif // ROBOT_FUMACINHA_RC
