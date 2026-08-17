#pragma once
#include "ConfigServer.hpp" // Precisamos dessa classe para conhecer o WebJoystickState

class Drive;
class WeaponSystem;

class WebRCMode {
  public:
    // Recebemos os motores, as armas e o estado atual do joystick que chegou via WiFi
    void run(Drive &motores, WeaponSystem &armas, const WebJoystickState &joyState);

  private:
    bool _autoDisarmLocked = false;
    bool _lastCircle = false;
    void handleWeapons(WeaponSystem &armas, const WebJoystickState &joyState);
};
