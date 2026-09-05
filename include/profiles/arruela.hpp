#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Arruela";

    static constexpr int RIGHT_POS_PIN = 19;
    static constexpr int RIGHT_NEG_PIN = 18;
    static constexpr int LEFT_POS_PIN = 16;
    static constexpr int LEFT_NEG_PIN = 17;

    static constexpr int MAX_THROTTLE = 100;     // Velocidade Máxima (Pra Frente, Pra Trás)
    static constexpr int TURN_COEFFICIENT = 83;  // Coeficiente de Curva
    static constexpr int PIVOT_COEFFICIENT = 70; // Coeficiente de Rotação

    static constexpr int NUM_SERVOS = 0;

    static constexpr int PIN_JS_ESQ = 27;
    static constexpr int PIN_JS_DIR = 14;
    static constexpr int PIN_JS_FRONT = 4;

    // Canal do AP de configuracao. Espalhados entre 1/6/11 (os tres
    // nao-sobrepostos) pra dois robos ligados na mesma bancada nao
    // disputarem o mesmo espectro e derrubarem o portal um do outro.
    static constexpr int WIFI_CHANNEL = 11;

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

    // Macro vazia: o MotionPlayer pula o saque cego e o robo cai direto no combate.
    static const MotionSequence MACRO_SEM_SAQUE = {nullptr, 0};

    // Tabela de macros mapeadas pelo ID que vem do site (0 = Frentão, 1 = Curvão, 2 = Sem Saque)
    static const MotionSequence *const TABELA_MACROS_ESQ[] = {&MACRO_FRENTAO, &MACRO_CURVAO_ESQUERDA, &MACRO_SEM_SAQUE};

    static const MotionSequence *const TABELA_MACROS_DIR[] = {&MACRO_FRENTAO, &MACRO_CURVAO_DIREITA, &MACRO_SEM_SAQUE};

    static constexpr const char *UI_PROFILE_JSON = R"({
        "robot_name": "Arruela",
        "macros": [
            {"id": 0, "name": "FRENTÃO"},
            {"id": 1, "name": "CURVÃO"},
            {"id": 2, "name": "SEM SAQUE"}
        ],
        "searches": [
            {"id": 1, "name": "BUSCA PADRÃO"},
            {"id": 2, "name": "BUSCA LENTA"},
            {"id": 3, "name": "BUSCA POR DISTÂNCIA"}
        ],
        "has_weapons": false
    })";

}