#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Smoker";

    static constexpr int RIGHT_POS_PIN = 17;
    static constexpr int RIGHT_NEG_PIN = 16;
    static constexpr int LEFT_POS_PIN = 18;
    static constexpr int LEFT_NEG_PIN = 19;

    static constexpr int MAX_THROTTLE = 90;
    static constexpr int TURN_COEFFICIENT = 83;
    static constexpr int PIVOT_COEFFICIENT = 70;

    static constexpr int NUM_SERVOS = 1;

    // Os tres JS40F do arco frontal. Leitura digital direta no registrador,
    // entao qualquer GPIO serve — mas 5 e 15 sao strapping pins: se o sensor
    // segurar eles no nivel errado durante o boot, a placa nao sobe limpa.
    static constexpr int PIN_JS_ESQ = 5;
    static constexpr int PIN_JS_DIR = 15;
    static constexpr int PIN_JS_FRONT = 14;

    // Os dois QRE1113 sao analogicos e ficam no ADC1 (34 e 39): o ADC2 morre
    // enquanto o WiFi do painel esta de pe.
    static constexpr uint8_t PIN_LINHA_ESQ = 34;
    static constexpr uint8_t PIN_LINHA_DIR = 39;

    // Limiares separados de proposito: os dois QRE nunca leem igual no mesmo
    // dojo. Calibre cada um pelo painel /sensors antes da luta.
    static constexpr uint16_t LINHA_THRESHOLD_ESQ = 2800;
    static constexpr uint16_t LINHA_THRESHOLD_DIR = 2800;

    // Canal do AP de configuracao. Espalhados entre 1/6/11 (os tres
    // nao-sobrepostos) pra dois robos ligados na mesma bancada nao
    // disputarem o mesmo espectro e derrubarem o portal um do outro.
    static constexpr int WIFI_CHANNEL = 6;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        // ta no 22
        {26, 15, 180}, // Pino 22 | Começa em 15° | Arma em 120°
    };

    // -----------------------------------------------------------------------
    // ABERTURAS (saque cego). Cada uma tem um par ESQ/DIR indexado pelo mesmo id
    // nas TABELA_MACROS_* la embaixo: o AutoMode toca a DIR quando o piloto marca
    // "DIREITA" na HUD, e a ESQ em qualquer outro caso.
    // Formato do passo: {velocidade esquerda, velocidade direita, duracao em ms}.
    // -----------------------------------------------------------------------

    static const MotionSequence CURVA_ESQ = MACRO(
        {-100,  100, 100}, // giro no eixo pra esquerda
        { 100,   40, 144}, // arco aberto, roda de dentro segurando
        { 100, -100, 120}, // giro no eixo de volta pra direita
        {  50,   50,  75}  // avanço curto pra assentar
    );

    static const MotionSequence CURVA_DIR = MACRO(
        { 100, -100, 100}, // giro no eixo pra direita
        {  50,  100, 144}, // arco aberto, roda de dentro segurando
        {-100,  100, 130}, // giro no eixo de volta pra esquerda
        {  50,   50,  75}  // avanço curto pra assentar
    );

    // CURVÃO, portado do curvao (3/4 do dohyo): mesma ideia da CURVA com o arco
    // muito mais aberto — a roda de dentro cai a 20 e depois a 8 (50 e 20 na matriz
    // original), fechando um raio bem maior ao longo de ~470 ms de arco.
    static const MotionSequence CURVAO_ESQ = MACRO(
        {-100,  100,  70}, // giro no eixo pra esquerda
        { 100,   20, 170}, // arco abrindo
        { 100,    8, 300}, // arco no raio máximo
        { 100, -100, 120}, // giro no eixo de volta pra direita
        { 100,  100,  30}  // avanço curto pra assentar
    );

    static const MotionSequence CURVAO_DIR = MACRO(
        { 100, -100,  70}, // giro no eixo pra direita
        {  27,  100, 150}, // arco abrindo
        {  16,  100, 220}, // arco no raio máximo
        {-100,  100, 150}, // giro no eixo de volta pra esquerda
        { 100,  100,  60}  // avanço curto pra assentar
    );

    // EM V, portado do emV (3/4 do dohyo): avança reto, gira no eixo e avança reto
    // de novo — o vértice do V é o giro do meio. Abre com um giro curto que aponta
    // a primeira perna pro lado escolhido.
    static const MotionSequence EM_V_ESQ = MACRO(
        {-100,  100,  40}, // aponta pra esquerda
        { 100,  100, 155}, // primeira perna do V
        { 100, -100, 190}, // vértice: giro no eixo
        { 100,  100, 170}  // segunda perna do V
    );

    static const MotionSequence EM_V_DIR = MACRO(
        { 100, -100,  45}, // aponta pra direita
        { 100,  100, 190}, // primeira perna do V
        {-100,  100, 155}, // vértice: giro no eixo
        { 100,  100, 200}  // segunda perna do V
    );

    // VZINHO, portado do vzinho: o mesmo V SEM o giro de entrada — larga reto e o
    // primeiro desvio já é o vértice. O lado escolhido só muda pra onde ele vira.
    static const MotionSequence VZINHO_ESQ = MACRO(
        { 100,  100, 190}, // primeira perna, reto da largada
        { 100, -100, 180}, // vértice: giro no eixo
        { 100,  100, 180}  // segunda perna do V
    );

    static const MotionSequence VZINHO_DIR = MACRO(
        { 100,  100, 190}, // primeira perna, reto da largada
        {-100,  100, 155}, // vértice: giro no eixo
        { 100,  100, 190}  // segunda perna do V
    );

    // VZÃO, portado do vzao: V com giro de entrada mais longo que o do EM V e
    // vértice mais fechado.
    static const MotionSequence VZAO_ESQ = MACRO(
        {-100,  100,  90}, // aponta pra esquerda
        { 100,  100, 145}, // primeira perna do V
        { 100, -100, 200}, // vértice: giro no eixo
        { 100,  100, 170}  // segunda perna do V
    );

    static const MotionSequence VZAO_DIR = MACRO(
        { 100, -100,  90}, // aponta pra direita
        { 100,  100, 170}, // primeira perna do V
        {-100,  100, 145}, // vértice: giro no eixo
        { 100,  100, 170}  // segunda perna do V
    );

    // RECUO: ré curta em arco, uma roda segurando a outra. É a única abertura que
    // larga andando pra trás — o robô cede terreno em vez de disputá-lo, e apresenta
    // ao adversário a asa, que abre pro lado oposto ao escolhido (ver a regra da asa
    // no _largada).
    //
    // Os nomes são pelo LADO SELECIONADO, não pelo sentido do arco: RECUO_LADO_DIR é
    // o que roda com "DIREITA" escolhida na HUD.
    static const MotionSequence RECUO_LADO_DIR = MACRO({-60, -100, 180});
    static const MotionSequence RECUO_LADO_ESQ = MACRO({-100, -60, 180});

    // DESEMPATE: deriva curta pra um lado e giro no próprio eixo, terminando de lado
    // em relação a quem larga de frente. Mesma regra de asa do RECUO — ela abre pro
    // lado oposto ao escolhido, ou seja, pro lado de onde o robô está saindo.
    static const MotionSequence DESEMPATE_LADO_DIR =
        MACRO(
        {  85,  100, 120}, // deriva: a roda de dentro segura, o robô sai de lado
        {-100,  100, 180}  // giro no próprio eixo, fechando a posição
    );

    static const MotionSequence DESEMPATE_LADO_ESQ = MACRO(
        { 100,   85, 120}, // espelhado
        { 100, -100, 180}
    );

    // FRENTÃO, portado do frenteSemDelay: {255, 255, 200} -> {100, 100, 200}.
    // Avanço reto de ~3/4 do dohyo em força máxima, sem curva de aceleração.
    static const MotionSequence ABERTURA_FRENTAO = MACRO({100, 100, 200});

    // FRENTINHO, portado do frentinho: {127, 127, 200} -> {50, 50, 200}.
    // Mesmo avanço reto, meia força: ~1/4 do dohyo.
    static const MotionSequence ABERTURA_FRENTINHO = MACRO({50, 50, 200});

    // Macro vazia: o MotionPlayer pula o saque cego e o robo cai direto no combate.
    static const MotionSequence MACRO_SEM_SAQUE = {nullptr, 0};

    static constexpr const char *UI_PROFILE_JSON = R"({
        "robot_name": "Smoker",
        "macros": [
            {"id": 0, "name": "FRENTÃO"},
            {"id": 1, "name": "FRENTINHO"},
            {"id": 2, "name": "CURVA"},
            {"id": 3, "name": "CURVÃO"},
            {"id": 4, "name": "EM V"},
            {"id": 5, "name": "VZINHO"},
            {"id": 6, "name": "VZÃO"},
            {"id": 7, "name": "RECUO"},
            {"id": 8, "name": "DESEMPATE"},
            {"id": 9, "name": "SEM SAQUE"}
        ],
        "searches": [
            {"id": 1, "name": "BUSCA PADRAO"}
        ],
        "has_weapons": true
    })";

    // As duas tabelas andam juntas pelo mesmo id da UI e precisam ter o mesmo
    // tamanho (tem um static_assert no AutoMode.cpp cobrando isso). As aberturas
    // sem lado — FRENTÃO, FRENTINHO e SEM SAQUE — repetem nas duas.
    static const MotionSequence *const TABELA_MACROS_ESQ[] = {
        &ABERTURA_FRENTAO,   // 0 - FRENTÃO
        &ABERTURA_FRENTINHO, // 1 - FRENTINHO
        &CURVA_ESQ,          // 2 - CURVA
        &CURVAO_ESQ,         // 3 - CURVÃO
        &EM_V_ESQ,           // 4 - EM V
        &VZINHO_ESQ,         // 5 - VZINHO
        &VZAO_ESQ,           // 6 - VZÃO
        &RECUO_LADO_ESQ,     // 7 - RECUO
        &DESEMPATE_LADO_ESQ, // 8 - DESEMPATE
        &MACRO_SEM_SAQUE     // 9 - SEM SAQUE
    };

    static const MotionSequence *const TABELA_MACROS_DIR[] = {
        &ABERTURA_FRENTAO,   // 0 - FRENTÃO
        &ABERTURA_FRENTINHO, // 1 - FRENTINHO
        &CURVA_DIR,          // 2 - CURVA
        &CURVAO_DIR,         // 3 - CURVÃO
        &EM_V_DIR,           // 4 - EM V
        &VZINHO_DIR,         // 5 - VZINHO
        &VZAO_DIR,           // 6 - VZÃO
        &RECUO_LADO_DIR,     // 7 - RECUO
        &DESEMPATE_LADO_DIR, // 8 - DESEMPATE
        &MACRO_SEM_SAQUE     // 9 - SEM SAQUE
    };

}
