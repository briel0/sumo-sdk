#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Marola";

    static constexpr int RIGHT_POS_PIN = 17;
    static constexpr int RIGHT_NEG_PIN = 16;
    static constexpr int LEFT_POS_PIN = 18;
    static constexpr int LEFT_NEG_PIN = 19;

    static constexpr int MAX_THROTTLE = 100;
    static constexpr int TURN_COEFFICIENT = 98;
    static constexpr int PIVOT_COEFFICIENT = 95;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_LDR_ESQ = 34;
    static constexpr int PIN_LDR_DIR = 36;
    static constexpr int PIN_LDR_FRONT = 39;
    static constexpr int LDR_THRESHOLD = 2500; // abaixo disso: oponente bloqueando a luz

    static constexpr uint8_t PIN_LINHA_ESQ = 15;
    static constexpr uint8_t PIN_LINHA_DIR = 14;
    static constexpr uint16_t LINHA_THRESHOLD_ESQ = 1700;
    static constexpr uint16_t LINHA_THRESHOLD_DIR = 1700;

    // Canal do AP de configuracao. Espalhados entre 1/6/11 (os tres
    // nao-sobrepostos) pra dois robos ligados na mesma bancada nao
    // disputarem o mesmo espectro e derrubarem o portal um do outro.
    static constexpr int WIFI_CHANNEL = 1;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {26, 125, 35}, // Pino 22 | Começa em 15° | Arma em 120°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO(
        {100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO(
        {-100, 100, 30},
        {100, 100, 200},);

    static const MotionSequence MACRO_CURVINHA_DIREITA = MACRO(
        {100, -100, 72},
        {100, 25, 245}, {100, 100, 72});

    static const MotionSequence MACRO_CURVINHA_ESQUERDA = MACRO(
        {-100, 100, 72},
        {25, 100, 245}, {100, 100, 72});

    static const MotionSequence MACRO_CURVAO_ESQUERDA = MACRO(
        {-100, 100, 70},
        {100, 40, 600}
    );

    static const MotionSequence MACRO_CURVAO_DIREITA = MACRO(
        {100, -100, 70},
        {40, 100, 500}
    );

    // Macro vazia: o MotionPlayer pula o saque cego e o robo cai direto no combate.
    static const MotionSequence MACRO_SEM_SAQUE = {nullptr, 0};

    static constexpr const char *UI_PROFILE_JSON = R"({
        "robot_name": "Marola",
        "macros": [
            {"id": 0, "name": "FRENTAO"},
            {"id": 1, "name": "SEM SAQUE"},
            {"id": 2, "name": "CURVAO"}
        ],
        "searches": [
            {"id": 1, "name": "BUSCA PADRAO"}
        ],
        "has_weapons": true
    })";

    static const MotionSequence *const TABELA_MACROS_ESQ[] = {&MACRO_FRENTAO, &MACRO_SEM_SAQUE, &MACRO_CURVAO_ESQUERDA};

    static const MotionSequence *const TABELA_MACROS_DIR[] = {&MACRO_FRENTAO, &MACRO_SEM_SAQUE, &MACRO_CURVAO_DIREITA};

}
