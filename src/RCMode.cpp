#include "RCMode.hpp"

void RCMode::init() {
    receptor.init();
}

void RCMode::run(Drive &motores) {
    receptor.update();

    int throttle = receptor.getThrottle();
    int steer = receptor.getSteer();

    int leftSpeed = throttle + steer;
    int rightSpeed = throttle - steer;

    motores.setSpeed(leftSpeed, rightSpeed);
}