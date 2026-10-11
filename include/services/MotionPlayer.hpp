#pragma once
#include "RobotTypes.hpp"

class Drive;

class MotionPlayer {
  public:
    // distancePercent estica/encurta só os passos com scalesWithDistance
    // (100 = tempo original). startDelayMs segura os motores parados antes do
    // primeiro passo — vale até pra macro vazia, que aí só espera.
    void play(const MotionSequence &seq, int distancePercent = 100, unsigned long startDelayMs = 0);
    void update(Drive &motores);

    bool isPlaying() const {
        return active;
    }
    // true só durante a espera inicial pedida no play(), antes do primeiro passo.
    bool isWaiting() const {
        return active && startDelayMs > 0;
    }
    void stop() {
        active = false;
    }

  private:
    MotionSequence sequence = {nullptr, 0};
    int currentStep = 0;
    int distancePercent = 100;
    unsigned long startDelayMs = 0; // 0 = já passou (ou nunca teve) a espera inicial
    unsigned long playStartTime = 0;
    unsigned long stepStartTime = 0;
    bool active = false;
};