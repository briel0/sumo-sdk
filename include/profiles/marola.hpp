#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Marola";

    static constexpr bool SKIP_SITE_CONFIG = false;

    static constexpr int RIGHT_POS_PIN = 19;
    static constexpr int RIGHT_NEG_PIN = 18;
    static constexpr int LEFT_POS_PIN = 17;
    static constexpr int LEFT_NEG_PIN = 16;

    static constexpr int MAX_THROTTLE = 100;
    static constexpr int TURN_COEFFICIENT = 98;
    static constexpr int PIVOT_COEFFICIENT = 95;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_LDR_ESQ = 34;
    static constexpr int PIN_LDR_DIR = 36;
    static constexpr int PIN_LDR_FRONT = 39;

    // Transistor na placa que liga a alimentação dos sensores (mesma placa
    // do Arruela, por isso o nome PIN_JS_POWER mesmo sem JS40F aqui). Precisa
    // ir pra HIGH o mais cedo possível no boot (ver main.cpp) pra dar o
    // máximo de tempo de estabilização antes de ler qualquer sensor.
    static constexpr int PIN_JS_POWER = 25;

    static constexpr uint8_t PIN_LINHA_ESQ = 15;
    static constexpr uint8_t PIN_LINHA_DIR = 14;
    static constexpr uint16_t LINHA_THRESHOLD_ESQ = 1000;
    static constexpr uint16_t LINHA_THRESHOLD_DIR = 1000;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {26, 125, 15}, // Pino 22 | Começa em 15° | Arma em 120°
    };

    // Distância até o adversário na largada, escolhida no site. O coeficiente
    // (em %) multiplica só o tempo dos passos marcados com `true` no fim —
    // {esq, dir, ms, true} —, que são os avanços e rés; giros e paradas ficam
    // no tempo fixo. MÉDIO é sempre 100%. Aplicado pelo AutoMode na largada.
    static constexpr int DISTANCIA_PERTO_PCT = 70;
    static constexpr int DISTANCIA_LONGE_PCT = 160;

    static const MotionSequence MACRO_FRENTAO = MACRO(
        {100, 100, 300, true});

    static const MotionSequence MACRO_CURVINHA_DIREITA = MACRO(
        {100, -100, 72},
        {100, 25, 245}, {100, 100, 72});

    static const MotionSequence MACRO_CURVINHA_ESQUERDA = MACRO(
        {-100, 100, 72},
        {25, 100, 245}, {100, 100, 72});

    static const MotionSequence MACRO_CURVAO_ESQUERDA = MACRO(
        {-100, 100, 70},
        {100, 40, 600}
    );

    static const MotionSequence MACRO_CURVAO_DIREITA = MACRO(
        {100, -100, 70},
        {40, 100, 500}
    );

    // Macro vazia: o MotionPlayer pula o saque cego e o robo cai direto no combate.
    static const MotionSequence MACRO_SEM_SAQUE = {nullptr, 0};

    // -----------------------------------------------------------------------
    // Aberturas portadas do firmware antigo (o switch de saques por letra).
    // Velocidade convertida de 0-255 pra 0-100; os tempos são os originais,
    // com o multiplicador de distância (multDist) em 1.0 — ainda não
    // calibrados no Marola. Os passos {0, 0, ms} são as paradas do original,
    // e os marcados com `true` são os que o original multiplicava por multDist.
    // -----------------------------------------------------------------------

    // 'f': rampa de aceleração até o talo, depois segue reto.
    static const MotionSequence MACRO_FRENTAO_PROGRESSIVO = MACRO(
        {19, 19, 4}, {24, 24, 4}, {28, 28, 4}, {33, 33, 4}, {38, 38, 4},
        {50, 50, 8}, {63, 63, 8}, {77, 77, 8}, {89, 89, 12},
        {100, 100, 200, true});

    // 'F': frentão anti pescador, no talo desde o primeiro instante.
    static const MotionSequence MACRO_FRENTAO_BOLADO = MACRO(
        {100, 100, 250, true});

    // 'e': fica parado antes de cair no combate.
    static const MotionSequence MACRO_ESPERA = MACRO(
        {0, 0, 400});

    // 'b' com o lado "frente": só a ré.
    static const MotionSequence MACRO_COSTAS_RETO = MACRO(
        {-100, -100, 150, true},
        {0, 0, 10});

    // 'b': ré, giro curto pro lado escolhido e avanço.
    static const MotionSequence MACRO_COSTAS_ESQUERDA = MACRO(
        {-100, -100, 120, true},
        {-100, 100, 30},
        {100, 100, 160, true},
        {0, 0, 10});

    static const MotionSequence MACRO_COSTAS_DIREITA = MACRO(
        {-100, -100, 120, true},
        {100, -100, 30},
        {100, 100, 160, true},
        {0, 0, 10});

    // 'c': curva leve.
    static const MotionSequence MACRO_CURVA_LEVE_ESQUERDA = MACRO(
        {-100, 100, 20},
        {100, 100, 120, true},
        {0, 0, 10},
        {100, -100, 70},
        {0, 0, 10});

    static const MotionSequence MACRO_CURVA_LEVE_DIREITA = MACRO(
        {100, -100, 20},
        {100, 100, 120, true},
        {0, 0, 10},
        {-100, 100, 70},
        {0, 0, 10});

    // 'C': curva pesada, com bote no final.
    static const MotionSequence MACRO_CURVA_PESADA_ESQUERDA = MACRO(
        {-100, 100, 40},
        {100, 100, 170, true},
        {0, 0, 10},
        {100, -100, 100},
        {100, 100, 50, true},
        {0, 0, 10});

    static const MotionSequence MACRO_CURVA_PESADA_DIREITA = MACRO(
        {100, -100, 40},
        {100, 100, 170, true},
        {0, 0, 10},
        {-100, 100, 100},
        {100, 100, 50, true},
        {0, 0, 10});

    // 'd': diagonal.
    static const MotionSequence MACRO_DIAGONAL_ESQUERDA = MACRO(
        {-100, 100, 70},
        {100, 100, 150, true},
        {0, 0, 10},
        {100, -100, 70});

    static const MotionSequence MACRO_DIAGONAL_DIREITA = MACRO(
        {100, -100, 70},
        {100, 100, 150, true},
        {0, 0, 10},
        {-100, 100, 70});

    // 'v': movimento em V.
    static const MotionSequence MACRO_V_ESQUERDA = MACRO(
        {-100, 100, 40},
        {100, 100, 140, true},
        {0, 0, 10},
        {100, -100, 190},
        {0, 0, 10},
        {100, 100, 180, true});

    static const MotionSequence MACRO_V_DIREITA = MACRO(
        {100, -100, 40},
        {100, 100, 140, true},
        {0, 0, 10},
        {-100, 100, 190},
        {0, 0, 10},
        {100, 100, 180, true});

    // 'a': aquele lá.
    static const MotionSequence MACRO_AQUELE_LA_ESQUERDA = MACRO(
        {-100, 100, 60},
        {100, 100, 110, true},
        {0, 0, 20},
        {100, -100, 130},
        {0, 0, 10});

    static const MotionSequence MACRO_AQUELE_LA_DIREITA = MACRO(
        {100, -100, 60},
        {100, 100, 110, true},
        {0, 0, 20},
        {-100, 100, 130},
        {0, 0, 10});

    // 'g': giro de 45° e espera parado.
    static const MotionSequence MACRO_GIRO_45_ESQUERDA = MACRO(
        {-100, 100, 50},
        {0, 0, 500});

    static const MotionSequence MACRO_GIRO_45_DIREITA = MACRO(
        {100, -100, 50},
        {0, 0, 500});

    // 'j': desempate.
    static const MotionSequence MACRO_DESEMPATE_ESQUERDA = MACRO(
        {-100, 100, 150},
        {0, 0, 20});

    static const MotionSequence MACRO_DESEMPATE_DIREITA = MACRO(
        {100, -100, 150},
        {0, 0, 20});

    // Fugas de borda: o MarolaAuto toca uma delas quando um QRE1113 ve a
    // linha branca. O nome e o lado do sensor que viu — a fuga gira pro lado
    // oposto.
    static const MotionSequence MACRO_FUGA_LINHA_ESQUERDA = MACRO(
        {-100, -100, 125}, // Da re com tudo
        {100, -100, 120}   // Gira pra direita pra fugir
    );

    static const MotionSequence MACRO_FUGA_LINHA_DIREITA = MACRO(
        {-100, -100, 125}, // Da re com tudo
        {-100, 100, 120}   // Gira pra esquerda pra fugir
    );

    // Giro inicial da ASA 100MS + GIRO (MarolaAuto::ASA_DELAY_100_GIRO): toca uma
    // vez ao entrar em combate, antes do avanço reto — o lado vem da direção
    // escolhida no site (cfg.direction), igual ao saque cego. Ajuste
    // velocidade/duração aqui pra calibrar o ângulo — nenhum outro arquivo
    // precisa mudar.
    static const MotionSequence MACRO_GIRO_INICIAL_ESQUERDA = MACRO(
        {-100, 100, 35});

    static const MotionSequence MACRO_GIRO_INICIAL_DIREITA = MACRO(
        {100, -100, 35});

    // Formato compacto, sem espaço nenhum: em "macros" o id é a posição na lista
    // (a mesma das TABELA_MACROS_*), e em "searches" cada item é [id, nome].
    static constexpr const char *UI_PROFILE_JSON =
        R"({"robot_name":"Marola",)"
        R"("macros":["FRENTAO","SEM SAQUE","CURVAO","FRENTAO PROGRESSIVO","FRENTAO BOLADO","ESPERA",)"
        R"("COSTAS RETO","COSTAS","CURVA LEVE","CURVA PESADA","DIAGONAL","V","AQUELE LA","GIRO 45","DESEMPATE"],)"
        R"("searches":[[1,"ASA 100MS"],[2,"ASA 500MS"],[3,"ASA 100MS + GIRO"],[4,"BUSCA SENSOR DE LINHA"]],)"
        R"("has_weapons":true,"has_distance":true})";

    // As duas tabelas andam juntas: a posição é o id que o site manda, na mesma
    // ordem do "macros" do UI_PROFILE_JSON. As aberturas sem lado repetem nas duas.
    static const MotionSequence *const TABELA_MACROS_ESQ[] = {
        &MACRO_FRENTAO,               // 0 - FRENTAO
        &MACRO_SEM_SAQUE,             // 1 - SEM SAQUE
        &MACRO_CURVAO_ESQUERDA,       // 2 - CURVAO
        &MACRO_FRENTAO_PROGRESSIVO,   // 3 - FRENTAO PROGRESSIVO
        &MACRO_FRENTAO_BOLADO,        // 4 - FRENTAO BOLADO
        &MACRO_ESPERA,                // 5 - ESPERA
        &MACRO_COSTAS_RETO,           // 6 - COSTAS RETO
        &MACRO_COSTAS_ESQUERDA,       // 7 - COSTAS
        &MACRO_CURVA_LEVE_ESQUERDA,   // 8 - CURVA LEVE
        &MACRO_CURVA_PESADA_ESQUERDA, // 9 - CURVA PESADA
        &MACRO_DIAGONAL_ESQUERDA,     // 10 - DIAGONAL
        &MACRO_V_ESQUERDA,            // 11 - V
        &MACRO_AQUELE_LA_ESQUERDA,    // 12 - AQUELE LA
        &MACRO_GIRO_45_ESQUERDA,      // 13 - GIRO 45
        &MACRO_DESEMPATE_ESQUERDA,    // 14 - DESEMPATE
    };

    static const MotionSequence *const TABELA_MACROS_DIR[] = {
        &MACRO_FRENTAO,              // 0 - FRENTAO
        &MACRO_SEM_SAQUE,            // 1 - SEM SAQUE
        &MACRO_CURVAO_DIREITA,       // 2 - CURVAO
        &MACRO_FRENTAO_PROGRESSIVO,  // 3 - FRENTAO PROGRESSIVO
        &MACRO_FRENTAO_BOLADO,       // 4 - FRENTAO BOLADO
        &MACRO_ESPERA,               // 5 - ESPERA
        &MACRO_COSTAS_RETO,          // 6 - COSTAS RETO
        &MACRO_COSTAS_DIREITA,       // 7 - COSTAS
        &MACRO_CURVA_LEVE_DIREITA,   // 8 - CURVA LEVE
        &MACRO_CURVA_PESADA_DIREITA, // 9 - CURVA PESADA
        &MACRO_DIAGONAL_DIREITA,     // 10 - DIAGONAL
        &MACRO_V_DIREITA,            // 11 - V
        &MACRO_AQUELE_LA_DIREITA,    // 12 - AQUELE LA
        &MACRO_GIRO_45_DIREITA,      // 13 - GIRO 45
        &MACRO_DESEMPATE_DIREITA,    // 14 - DESEMPATE
    };

}
