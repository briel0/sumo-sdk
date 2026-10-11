#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Caipora";

    static constexpr bool SKIP_SITE_CONFIG = false;

    static constexpr int RIGHT_POS_PIN = 16;
    static constexpr int RIGHT_NEG_PIN = 17;
    static constexpr int LEFT_POS_PIN = 18;
    static constexpr int LEFT_NEG_PIN = 19;

    static constexpr int MAX_THROTTLE = 100;
    static constexpr int TURN_COEFFICIENT = 100;
    static constexpr int PIVOT_COEFFICIENT = 100;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_JS_ESQ = 32;
    static constexpr int PIN_JS_DIR = 33;
    static constexpr int PIN_JS_FRONT = 34;

    static constexpr uint8_t PIN_LINHA_ESQ = 34;
    static constexpr uint8_t PIN_LINHA_DIR = 35;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {23, 80, 150}, // Pino 23 | Começa em 80° | Arma em 145°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 250});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

    // Macro vazia: o MotionPlayer pula o saque cego e o robo cai direto no combate.
    static const MotionSequence MACRO_SEM_SAQUE = {nullptr, 0};

    // Formato compacto, sem espaço nenhum: em "macros" o id é a posição na lista
    // (a mesma das TABELA_MACROS_*), e em "searches" cada item é [id, nome].
    static constexpr const char *UI_PROFILE_JSON =
        R"({"robot_name":"Caipora",)"
        R"("macros":["FRENTÃO","DIAGONAL","SEM SAQUE"],)"
        R"("searches":[[2,"RÁPIDA DEFENSIVA"],[3,"LENTA DEFENSIVA"],[4,"RÁPIDA OFENSIVA"],[5,"LENTA OFENSIVA"]],)"
        R"("has_weapons":true})";

    static const MotionSequence *const TABELA_MACROS_ESQ[] = {&MACRO_FRENTAO, &MACRO_DIAGONAL, &MACRO_SEM_SAQUE};

    static const MotionSequence *const TABELA_MACROS_DIR[] = {&MACRO_FRENTAO, &MACRO_DIAGONAL, &MACRO_SEM_SAQUE};

}