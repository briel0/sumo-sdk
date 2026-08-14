#include "Fumacinha_FSM.hpp"
#include "Drive.hpp"
#include "HardwareCore.hpp"
#include <Arduino.h>

void Fumacinha_FSM::init(CombatProfile profile, Drive &motores, HardwareCore &hardware) {
    activeProfile = profile;
    _motores      = &motores;
    _hardware     = &hardware;

    // Garante a asa recolhida no início de CADA luta. HardwareCore/WingServo só
    // recolhem automaticamente uma vez, no primeiro begin() depois do boot — sem
    // isso, uma segunda luta sem reboot herdaria a posição de asa deixada pela
    // luta anterior (SEARCHING/ATTACKING são as únicas fases que mexem na asa).
    _hardware->setWing(WingPosition::RETRACTED);

    changePhase(CombatPhase::OPENING);
    Serial.println("[FSM] Fumacinha: perfil de combate carregado. Fase inicial: OPENING.");
}

void Fumacinha_FSM::update() {
    if(!_motores || !_hardware) {
        return; // init() ainda não foi chamado — não há o que fazer.
    }

    // Regra de sobrevivência: tem prioridade absoluta sobre qualquer fase ou
    // tática em andamento. Se a luminosidade da rampa despencou, o oponente já
    // está embarcado — força STEAMROLLER e a fase ATTACKING agora, e deixa cair
    // no switch abaixo, que vai despachar direto pra executeSteamroller() neste
    // mesmo frame. Isso aborta OPENING (linha branca, evasão) ou SEARCHING em
    // andamento sem precisar duplicar a lógica de avanço aqui.
    if(readRampLDR()) {
        activeProfile.attackTactic = AttackTactic::STEAMROLLER;
        if(currentPhase != CombatPhase::ATTACKING) {
            changePhase(CombatPhase::ATTACKING);
        }
    }

    switch(currentPhase) {
        case CombatPhase::OPENING:
            handleOpeningPhase();
            break;
        case CombatPhase::SEARCHING:
            handleSearchingPhase();
            break;
        case CombatPhase::ATTACKING:
            handleAttackingPhase();
            break;
    }
}

void Fumacinha_FSM::changePhase(CombatPhase newPhase) {
    // Trava de segurança: nunca deixamos os motores girando "de carona" entre fases.
    if(_motores) {
        _motores->setSpeed(0, 0);
    }

    currentPhase   = newPhase;
    stateStartTime = millis();

    // Reseta o sub-estado interno de EDGE_POSITIONING, garantindo que a próxima
    // vez que essa tática rodar sempre comece pelo avanço (sub-estado A).
    edgeSubState = EdgeSubState::ADVANCING;
}

unsigned long Fumacinha_FSM::elapsedInState() const {
    return millis() - stateStartTime;
}

bool Fumacinha_FSM::readRampLDR() const {
    return _hardware->opponentOnRamp();
}

// === Despachantes de fase ======================================================
// Cada um lê a tática escolhida no CombatProfile e chama o método correspondente.

void Fumacinha_FSM::handleOpeningPhase() {
    // Política de emissor: OPENING é SEMPRE furtivo. Nenhuma tática de abertura
    // usa detecção lateral, então ligar os emissores aqui só anunciaria nossa
    // posição de graça. (Ver setStealth: 'on=false' = emissores desligados.)
    _hardware->setStealth(false);

    switch(activeProfile.openingTactic) {
        case OpeningTactic::STANDARD_EVASION:
            executeEvasion();
            break;
        case OpeningTactic::EDGE_POSITIONING:
            executeEdgePositioning();
            break;
        case OpeningTactic::CENTER_DOMINANCE:
            executeCenterDominance();
            break;
    }
}

void Fumacinha_FSM::handleSearchingPhase() {
    // Política de emissor da BUSCA: escolhida no dashboard (searchEmitters), não
    // mais fixa por tática. Ligado = detecção lateral por JSumo (ativo, mas
    // visível); desligado = IR puro (passivo, furtivo). Ver HardwareCore.
    _hardware->setStealth(activeProfile.searchEmitters);

    // Condição de transição comum às três táticas de busca: assim que o sensor
    // frontal enxergar o oponente, vai direto para ATTACKING, não importa qual
    // tática estava ativa no momento.
    if(_hardware->frontDetected()) {
        changePhase(CombatPhase::ATTACKING);
        return;
    }

    switch(activeProfile.searchTactic) {
        case SearchTactic::STEALTH_SWEEP:
            executeStealthSweep();
            break;
        case SearchTactic::ACTIVE_FISHING:
            executeActiveFishing();
            break;
        case SearchTactic::ZOMBIE_NAV:
            executeZombieNav();
            break;
    }
}

