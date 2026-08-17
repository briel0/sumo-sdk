#pragma once
#include "RobotTypes.hpp"
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

struct WebJoystickState {
    int x = 0;
    int y = 0;
    bool btnTriangle = false;
    bool btnSquare = false;
    bool btnCircle = false;
    bool btnCross = false;
};

class ConfigServer {
  public:
    ConfigServer(uint16_t port = 80);
    void begin();
    void shutdown();
    void update();

    void teardownBluetooth();
    bool setupAccessPoint();
    void setupWebRoutes();

    bool consumePayload(AutoStrategy &outStrategy);
    void setMacroTestCallback(std::function<void(MotionSequence)> cb) {
        _macroTestCallback = cb;
    }

    WebJoystickState getJoystickState() const { return currentJoyState; }

  private:
    std::function<void(MotionSequence)> _macroTestCallback = nullptr;
    MotionStep _testSteps[8];

    AsyncWebServer server;
    AsyncWebSocket ws; 

    AutoStrategy currentAutoStrategy;
    WebJoystickState currentJoyState; 
    
    DNSServer dnsServer;

    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
};