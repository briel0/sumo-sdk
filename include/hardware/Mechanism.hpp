#pragma once

/**
    @class Mechanism
    @brief Contrato comum de qualquer atuador de arma. O WeaponSystem só conhece essa interface — quem sabe se
   "armar" é ir pra um ângulo (ServoMechanism) ou girar por um tempo (ContinuousServoMechanism) é cada
   implementação.
*/
class Mechanism {
  public:
    virtual ~Mechanism() = default;

    virtual void init() = 0;
    virtual void deploy() = 0;
    virtual void retract() = 0;

    /**
    @brief Chamado todo frame. Cada mecanismo cuida do próprio tempo aqui (cortar o sinal depois de armar, parar
   de girar etc.).
    */
    virtual void update() = 0;

    /**
    @brief Joga o mecanismo pra um ângulo arbitrário. Só faz sentido em servo de posição — nos outros é no-op.
    */
    virtual void setAngle(int angle) {}
};
