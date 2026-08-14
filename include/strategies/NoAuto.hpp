#pragma once
#include "CombatStrategy.hpp"
#include "Drive.hpp"

class NoAuto : public CombatStrategy {
  public:
    void init(HardwareCore &hardware) override {
        // Não faz nada. Não tem sensores nem periféricos para inicializar.
        (void)hardware;
    }

    void autoEngage(Drive &motores, WeaponSystem &armas, HardwareCore &hardware) override {
        // Trava de segurança máxima: se por algum milagre o robô
        // cair no estado FIGHTING, ele corta os motores para não enlouquecer.
        (void)armas;
        (void)hardware;
        motores.setSpeed(0, 0);
    }
};
