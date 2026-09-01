#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Caipora";

    // === Rádio =================================================================
    // Canal WiFi do AP de configuração (2.4GHz), espalhado entre os canais
    // não-sobrepostos (1/6/11) pra dois robôs em bancada não brigarem pelo mesmo
    // espectro na seleção de tática. O AP morre na largada (ConfigServer::
    // shutdown()), então isso só importa durante a configuração.
    static constexpr int WIFI_CHANNEL = 1;

    // MAC do controle Bluetooth "dono" — allowlist em tempo de compilação. Só
    // este controle pareia, eliminando cross-pairing quando dois robôs nossos
    // entram em RC ao mesmo tempo. Tudo-zero = modo descoberta (aceita o
    // primeiro controle e imprime o MAC no serial pra você fixar aqui).
    static constexpr uint8_t CONTROLLER_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static constexpr int RIGHT_POS_PIN = 16;
    static constexpr int RIGHT_NEG_PIN = 17;
    static constexpr int LEFT_POS_PIN = 19;
    static constexpr int LEFT_NEG_PIN = 18;

    static constexpr int MAX_THROTTLE = 85;
    static constexpr int TURN_COEFFICIENT = 90;
    static constexpr int PIVOT_COEFFICIENT = 95;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_STATUS_LED = 33;
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
        {4, 15, 90}, // Pino 4 | Começa em 15° | Arma em 85°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

    static constexpr int PIN_VL_CENT_ESQ = 15;
    static constexpr int PIN_VL_CENT_DIR = 36;
    static constexpr int PIN_VL_ESQ = 4;
    static constexpr int PIN_VL_DIR = 14;


    static constexpr uint8_t PIN_LINHA_ESQ = 30;
    static constexpr uint8_t PIN_LINHA_DIR = 35;
    static constexpr uint16_t LINHA_THRESHOLD = 2800;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {};

    static const MotionSequence MACRO_FRENTAO = MACRO(
        {100, 100, 200},);

    static const MotionSequence MACRO_DIAGONAL = MACRO(
        {-100, 100, 50},
        {100, 100, 100});

    static const MotionSequence MACRO_CURVINHA_DIREITA = MACRO(
        {100, -100, 70},
        {60, 100, 150}, {-100, 100, 60});

    static const MotionSequence MACRO_CURVINHA_ESQUERDA = MACRO(
        {-100, 100, 70},
        {100, 60, 150}, {100, -100, 60});

    static const MotionSequence MACRO_CURVAO_ESQUERDA = MACRO(
        {-100, 100, 70},
        {100, 40, 600}
    );
    
    static const MotionSequence MACRO_CURVAO_DIREITA = MACRO(
        {100, -100, 70},
        {40, 100, 500}
    );


    // Tabela de macros mapeadas pelo ID que vem do site (0 = Frentão, 1 = Curvão)
    static const MotionSequence* const TABELA_MACROS_ESQ[] = {
        &MACRO_FRENTAO,
        &MACRO_CURVAO_ESQUERDA,
        &MACRO_CURVINHA_ESQUERDA

    };

    static const MotionSequence* const TABELA_MACROS_DIR[] = {
        &MACRO_FRENTAO,
        &MACRO_CURVAO_DIREITA,
        &MACRO_CURVINHA_DIREITA
    };

    static constexpr const char* UI_PROFILE_JSON = R"({
        "robot_name": "Arruela",
        "macros": [
            {"id": 0, "name": "FRENTAO"},
            {"id": 1, "name": "CURVAO"}
        ],
        "searches": [
            {"id": 1, "name": "BUSCA PADRAO"}
        ],
        "has_weapons": false
    })";

}