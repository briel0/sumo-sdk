#pragma once
#include <stdint.h>

// Esta struct é apenas um envelope de dados. Não aciona hardware nenhum.
struct ServoConfig {
    int pin;
    int retractAngle;
    int deployAngle;
};

struct MotionStep {
    int leftSpeed;            // Velocidade do motor esquerdo (-100 a 100). Negativo = ré.
    int rightSpeed;           // Velocidade do motor direito  (-100 a 100). Negativo = ré.
    unsigned long durationMs; // Quanto tempo manter essa velocidade (em milissegundos).
};

// Representa uma fita cassete completa (uma lista de passos)
struct MotionSequence {
    const MotionStep *steps;
    int numSteps;
};

struct AutoStrategy {
    int macro = 0;        // Índice na TABELA_DE_ESTRATEGIAS[] do AutoMode.cpp
    char direction = 'X'; // 'E' = esquerda, 'D' = direita, 'X' = sem preferência
    int search = 1;       // 1 = busca padrão, 2 = busca lenta
    int weapon = 0;       // 0 = não armar, 1 = armar no começo da luta
    bool isNew = false;   // Sinaliza que chegou novos dados do celular
};

enum class Direction {
    left,
    right
};

/**
    Posições fundamentais do servo da asa lateral (TPU). Compartilhado entre
    WingServo (hardware real) e Fumacinha_FSM (HAL placeholder) para não existir
    duas definições incompatíveis do mesmo conceito físico.
*/
enum class WingPosition {
    RETRACTED,
    LEFT,
    RIGHT
};

/**
    Fases da luta. A Fumacinha_FSM percorre essas fases conforme o combate evolui.
*/
enum class CombatPhase {
    OPENING,   // Saque inicial, logo após a largada
    SEARCHING, // Varredura até localizar o oponente
    ATTACKING  // Engajamento direto com o oponente localizado
};

/**
    Táticas disponíveis para a fase OPENING (abertura de round).
*/
enum class OpeningTactic {
    STANDARD_EVASION, // Saque padrão com fuga do centro
    EDGE_POSITIONING, // Posicionamento propositalmente próximo à borda
    CENTER_DOMINANCE  // Ocupação agressiva do centro do dohyo
};

/**
    Táticas disponíveis para a fase SEARCHING (varredura pelo oponente).
*/
enum class SearchTactic {
    STEALTH_SWEEP,  // Varredura com os emissores IR laterais desligados
    ACTIVE_FISHING, // Varredura ativa, movimentando-se para provocar reação
    ZOMBIE_NAV      // Navegação pelo último rumo conhecido, sem varredura ativa
};

/**
    Táticas disponíveis para a fase ATTACKING (engajamento).
*/
enum class AttackTactic {
    ALIGNMENT_STRIKE, // Corrige o alinhamento antes de avançar
    STEAMROLLER        // Avanço direto em força máxima
};

/**
    @struct CombatTuning
    @brief Velocidades (PWM, escala -100..100) e tempos (ms) de cada passo do
           Fumacinha_FSM. Os defaults aqui são exatamente os valores que eram
           constantes constexpr no FSM — o Fumacinha_FSM lê tudo daqui via
           activeProfile.tuning. Hoje NÃO há controle no dashboard pra mexer
           nesses valores (o ajuste ao vivo é feito pelo construtor de macro,
           que é um testador à parte), então na prática rodam sempre nos
           defaults; a struct existe pra deixar a porta aberta sem espalhar
           constantes soltas pelo .cpp.
*/
struct CombatTuning {
    // --- OPENING / ARCO EVASIVO (STANDARD_EVASION) ---
    unsigned long evasionDurationMs = 400; // duração da curva de fuga
    int           evasionPwmOuter   = 100; // roda externa da curva
    int           evasionPwmInner   = 40;  // roda interna (fecha a curva)

    // --- OPENING / CURVA DE BORDA (EDGE_POSITIONING) ---
    int           edgeAdvancePwmOuter = 100; // roda externa do arco até a borda
    int           edgeAdvancePwmInner = 55;  // roda interna do arco até a borda
    unsigned long spinDurationMs      = 300; // duração do giro no próprio eixo
    int           spinPwm             = 90;  // módulo do giro (sinais opostos)

    // --- OPENING / ARRANCADA RETA (CENTER_DOMINANCE) ---
    unsigned long centerRushMs  = 200; // duração do avanço ao centro
    int           centerRushPwm = 100; // potência do avanço reto

    // --- SEARCHING / ZIGUEZAGUE IR (STEALTH_SWEEP) ---
    unsigned long sweepHalfPeriodMs = 250; // duração de cada perna do zigue-zague
    int           sweepPwmOuter     = 70;  // roda externa da perna
    int           sweepPwmInner     = 20;  // roda interna da perna

    // --- SEARCHING / PULSO PERIÓDICO (ACTIVE_FISHING) ---
    unsigned long fishingIntervalMs = 800; // troca de lado da asa

    // --- SEARCHING / AVANÇO CEGO (ZOMBIE_NAV) ---
    int           zombieNavPwm = 80; // velocidade de cruzeiro do avanço cego

    // --- ATTACKING / EMPUXO CORRIGIDO (ALIGNMENT_STRIKE) ---
    int           alignmentPwmMax       = 100; // lado alinhado / avanço reto
    int           alignmentPwmCorrected = 70;  // lado reduzido pra puxar a mira

    // --- ATTACKING / EMPUXO CEGO (STEAMROLLER) ---
    int           steamrollerPwm = 100; // avanço puro, sem correção
};

/**
    @struct CombatProfile
    @brief Envelope de dados com as táticas escolhidas para o round atual do
           Fumacinha_FSM. Mesmo papel que AutoStrategy cumpre para o AutoMode
           legado — preenchido pelo ConfigServer a partir do payload do celular.
*/
struct CombatProfile {
    OpeningTactic openingTactic = OpeningTactic::STANDARD_EVASION;
    SearchTactic  searchTactic  = SearchTactic::STEALTH_SWEEP;
    AttackTactic  attackTactic  = AttackTactic::ALIGNMENT_STRIKE;
    // Lado preferencial das táticas assimetricas (curva de EDGE_POSITIONING/
    // STANDARD_EVASION, lado da asa em ZOMBIE_NAV/ACTIVE_FISHING/STEALTH_SWEEP).
    // Default = right pra preservar o comportamento antigo (hardcoded) de quem
    // não mexer nesse controle novo no dashboard.
    Direction     preferredSide = Direction::right;
    // Emissores IR laterais LIGADOS (true) por fase. OPENING é sempre furtivo
    // (não usa detecção lateral), então não tem campo aqui — só BUSCA e ATAQUE
    // são selecionáveis. Lembrando que o estado do emissor também troca o sensor
    // lateral usado (ligado = JSumo ativo; desligado = IR puro passivo), ver
    // HardwareCore. Default false = furtivo.
    bool          searchEmitters = false;
    bool          attackEmitters = false;
    // Velocidades e tempos de cada passo do FSM, ajustáveis pelo dashboard. Ver
    // CombatTuning — viaja junto neste mesmo pacote.
    CombatTuning  tuning;
    bool          weapon        = false; // arma o sistema de armas no início da luta
    bool          isNew         = false; // sinaliza que chegou novos dados do celular
};

// Isso serve pra deixar mais fácil na hora de escrever as macros no profile
#define MACRO(...)                                                                                                     \
    []() -> MotionSequence {                                                                                           \
        static constexpr MotionStep steps[] = {__VA_ARGS__};                                                           \
        return {steps, (int)(sizeof(steps) / sizeof(steps[0]))};                                                       \
    }()
