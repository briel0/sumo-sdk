#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Caipora";

    static constexpr int RIGHT_POS_PIN = 16;
    static constexpr int RIGHT_NEG_PIN = 17;
    static constexpr int LEFT_POS_PIN = 18;
    static constexpr int LEFT_NEG_PIN = 19;

    static constexpr int MAX_THROTTLE = 100;
    static constexpr int TURN_COEFFICIENT = 95;
    static constexpr int PIVOT_COEFFICIENT = 95;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_JS_ESQ = 32;
    static constexpr int PIN_JS_DIR = 33;
    static constexpr int PIN_JS_FRONT = 34;

    static constexpr uint8_t PIN_LINHA_ESQ = 34;
    static constexpr uint8_t PIN_LINHA_DIR = 35;
    static constexpr uint16_t LINHA_THRESHOLD = 2800;

    // Canal do AP de configuracao. Espalhados entre 1/6/11 (os tres
    // nao-sobrepostos) pra dois robos ligados na mesma bancada nao
    // disputarem o mesmo espectro e derrubarem o portal um do outro.
    static constexpr int WIFI_CHANNEL = 1;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {23, 80, 150}, // Pino 23 | Começa em 80° | Arma em 145°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 170});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

    // Macro vazia: o MotionPlayer pula o saque cego e o robo cai direto no combate.
    static const MotionSequence MACRO_SEM_SAQUE = {nullptr, 0};

    static constexpr const char *UI_PROFILE_JSON = R"({
        "robot_name": "Caipora",
        "macros": [
            {"id": 0, "name": "FRENTÃO"},
            {"id": 1, "name": "DIAGONAL"},
            {"id": 2, "name": "SEM SAQUE"}
        ],
        "searches": [
            {"id": 1, "name": "BUSCA PADRÃO"}
        ],
        "has_weapons": true
    })";

    static const MotionSequence *const TABELA_MACROS_ESQ[] = {&MACRO_FRENTAO, &MACRO_DIAGONAL, &MACRO_SEM_SAQUE};

    static const MotionSequence *const TABELA_MACROS_DIR[] = {&MACRO_FRENTAO, &MACRO_DIAGONAL, &MACRO_SEM_SAQUE};

}