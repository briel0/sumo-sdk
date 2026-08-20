#pragma once
#include "RobotTypes.hpp"
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

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
    
    void setSensorReadCallback(std::function<String()> cb) {
        _sensorReadCallback = cb;
    }

  private:
    std::function<void(MotionSequence)> _macroTestCallback = nullptr;
    std::function<String()> _sensorReadCallback = nullptr;
    MotionStep _testSteps[8];

    AsyncWebServer server;
    AutoStrategy currentAutoStrategy;
    DNSServer dnsServer;
};