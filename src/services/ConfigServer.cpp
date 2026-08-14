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

    // A escolha de qual HUD servir é decidida em tempo de compilação (Config::),
    // igual ao resto do projeto — este arquivo não sabe qual robô está compilando,
    // só lê a flag. Ver Config::USES_FUMACINHA_FSM em cada perfil.
    //
    // IMPORTANTE — servir via overload (const uint8_t*, len): a variante
    // send(code, type, const char*) COPIA a página inteira pra uma String no
    // heap A CADA request (AsyncBasicResponse::_content). Quando um celular
    // conecta, o SO dispara vários probes de captive portal ao mesmo tempo, e
    // várias cópias de ~35KB simultâneas estouravam o heap -> panic -> reboot
    // ("o robô reinicia quando conecto"). O overload com ponteiro+tamanho usa
    // AsyncProgmemResponse, que faz stream direto da flash, sem cópia.
    auto serveDashboard = [](AsyncWebServerRequest *request) {
        // strlen roda uma vez por página (static) — flash é mapeada em memória
        // no ESP32, ler direto é válido.
        static const size_t FUMACINHA_LEN = strlen(FUMACINHA_DASHBOARD_HTML);
        static const size_t LEGACY_LEN    = strlen(DASHBOARD_HTML);
        if(Config::USES_FUMACINHA_FSM) {
            request->send(200, "text/html", (const uint8_t *)FUMACINHA_DASHBOARD_HTML, FUMACINHA_LEN);
        }
        else {
            request->send(200, "text/html", (const uint8_t *)DASHBOARD_HTML, LEGACY_LEN);
        }
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
        // As duas HUDs (legado e Fumacinha) nunca são compiladas para o mesmo
        // robô, mas a rota é a mesma para as duas — cada bloco só roda se os
        // parâmetros da respectiva HUD estiverem presentes no request.
        bool hasLegacyPayload = request->hasParam("macro");
        bool hasCombatProfilePayload = request->hasParam("openingTactic");

        if(!hasLegacyPayload && !hasCombatProfilePayload) {
            request->send(400, "text/plain", "ERRO FATAL: Faltou a Movimentação Inicial");
            return;
        }

        // --- HUD legada: AutoStrategy (macro/direction/search/weapon) --------------
        if(hasLegacyPayload) {
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

            Serial.printf("[WIFI] PACOTE TÁTICO (legado) -> MACRO: %d | DIR: %c | BUSCA: %d | ARMA: %d\n",
                          currentAutoStrategy.macro, currentAutoStrategy.direction, currentAutoStrategy.search,
                          currentAutoStrategy.weapon);
        }

        // --- HUD Fumacinha: CombatProfile (openingTactic/searchTactic/attackTactic) -
        if(hasCombatProfilePayload) {
            int opening = constrain(request->getParam("openingTactic")->value().toInt(), 0, 2);
            currentCombatProfile.openingTactic = static_cast<OpeningTactic>(opening);

            if(request->hasParam("searchTactic")) {
                int search = constrain(request->getParam("searchTactic")->value().toInt(), 0, 2);
                currentCombatProfile.searchTactic = static_cast<SearchTactic>(search);
            }

            if(request->hasParam("attackTactic")) {
                int attack = constrain(request->getParam("attackTactic")->value().toInt(), 0, 1);
                currentCombatProfile.attackTactic = static_cast<AttackTactic>(attack);
            }

            if(request->hasParam("direction")) {
                int direction = constrain(request->getParam("direction")->value().toInt(), 0, 1);
                currentCombatProfile.preferredSide = (direction == 0) ? Direction::left : Direction::right;
            }

            // Emissor por fase (1 = ligado/JSumo, 0 = furtivo/IR puro). OPENING
            // não é enviado — é sempre furtivo na FSM.
            if(request->hasParam("searchEmitters")) {
                currentCombatProfile.searchEmitters = request->getParam("searchEmitters")->value().toInt() != 0;
            }

            if(request->hasParam("attackEmitters")) {
                currentCombatProfile.attackEmitters = request->getParam("attackEmitters")->value().toInt() != 0;
            }

            if(request->hasParam("weapon")) {
                currentCombatProfile.weapon = request->getParam("weapon")->value().toInt();
            }

            // --- Ajustes finos (CombatTuning): velocidades e tempos de cada passo,
            // todos opcionais. Ausentes mantêm o valor atual (ex.: primeiro envio
            // usa os defaults do struct). Nome do parâmetro = nome do campo.
            auto tuneInt = [request](const char *name, long current) -> long {
                return request->hasParam(name) ? request->getParam(name)->value().toInt() : current;
            };
            CombatTuning &tn = currentCombatProfile.tuning;
            // PWM: -100..100 (o Drive já satura, mas mantemos são); tempos: 0..60000 ms.
            tn.evasionDurationMs   = constrain(tuneInt("evasionDurationMs", tn.evasionDurationMs), 0, 60000);
            tn.evasionPwmOuter     = constrain(tuneInt("evasionPwmOuter", tn.evasionPwmOuter), -100, 100);
            tn.evasionPwmInner     = constrain(tuneInt("evasionPwmInner", tn.evasionPwmInner), -100, 100);
            tn.edgeAdvancePwmOuter = constrain(tuneInt("edgeAdvancePwmOuter", tn.edgeAdvancePwmOuter), -100, 100);
            tn.edgeAdvancePwmInner = constrain(tuneInt("edgeAdvancePwmInner", tn.edgeAdvancePwmInner), -100, 100);
            tn.spinDurationMs      = constrain(tuneInt("spinDurationMs", tn.spinDurationMs), 0, 60000);
            tn.spinPwm             = constrain(tuneInt("spinPwm", tn.spinPwm), -100, 100);
            tn.centerRushMs        = constrain(tuneInt("centerRushMs", tn.centerRushMs), 0, 60000);
            tn.centerRushPwm       = constrain(tuneInt("centerRushPwm", tn.centerRushPwm), -100, 100);
            tn.sweepHalfPeriodMs   = constrain(tuneInt("sweepHalfPeriodMs", tn.sweepHalfPeriodMs), 0, 60000);
            tn.sweepPwmOuter       = constrain(tuneInt("sweepPwmOuter", tn.sweepPwmOuter), -100, 100);
            tn.sweepPwmInner       = constrain(tuneInt("sweepPwmInner", tn.sweepPwmInner), -100, 100);
            tn.fishingIntervalMs   = constrain(tuneInt("fishingIntervalMs", tn.fishingIntervalMs), 0, 60000);
            tn.zombieNavPwm        = constrain(tuneInt("zombieNavPwm", tn.zombieNavPwm), -100, 100);
            tn.alignmentPwmMax     = constrain(tuneInt("alignmentPwmMax", tn.alignmentPwmMax), -100, 100);
            tn.alignmentPwmCorrected = constrain(tuneInt("alignmentPwmCorrected", tn.alignmentPwmCorrected), -100, 100);
            tn.steamrollerPwm      = constrain(tuneInt("steamrollerPwm", tn.steamrollerPwm), -100, 100);

            currentCombatProfile.isNew = true;

            Serial.printf("[WIFI] PACOTE TÁTICO (Fumacinha) -> OPENING: %d | SEARCH: %d | ATTACK: %d | LADO: %s | "
                          "EMISSOR B/A: %d/%d\n",
                          (int)currentCombatProfile.openingTactic, (int)currentCombatProfile.searchTactic,
                          (int)currentCombatProfile.attackTactic,
                          currentCombatProfile.preferredSide == Direction::left ? "ESQ" : "DIR",
                          currentCombatProfile.searchEmitters, currentCombatProfile.attackEmitters);
        }

        request->send(200, "text/plain", "CONFIGURADO");
    });

    // Toggles de diagnóstico de bancada (HUD Fumacinha) — ao vivo, sem o fluxo
    // de confirmação/transmissão das táticas. Mutuamente exclusivos.
    server.on("/set-test", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if(request->hasParam("sensorTest")) {
            bool on = request->getParam("sensorTest")->value().toInt() != 0;
            sensorTestActive = on;
            if(on) {
                motorTestActive = false;
                servoTestActive = false;
            }
        }

        if(request->hasParam("motorTest")) {
            bool on = request->getParam("motorTest")->value().toInt() != 0;
            motorTestActive = on;
            if(on) {
                sensorTestActive = false;
                servoTestActive = false;
            }
        }

        if(request->hasParam("servoTest")) {
            bool on = request->getParam("servoTest")->value().toInt() != 0;
            servoTestActive = on;
            if(on) {
                sensorTestActive = false;
                motorTestActive  = false;
            }
        }

        if(!sensorTestActive && !motorTestActive && !servoTestActive) {
            testReadoutJson = "{}"; // nenhum teste ativo: não há leitura pra mostrar
        }

        Serial.printf("[WIFI] TESTE DE BANCADA -> SENSOR: %d | MOTOR: %d | SERVO: %d\n", sensorTestActive,
                      motorTestActive, servoTestActive);

        request->send(200, "text/plain", "OK");
    });

    // Log ao vivo (celular) das leituras de sensores/motores durante um teste
    // de bancada. Só tem dado relevante enquanto isSensorTestActive() ou
    // isMotorTestActive() é true — ver setTestReadout(), chamado a cada frame
    // pelo dono do HardwareCore/Drive (FumacinhaMode).
    server.on("/get-test-data", HTTP_GET,
              [this](AsyncWebServerRequest *request) { request->send(200, "application/json", testReadoutJson); });

    // Testador de macro ao vivo. Params: steps=N&l0=..&r0=..&d0=..&l1=.. (l=vel
    // esquerda, r=vel direita, d=tempo ms). Só guarda os passos + marca pendente
    // (padrão polled) — quem toca é o FumacinhaMode, fora da task async.
    server.on("/test-macro", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if(!request->hasParam("steps")) {
            request->send(400, "text/plain", "ERRO: faltou steps");
            return;
        }

        int numSteps = constrain(request->getParam("steps")->value().toInt(), 1, MAX_MACRO_STEPS);
        for(int i = 0; i < numSteps; i++) {
            String li = "l" + String(i);
            String ri = "r" + String(i);
            String di = "d" + String(i);

            testMacroSteps[i].leftSpeed  = request->hasParam(li) ? request->getParam(li)->value().toInt() : 0;
            testMacroSteps[i].rightSpeed = request->hasParam(ri) ? request->getParam(ri)->value().toInt() : 0;
            long dur = request->hasParam(di) ? request->getParam(di)->value().toInt() : 200;
            testMacroSteps[i].durationMs = (dur < 0) ? 0 : (unsigned long)dur;
        }

        testMacroCount   = numSteps;
        testMacroPending = true;

        Serial.printf("[WIFI] MACRO DE TESTE recebida -> %d passos.\n", numSteps);
        request->send(200, "text/plain", "MACRO DISPARADA");
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
    // reboot. Era exatamente isso que reiniciava o robô logo depois de enviar a
    // estratégia. Uma versão anterior fazia server.end()/softAPdisconnect/
    // WiFi.mode(WIFI_OFF) aqui — foi revertida por causa disso.
    //
    // Manter o AP no ar durante a luta é seguro e funcional:
    //  - Nenhum sensor analógico depende mais do ADC2 (todos foram pra ADC1), então
    //    não há mais o motivo de "liberar o ADC2 desligando o WiFi".
    //  - O combate roda no Core 1; o WiFi/LwIP vive no Core 0. Não se bloqueiam.
    //  - A interferência de espectro entre dois robôs já é suavizada por
    //    Config::WIFI_CHANNEL (canais diferentes por perfil).
    //
    // Só paramos o DNS do captive portal — é um servidor UDP à parte (WiFiUDP),
    // sem relação com o AsyncTCP, então parar é seguro e evita o portal ficar
    // redirecionando à toa depois que a config já foi recebida.
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

bool ConfigServer::consumeCombatProfile(CombatProfile &outProfile) {
    if(!currentCombatProfile.isNew) {
        return false;
    }
    outProfile = currentCombatProfile;
    currentCombatProfile.isNew = false;
    return true;
}

bool ConfigServer::consumeMacroTest(MotionStep *dst, int dstCap, int &outCount) {
    if(!testMacroPending) {
        outCount = 0;
        return false;
    }
    int n = testMacroCount < dstCap ? testMacroCount : dstCap;
    for(int i = 0; i < n; i++) {
        dst[i] = testMacroSteps[i];
    }
    outCount         = n;
    testMacroPending = false;
    return true;
}

void ConfigServer::clearTests() {
    sensorTestActive = false;
    motorTestActive  = false;
    servoTestActive  = false;
    testReadoutJson  = "{}";
}