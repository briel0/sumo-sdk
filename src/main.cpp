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

#include "BootModeSelector.hpp"
#include "BootModeStore.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "HardwareCore.hpp"
#include "IRreader.hpp"
#include "RCMode.hpp"
#include "ServoMechanism.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

// RobotState mora em RobotTypes.hpp — o BootModeStore grava esse valor na NVS.

// Aqui mudamos pra ir pra RC ou AUTO direto. Continua sendo o default de
// fábrica, mas não é mais a palavra final: o BootModeSelector pode gravar outro
// modo na NVS por senha IR. Mexer NESTA linha e regravar o firmware descarta a
// senha gravada e devolve o comando ao código (ver BootModeStore).
static constexpr RobotState BOOT_DEFAULT = RobotState::RC;

RobotState currentState = BOOT_DEFAULT;

Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
WeaponSystem sistemaDeArmas;
IRReader ir;
StatusLed statusLed;
RCMode modoRC;
ActiveAutoMode modoAuto;
HardwareCore hardwareCore;
BootModeSelector bootSelector;

ActiveAuto taticaAtual;

// O receptor IR fica vivo durante o pareamento do controle no RC (é a janela em
// que a senha de boot pode ser digitada) e só é desligado quando o controle
// conecta — a partir daí não há mais IR na sessão de RC, como antes.
static bool irDesligadoNoRC = false;

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

// A senha de boot por IR só vale nas janelas em que o robô está parado
// esperando alguém: escolha de modo no IDLE, pareamento do controle no RC e
// seleção de tática no AUTO (enquanto o AP de configuração está no ar). Assim
// que o controle conecta ou o perfil tático chega do site, a janela fecha e não
// reabre até o próximo boot.
static bool janelaDeConfigIRAberta() {
    switch(currentState) {
        case RobotState::IDLE:
            return true;
        case RobotState::RC:
            return !modoRC.controllerConnected();
        case RobotState::AUTO:
            return modoAuto.getSubState() == ActiveAutoMode::SubState::SELECTING_ESTRATEGIA;
    }
    return false;
}

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô.");
    statusLed.init(LED_BUILTIN, Config::PIN_STATUS_LED, Config::STATUS_LED_COUNT);
    delay(100);

    // Modo de boot: o gravado por senha IR vence o default compilado, exceto
    // quando o próprio default mudou desde a gravação (ver BootModeStore).
    currentState = BootModeStore::load(BOOT_DEFAULT);
    bootSelector.begin(BOOT_DEFAULT);

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
            // Sem ir.shutdown() aqui: o receptor precisa sobreviver ao
            // pareamento pra senha de boot poder ser digitada. Ele é desligado
            // no loop(), assim que o controle conecta.
            Serial.println("[MAIN] BOOT DIRETO: Modo RC engatilhado.");
            break;
        case RobotState::AUTO:
            modoAuto.init(taticaAtual, hardwareCore);
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

    // Senha de boot por IR: tem que vir ANTES de qualquer tratamento semântico,
    // porque os dígitos 8 e 9 (que fecham 4568/4569) são os mesmos botões de
    // "modo AUTO" e "modo RC" — sem o consumeEvent(), fechar a senha engataria o
    // modo na hora, em vez de agendar o próximo boot.
    if(janelaDeConfigIRAberta()) {
        if(bootSelector.feed(ir.digit())) {
            ir.consumeEvent();
        }
    }
    else {
        bootSelector.cancel(statusLed);
    }
    bootSelector.update(statusLed);

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
            // IR segue vivo durante o pareamento (janela da senha de boot);
            // desligado no case RC abaixo, quando o controle conectar.
            currentState = RobotState::RC;
            statusLed.setState(CRGB::Green);
            Serial.println("[MAIN] -> MODO RC ENGATILHADO");
        }
        else if(ir.modeAuto()) {
            modoAuto.init(taticaAtual, hardwareCore);
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
                // Controle conectou: fecha a janela da senha de boot desligando
                // o receptor IR, como era feito na entrada do modo antes.
                if(!irDesligadoNoRC) {
                    ir.shutdown();
                    irDesligadoNoRC = true;
                    Serial.println("[MAIN] Controle conectado. Receptor IR desligado.");
                }
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
            // O AutoMode é o dono ÚNICO do feedback de LED no modo AUTO
            // (inclusive o dos testes de bancada) — o strategyWave e o painel de
            // teste escrevendo no mesmo frame era o que fazia os LEDs piscarem.
            modoAuto.run(motores, sistemaDeArmas, ir.start(), ir.ready(), statusLed);
            break;
        case RobotState::IDLE:
            statusLed.heartbeat();
            break;
    }
    yield();
}
