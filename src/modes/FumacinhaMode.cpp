#include "FumacinhaMode.hpp"
#include "HardwareCore.hpp"
#include <Arduino.h>

void FumacinhaMode::init(HardwareCore &hardware) {
    Serial.println("[FUMACINHA] Modo Auto (Fumacinha_FSM) iniciado.");
    subState = SubState::SELECTING_ESTRATEGIA;
    combatProfile = CombatProfile();
    _benchTested = false;
    _hardware = &hardware;

    // HardwareCore é dependência obrigatória da Fumacinha_FSM (sensores +
    // periféricos de estado) — diferente do AutoMode legado, aqui não existe
    // cenário sem ele, então inicializa incondicionalmente, sem gate de
    // Config::USES_HARDWARE_CORE.
    _hardware->begin();
    configServer.begin();
}

void FumacinhaMode::run(Drive &motores, WeaponSystem &armas, bool irStart, bool irReady, StatusLed &statusLed) {
    armas.update();

    if(_hardware && _hardware->isInitialized()) {
        _hardware->update();
    }

    // FIGHTING assume o frame inteiro — sem overlay de diagnóstico durante a luta
    if(subState == SubState::FIGHTING) {
        _executeCombat(motores);
        return;
    }

    // Teste de macro ao vivo
    int macroCount = 0;
    if(configServer.consumeMacroTest(_macroSteps, 8, macroCount) && macroCount > 0) {
        configServer.clearTests();
        _macroPlayer.play({_macroSteps, macroCount});
    }
    if(_macroPlayer.isPlaying()) {
        _macroPlayer.update(motores);
        configServer.update();
        return;
    }

    // Feedback de LED fora de combate — idêntico ao original
    if(configServer.isSensorTestActive()) {
        runSensorTestCycle(motores, statusLed);
        _benchTested = true;
    }
    else if(configServer.isMotorTestActive()) {
        runMotorTestCycle(motores, statusLed);
        _benchTested = true;
    }
    else if(configServer.isServoTestActive()) {
        runServoTestCycle(statusLed);
        _benchTested = true;
    }
    else if(subState == SubState::READY) {
        if(irReady) {
            statusLed.blinkDebug(1, 20);
            statusLed.setAll(CRGB::Red);
        }
        if(!_readyReceived) {
            statusLed.setState(CRGB::Orange);
        }
        if(irStart) {
            statusLed.setState(CRGB::Black);
        }
    }
    else if(_benchTested) {
        statusLed.setState(CRGB::Green);
    }
    else {
        statusLed.strategyWave();
    }

    // FSM principal — idêntica ao original
    switch(subState) {
        case SubState::SELECTING_ESTRATEGIA:
            configServer.update();
            if(configServer.consumeCombatProfile(combatProfile)) {
                _tempoDesligamento = millis();
                subState = SubState::DISCONNECTING_WIFI;
                Serial.println("[FUMACINHA] Perfil tático recebido. Aguardando buffer HTTP...");
            }
            break;
        case SubState::DISCONNECTING_WIFI:
            if(millis() - _tempoDesligamento > 500) {
                configServer.shutdown();
                configServer.clearTests();
                subState = SubState::READY;
                Serial.println("[FUMACINHA] Rádio morto. Aguardando largada IR.");
            }
            break;
        case SubState::READY:
            if(irReady) {
                _readyReceived = true;
            }
            if(irStart) {
                _readyReceived = false;
                subState = SubState::FIGHTING;
                _hardware->setWing(combatProfile.preferredSide == Direction::left ? WingPosition::LEFT
                                                                                  : WingPosition::RIGHT);
                Serial.println("[FUMACINHA] LARGADA! Asa deployada, combate iniciado.");
            }
            break;
        case SubState::FIGHTING:
            break;
    }
}

// Define a sequência inline com a macro MACRO()
static const MotionSequence RECUO_BORDA_DIREITA = MACRO(
    {-100, -100, 120}, // recua 75ms
    { 100, -100, 120}  // gira 83ms
);

static const MotionSequence RECUO_BORDA_ESQUERDA = MACRO(
    {-100, -100, 120}, // recua 75ms
    { -100, 100, 120}  // gira 83ms
);
// No _executeCombat, em vez de setSpeed() + delay():

