#include "ConfigServer.hpp"
#include "Config.hpp"
#include "RobotTypes.hpp"
#include "WebUI.hpp"
#include "esp_bt.h"

const byte DNS_PORT = 53;

ConfigServer::ConfigServer(uint16_t port) : server(port) {}

void ConfigServer::begin() {
    btStop();

    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    esp_bt_mem_release(ESP_BT_MODE_BTDM);

    delay(100);

    WiFi.mode(WIFI_AP);

    IPAddress ip(4, 3, 2, 1);
    IPAddress netmask(255, 255, 255, 0);
    WiFi.softAPConfig(ip, ip, netmask);

    WiFi.softAP(Config::ROBOT_NAME, "sumo1234", 1, 0, 4);

    Serial.printf("[WIFI] IP: %s\n", WiFi.softAPIP().toString().c_str());

    if(WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {
        Serial.println("[WIFI] ERRO: softAP falhou!");
        return;
    }

    dnsServer.start(DNS_PORT, "*", ip);

    // 1. DRY: Criamos um handler reutilizável para servir o painel HTML
    auto serveDashboard = [](AsyncWebServerRequest *request) { request->send(200, "text/html", DASHBOARD_HTML); };

    // 2. Aplicamos o handler nas rotas padrão e de Captive Portal de forma limpa
    server.on("/", HTTP_GET, serveDashboard);
    server.on("/generate_204", HTTP_GET, serveDashboard);
    server.on("/fwlink", HTTP_GET, serveDashboard);

    server.on("/hotspot-detect.html", HTTP_GET, serveDashboard);
    server.on("/library/test/success.html", HTTP_GET, serveDashboard);
    server.on("/success.txt", HTTP_GET, serveDashboard);

    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *r) { r->send(200, "text/plain", "Microsoft NCSI"); });
    server.on("/connecttest.txt", HTTP_GET,
              [](AsyncWebServerRequest *r) { r->send(200, "text/plain", "Microsoft Connect Test"); });

    server.onNotFound(serveDashboard);

    // 3. A Rota de configuração isolada e com quebras de linha respeitadas
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

    server.begin();
    Serial.println("[WIFI] Servidores HTTP e DNS ativos.");
}

void ConfigServer::update() {
    if(WiFi.getMode() == WIFI_AP) {
        dnsServer.processNextRequest();
    }
}

void ConfigServer::shutdown() {
    Serial.println("[WIFI] Desligando rede por comando do AutoMode...");
    dnsServer.stop();
    server.end();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[WIFI] Rádio 2.4GHz totalmente desligado.");
}

bool ConfigServer::consumePayload(AutoStrategy &outStrategy) {
    if(!currentAutoStrategy.isNew) {
        return false;
    }
    outStrategy = currentAutoStrategy;
    currentAutoStrategy.isNew = false;
    return true;
}