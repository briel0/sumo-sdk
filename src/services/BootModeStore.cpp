#include "BootModeStore.hpp"
#include <Arduino.h>
#include <Preferences.h>

namespace {
    constexpr const char *NVS_NAMESPACE = "bootcfg";
    constexpr const char *KEY_MODE = "mode";   // RobotState escolhido por IR
    constexpr const char *KEY_FWDEF = "fwdef"; // default compilado na hora da gravação
    constexpr uint8_t SEM_VALOR = 0xFF;

    bool valorValido(uint8_t bruto) {
        return bruto <= static_cast<uint8_t>(RobotState::AUTO);
    }
}

RobotState BootModeStore::load(RobotState compiledDefault) {
    Preferences prefs;
    if(!prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[BOOT] NVS indisponível. Usando o modo compilado.");
        return compiledDefault;
    }

    uint8_t gravado = prefs.getUChar(KEY_MODE, SEM_VALOR);
    uint8_t defaultNaHora = prefs.getUChar(KEY_FWDEF, SEM_VALOR);

    if(!valorValido(gravado)) {
        prefs.end();
        return compiledDefault;
    }

    // O firmware mudou de default desde a gravação: quem acabou de subir código
    // novo quer o modo do código, não uma senha esquecida de semanas atrás.
    if(defaultNaHora != static_cast<uint8_t>(compiledDefault)) {
        prefs.remove(KEY_MODE);
        prefs.remove(KEY_FWDEF);
        prefs.end();
        Serial.printf("[BOOT] Default do firmware mudou para %s. Senha IR anterior descartada.\n",
                      name(compiledDefault));
        return compiledDefault;
    }

    prefs.end();

    RobotState modo = static_cast<RobotState>(gravado);
    Serial.printf("[BOOT] Modo gravado por IR: %s.\n", name(modo));
    return modo;
}

void BootModeStore::save(RobotState mode, RobotState compiledDefault) {
    Preferences prefs;
    if(!prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[BOOT] ERRO: NVS indisponível, modo NÃO gravado.");
        return;
    }
    prefs.putUChar(KEY_MODE, static_cast<uint8_t>(mode));
    prefs.putUChar(KEY_FWDEF, static_cast<uint8_t>(compiledDefault));
    prefs.end();
}

void BootModeStore::clear() {
    Preferences prefs;
    if(!prefs.begin(NVS_NAMESPACE, false)) {
        return;
    }
    prefs.remove(KEY_MODE);
    prefs.remove(KEY_FWDEF);
    prefs.end();
}

const char *BootModeStore::name(RobotState mode) {
    switch(mode) {
        case RobotState::IDLE:
            return "IDLE";
        case RobotState::RC:
            return "RC";
        case RobotState::AUTO:
            return "AUTO";
    }
    return "?";
}
