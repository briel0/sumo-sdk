#pragma once
#include "RobotTypes.hpp"

class Drive;

class MotionPlayer {
  public:
    // distancePercent estica/encurta só os passos com scalesWithDistance
    // (100 = tempo original).
    void play(const MotionSequence &seq, int distancePercent = 100);
    void update(Drive &motores);

    bool isPlaying() const {
        return active;
    }
    void stop() {
        active = false;
    }

  private:
    MotionSequence sequence = {nullptr, 0};
    int currentStep = 0;
    int distancePercent = 100;
    unsigned long stepStartTime = 0;
    bool active = false;
};