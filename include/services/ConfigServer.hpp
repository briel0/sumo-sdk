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
    
    void setTestReadout(const String &json) {
        _testReadoutJson = json;
    }
    
    void setMotorTestCallback(std::function<void(bool)> cb) {
        _motorTestCallback = cb;
    }

    void setSensorTestCallback(std::function<void(bool)> cb) {
        _sensorTestCallback = cb;
    }

  private:
    std::function<void(MotionSequence)> _macroTestCallback = nullptr;
    std::function<void(bool)> _motorTestCallback = nullptr;
    std::function<void(bool)> _sensorTestCallback = nullptr;
    String _testReadoutJson = "{}";
    MotionStep _testSteps[8];

    AsyncWebServer server;
    AutoStrategy currentAutoStrategy;
    DNSServer dnsServer;
};