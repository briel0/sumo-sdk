/* 
███████████████████████████████████████████████████████████████████████████████████████████████████
█                                                                                                 █
█                                           __________                                            █
█                                        .~#########%%;~.                                         █
█                                       /############%%;`\                                        █
█                                      /######/~\/~\%%;,;,\                                       █ 
█                                     |#######\    /;;;;.,.|                                      █
█                                     |#########\/%;;;;;.,.|                                      █
█                            XX       |##/~~\####%;;;/~~\;,|       XX                             █
█                          XX..X      |#|  o  \##%;/  o  |.|      X..XX                           █
█                        XX.....X     |##\____/##%;\____/.,|     X.....XX                         █
█                       X.....XX      \#########/\;;;;;;,, /      XX.....X                        █
█                      X |..XX%,.@      \######/%;\;;;;, /      @#%,XX..| X                       █
█                     X |..X  @#%,.@     |######%%;;;;,.|     @#%,.@  X..| X                      █
█                    X  \.X     @#%,.@   |# # # % ; ; ;,|   @#%,.@     X./  X                     █
█                   X# \.X        @#%,.@                  @#%,.@         X./ #                    █
█                   #  X            @#%,.@              @#%,.@           X   #                    █
█                   ##X               @#%,.@          @#%,.@              X ##                    █
█                   `###X               @#%,.@      @#%,.@               ####'                    █
█                   ' ###                 @#%.,@  @#%,.@                 ###`"                    █
█                    . ";"                  @#%.@#%,.@                  ;"` ' .                   █
█                       '                     @#%,.@                    ,.                        █
█                       ` ,                 @#%,.@  @@                 `                          █
█                                            @@@  @@@               .                             █
█                                                                                                 █
█                   #######                              /                                        █
█                 /       ###                          #/                                         █
█                /         ##                          ##                                         █
█                ##        #                           ##                                         █
█                 ###                                  ##                                         █
█                ## ###      ### /### /###     /###    ##  /##      /##  ###  /###                █
█                 ### ###     ##/ ###/ /##  / / ###  / ## / ###    / ###  ###/ #### /             █
█                   ### ###    ##  ###/ ###/ /   ###/  ##/   /    /   ###  ##   ###/              █
█                     ### /##  ##   ##   ## ##    ##   ##   /    ##    ### ##                     █
█                       #/ /## ##   ##   ## ##    ##   ##  /     ########  ##                     █
█                        #/ ## ##   ##   ## ##    ##   ## ##     #######   ##                     █
█                         # /  ##   ##   ## ##    ##   ######    ##        ##                     █
█               /##        /   ##   ##   ## ##    ##   ##  ###   ####    / ##                     █
█              /  ########/    ###  ###  ### ######    ##   ### / ######/  ###                    █ 
█             /     #####       ###  ###  ### ####      ##   ##/   #####    ###                   █
█             |                                                                                   █
█              \)                                                                                 █
█                                                                                                 █
███████████████████████████████████████████████████████████████████████████████████████████████████
*/

#include "Config.hpp"
#include "Drive.hpp"
#include "HardwareCore.hpp"
#include "IRreader.hpp"
#include "RCMode.hpp"
#include "ServoMechanism.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

enum class RobotState {
    
    IDLE,
    RC,
    AUTO
};

// Aqui mudamos pra ir pra RC ou AUTO direto
RobotState currentState = RobotState::AUTO;

Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
WeaponSystem sistemaDeArmas;
IRReader ir;
StatusLed statusLed;
RCMode modoRC;
ActiveAutoMode modoAuto;
HardwareCore hardwareCore;

ActiveAuto taticaAtual;