void FumacinhaMode::_executeCombat(Drive &motores) {
    if(!_hardware)
        return;

    if(_macroPlayer.isPlaying()) {
        _macroPlayer.update(motores);
        return;
    }

    bool linhaEsq = !_hardware->lineLeft();
    bool linhaDir = !_hardware->lineRight();
    bool jsumoEsq = _hardware->jsumoLeftRaw();
    bool jsumoDir = _hardware->jsumoRightRaw();
    bool jsumoFrente = _hardware->frontDetected();

    if(linhaEsq || linhaDir) {
        if(linhaDir) {
            _macroPlayer.play(RECUO_BORDA_DIREITA);
        }
        else {
            _macroPlayer.play(RECUO_BORDA_ESQUERDA);
        }
        _macroPlayer.update(motores);
        return;
    }

    motores.setSpeed(55, 55);

    /*
    // Prioridade 3: inimigo lateral — centraliza devagar
    if(jsumoEsq && !jsumoDir) {
        _hardware->setWing(WingPosition::LEFT);
        motores.setSpeed(-25, 25); // giro lento para esquerda
        return;
    }
    if(jsumoDir && !jsumoEsq) {
        _hardware->setWing(WingPosition::RIGHT);
        motores.setSpeed(25, -25); // giro lento para direita
        return;
    }

    // Prioridade 4: cegueira total — busca lenta
    _hardware->setWing(combatProfile.preferredSide == Direction::left ? WingPosition::LEFT : WingPosition::RIGHT);
    motores.setSpeed(25, 25); // avança devagar
    */
}

void FumacinhaMode::runSensorTestCycle(Drive &motores, StatusLed &statusLed) {
    if(!_hardware) {
        return;
    }

    // Trava de segurança: os motores NUNCA se movem durante o teste de sensores.
    motores.setSpeed(0, 0);

    // LEDs 2/3 mostram os IRs puros (irLeftRaw/irRightRaw), não os JSumos: o IR
    // puro fica sempre energizado, então é a leitura crua mais confiável pra
    // conferir na bancada (o JSumo depende do emissor furtivo estar ligado).
    DiagnosticsPanel panel(statusLed);
    panel.showSensorReadings(_hardware->opponentOnRamp(), _hardware->lineLeft(), _hardware->irLeftRaw(),
                             _hardware->irRightRaw(), _hardware->lineRight());

    // Log ao vivo pro celular: leitura exata (booleano já filtrado + valor cru
    // de ADC quando aplicável) de cada sensor, não só o resumo dos 5 LEDs.
    String json = "{";
    json += "\"rampaLdr\":";
    json += (_hardware->opponentOnRamp() ? "true" : "false");
    json += ",\"rampaLdrRaw\":" + String(_hardware->ldrFiltered());
    json += ",\"linhaEsq\":";
    json += (_hardware->lineLeft() ? "true" : "false");
    json += ",\"linhaEsqRaw\":" + String(_hardware->lineLeftRaw());
    json += ",\"linhaDir\":";
    json += (_hardware->lineRight() ? "true" : "false");
    json += ",\"linhaDirRaw\":" + String(_hardware->lineRightRaw());
    json += ",\"jsumoEsq\":";
    json += (_hardware->jsumoLeftRaw() ? "true" : "false");
    json += ",\"jsumoDir\":";
    json += (_hardware->jsumoRightRaw() ? "true" : "false");
    json += ",\"jsumoAsa\":";
    json += (_hardware->jsumoAsaRaw() ? "true" : "false");
    json += ",\"irEsq\":";
    json += (_hardware->irLeftRaw() ? "true" : "false");
    json += ",\"irDir\":";
    json += (_hardware->irRightRaw() ? "true" : "false");
    json += "}";

    configServer.setTestReadout(json);
}

