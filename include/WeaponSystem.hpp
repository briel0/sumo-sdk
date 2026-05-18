#pragma once
#include <ServoMechanism.hpp>

class WeaponSystem {
  public:
    void init() {
        for(int i = 0; i < servoCount; i++) {
            if(servos[i] != nullptr)
                servos[i]->init();
        }
    }

    void addServo(ServoMechanism *servo) {
        if(servoCount < MAX_SERVOS && servo != nullptr) {
            servos[servoCount] = servo;
            servoCount++;
        }
    }

    void deployAll() {
        for(int i = 0; i < servoCount; i++) {
            if(servos[i] != nullptr)
                servos[i]->deploy();
        }
    }

    void retractAll() {
        for(int i = 0; i < servoCount; i++) {
            if(servos[i] != nullptr)
                servos[i]->retract();
        }
    }

    void relaxAll() {
        for(int i = 0; i < servoCount; i++) {
            if(servos[i] != nullptr)
                servos[i]->relax();
        }
    }

  private:
    /**
    @brief Maximum number of servos that can be managed by the weapon system.
    */
    static constexpr int MAX_SERVO = 4;

    /**
    @brief Array of pointers to the attached ServoMechanism objects.
    */
    ServoMechanism *servos[MAX_SERVO] = {nullptr};

    /**
    @brief Current count of registered servos in the system.
    */
    int servoCount = 0;
};