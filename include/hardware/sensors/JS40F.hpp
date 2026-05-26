#pragma once
#include <Arduino.h>

class JS40F {
  public:
    explicit JS40F(uint8_t pin);

    void init() const;

    bool temAlvo() const;

  private:
    uint8_t _pin;
};