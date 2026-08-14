#pragma once
#include "RobotTypes.hpp"

class Drive;

class MotionPlayer {
  public:
    void play(const MotionSequence &seq);
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
    unsigned long stepStartTime = 0;
    bool active = false;
};