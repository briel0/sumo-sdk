#include "Receiver.hpp"
#include "Config.hpp"
#include <esp_mac.h>

Receiver *Receiver::instance = nullptr;

Receiver::Receiver() {
    instance = this;
}

bool Receiver::macIsZero(const uint8_t *mac) {
    for(int i = 0; i < 6; i++) {
        if(mac[i] != 0) {
            return false;
        }
    }
    return true;
}

void Receiver::init() {
    // Allowlist em tempo de compilação: o MAC "dono" vem do perfil, não mais da
    // NVS. Perfil com MAC fixo => pareamento determinístico (só aquele controle).
    // Perfil com MAC tudo-zero => modo descoberta (aceita o primeiro e imprime
    // o MAC pra você fixar no perfil).
    memcpy(savedMac, Config::CONTROLLER_MAC, 6);
    discoveryMode = macIsZero(savedMac);

    BP32.setup(&Receiver::onConnected, &Receiver::onDisconnected);

    if(discoveryMode) {
        Serial.println("[Receiver] MODO DESCOBERTA: nenhum MAC fixado no perfil. Vou aceitar o primeiro "
                       "controle e imprimir o MAC dele — copie pra Config::CONTROLLER_MAC.");
    }
    else {
        Serial.printf("[Receiver] MODO SEGURO: só o controle " MACSTR " pode parear.\n", MAC2STR(savedMac));
    }
}

void Receiver::disconnect() {
    if(controller && controller->isConnected()) {
        controller->disconnect();
        controller = nullptr;
        Serial.println("[Receiver] Bluetooth disconnected instantly.");
    }
}

void Receiver::onConnected(ControllerPtr ctl) {
    const uint8_t *incomingMac = ctl->getProperties().btaddr;

    if(instance->discoveryMode) {
        // Primeiro controle da sessão: aceita, trava nele (pro resto do
        // power-cycle) e imprime o MAC bem visível pra ser fixado no perfil.
        // Não persiste em NVS de propósito — "lembrar do último pareado" é
        // justamente o que permitia cross-pairing acidental entre dois robôs.
        Serial.printf("\n[Receiver] >>> CONTROLE DESCOBERTO! Fixe este MAC em Config::CONTROLLER_MAC:\n"
                      "[Receiver] >>> { 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X }\n\n",
                      incomingMac[0], incomingMac[1], incomingMac[2], incomingMac[3], incomingMac[4],
                      incomingMac[5]);

        memcpy(instance->savedMac, incomingMac, 6);
        instance->discoveryMode = false; // trava: um segundo controle não rouba mais a sessão
        instance->controller    = ctl;
    }
    else {
        if(memcmp(incomingMac, instance->savedMac, 6) == 0) {
            Serial.println("[Receiver] ✅ Piloto oficial reconhecido. Conexão liberada.");
            instance->controller = ctl;
        }
        else {
            Serial.printf("[Receiver] ❌ Controle não autorizado bloqueado. MAC: " MACSTR "\n", MAC2STR(incomingMac));
            ctl->disconnect();
        }
    }
}

void Receiver::onDisconnected(ControllerPtr ctl) {
    Serial.println("[Receiver] ❌ Controller Disconnected or signal lost!");
    if(instance) {
        instance->controller = nullptr;
    }
}

void Receiver::update() {
    BP32.update();

    if(controller && controller->isConnected()) {
        updateAxes();
        updateButtons();
    }
    else {
        applyFailsafe();
    }

    /*
    // Opcional: Descomente para debugar os eixos na serial
    static unsigned long lastPrint = 0;
    if(millis() - lastPrint > 2000) {
        Serial.printf("LX: %d | LY: %d | RX: %d | RY: %d | LT: %d | RT: %d\n", leftStickXVal, leftStickYVal,
                      rightStickXVal, rightStickYVal, leftTriggerVal, rightTriggerVal);
        lastPrint = millis();
    }
    */
}

void Receiver::updateAxes() {
    int rawLT = controller->brake();
    int rawRT = controller->throttle();
    int rawLX = controller->axisX();
    int rawLY = controller->axisY();
    int rawRX = controller->axisRX();
    int rawRY = controller->axisRY();

    // Mapeamento idêntico à versão mais recente (compatível com os getters do RCMode)
    leftTriggerVal = (rawLT < TRIGGER_DEADZONE) ? 0 : (rawLT * 100) / 1023;
    rightTriggerVal = (rawRT < TRIGGER_DEADZONE) ? 0 : (rawRT * 100) / 1023;

    leftStickXVal = (abs(rawLX) < STICKER_DEADZONE) ? 0 : constrain((rawLX * 100) / 511, -100, 100);
    leftStickYVal = (abs(rawLY) < STICKER_DEADZONE) ? 0 : constrain((rawLY * 100) / 511, -100, 100);
    rightStickXVal = (abs(rawRX) < STICKER_DEADZONE) ? 0 : constrain((rawRX * 100) / 511, -100, 100);
    rightStickYVal = (abs(rawRY) < STICKER_DEADZONE) ? 0 : constrain((rawRY * 100) / 511, -100, 100);
}

void Receiver::updateButtons() {
    uint8_t currentDpad = controller->dpad();
    dpadUpFlag = (currentDpad & DPAD_UP) && !(lastDpad & DPAD_UP);
    dpadDownFlag = (currentDpad & DPAD_DOWN) && !(lastDpad & DPAD_DOWN);
    dpadLeftFlag = (currentDpad & DPAD_LEFT) && !(lastDpad & DPAD_LEFT);
    dpadRightFlag = (currentDpad & DPAD_RIGHT) && !(lastDpad & DPAD_RIGHT);
    lastDpad = currentDpad;

    uint16_t currentBtns = controller->buttons();

    bool currentCircle = currentBtns & BUTTON_B;
    circleFlag = currentCircle && !lastCircle;
    lastCircle = currentCircle;

    bool currentCross = currentBtns & BUTTON_A;
    crossFlag = currentCross && !lastCross;
    lastCross = currentCross;

    bool currentSquare = currentBtns & BUTTON_X;
    squareFlag = currentSquare && !lastSquare;
    lastSquare = currentSquare;

    bool currentTriangle = currentBtns & BUTTON_Y;
    triangleFlag = currentTriangle && !lastTriangle;
    lastTriangle = currentTriangle;
}

void Receiver::applyFailsafe() {
    leftTriggerVal = 0;
    rightTriggerVal = 0;
    leftStickXVal = 0;
    leftStickYVal = 0;
    rightStickXVal = 0;
    rightStickYVal = 0;

    lastDpad = 0;
    lastCircle = false;
    lastCross = false;
    lastSquare = false;
    lastTriangle = false;

    dpadUpFlag = false;
    dpadDownFlag = false;
    dpadLeftFlag = false;
    dpadRightFlag = false;
    circleFlag = false;
    crossFlag = false;
    squareFlag = false;
    triangleFlag = false;
}