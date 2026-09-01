#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Marola";

    // === Rádio =================================================================
    // Canal WiFi do AP de configuração (2.4GHz), espalhado entre os canais
    // não-sobrepostos (1/6/11) pra dois robôs em bancada não brigarem pelo mesmo
    // espectro na seleção de tática. O AP morre na largada (ConfigServer::
    // shutdown()), então isso só importa durante a configuração.
    static constexpr int WIFI_CHANNEL = 6;

    // MAC do controle Bluetooth "dono" — allowlist em tempo de compilação. Só
    // este controle pareia, eliminando cross-pairing quando dois robôs nossos
    // entram em RC ao mesmo tempo. Tudo-zero = modo descoberta (aceita o
    // primeiro controle e imprime o MAC no serial pra você fixar aqui).
    static constexpr uint8_t CONTROLLER_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static constexpr int RIGHT_POS_PIN = 18;
    static constexpr int RIGHT_NEG_PIN = 19;
    static constexpr int LEFT_POS_PIN = 16;
    static constexpr int LEFT_NEG_PIN = 17;

    static constexpr int MAX_THROTTLE = 90;
    static constexpr int TURN_COEFFICIENT = 93;
    static constexpr int PIVOT_COEFFICIENT = 75;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_STATUS_LED   = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // === Núcleo de Hardware ======================================================
    // Família HW_FAMILY_NONE (ver HardwareFamily.hpp): robô sem front-end de
    // sensores de combate, então não define pino de sensor/atuador nenhum — o
    // HardwareCore desta família não instancia nada. Se ganhar sensores, mova o
    // robô para outra família em HardwareFamily.hpp e adicione os pinos aqui.
    static constexpr bool USES_HARDWARE_CORE = false;

    // === FumacinhaAuto (modo AUTO) ==============================================
    static constexpr bool USES_FUMACINHA_FSM = false;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {26, 15, 120}, // Pino 22 | Começa em 15° | Arma em 120°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO(
        {100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO(
        {-100, 100, 30},
        {100, 100, 200},);

}