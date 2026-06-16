#pragma once
#include "CombatStrategy.hpp"
#include "Drive.hpp"

class NoAuto : public CombatStrategy {
  public:
    void init() override {
        // Não faz nada. Não tem sensores para inicializar.
    }

    void autoEngage(Drive &motores, WeaponSystem &armas) override {
        // Trava de segurança máxima: se por algum milagre o robô
        // cair no estado FIGHTING, ele corta os motores para não enlouquecer.
        motores.setSpeed(0, 0);
    }
};