#pragma once
#include <stdint.h>

/**
    Modo em que o robô opera depois do setup(). Vive aqui (e não no main.cpp)
    porque o BootModeStore grava esse valor na NVS e o BootModeSelector o
    escolhe por senha IR — os dois precisam do mesmo tipo.

    ATENÇÃO: os valores numéricos são o que vai gravado na NVS. Reordenar ou
    inserir no meio faz um robô com valor antigo gravado subir no modo errado
    (o carimbo de default do BootModeStore não pega isso — ele só detecta
    mudança do default compilado). Só acrescente no fim.
*/
enum class RobotState : uint8_t {
    IDLE = 0,
    RC = 1,
    AUTO = 2
};

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
    WingServo (hardware real) e FumacinhaAuto (HAL placeholder) para não existir
    duas definições incompatíveis do mesmo conceito físico.
*/
enum class WingPosition {
    RETRACTED,
    LEFT,
    RIGHT
};

/**
    Fases da luta. A FumacinhaAuto percorre essas fases conforme o combate evolui.
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
    FRENTAO,          // Avanço reto e longo (3/4 do dohyo) em força máxima
    FRENTINHO,        // Avanço reto e curto (1/4 do dohyo) em meia força
    CURVAO,           // Curva aberta e longa (3/4 do dohyo)
    CURVA,            // Curva fechada saindo do centro (1/4 do dohyo)
    EDGE_POSITIONING, // Posicionamento propositalmente próximo à borda
    EM_V,             // Avança, vira no eixo e avança de novo (3/4 do dohyo)
    VZAO,             // V longo, com giro de entrada
    VZINHO,           // Mesmo V, sem o giro de entrada — mais curto
    RECUO,            // Ré curta em arco, cedendo terreno, com a asa do lado oposto
    DESEMPATE         // Deriva curta + giro no eixo, com a asa do lado oposto
};

/**
    Táticas disponíveis para a fase SEARCHING (varredura pelo oponente).
*/
enum class SearchTactic {
    OFFENSIVE_SEARCH, // Gira procurando e ataca em arco assim que um lateral vê o alvo
    PULSED_SEARCH,    // Parado a maior parte do tempo, com pulsos curtos de avanço
    LINE_SEARCH,      // Cruzeiro reto usando a linha da borda como referência
    DEFENSIVE_SEARCH, // Rasteja e só alinha; escala para LINE_SEARCH se estagnar
    WING_SEARCH       // Gira no eixo mantendo o alvo varrido pelo sensor da asa
};
// Os valores destes enums são o que a HUD transmite (data-val dos botões) e o
// que o ConfigServer limita no constrain — acrescentar tática é sempre no FIM,
// senão os botões existentes passam a apontar pra outra coisa.

/**
    Como o robô pretende ENCERRAR a luta. As duas são mutuamente exclusivas de
    propósito: cada uma desliga o gatilho da outra.
*/
enum class AttackTactic {
    // Finalização por LDR: o comportamento de sempre. O LDR de rampa é quem
    // decide — oponente embarcado, o robô empurra em força máxima até o fim, e
    // a borda de descida dispara o "S" de reengate se ele escapar.
    LDR_FINISH,

    // Finalização por tempo: o LDR é IGNORADO a luta inteira, desde o primeiro
    // frame. Quem decide é o relógio — passados CombatProfile::finishTimeS
    // segundos da largada, os emissores ligam e a BUSCA OFENSIVA assume o resto
    // da luta, seja qual for a tática de busca escolhida na HUD.
    TIME_FINISH
};

/**
    @struct CombatTuning
    @brief Velocidades (PWM, escala -100..100) e tempos (ms) de cada passo do
           FumacinhaAuto. Os defaults aqui são exatamente os valores que eram
           constantes constexpr no FSM — o FumacinhaAuto lê tudo daqui via
           activeProfile.tuning. Hoje NÃO há controle no dashboard pra mexer
           nesses valores (o ajuste ao vivo é feito pelo construtor de macro,
           que é um testador à parte), então na prática rodam sempre nos
           defaults; a struct existe pra deixar a porta aberta sem espalhar
           constantes soltas pelo .cpp.
*/
struct CombatTuning {
    // --- OPENING / CURVA (CURVA) ---
    unsigned long evasionDurationMs = 400; // duração da curva de fuga
    int           evasionPwmOuter   = 100; // roda externa da curva
    int           evasionPwmInner   = 40;  // roda interna (fecha a curva)

