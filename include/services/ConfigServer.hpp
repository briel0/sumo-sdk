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

    bool consumePayload(AutoStrategy &outStrategy);

  private:
    AsyncWebServer server;
    AutoStrategy currentAutoStrategy;
    DNSServer dnsServer;
};