void FumacinhaMode::runMotorTestCycle(Drive &motores, StatusLed &statusLed) {
    if(millis() - _motorTestStartMs >= MOTOR_TEST_STEP_MS) {
        _motorTestStartMs = millis();
        switch(_motorTestState) {
            case MotorTestState::PARADO:
                _motorTestState = MotorTestState::FRENTE;
                break;
            case MotorTestState::FRENTE:
                _motorTestState = MotorTestState::TRAS;
                break;
            case MotorTestState::TRAS:
                _motorTestState = MotorTestState::ESQUERDA;
                break;
            case MotorTestState::ESQUERDA:
                _motorTestState = MotorTestState::DIREITA;
                break;
            case MotorTestState::DIREITA:
                _motorTestState = MotorTestState::PARADO;
                break;
        }
    }

    applyMotorTestState(motores);

    DiagnosticsPanel panel(statusLed);
    panel.showMotionVector(_motorTestState);

    // Log ao vivo pro celular: o PWM exato que acabou de ser ENVIADO pra cada
    // motor (não uma leitura de volta — Drive não guarda getter público, e não
    // precisa: quem chamou setSpeed() já sabe o valor).
    int leftPWM = 0, rightPWM = 0;
    motorPwmForState(_motorTestState, leftPWM, rightPWM);

    String json = "{\"estado\":\"";
    json += motorTestStateLabel(_motorTestState);
    json += "\",\"motorEsq\":" + String(leftPWM) + ",\"motorDir\":" + String(rightPWM) + "}";

    configServer.setTestReadout(json);
}

void FumacinhaMode::applyMotorTestState(Drive &motores) {
    int leftPWM = 0, rightPWM = 0;
    motorPwmForState(_motorTestState, leftPWM, rightPWM);
    motores.setSpeed(leftPWM, rightPWM);
}

void FumacinhaMode::motorPwmForState(MotorTestState state, int &leftPWM, int &rightPWM) const {
    switch(state) {
        case MotorTestState::PARADO:
            leftPWM = 0;
            rightPWM = 0;
            break;
        case MotorTestState::FRENTE:
            leftPWM = MOTOR_TEST_PWM;
            rightPWM = MOTOR_TEST_PWM;
            break;
        case MotorTestState::TRAS:
            leftPWM = -MOTOR_TEST_PWM;
            rightPWM = -MOTOR_TEST_PWM;
            break;
        case MotorTestState::ESQUERDA:
            leftPWM = -MOTOR_TEST_PWM;
            rightPWM = MOTOR_TEST_PWM;
            break;
        case MotorTestState::DIREITA:
            leftPWM = MOTOR_TEST_PWM;
            rightPWM = -MOTOR_TEST_PWM;
            break;
    }
}

const char *FumacinhaMode::motorTestStateLabel(MotorTestState state) {
    switch(state) {
        case MotorTestState::PARADO:
            return "PARADO";
        case MotorTestState::FRENTE:
            return "FRENTE";
        case MotorTestState::TRAS:
            return "TRAS";
        case MotorTestState::ESQUERDA:
            return "ESQUERDA";
        case MotorTestState::DIREITA:
            return "DIREITA";
    }
    return "PARADO";
}

void FumacinhaMode::runServoTestCycle(StatusLed &statusLed) {
    if(!_hardware) {
        return;
    }

    if(millis() - _servoTestStartMs >= SERVO_TEST_STEP_MS) {
        _servoTestStartMs = millis();
        _servoTestStep = (_servoTestStep + 1) % 4;
    }

    // RETRACTED -> LEFT -> RETRACTED -> RIGHT -> (repete). Passa sempre por
    // RETRACTED entre os dois lados pra deixar bem visível cada transição.
    WingPosition target;
    const char *label;
    switch(_servoTestStep) {
        case 1:
            target = WingPosition::LEFT;
            label = "LEFT";
            break;
        case 3:
            target = WingPosition::RIGHT;
            label = "RIGHT";
            break;
        default:
            target = WingPosition::RETRACTED;
            label = "RETRACTED";
            break;
    }

    // Mesmo caminho de produção que a Fumacinha_FSM usa em combate
    // (HardwareCore::setWing(), aplicado em HardwareCore::update() logo no
    // topo de run()) — não um attach() isolado só pra esse teste, senão o
    // teste passar não prova que o caminho real de combate funciona.
    _hardware->setWing(target);

    // Representação nos LEDs: espelha pra onde a asa aponta.
    DiagnosticsPanel panel(statusLed);
    panel.showServoState(target);

    String json = "{\"estado\":\"";
    json += label;
    json += "\"}";
    configServer.setTestReadout(json);
}
