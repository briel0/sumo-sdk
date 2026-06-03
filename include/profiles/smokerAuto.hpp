#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Smoker";

    static constexpr int RIGHT_POS_PIN = 18;
    static constexpr int RIGHT_NEG_PIN = 19;
    static constexpr int LEFT_POS_PIN = 16;
    static constexpr int LEFT_NEG_PIN = 17;

    static constexpr int MAX_THROTTLE = 90;
    static constexpr int TURN_COEFFICIENT = 83;
    static constexpr int PIVOT_COEFFICIENT = 70;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_JS_ESQ = 39;
    static constexpr int PIN_JS_DIR = 36;
    static constexpr int PIN_JS_FRONT = 21;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {22, 15, 120}, // Pino 22 | Começa em 15° | Arma em 120°
    };

    // Velocidade no motor direito, velocidade no motor esquerdo, tempo
    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

}