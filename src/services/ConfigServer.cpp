#include "ConfigServer.hpp"
#include "Config.hpp"
#include "RobotTypes.hpp"
#include "WebUI.hpp"
#include "esp_bt.h"
#include "esp_wifi.h" // INJETADO: Necessário para burlar a queda por inatividade

const byte DNS_PORT = 53;
const uint16_t AP_INACTIVITY_TIMEOUT_S = 65535; // INJETADO: Mantém o piloto conectado indefinidamente

ConfigServer::ConfigServer(uint16_t port) : server(port) {}

void ConfigServer::begin() {
    teardownBluetooth();

    if(!setupAccessPoint()) {
        return;
    }

    setupWebRoutes();

    server.begin();
    Serial.println("[WIFI] Servidores HTTP e DNS ativos.");
}

void ConfigServer::teardownBluetooth() {
    btStop();
    delay(100);
}

bool ConfigServer::setupAccessPoint() {
    WiFi.mode(WIFI_AP);

    IPAddress ip(4, 3, 2, 1);
    IPAddress netmask(255, 255, 255, 0);
    WiFi.softAPConfig(ip, ip, netmask);

    WiFi.softAP(Config::ROBOT_NAME, "sumo1234", 1, 0, 4);

    // INJETADO: Desativa o corte de conexão por inatividade do ESP32
    esp_wifi_set_inactive_time(WIFI_IF_AP, AP_INACTIVITY_TIMEOUT_S);

    Serial.printf("[WIFI] IP: %s\n", WiFi.softAPIP().toString().c_str());

    if(WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {
        Serial.println("[WIFI] ERRO: softAP falhou!");
        return false;
    }

    dnsServer.start(DNS_PORT, "*", ip);
    return true;
}

void ConfigServer::setupWebRoutes() {
    // INJETADO: Fecha conexões imediatamente para impedir o esgotamento de sockets do ESP32
    DefaultHeaders::Instance().addHeader("Connection", "close");

    auto serveDashboard = [](AsyncWebServerRequest *request) { request->send(200, "text/html", DASHBOARD_HTML); };

    server.on("/", HTTP_GET, serveDashboard);

    // INJETADO: Força o redirecionamento (302) em vez de 204 para forçar a abertura agressiva do Captive Portal
    auto redirectToPortal = [](AsyncWebServerRequest *r) { r->redirect("/"); };
    server.on("/generate_204", HTTP_GET, redirectToPortal);
    server.on("/gen_204", HTTP_GET, redirectToPortal);

    server.on("/fwlink", HTTP_GET, serveDashboard);
    server.on("/hotspot-detect.html", HTTP_GET, serveDashboard);
    server.on("/library/test/success.html", HTTP_GET, serveDashboard);
    server.on("/success.txt", HTTP_GET, serveDashboard);

    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *r) { r->send(200, "text/plain", "Microsoft NCSI"); });
    server.on("/connecttest.txt", HTTP_GET,
              [](AsyncWebServerRequest *r) { r->send(200, "text/plain", "Microsoft Connect Test"); });

    server.onNotFound(serveDashboard);

    server.on("/set-strat", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if(!request->hasParam("macro")) {
            request->send(400, "text/plain", "ERRO FATAL: Faltou Macro Inicial");
            return;
        }

        currentAutoStrategy.macro = request->getParam("macro")->value().toInt();

        if(request->hasParam("direction")) {
            currentAutoStrategy.direction = request->getParam("direction")->value().charAt(0);
        }

        if(request->hasParam("search")) {
            currentAutoStrategy.search = request->getParam("search")->value().toInt();
        }

        if(request->hasParam("weapon")) {
            currentAutoStrategy.weapon = request->getParam("weapon")->value().toInt();
        }

        currentAutoStrategy.isNew = true;

        Serial.printf("[WIFI] PACOTE TÁTICO RECEBIDO -> MACRO: %d | DIR: %c | BUSCA: %d | ARMA: %d\n",
                      currentAutoStrategy.macro, currentAutoStrategy.direction, currentAutoStrategy.search,
                      currentAutoStrategy.weapon);

        request->send(200, "text/plain", "CONFIGURADO");
    });
}

void ConfigServer::update() {
    if(WiFi.getMode() == WIFI_AP) {
        dnsServer.processNextRequest();
    }
}

void ConfigServer::shutdown() {
    Serial.println("[WIFI] Encerrando conexões com o piloto...");
    // Futuramente vamos utilizar essa função pra recuperar o core 0
}

bool ConfigServer::consumePayload(AutoStrategy &outStrategy) {
    if(!currentAutoStrategy.isNew) {
        return false;
    }
    outStrategy = currentAutoStrategy;
    currentAutoStrategy.isNew = false;
    return true;
}