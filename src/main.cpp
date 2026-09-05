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
█                       #/ /## ##   ##   ## ##    ## ##    ##  /     ########  ##                     █
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
#include "ServoMechanism.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

// Firmware AUTO — desacoplado do RC (esta branch não tem mais firmware RC pro
// esp32dev, ver a decisão no histórico: BLE nativo (Bluedroid) e Bluepad32
// (BTstack) não cabem no mesmo binário nesse fork da Espressif). Trade-off
// aceito: perde a troca RC<->AUTO por senha IR/NVS (BootModeSelector/
// BootModeStore) que existia nesta branch — não faz sentido sem os dois
// modos no mesmo binário. Modo é fixado no boot, trocar é reflash.

Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
WeaponSystem sistemaDeArmas;
IRReader ir;
StatusLed statusLed;
ActiveAutoMode modoAuto;
HardwareCore hardwareCore;

ActiveAuto taticaAtual;

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô (firmware AUTO).");
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

    modoAuto.init(taticaAtual, hardwareCore);
    Serial.println("[MAIN] Modo AUTO engatilhado.");

    statusLed.confirmStep();
    statusLed.confirmStep();
    statusLed.confirmStep();
}

void loop() {
    ir.update();

    if(ir.stop()) {
        motores.setSpeed(0, 0);
        Serial.println("[MAIN] COMANDO DE PARAGEM (3). Reiniciando o sistema...");
        delay(50);
        ESP.restart();
    }

    // O AutoMode é o dono ÚNICO do feedback de LED no modo AUTO (inclusive o dos
    // testes de bancada) — ver o doc de AutoMode::run().
    modoAuto.run(motores, sistemaDeArmas, ir.start(), ir.ready(), statusLed);

    yield();
}
