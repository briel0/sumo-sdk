#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "SmokerAuto";

    // === Rádio =================================================================
    // Canal WiFi do AP de configuração (2.4GHz), espalhado entre os canais
    // não-sobrepostos (1/6/11) pra dois robôs em bancada não brigarem pelo mesmo
    // espectro na seleção de tática. O AP morre na largada (ConfigServer::
    // shutdown()), então isso só importa durante a configuração.
    static constexpr int WIFI_CHANNEL = 11;

    // MAC do controle Bluetooth "dono" — allowlist em tempo de compilação. Só
    // este controle pareia, eliminando cross-pairing quando dois robôs nossos
    // entram em RC ao mesmo tempo. Tudo-zero = modo descoberta (aceita o
    // primeiro controle e imprime o MAC no serial pra você fixar aqui).
    static constexpr uint8_t CONTROLLER_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static constexpr int RIGHT_POS_PIN = 16;
    static constexpr int RIGHT_NEG_PIN = 17;
    static constexpr int LEFT_POS_PIN = 19;
    static constexpr int LEFT_NEG_PIN = 18;

    static constexpr int MAX_THROTTLE = 90;
    static constexpr int TURN_COEFFICIENT = 83;
    static constexpr int PIVOT_COEFFICIENT = 70;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_JS_ESQ = 39;
    static constexpr int PIN_JS_DIR = 36;
    static constexpr int PIN_JS_FRONT = 21;

    static constexpr uint8_t  PIN_LINHA_ESQ = 34;
    static constexpr uint8_t  PIN_LINHA_DIR = 35;
    static constexpr uint16_t LINHA_THRESHOLD = 2800;

    static constexpr int PIN_STATUS_LED   = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // === Núcleo de Hardware ======================================================
    // Inicializa o HardwareCore (sensores + periféricos de estado) quando este robô entra em AUTO.
    static constexpr bool USES_HARDWARE_CORE = true;

    // Transistor low-side que chaveia o GND dos emissores IR laterais (modo furtivo).
    static constexpr int PIN_STEALTH_EMITTER = 25; // HIGH = emissores ligados

    // Sem LDR de rampa: é exclusivo da família FUMACINHA (ver HardwareFamily.hpp).
    // A família LEGACY deste robô não instancia LDR, então não define seus pinos.

    // Servo da asa lateral — posições fundamentais.
    static constexpr int PIN_SERVO_ASA     = 26;
    static constexpr int ASA_ANGLE_RETRACT = 90;  // recolhida
    static constexpr int ASA_ANGLE_LEFT    = 0;   // aberta para a esquerda
    static constexpr int ASA_ANGLE_RIGHT   = 180; // aberta para a direita

    // === Fumacinha_FSM (modo AUTO) ==============================================
    static constexpr bool USES_FUMACINHA_FSM = false;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {22, 15, 120}, // Pino 22 | Começa em 15° | Arma em 120°
    };

    // Velocidade no motor direito, velocidade no motor esquerdo, tempo
    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

}