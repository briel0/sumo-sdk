#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Marola";

    static constexpr bool SKIP_SITE_CONFIG = false;

    static constexpr int RIGHT_POS_PIN = 19;
    static constexpr int RIGHT_NEG_PIN = 18;
    static constexpr int LEFT_POS_PIN = 17;
    static constexpr int LEFT_NEG_PIN = 16;

    static constexpr int MAX_THROTTLE = 100;
    static constexpr int TURN_COEFFICIENT = 98;
    static constexpr int PIVOT_COEFFICIENT = 95;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_LDR_ESQ = 34;
    static constexpr int PIN_LDR_DIR = 36;
    static constexpr int PIN_LDR_FRONT = 39;

    // Transistor na placa que liga a alimentação dos sensores (mesma placa
    // do Arruela, por isso o nome PIN_JS_POWER mesmo sem JS40F aqui). Precisa
    // ir pra HIGH o mais cedo possível no boot (ver main.cpp) pra dar o
    // máximo de tempo de estabilização antes de ler qualquer sensor.
    static constexpr int PIN_JS_POWER = 25;

    static constexpr uint8_t PIN_LINHA_ESQ = 15;
    static constexpr uint8_t PIN_LINHA_DIR = 14;
    static constexpr uint16_t LINHA_THRESHOLD_ESQ = 1000;
    static constexpr uint16_t LINHA_THRESHOLD_DIR = 1000;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {26, 125, 15}, // Pino 22 | Começa em 15° | Arma em 120°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO(
        {100, 100, 300});

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

    // Fugas de borda: o MarolaAuto toca uma delas quando um QRE1113 ve a
    // linha branca. O nome e o lado do sensor que viu — a fuga gira pro lado
    // oposto.
    static const MotionSequence MACRO_FUGA_LINHA_ESQUERDA = MACRO(
        {-100, -100, 125}, // Da re com tudo
        {100, -100, 120}   // Gira pra direita pra fugir
    );

    static const MotionSequence MACRO_FUGA_LINHA_DIREITA = MACRO(
        {-100, -100, 125}, // Da re com tudo
        {-100, 100, 120}   // Gira pra esquerda pra fugir
    );

    // Giro inicial da ASA 100MS + GIRO (MarolaAuto::ASA_DELAY_100_GIRO): toca uma
    // vez ao entrar em combate, antes do avanço reto — o lado vem da direção
    // escolhida no site (cfg.direction), igual ao saque cego. Ajuste
    // velocidade/duração aqui pra calibrar o ângulo — nenhum outro arquivo
    // precisa mudar.
    static const MotionSequence MACRO_GIRO_INICIAL_ESQUERDA = MACRO(
        {-100, 100, 35});

    static const MotionSequence MACRO_GIRO_INICIAL_DIREITA = MACRO(
        {100, -100, 35});

    static constexpr const char *UI_PROFILE_JSON = R"({
        "robot_name": "Marola",
        "macros": [
            {"id": 0, "name": "FRENTAO"},
            {"id": 1, "name": "SEM SAQUE"},
            {"id": 2, "name": "CURVAO"}
        ],
        "searches": [
            {"id": 1, "name": "ASA 100MS"},
            {"id": 2, "name": "ASA 500MS"},
            {"id": 3, "name": "ASA 100MS + GIRO"},
            {"id": 4, "name": "BUSCA SENSOR DE LINHA"}
        ],
        "has_weapons": true
    })";

    static const MotionSequence *const TABELA_MACROS_ESQ[] = {&MACRO_FRENTAO, &MACRO_SEM_SAQUE, &MACRO_CURVAO_ESQUERDA};

    static const MotionSequence *const TABELA_MACROS_DIR[] = {&MACRO_FRENTAO, &MACRO_SEM_SAQUE, &MACRO_CURVAO_DIREITA};

}
