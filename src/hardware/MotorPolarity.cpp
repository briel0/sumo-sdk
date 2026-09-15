#include "MotorPolarity.hpp"

namespace {
    constexpr char NVS_NAMESPACE[] = "motorpol";
    constexpr char NVS_KEY[] = "idx";
}

void MotorPolarity::init() {
    _prefs.begin(NVS_NAMESPACE, false);
    _index = _prefs.getUChar(NVS_KEY, 0) % NUM_CONFIGS;
}

void MotorPolarity::apply(int &leftSpeed, int &rightSpeed) const {
    int l = leftSpeed;
    int r = rightSpeed;

    if(_index & SWAP_SIDES) {
        int tmp = l;
        l = r;
        r = tmp;
    }
    if(_index & INVERT_LEFT) {
        l = -l;
    }
    if(_index & INVERT_RIGHT) {
        r = -r;
    }

    leftSpeed = l;
    rightSpeed = r;
}

uint8_t MotorPolarity::cycle() {
    _index = (_index + 1) % NUM_CONFIGS;
    _prefs.putUChar(NVS_KEY, _index);
    return _index;
}
