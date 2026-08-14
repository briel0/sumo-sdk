#pragma once
#include <Arduino.h>

class JS40F {
  public:
    // activeLow inverte a leitura por software: alguns receptores IR passivos
    // frontais operam em active-low (0 = detectado, 1 = vazio). Mantendo o
    // padrão antigo, o default é active-high para não quebrar os perfis atuais.
    explicit JS40F(uint8_t pin, bool activeLow = false);

    void init() const;

    bool temAlvo() const;

  private:
    uint8_t _pin;
    bool    _activeLow;
};