void Fumacinha_FSM::handleAttackingPhase() {
    // Política de emissor do ATAQUE: escolhida no dashboard (attackEmitters).
    // Importa pro EMPUXO CORRIGIDO, cuja correção lê os sensores laterais —
    // ligado corrige por JSumo (ativo), desligado por IR puro (passivo). O
    // EMPUXO CEGO não usa lateral, então o estado aqui é só furtividade.
    _hardware->setStealth(activeProfile.attackEmitters);

    switch(activeProfile.attackTactic) {
        case AttackTactic::ALIGNMENT_STRIKE:
            executeAlignmentStrike();
            break;
        case AttackTactic::STEAMROLLER:
            executeSteamroller();
            break;
    }
}

// === Táticas de OPENING =========================================================

void Fumacinha_FSM::executeEvasion() {
    const CombatTuning &t = activeProfile.tuning;

    // Malha aberta: curva de evasão rápida (PWM assimétrico, roda externa em
    // potência máxima) para sair do centro antes que o oponente feche distância
    // logo após a largada. Lado da curva segue CombatProfile::preferredSide.
    if(activeProfile.preferredSide == Direction::left) {
        _motores->setSpeed(t.evasionPwmInner, t.evasionPwmOuter);
    }
    else {
        _motores->setSpeed(t.evasionPwmOuter, t.evasionPwmInner);
    }

    if(elapsedInState() >= t.evasionDurationMs) {
        changePhase(CombatPhase::SEARCHING);
    }
}

void Fumacinha_FSM::executeEdgePositioning() {
    const CombatTuning &t = activeProfile.tuning;
    bool                left = (activeProfile.preferredSide == Direction::left);

    switch(edgeSubState) {
        case EdgeSubState::ADVANCING:
            // Sub-estado A: avanço agressivo em arco na direção da lateral do
            // dohyo escolhida em CombatProfile::preferredSide (mesma assimetria
            // de rodas usada em toda a tática).
            if(left) {
                _motores->setSpeed(t.edgeAdvancePwmInner, t.edgeAdvancePwmOuter);
            }
            else {
                _motores->setSpeed(t.edgeAdvancePwmOuter, t.edgeAdvancePwmInner);
            }

            if(_hardware->lineLeft() || _hardware->lineRight()) {
                // Linha branca da borda à frente: encerra o avanço e inicia o giro.
                edgeSubState  = EdgeSubState::SPINNING;
                spinStartTime = millis();
            }
            break;

        case EdgeSubState::SPINNING:
            // Sub-estado B: inverte a roda que estava "por fora" no arco e
            // normaliza as duas para spinPwm — sinais opostos em módulo igual
            // é o que produz giro no próprio eixo, não um mero pivô sobre uma
            // roda parada. Mesmo lado escolhido na ADVANCING acima.
            if(left) {
                _motores->setSpeed(t.spinPwm, -t.spinPwm);
            }
            else {
                _motores->setSpeed(-t.spinPwm, t.spinPwm);
            }

            if(millis() - spinStartTime >= t.spinDurationMs) {
                changePhase(CombatPhase::SEARCHING);
            }
            break;
    }
}

void Fumacinha_FSM::executeCenterDominance() {
    const CombatTuning &t = activeProfile.tuning;

    // Malha aberta: avanço reto (PWM simétrico) tomando o centro do dohyo antes
    // que o oponente o faça. Duração muito curta — ver tuning.centerRushMs.
    _motores->setSpeed(t.centerRushPwm, t.centerRushPwm);

    if(elapsedInState() >= t.centerRushMs) {
        changePhase(CombatPhase::SEARCHING);
    }
}

// === Táticas de SEARCHING ========================================================
// A condição de transição comum (frontDetected() -> ATTACKING) já foi resolvida em
// handleSearchingPhase() antes de chegar aqui — cada método abaixo só cuida da
// própria movimentação.

