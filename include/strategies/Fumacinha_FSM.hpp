#pragma once

#include "RobotTypes.hpp"

class Drive;
class HardwareCore;

/**
    @class Fumacinha_FSM
    @brief Máquina de estados de combate do Fumacinha.

    Percorre as fases OPENING -> SEARCHING -> ATTACKING. Dentro de cada fase,
    despacha para o método de tática correspondente à escolha feita no
    CombatProfile carregado em init().

    Fala com o hardware real através de duas dependências injetadas em init()
    e guardadas como ponteiro (mesmo padrão de AutoMode::_estrategia/_hardware):
    Drive (motores, -100 a 100, igual Drive::setSpeed) e HardwareCore (sensores
    já filtrados + periféricos de estado — emissores laterais e asa). A FSM
    nunca lê registrador nem toca em pino diretamente; toda a complexidade de
    hardware fica isolada dentro de HardwareCore/Drive.

    As três fases já executam lógica real de malha aberta com sensores reais.

    Regra de sobrevivência: update() checa readRampLDR() com prioridade absoluta
    sobre qualquer fase/tática em andamento. Se o LDR indicar oponente embarcado
    na rampa, força STEAMROLLER e a fase ATTACKING instantaneamente, abortando
    qualquer leitura de linha branca ou tentativa de evasão em curso — não é uma
    tática normal, é um override de segurança.

    Todo o controle de tempo é não-bloqueante (baseado em millis()). update()
    deve ser chamado a cada iteração do loop() principal; nenhum método aqui
    usa delay().
*/
class Fumacinha_FSM {
  public:
    Fumacinha_FSM() = default;

    /**
    @brief Carrega o perfil tático do round, injeta as dependências de hardware
           e reseta a FSM para a fase OPENING. Chamado uma única vez, na largada.
    @param profile  Táticas escolhidas para o round (abertura, busca e ataque).
    @param motores  Referência aos motores. Guardada como ponteiro; motores
                     precisa sobreviver por toda a luta (objeto global em main.cpp).
    @param hardware Núcleo de hardware (sensores filtrados + periféricos de
                     estado). Precisa já estar inicializado (isInitialized()).
    */
    void init(CombatProfile profile, Drive &motores, HardwareCore &hardware);

    /**
    @brief Executa um frame da FSM. Chamado repetidamente pelo loop() principal.
           No-op se init() ainda não foi chamado.
    */
    void update();

    /**
    @brief Fase atual da luta. Leitura externa (ex: feedback de LED, debug).
    */
    CombatPhase getCurrentPhase() const {
        return currentPhase;
    }

  private:
    // Sub-estados internos exclusivos da tática EDGE_POSITIONING. Ficam aninhados
    // porque são um detalhe de implementação — nada fora desta classe precisa
    // conhecer o meio-do-caminho de uma tática específica.
    enum class EdgeSubState {
        ADVANCING, // Sub-estado A: avanço em arco até esbarrar na borda
        SPINNING   // Sub-estado B: giro no próprio eixo, de costas para a borda
    };

    Drive        *_motores  = nullptr;
    HardwareCore *_hardware = nullptr;

    CombatProfile activeProfile;
    CombatPhase   currentPhase = CombatPhase::OPENING;

    // Timestamp (millis()) de quando a fase atual começou. Base de toda
    // temporização não-bloqueante de transição entre fases (e também das
    // oscilações periódicas das táticas de SEARCHING, via elapsedInState()).
    unsigned long stateStartTime = 0;

    // Estado interno de EDGE_POSITIONING. Precisa ser variável de classe (e não
    // local ao método) porque executeEdgePositioning() é chamado uma vez por
    // frame e o sub-estado tem que sobreviver entre chamadas.
    EdgeSubState  edgeSubState  = EdgeSubState::ADVANCING;
    unsigned long spinStartTime = 0; // millis() de quando o giro (sub-estado B) começou

    /**
    @brief Move a FSM para uma nova fase: para os motores, reseta o tempo de
           referência e limpa qualquer sub-estado interno de tática.
    Único ponto de saída de uma fase — garante que nunca trocamos de fase com
    os motores girando "de carona".
    */
    void changePhase(CombatPhase newPhase);

    /**
    @brief Tempo (ms) decorrido desde que a fase atual começou.
    */
    unsigned long elapsedInState() const;

    /**
    @brief true se a luminosidade da rampa caiu drasticamente — sinal de que o
           oponente subiu/embarcou na rampa. Wrapper fino sobre
           HardwareCore::opponentOnRamp() (LDR + média móvel + threshold, já
           implementado de verdade em HardwareCore); existe como método próprio
           só para dar ao update() um nome de domínio (readRampLDR()) em vez de
           espalhar _hardware->opponentOnRamp() pelo código.
    */
    bool readRampLDR() const;

    // --- Despachantes de fase ---------------------------------------------------
    // Lêem a tática escolhida no perfil ativo e chamam o método correspondente.
    void handleOpeningPhase();
    void handleSearchingPhase();
    void handleAttackingPhase();

    // --- Táticas de OPENING -------------------------------------------------------
    void executeEvasion();
    void executeEdgePositioning();
    void executeCenterDominance();

    // --- Táticas de SEARCHING ------------------------------------------------------
    void executeStealthSweep();
    void executeActiveFishing();
    void executeZombieNav();

    // --- Táticas de ATTACKING -----------------------------------------------------
    void executeAlignmentStrike();
    void executeSteamroller();

    // As velocidades e tempos de cada passo NÃO são mais constantes aqui — viraram
    // campos de runtime em CombatTuning (ver RobotTypes.hpp), ajustáveis pelo
    // dashboard sem reflash. A FSM lê tudo de activeProfile.tuning.
};