// Prepara o HardwareCore ao entrar em RC: recolhe a asa (senão ela fica onde a
// última luta AUTO deixou — ver ponto de "atuador com dois donos") e desliga os
// emissores furtivos (não há combate no RC). Para robôs sem HardwareCore
// (família NONE), é um no-op. A aplicação real acontece no update() por frame,
// chamado no case RC do loop().
static void prepararHardwareParaRC() {
    if(Config::USES_HARDWARE_CORE) {
        hardwareCore.begin();
        hardwareCore.setStealth(false);
        hardwareCore.setWing(WingPosition::RETRACTED);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô.");
    statusLed.init(LED_BUILTIN, Config::PIN_STATUS_LED, Config::STATUS_LED_COUNT);
    delay(100);

    ir.init(IR_PIN);
    statusLed.confirmStep();

    // Timers LEDC alocados UMA vez, aqui, antes de qualquer servo prender o pino.
    // Os dois donos de servo (WeaponSystem/ServoMechanism e o WingServo dentro do
    // HardwareCore) só chamam attach() — não realocam timers cada um por conta
    // própria, o que corrompia a contabilidade da ESP32PWM quando um robô tinha
    // os dois (ver ServoMechanism::init / WingServo::init).
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    for(int i = 0; i < Config::NUM_SERVOS; i++) {
        ServoMechanism *s =
            new ServoMechanism(Config::SERVOS[i].pin, Config::SERVOS[i].retractAngle, Config::SERVOS[i].deployAngle);
        s->init();
        sistemaDeArmas.addServo(s);
    }
    statusLed.confirmStep();

    switch(currentState) {
        case RobotState::RC:
            modoRC.init();
            prepararHardwareParaRC();
            ir.shutdown();
            Serial.println("[MAIN] BOOT DIRETO: Modo RC engatilhado.");
            break;
        case RobotState::AUTO:
#if defined(ROBOT_FUMACINHA)
            modoAuto.init(hardwareCore);
#else
            modoAuto.init(taticaAtual, hardwareCore);
#endif
            Serial.println("[MAIN] BOOT DIRETO: Modo AUTO engatilhado.");
            break;
        case RobotState::IDLE:
            Serial.println("[MAIN] Setup concluído. Aguardando sinal IR do juiz...");
            break;
    }

    statusLed.confirmStep();
    statusLed.confirmStep();
    statusLed.confirmStep();
}

void loop() {
    ir.update();

    // Feedback visual para códigos IRs válidos (menos '1')
    if(ir.modeRC() || ir.modeAuto() || ir.stop()) {
        statusLed.blinkDebug(5, 20);
    }

    if(ir.stop()) {
        motores.setSpeed(0, 0);
        Serial.println("[MAIN] COMANDO DE PARAGEM (3). Desconectando controle...");
        modoRC.disconnectController();
        delay(100);
        Serial.println("[MAIN] Reiniciando o sistema...");
        delay(50);
        ESP.restart();
    }

    if(currentState == RobotState::IDLE) {
        if(ir.modeRC()) {
            modoRC.init();
            prepararHardwareParaRC();
            ir.shutdown();
            currentState = RobotState::RC;
            statusLed.setState(CRGB::Green);
            Serial.println("[MAIN] -> MODO RC ENGATILHADO");
        }
        else if(ir.modeAuto()) {
#if defined(ROBOT_FUMACINHA)
            modoAuto.init(hardwareCore);
#else
            modoAuto.init(taticaAtual, hardwareCore);
#endif
            currentState = RobotState::AUTO;
            statusLed.setState(CRGB::Orange);
            Serial.println("[MAIN] -> MODO AUTO ENGATILHADO");
        }
    }

    switch(currentState) {
        case RobotState::RC:
            if(!modoRC.controllerConnected()) {
                statusLed.pairingWave();
            }
            else {
                statusLed.setState(CRGB::Green); // limpa o laranja residual
            }
            // Aplica a intenção de asa/furtivo definida na entrada do RC (recolher
            // a asa, emissores desligados). No-op para robôs da família NONE.
            if(Config::USES_HARDWARE_CORE && hardwareCore.isInitialized()) {
                hardwareCore.update();
            }
            modoRC.run(motores, sistemaDeArmas);
            break;
        case RobotState::AUTO:
#if defined(ROBOT_FUMACINHA)
            // FumacinhaMode é o dono ÚNICO do feedback de LED no modo AUTO
            // (inclusive dos testes de bancada) — evita a briga entre o
            // strategyWave e o painel de teste que causava o piscar. Ver
            // FumacinhaMode::run.
            modoAuto.run(motores, sistemaDeArmas, ir.start(), ir.ready(), statusLed);
#else
            if(modoAuto.getSubState() == ActiveAutoMode::SubState::SELECTING_ESTRATEGIA ||
               modoAuto.getSubState() == ActiveAutoMode::SubState::DISCONNECTING_WIFI) {
                statusLed.strategyWave();
            }
            if(modoAuto.getSubState() == ActiveAutoMode::SubState::READY) {
                if(ir.ready()) {
                    statusLed.blinkDebug(1, 20);
                    statusLed.setAll(CRGB::Red);
                }
                if(!modoAuto.readyReceived()) {
                    statusLed.setState(CRGB::Orange);
                }
                if(ir.start()) {
                    statusLed.setState(CRGB::Black);
                }
            }
            modoAuto.run(motores, sistemaDeArmas, ir.start(), ir.ready());
#endif
            break;
        case RobotState::IDLE:
            statusLed.heartbeat();
            break;
    }
    yield();
}