void Fumacinha_FSM::executeStealthSweep() {
    // (Estado do emissor agora é definido em handleSearchingPhase, via perfil.)
    const CombatTuning &t = activeProfile.tuning;

    // Oscila a orientação em zigue-zague enquanto avança, alternando a cada
    // sweepHalfPeriodMs. Não precisa de estado de classe: a perna atual do
    // zigue-zague é só uma função pura do tempo decorrido nesta fase. A
    // primeira perna começa no lado de CombatProfile::preferredSide.
    bool startRight    = (activeProfile.preferredSide != Direction::left);
    bool onFirstLeg    = (elapsedInState() / t.sweepHalfPeriodMs) % 2 == 0;
    bool sweepingRight = onFirstLeg ? startRight : !startRight;

    if(sweepingRight) {
        _motores->setSpeed(t.sweepPwmOuter, t.sweepPwmInner);
    }
    else {
        _motores->setSpeed(t.sweepPwmInner, t.sweepPwmOuter);
    }
}

void Fumacinha_FSM::executeActiveFishing() {
    // (Estado do emissor agora é definido em handleSearchingPhase, via perfil.
    // Pra isca "de verdade" atrair o oponente, selecione o emissor LIGADO na
    // busca no dashboard — a tática em si só cuida do movimento da asa.)
    _motores->setSpeed(0, 0); // parado — só a asa "pesca"

    // Alterna o lado da asa a cada fishingIntervalMs, também sem estado de
    // classe (mesmo truque do zigue-zague acima). Começa no lado de
    // CombatProfile::preferredSide.
    bool startLeft  = (activeProfile.preferredSide == Direction::left);
    bool onFirstLeg = (elapsedInState() / activeProfile.tuning.fishingIntervalMs) % 2 == 0;
    bool wingLeft   = onFirstLeg ? startLeft : !startLeft;
    _hardware->setWing(wingLeft ? WingPosition::LEFT : WingPosition::RIGHT);
}

void Fumacinha_FSM::executeZombieNav() {
    // (Estado do emissor agora é definido em handleSearchingPhase, via perfil.)

    // Asa fixa para o lado escolhido em CombatProfile::preferredSide age como
    // rede de arrasto física, varrendo o adversário para dentro do alcance do
    // sensor frontal enquanto o robô avança cego.
    bool left = (activeProfile.preferredSide == Direction::left);
    _hardware->setWing(left ? WingPosition::LEFT : WingPosition::RIGHT);
    _motores->setSpeed(activeProfile.tuning.zombieNavPwm, activeProfile.tuning.zombieNavPwm);
}

// === Táticas de ATTACKING =========================================================

void Fumacinha_FSM::executeAlignmentStrike() {
    const CombatTuning &t = activeProfile.tuning;

    bool viuEsq    = _hardware->leftDetected();
    bool viuDir    = _hardware->rightDetected();
    bool viuFrente = _hardware->frontDetected();

    if(!viuFrente && !viuEsq && !viuDir) {
        // Perdeu contato tático: nenhum dos três IR frontais vê mais o oponente.
        changePhase(CombatPhase::SEARCHING);
        return;
    }

    if(viuEsq && !viuDir) {
        // Alvo puxando pra esquerda: reduz a roda esquerda pra fechar a mira nesse lado.
        _motores->setSpeed(t.alignmentPwmCorrected, t.alignmentPwmMax);
    }
    else if(viuDir && !viuEsq) {
        _motores->setSpeed(t.alignmentPwmMax, t.alignmentPwmCorrected);
    }
    else {
        // Frente limpo, ou os dois laterais ao mesmo tempo: alvo centralizado, força bruta reta.
        _motores->setSpeed(t.alignmentPwmMax, t.alignmentPwmMax);
    }
}

void Fumacinha_FSM::executeSteamroller() {
    // Recolhe a asa imediatamente — estendida, ela quebraria no impacto contra
    // o oponente ou a rampa.
    _hardware->setWing(WingPosition::RETRACTED);

    // Avanço puro, sem checagem de contato ou correção: rolo compressor não negocia.
    _motores->setSpeed(activeProfile.tuning.steamrollerPwm, activeProfile.tuning.steamrollerPwm);
}
