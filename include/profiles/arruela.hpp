#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Arruela";

    static constexpr int RIGHT_POS_PIN = 17;
    static constexpr int RIGHT_NEG_PIN = 16;
    static constexpr int LEFT_POS_PIN = 18;
    static constexpr int LEFT_NEG_PIN = 19;

    static constexpr int MAX_THROTTLE = 100;      // Velocidade Máxima (Pra Frente, Pra Trás)
    static constexpr int TURN_COEFFICIENT = 83;  // Coeficiente de Curva
    static constexpr int PIVOT_COEFFICIENT = 70; // Coeficiente de Rotação

    static constexpr int NUM_SERVOS = 0;

    static constexpr int PIN_JS_ESQ = 34;
    static constexpr int PIN_JS_DIR = 36;
    static constexpr int PIN_JS_FRONT = 39;

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

    // Tabela de macros mapeadas pelo ID que vem do site (0 = Frentão, 1 = Curvão)
    static const MotionSequence* const TABELA_MACROS_ESQ[] = {
        &MACRO_FRENTAO,
        &MACRO_CURVAO_ESQUERDA
    };

    static const MotionSequence* const TABELA_MACROS_DIR[] = {
        &MACRO_FRENTAO,
        &MACRO_CURVAO_DIREITA
    };

    static constexpr const char* UI_PROFILE_JSON = R"({
        "robot_name": "Arruela",
        "macros": [
            {"id": 0, "name": "FRENTÃO"},
            {"id": 1, "name": "CURVÃO"}
        ],
        "searches": [
            {"id": 1, "name": "BUSCA PADRÃO"}
        ],
        "has_weapons": false
    })";

}