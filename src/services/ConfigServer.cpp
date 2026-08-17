#include "ConfigServer.hpp"
#include "Config.hpp"
#include "RobotTypes.hpp"
#include "WebUI.hpp"
#include "esp_bt.h"
#include "esp_wifi.h" // INJETADO: Necessário para burlar a queda por inatividade

const byte DNS_PORT = 53;
const uint16_t AP_INACTIVITY_TIMEOUT_S = 65535; // INJETADO: Mantém o piloto conectado indefinidamente

ConfigServer::ConfigServer(uint16_t port) : server(port), ws("/ws-joy") {}

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

    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_DATA) {
            AwsFrameInfo *info = (AwsFrameInfo*)arg;
            if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                this->handleWebSocketMessage(arg, data, len);
            }
        }
    });
    server.addHandler(&ws);

    auto serveDashboard = [](AsyncWebServerRequest *request) { request->send(200, "text/html", INDEX_HTML); };

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

    server.on("/test-macro", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if(!_macroTestCallback) {
            request->send(503, "text/plain", "ERRO: Callback não registrado.");
            return;
        }

        // Espera parâmetros: steps=N&l0=100&r0=100&d0=300&l1=...
        if(!request->hasParam("steps")) {
            request->send(400, "text/plain", "ERRO: Faltou steps");
            return;
        }

        int numSteps = constrain(request->getParam("steps")->value().toInt(), 1, 8);

        for(int i = 0; i < numSteps; i++) {
            String li = "l" + String(i);
            String ri = "r" + String(i);
            String di = "d" + String(i);

            _testSteps[i].leftSpeed = request->hasParam(li) ? request->getParam(li)->value().toInt() : 0;
            _testSteps[i].rightSpeed = request->hasParam(ri) ? request->getParam(ri)->value().toInt() : 0;
            _testSteps[i].durationMs = request->hasParam(di) ? request->getParam(di)->value().toInt() : 200;
        }

        MotionSequence seq = {_testSteps, numSteps};
        _macroTestCallback(seq);

        request->send(200, "text/plain", "MACRO DISPARADA");
    });
}

void ConfigServer::update() {
    if(WiFi.getMode() == WIFI_AP) {
        dnsServer.processNextRequest();
        ws.cleanupClients();
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

void ConfigServer::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    String msg((char*)data, len);
    
    if (msg.startsWith("J:")) {
        int commaIndex = msg.indexOf(',');
        if (commaIndex > 0) {
            currentJoyState.x = msg.substring(2, commaIndex).toInt();
            currentJoyState.y = msg.substring(commaIndex + 1).toInt();
        }
    } 
    else if (msg.startsWith("B:")) {
        int commaIndex = msg.indexOf(',');
        if (commaIndex > 0) {
            String btn = msg.substring(2, commaIndex);
            bool state = msg.substring(commaIndex + 1).toInt() > 0;
            
            if (btn == "TRI") currentJoyState.btnTriangle = state;
            else if (btn == "SQR") currentJoyState.btnSquare = state;
            else if (btn == "CIR") currentJoyState.btnCircle = state;
            else if (btn == "X") currentJoyState.btnCross = state;
        }
    }
}