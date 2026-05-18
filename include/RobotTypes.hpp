#pragma once

// Esta struct é apenas um envelope de dados. Não aciona hardware nenhum.
struct ServoConfig {
    /**
    @brief The GPIO pin to which the servo is connected.
    */
    int pin;

    /**
    @brief The default angle in degrees when the servo is in the retracted/disarmed position.
    */
    int retractAngle;

    /**
    @brief The default angle in degrees when the servo is in the deployed/armed position.
    */
    int deployAngle;
};