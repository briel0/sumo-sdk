#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Caipora";

    // === Rádio =================================================================
    // Canal WiFi do AP de configuração (2.4GHz), espalhado entre os canais
    // não-sobrepostos (1/6/11) pra dois robôs em bancada não brigarem pelo mesmo
    // espectro na seleção de tática. O AP morre na largada (ConfigServer::
    // shutdown()), então isso só importa durante a configuração.
    static constexpr int WIFI_CHANNEL = 1;

    // MAC do controle Bluetooth "dono" — allowlist em tempo de compilação. Só
    // este controle pareia, eliminando cross-pairing quando dois robôs nossos
    // entram em RC ao mesmo tempo. Tudo-zero = modo descoberta (aceita o
    // primeiro controle e imprime o MAC no serial pra você fixar aqui).
    static constexpr uint8_t CONTROLLER_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static constexpr int RIGHT_POS_PIN = 16;
    static constexpr int RIGHT_NEG_PIN = 17;
    static constexpr int LEFT_POS_PIN = 19;
    static constexpr int LEFT_NEG_PIN = 18;

    static constexpr int MAX_THROTTLE = 85;
    static constexpr int TURN_COEFFICIENT = 90;
    static constexpr int PIVOT_COEFFICIENT = 95;

    static constexpr int NUM_SERVOS = 1;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // === Sensores de combate (VL53L0X / ToF) ====================================
    // Este robô não tem IR nem JSumo: quem enxerga o oponente são quatro VL53L0X.
    // Os pinos abaixo são os XSHUT de cada um — o endereço I2C definitivo é
    // atribuído pelo HardwareCore (0x30..0x33), porque todos nascem em 0x29.
    //
    //   CENT_*  (centrais) -> juntos, fazem o papel do SENSOR FRONTAL
    //   ESQ/DIR (extremos) -> fazem o papel dos JSUMO laterais
    static constexpr int PIN_VL_CENT_ESQ = 15;
    static constexpr int PIN_VL_CENT_DIR = 36;
    static constexpr int PIN_VL_ESQ = 4;
    static constexpr int PIN_VL_DIR = 14;

    // Distância que conta como "oponente à vista". O ToF devolve milímetros, ao
    // contrário do IR/JSumo, que só davam um booleano — então o alcance de
    // combate deste robô é um número ajustável, e não uma propriedade do sensor.
    static constexpr uint16_t TOF_THRESHOLD_MM = 400;

    static constexpr uint8_t PIN_LINHA_ESQ = 30;
    static constexpr uint8_t PIN_LINHA_DIR = 35;
    static constexpr uint16_t LINHA_THRESHOLD = 2800;

    // LDR de confirmação de rampa (divisor de tensão com resistor de 10k).
    // ATENÇÃO ao threshold: Fuego usa 300 e Fumacinha usa 100 — NÃO são iguais,
    // porque dependem do LDR, do divisor e da luz do dohyo. Ficou no valor do
    // Fumacinha, que é o mais conservador (dispara menos ataque falso). Calibre
    // no painel de bancada olhando o rampaLdrRaw antes de confiar nele.
    static constexpr uint8_t PIN_LDR = 39;
    static constexpr uint16_t LDR_THRESHOLD = 100; // abaixo disso: oponente sobre a rampa
    static constexpr uint8_t LDR_FILTER_SIZE = 8;  // janela da média móvel

    // === Núcleo de Hardware ======================================================
    // Família HW_FAMILY_CAIPORA (ver HardwareFamily.hpp): 4 ToF + 2 linha + LDR.
    // Sem emissor furtivo (ToF é sempre ativo) e sem asa.
    static constexpr bool USES_HARDWARE_CORE = true;

    // === Modo AUTO ==============================================================
    // true = HUD tática (abertura/busca/ataque) + FumacinhaAuto como estratégia,
    // igual ao Fumacinha. Ver ActiveAuto no Config.hpp.
    static constexpr bool USES_FUMACINHA_FSM = true;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {
        {4, 15, 90}, // Pino 4 | Começa em 15° | Arma em 85°
    };

    // Ordem dos campos do MACRO: {motor ESQUERDO, motor DIREITO, tempo_ms}.
    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

    static const MotionSequence MACRO_CURVINHA_DIREITA = MACRO(
        {100, -100, 70},
        {60, 100, 150}, {-100, 100, 60});

    static const MotionSequence MACRO_CURVINHA_ESQUERDA = MACRO(
        {-100, 100, 70},
        {100, 60, 150}, {100, -100, 60});

    static const MotionSequence MACRO_CURVAO_ESQUERDA = MACRO(
        {-100, 100, 70},
        {100, 40, 600});

    static const MotionSequence MACRO_CURVAO_DIREITA = MACRO(
        {100, -100, 70},
        {40, 100, 500});

}
