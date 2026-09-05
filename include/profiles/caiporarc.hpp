#pragma once
#include "./RobotTypes.hpp"

// Perfil do firmware RC do Caipora (src/tools/RCFirmware/main.cpp) —
// separado de profiles/caipora.hpp (o do AUTO) porque o RC não usa nada de
// estratégia autônoma: sem UI_PROFILE_JSON, WIFI_CHANNEL nem os pinos de
// sensor (ToF/linha/LDR), só o que RCMode/Receiver realmente tocam. Valores
// físicos (pinos de motor, servo, throttle) são os mesmos do Caipora real —
// se calibrar diferente pro manual, mexe só aqui, não afeta o AUTO.
namespace Config {

    static constexpr const char *ROBOT_NAME = "Caipora";

    static constexpr int RIGHT_POS_PIN = 16;
    static constexpr int RIGHT_NEG_PIN = 17;
    static constexpr int LEFT_POS_PIN = 19;
    static constexpr int LEFT_NEG_PIN = 18;

    static constexpr int MAX_THROTTLE = 100;
    static constexpr int TURN_COEFFICIENT = 95;
    static constexpr int PIVOT_COEFFICIENT = 70;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {4, 15, 90}, // Pino 23 | Começa em 80° | Arma em 145°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 170});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

    // Macro vazia: mantém o mesmo índice 2 da tabela do AUTO (não usada pelo
    // RCMode hoje, mas TABELA_MACROS_ESQ/DIR precisam bater com o formato
    // que o resto do código espera).
    static const MotionSequence MACRO_SEM_SAQUE = {nullptr, 0};

    static const MotionSequence *const TABELA_MACROS_ESQ[] = {&MACRO_FRENTAO, &MACRO_DIAGONAL, &MACRO_SEM_SAQUE};

    static const MotionSequence *const TABELA_MACROS_DIR[] = {&MACRO_FRENTAO, &MACRO_DIAGONAL, &MACRO_SEM_SAQUE};

}
