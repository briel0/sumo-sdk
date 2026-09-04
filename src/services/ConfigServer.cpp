#include "ConfigServer.hpp"
#include "Config.hpp"
#include "RobotTypes.hpp"
#include "WebUI.hpp"
#include "esp_bt.h"
#include "esp_wifi.h"

const byte DNS_PORT = 53;

// O driver WiFi do ESP32 desautentica qualquer estação parada (sem tráfego)
// por mais que este tempo — o padrão do ESP-IDF é 300s (5 min). Isso derrubava
// o celular do piloto se ele demorasse mais que isso escolhendo a tática no
// mesmo AP, mesmo com o portal ainda aberto na tela. Sobe pro máximo do campo
// (uint16_t) pra, na prática, eliminar esse desligamento por inatividade
// durante a configuração de bancada.
const uint16_t AP_INACTIVITY_TIMEOUT_S = 65535;

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

    // Canal vem do perfil (Config::WIFI_CHANNEL), espalhado entre os canais
    // não-sobrepostos (1/6/11) pra dois robôs em bancada não competirem pelo
    // mesmo espectro durante a seleção de tática.
    WiFi.softAP(Config::ROBOT_NAME, "sumo1234", Config::WIFI_CHANNEL, 0, 4);

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
    // ESPAsyncWebServer mantém as conexões vivas (keep-alive) por padrão. Num
    // captive portal isso é um problema: o DNS wildcard faz todo hostname
    // apontar pro mesmo IP, então os mini-browsers de captive portal (e os
    // vários probes automáticos do SO) abrem várias conexões que nunca fecham
    // sozinhas, esgotando o número (bem baixo) de sockets TCP simultâneos que
    // o LwIP do ESP32 aguenta — depois disso o servidor simplesmente para de
    // responder a clientes novos. Fechar a conexão em toda resposta evita isso.
    DefaultHeaders::Instance().addHeader("Connection", "close");

    // IMPORTANTE — servir via overload (const uint8_t*, len): a variante
    // send(code, type, const char*) COPIA a página inteira pra uma String no
    // heap A CADA request (AsyncBasicResponse::_content). Quando um celular
    // conecta, o SO dispara vários probes de captive portal ao mesmo tempo, e
    // várias cópias de ~24KB simultâneas estouravam o heap -> o AsyncTCP para
    // de aceitar conexão -> o portal cai e reabre. O overload com
    // ponteiro+tamanho usa AsyncProgmemResponse, que faz stream direto da
    // flash, sem cópia.
    auto serveDashboard = [](AsyncWebServerRequest *request) {
        // strlen roda uma vez (static) — flash é mapeada em memória no ESP32,
        // ler direto é válido.
        static const size_t DASHBOARD_LEN = strlen(DASHBOARD_HTML);
        request->send(200, "text/html", (const uint8_t *)DASHBOARD_HTML, DASHBOARD_LEN);
    };

    // A página grande só sai na rota "/" — todo o resto (probes de captive
    // portal, favicon, URLs aleatórias do mini-browser) recebe um redirect 302
    // minúsculo pra "/". Além de ser o padrão de captive portal (é o que
    // dispara o popup no Android/iOS), evita responder a rajada de probes da
    // conexão com N cópias da página ao mesmo tempo.
    server.on("/", HTTP_GET, serveDashboard);

    auto redirectToPortal = [](AsyncWebServerRequest *r) { r->redirect("/"); };
    server.on("/generate_204", HTTP_GET, redirectToPortal);
    server.on("/gen_204", HTTP_GET, redirectToPortal);
    server.on("/fwlink", HTTP_GET, redirectToPortal);
    server.on("/hotspot-detect.html", HTTP_GET, redirectToPortal);
    server.on("/library/test/success.html", HTTP_GET, redirectToPortal);
    server.on("/success.txt", HTTP_GET, redirectToPortal);

    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *r) { r->send(200, "text/plain", "Microsoft NCSI"); });
    server.on("/connecttest.txt", HTTP_GET,
              [](AsyncWebServerRequest *r) { r->send(200, "text/plain", "Microsoft Connect Test"); });

    server.onNotFound(redirectToPortal);

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

    server.on("/api/profile", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", Config::UI_PROFILE_JSON);
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
            // durationMs e unsigned: um "d" negativo viraria ~49 dias de passo,
            // travando os motores ligados ate o reset.
            long dur = request->hasParam(di) ? request->getParam(di)->value().toInt() : 200;
            _testSteps[i].durationMs = (dur < 0) ? 0 : (unsigned long)dur;
        }

        MotionSequence seq = {_testSteps, numSteps};
        _macroTestCallback(seq);

        request->send(200, "text/plain", "MACRO DISPARADA");
    });

    server.on("/sensors", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "application/json", _testReadoutJson);
    });

    server.on("/set-test", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if(request->hasParam("motor")) {
            bool turnOn = request->getParam("motor")->value() == "1";
            if(_motorTestCallback) _motorTestCallback(turnOn);
        }
        if(request->hasParam("sensor")) {
            bool turnOn = request->getParam("sensor")->value() == "1";
            if(_sensorTestCallback) _sensorTestCallback(turnOn);
        }
        request->send(200, "text/plain", "OK");
    });
}

void ConfigServer::update() {
    if(WiFi.getMode() == WIFI_AP) {
        dnsServer.processNextRequest();
    }
}

void ConfigServer::shutdown() {
    // ATENÇÃO — NÃO desligue o rádio WiFi nem chame server.end() aqui.
    //
    // Derrubar o AsyncWebServer/AsyncTCP + WiFi com conexões TCP ainda vivas (o
    // celular do piloto e os probes de captive portal do SO seguem com sockets
    // abertos) provoca use-after-free na task do AsyncTCP (Core 0) -> panic ->
    // reboot, justamente no instante em que a estratégia acabou de chegar.
    //
    // Manter o AP no ar durante a luta é seguro e funcional:
    //  - Nenhum sensor analógico depende do ADC2 (linha e LDR estão todos no
    //    ADC1), então não há o motivo de "liberar o ADC2 desligando o WiFi".
    //  - O combate roda no Core 1; o WiFi/LwIP vive no Core 0. Não se bloqueiam.
    //  - A interferência de espectro entre dois robôs já é suavizada por
    //    Config::WIFI_CHANNEL (canais diferentes por perfil).
    //
    // Só paramos o DNS do captive portal — é um servidor UDP à parte (WiFiUDP),
    // sem relação com o AsyncTCP, então parar é seguro e evita o portal ficar
    // reabrindo à toa no celular depois que a config já foi recebida.
    Serial.println("[WIFI] Config recebida. Parando DNS (o AP segue no ar, de propósito).");
    dnsServer.stop();
}

bool ConfigServer::consumePayload(AutoStrategy &outStrategy) {
    if(!currentAutoStrategy.isNew) {
        return false;
    }
    outStrategy = currentAutoStrategy;
    currentAutoStrategy.isNew = false;
    return true;
}