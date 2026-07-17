#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Caipora";

    static constexpr int RIGHT_POS_PIN = 16;
    static constexpr int RIGHT_NEG_PIN = 17;
    static constexpr int LEFT_POS_PIN = 19;
    static constexpr int LEFT_NEG_PIN = 18;

    static constexpr int MAX_THROTTLE = 100;
    static constexpr int TURN_COEFFICIENT = 83;
    static constexpr int PIVOT_COEFFICIENT = 70;

    static constexpr int NUM_SERVOS = 1;

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
        {23, 8, 62}, // Pino 23 | Começa em 15° | Arma em 120°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 170});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

}