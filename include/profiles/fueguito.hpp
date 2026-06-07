#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Fueguito";

    static constexpr int RIGHT_POS_PIN = 17;
    static constexpr int RIGHT_NEG_PIN = 16;
    static constexpr int LEFT_POS_PIN = 19;
    static constexpr int LEFT_NEG_PIN = 18;

    static constexpr int MAX_THROTTLE = 90;
    static constexpr int TURN_COEFFICIENT = 93;
    static constexpr int PIVOT_COEFFICIENT = 75;

    static constexpr int NUM_SERVOS = 0;

    static constexpr int PIN_JS_ESQ = 32;
    static constexpr int PIN_JS_DIR = 33;
    static constexpr int PIN_JS_FRONT = 34;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {};

    static const MotionSequence MACRO_FRENTAO = MACRO(
        {100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO(
        {-100, 100, 30},
        {100, 100, 200},);

}