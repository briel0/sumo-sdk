#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Fueguito";

    static constexpr int RIGHT_POS_PIN = 17;
    static constexpr int RIGHT_NEG_PIN = 16;
    static constexpr int LEFT_POS_PIN = 18;
    static constexpr int LEFT_NEG_PIN = 19;

    static constexpr int MAX_THROTTLE = 90;
    static constexpr int TURN_COEFFICIENT = 93;
    static constexpr int PIVOT_COEFFICIENT = 75;

    static constexpr int NUM_SERVOS = 2;

    // Pinos de sensores de distância, não utilizados neste perfil RC.
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
        {32, 0, 90},   // Servo Esquerdo: PINO_SERVO_ASA_ESQ, retrai em 0°, avança para 90°
        {26, 180, 90}, // Servo Direito: PINO_SERVO_ASA_DIR, retrai em 180°, avança para 90°
    };

    static const MotionSequence MACRO_FRENTAO = MACRO(
        {100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO(
        {-100, 100, 30},
        {100, 100, 200},);

    static const MotionSequence MACRO_CURVINHA_DIREITA = MACRO(
        {100, -100, 72},
        {100, 25, 245}, {100, 100, 72});

    static const MotionSequence MACRO_CURVINHA_ESQUERDA = MACRO(
        {-100, 100, 72},
        {25, 100, 245}, {100, 100, 72});


    static constexpr const char* UI_PROFILE_JSON = R"({
        "robot_name": "Fueguito",
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
