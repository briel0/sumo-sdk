#pragma once

class Mechanism;
struct ServoConfig;

class WeaponSystem {
  public:
    void init();
    void addServo(Mechanism *servo);

    /**
    @brief Cria o mecanismo certo pro tipo declarado no profile (posição ou contínuo) e adiciona. Não inicializa
   — chamar init() depois de adicionar todos.
    */
    void addServo(const ServoConfig &config);

    void deploy();
    void retract();
    void update();
    bool isDeployed() const {
        return isDeployedFlag;
    }

    void setServoAngle(int index, int angle);

  private:
    static constexpr int MAX = 4;

    Mechanism *servos[MAX] = {nullptr};
    int servoCount = 0;

    bool isDeployedFlag = false;

    void deployAll();
    void retractAll();
};
