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

    void setServoAngle(int index, int angle);

    // Anda o servo `index` delta graus dentro dos limites dele — não mexe
    // em isDeployed/relax, o servo segura a posição.
    void nudgeServo(int index, int delta);

  private:
    static constexpr int MAX = 4;
    static constexpr unsigned long RELAX_TIMEOUT_MS = 1000;

    ServoMechanism *servos[MAX] = {nullptr};
    int servoCount = 0;

    bool isDeployedFlag = false;
    bool isRelaxedFlag = false;
    unsigned long deployTimeStart = 0;

    void deployAll();
    void retractAll();
    void relaxAll();
};