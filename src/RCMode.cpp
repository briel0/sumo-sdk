#include "RCMode.hpp"
#include "Config.hpp"

void RCMode::init() {
    receptor.init();
}

void RCMode::run(Drive &motores) {
    receptor.update();

    int throttle = receptor.getThrottle();
    int steer = receptor.getSteer();

    throttle = (throttle * MAX_THROTTLE) / 100;
    if(throttle == 0) {
        steer = (steer * PIVOT_COEFFICIENT) / 100;
    }
    else {
        steer = (steer * TURN_COEFFICIENT) / 100;
    }

    int leftSpeed = throttle + steer;
    int rightSpeed = throttle - steer;

    motores.setSpeed(leftSpeed, rightSpeed);
}