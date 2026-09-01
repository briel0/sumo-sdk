#include "RCMode.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

void RCMode::init() {
    receptor.init();
}

void RCMode::handleWeapons(WeaponSystem &armas, int throttle, int steer) {
    if(receptor.circle()) {
        _autoDisarmLocked = !_autoDisarmLocked;
        Serial.printf("[SUMÔ] Auto-desarme: %s\n", _autoDisarmLocked ? "TRAVADO" : "ATIVO");
    }

    bool playerIsMoving = throttle != 0 || steer != 0;
    bool weaponsArmed = armas.isDeployed();
    bool macroRunning = macroPlayer.isPlaying();
    bool autoDisarmFree = !_autoDisarmLocked;

    if(!weaponsArmed && (playerIsMoving || receptor.dpadUp()))
        armas.deploy();

    if(receptor.dpadDown() && weaponsArmed && autoDisarmFree)
        armas.retract();

    // Removed: auto-disarm when stopped - weapon now stays deployed until manually retracted
}


void RCMode::handleMacros(Drive &motores, WeaponSystem &armas) {
    if(macroPlayer.isPlaying()) {
        macroPlayer.update(motores);
        return;
    }

    auto triggerMacro = [&](const MotionSequence &seq) {
        if(!_autoDisarmLocked) {
            armas.deploy();
        }
        macroPlayer.play(seq);
    };

    // CONFIGURE AS MACROS AQUI!!!
    // O X NÃO dispara mais macro: ele virou o override de aceleração total no
    // topo de run(), que sai do frame antes de chegar aqui. Se o FRENTÃO no
    // controle fizer falta, remapeie pra um botão livre.
    if(receptor.square()) {
        // triggerMacro(Config::MACRO_CURVAO_ESQ);
    }
    else if(receptor.triangle()) {
        // triggerMacro(Config::MACRO_CURVAO_DIR);
    }
#if defined(ROBOT_FUMACINHA)
    // Setas = giro de 180° no próprio eixo, pro lado da seta. Só o Fumacinha
    // define essas macros; deixar o binding sem guarda quebraria o build dos
    // outros perfis, e defini-las em todos daria a eles um atalho de RC que
    // ninguém pediu (o tempo de 135 ms é calibrado pra ESTE chassi).
    else if(receptor.dpadLeft()) {
        triggerMacro(Config::MACRO_GIRO_ESQ);
    }
    else if(receptor.dpadRight()) {
        triggerMacro(Config::MACRO_GIRO_DIR);
    }
#else
    else if(receptor.dpadRight()) {
        // triggerMacro(Config::MACRO_CURVAO_DIR);
    }
    else if(receptor.dpadLeft()) {
        // triggerMacro(Config::MACRO_CURVAO_ESQ);
    }
#endif
}

void RCMode::run(Drive &motores, WeaponSystem &armas) {
    receptor.update();
    armas.update();

    // === OVERRIDE DE ACELERAÇÃO TOTAL (botão X) =============================
    // Roda ANTES de tudo e sai do frame: enquanto o X estiver segurado, o robô
    // vai pra frente em potência crua e nada mais é lido — nem gatilhos, nem
    // direcional, nem macro. Sair aqui é o que garante isso, porque as três
    // coisas moram abaixo.
    //
    // A macro em curso é ENCERRADA, não só ignorada. O MotionPlayer é baseado em
    // millis(): se ela ficasse viva, o relógio dela correria durante o override e
    // ela voltaria adiantada (ou já vencida) quando o X fosse solto. Encerrando,
    // soltar o X devolve o controle limpo ao piloto no frame seguinte, que é o
    // comportamento pedido.
    //
    // armas.update() acima continua rodando, então o servo em movimento termina o
    // curso normalmente. O que pausa é a LEITURA dos botões de arma
    // (handleWeapons), que fica abaixo — segurar o X é um comando de movimento.
    if(receptor.crossHeld()) {
        macroPlayer.stop();
        motores.setSpeed(CROSS_BOOST_PWM, CROSS_BOOST_PWM);
        return;
    }


    int throttle = receptor.rightTrigger() - receptor.leftTrigger();
    int steer = receptor.leftStickX();

    handleWeapons(armas, throttle, steer);

    handleMacros(motores, armas);

    if(macroPlayer.isPlaying()) {
        return;
    }

    throttle = (throttle * Config::MAX_THROTTLE) / 100;
    if(throttle == 0) {
        steer = (steer * Config::PIVOT_COEFFICIENT) / 100;
    }
    else {
        steer = (steer * Config::TURN_COEFFICIENT) / 100;
    }

    motores.setSpeed(throttle + steer, throttle - steer);
}