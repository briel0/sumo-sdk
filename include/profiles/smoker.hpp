#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Smoker";

    static constexpr int RIGHT_POS_PIN = 26;
    static constexpr int RIGHT_NEG_PIN = 25;
    static constexpr int LEFT_POS_PIN = 18;
    static constexpr int LEFT_NEG_PIN = 19;

    static constexpr int MAX_THROTTLE = 90;
    static constexpr int TURN_COEFFICIENT = 83;
    static constexpr int PIVOT_COEFFICIENT = 70;

    static constexpr int NUM_SERVOS = 2;

    static constexpr int PIN_JS_ESQ = 32;
    static constexpr int PIN_JS_DIR = 33;
    static constexpr int PIN_JS_FRONT = 34;

    static constexpr uint8_t PIN_LINHA_ESQ = 34;
    static constexpr uint8_t PIN_LINHA_DIR = 35;
    static constexpr uint16_t LINHA_THRESHOLD = 2800;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {22, 15, 120}, // Pino 22 | Começa em 15° | Arma em 120°
        {23, 180, 45}  // Pino 23 | Começa em 180°| Arma em 45°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO(
        {100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO(
        {-100, 100, 30}, 
        {100, 100, 200});


    static constexpr const char* UI_PROFILE_JSON = R"({
        "robot_name": "Smoker",
        "macros": [
            {"id": 0, "name": "FRENTAO"}
        ],
        "searches": [
            {"id": 1, "name": "BUSCA PADRAO"}
        ],
        "has_weapons": true
    })";

    static const MotionSequence* const TABELA_MACROS_ESQ[] = {
        &MACRO_FRENTAO
    };

    static const MotionSequence* const TABELA_MACROS_DIR[] = {
        &MACRO_FRENTAO
    };

}
