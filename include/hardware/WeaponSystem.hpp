#pragma once

class ServoMechanism;

class WeaponSystem {
  public:
    void init();
    void addServo(ServoMechanism *servo);
    void deploy();
    void retract();
    void update();
    bool isDeployed() const {
        return isDeployedFlag;
    }

  private:
    static constexpr int MAX_SERVO = 4;
    static constexpr unsigned long RELAX_TIMEOUT_MS = 1000;

    ServoMechanism *servos[MAX_SERVO] = {nullptr};
    int servoCount = 0;

    bool isDeployedFlag = false;
    bool isRelaxedFlag = false;
    unsigned long deployTimeStart = 0;

    void deployAll();
    void retractAll();
    void relaxAll();
};