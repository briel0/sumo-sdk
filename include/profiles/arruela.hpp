#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Arruela";

    static constexpr int RIGHT_POS_PIN = 18;
    static constexpr int RIGHT_NEG_PIN = 19;
    static constexpr int LEFT_POS_PIN = 16;
    static constexpr int LEFT_NEG_PIN = 17;

    static constexpr int MAX_THROTTLE = 90;
    static constexpr int TURN_COEFFICIENT = 83;
    static constexpr int PIVOT_COEFFICIENT = 70;

    static constexpr int NUM_SERVOS = 0;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {};

    static constexpr MotionStep FRENTAO_STEPS[] = {
        {100, 100, 200},
    };

    static constexpr MotionSequence MACRO_FRENTAO = MOTION_SEQ(FRENTAO_STEPS);

    static constexpr MotionStep DIAGONAL_STEPS[] = {
        {-100, 100, 50},
        {100, 100, 100},
    };

    static constexpr MotionSequence MACRO_DIAGONAL = MOTION_SEQ(DIAGONAL_STEPS);

}