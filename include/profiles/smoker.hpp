#pragma once
#include "./RobotTypes.hpp"

namespace Config {
    static constexpr int RIGHT_POS_PIN = 26;
    static constexpr int RIGHT_NEG_PIN = 25;
    static constexpr int LEFT_POS_PIN = 18;
    static constexpr int LEFT_NEG_PIN = 19;

    static constexpr int MAX_THROTTLE = 90;
    static constexpr int TURN_COEFFICIENT = 83;
    static constexpr int PIVOT_COEFFICIENT = 70;

    static constexpr int NUM_SERVOS = 2;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {22, 15, 120}, // Pino 22 | Começa em 15° | Arma em 120°
        {23, 180, 45}  // Pino 23 | Começa em 180°| Arma em 45°
    };

    static constexpr MotionStep FRENTAO_STEPS[] = {
        {100, 100, 300},
    };

    static constexpr MotionSequence MACRO_FRENTAO = MOTION_SEQ(FRENTAO_STEPS);

    static constexpr MotionStep DIAGONAL_STEPS[] = {
        {-100, 100, 30},
        {100, 100, 200},
    };

    static constexpr MotionSequence MACRO_DIAGONAL = MOTION_SEQ(DIAGONAL_STEPS);

}