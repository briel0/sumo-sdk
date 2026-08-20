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

  private:
    std::function<void(MotionSequence)> _macroTestCallback = nullptr;
    String _testReadoutJson = "{}";
    MotionStep _testSteps[8];

    AsyncWebServer server;
    AutoStrategy currentAutoStrategy;
    DNSServer dnsServer;
};