    // --- OPENING / CURVA DE BORDA (EDGE_POSITIONING) ---
    int           edgeAdvancePwmOuter = 100; // roda externa do arco até a borda
    int           edgeAdvancePwmInner = 55;  // roda interna do arco até a borda
    unsigned long spinDurationMs      = 300; // duração do giro no próprio eixo
    int           spinPwm             = 90;  // módulo do giro (sinais opostos)

    // --- OPENING / FRENTÃO (FRENTAO) ---
    unsigned long frentaoMs  = 200; // duração do avanço reto longo
    int           frentaoPwm = 100; // potência do frentão

    // --- OPENING / FRENTINHO (FRENTINHO) ---
    unsigned long frentinhoMs  = 200; // duração do avanço reto curto
    int           frentinhoPwm = 50;  // potência do frentinho (127/255 do original)

    // --- SEARCHING / BUSCA OFENSIVA (OFFENSIVE_SEARCH) ---
    unsigned long sweepHalfPeriodMs = 250; // duração de cada perna do zigue-zague
    int           sweepPwmOuter     = 70;  // roda externa da perna
    int           sweepPwmInner     = 20;  // roda interna da perna

    // --- SEARCHING / BUSCA PULSADA (PULSED_SEARCH) ---
    unsigned long pulseIntervalMs = 1500; // período do ciclo (1 pulso + espera parada)
    unsigned long pulseDurationMs = 90;   // duração do avanço de cada pulso
    int           pulsePwm        = 100;  // potência do pulso (as duas rodas)
    uint8_t       pulseCount      = 4;    // pulsos da série antes de cair no busca linha

    // --- SEARCHING / BUSCA LINHA (LINE_SEARCH) ---
    int           zombieNavPwm = 80; // velocidade de cruzeiro do busca linha

    // --- ATTACKING / EMPUXO CORRIGIDO (ALIGNMENT_STRIKE) ---
    int           alignmentPwmMax       = 100; // lado alinhado / avanço reto
    int           alignmentPwmCorrected = 70;  // lado reduzido pra puxar a mira

    // --- ATTACKING / EMPUXO CEGO (STEAMROLLER) ---
    int           steamrollerPwm = 100; // avanço puro, sem correção
};

/**
    @struct CombatProfile
    @brief Envelope de dados com as táticas escolhidas para o round atual do
           FumacinhaAuto. Mesmo papel que AutoStrategy cumpre para o AutoMode
           legado — preenchido pelo ConfigServer a partir do payload do celular.
*/
struct CombatProfile {
    OpeningTactic openingTactic = OpeningTactic::CURVA;
    SearchTactic  searchTactic  = SearchTactic::OFFENSIVE_SEARCH;
    AttackTactic  attackTactic  = AttackTactic::LDR_FINISH;
    // Segundos de luta até a FINALIZAÇÃO POR TEMPO assumir. Ignorado pela
    // finalização por LDR. Faixa 0..1023 s (o ConfigServer limita) — uint16_t
    // cobre isso com folga; 0 = a busca ofensiva assume já no primeiro frame.
    // O valor é escolhido na HUD a cada luta; este default só vale se o campo
    // não vier no pacote.
    uint16_t      finishTimeS = 20;
    // Lado preferencial das táticas assimetricas (lado de todas as macros de
    // abertura, primeira perna do OFFENSIVE_SEARCH, lado da asa em
    // LINE_SEARCH — e, por tabela, no trecho de busca linha do PULSED_SEARCH).
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
    // === Especificações do adversário =======================================
    // O adversário tem asa. Muda o que o LDR coberto significa: pode ser a ASA
    // dele na nossa rampa, com o corpo de lado — ver a correção de asa
    // adversária na FumacinhaAuto. Também força os emissores laterais ligados a
    // luta inteira, porque a correção depende do JSumo estar confiável no exato
    // frame em que o LDR fecha, e o emissor não liga retroativamente.
    bool          opponentHasWing = false;
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
