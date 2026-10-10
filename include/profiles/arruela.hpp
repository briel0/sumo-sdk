#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Arruela";

    static constexpr bool SKIP_SITE_CONFIG = false;

    static constexpr int RIGHT_POS_PIN = 19;
    static constexpr int RIGHT_NEG_PIN = 18;
    static constexpr int LEFT_POS_PIN = 16;
    static constexpr int LEFT_NEG_PIN = 17;

    static constexpr int MAX_THROTTLE = 100;     // Velocidade Máxima (Pra Frente, Pra Trás)
    static constexpr int TURN_COEFFICIENT = 83;  // Coeficiente de Curva
    static constexpr int PIVOT_COEFFICIENT = 70; // Coeficiente de Rotação

    static constexpr int NUM_SERVOS = 0;

    static constexpr int PIN_JS_ESQ = 27;
    static constexpr int PIN_JS_DIR = 14;
    static constexpr int PIN_JS_FRONT = 4;

    // Transistor na placa que liga a alimentação dos três JS40F. Precisa ir
    // pra HIGH o mais cedo possível no boot (ver main.cpp) pra dar o máximo
    // de tempo de estabilização antes de ler qualquer sensor.
    static constexpr int PIN_JS_POWER = 25;

    // Os dois QRE1113 (linha, analogico), usados pela fuga de borda do
    // ArruelaAuto.
    // ATENCAO: 26 e ADC2, que morre com WiFi ligado. O painel hoje e BLE (sem
    // WiFi), entao le normal; se o WiFi voltar, mover pro ADC1 (32-39). O 32
    // ja e ADC1.
    static constexpr uint8_t PIN_LINHA_ESQ = 26;
    static constexpr uint8_t PIN_LINHA_DIR = 32;

    // Limiares separados de proposito: os dois QRE nunca leem igual no mesmo
    // dojo. Calibre cada um na bancada antes da luta.
    static constexpr uint16_t LINHA_THRESHOLD_ESQ = 1000;
    static constexpr uint16_t LINHA_THRESHOLD_DIR = 1000;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Cada linha { } é um servo físico!
    static constexpr ServoConfig SERVOS[] = {};

    static const MotionSequence MACRO_FRENTAO = MACRO(
        {100, 100, 200},);

    static const MotionSequence MACRO_FRENTINHA = MACRO(
        {100, 100, 60},);

    static const MotionSequence MACRO_RECUO = MACRO(
        {-80, -80, 60},);

    // Fugas de borda: o ArruelaAuto toca uma delas quando um QRE1113 ve a
    // linha branca. O nome e o lado do sensor que viu — a fuga gira pro lado
    // oposto.
    static const MotionSequence MACRO_FUGA_LINHA_ESQUERDA = MACRO(
        {-100, -100, 150}, // Da re com tudo
        {100, -100, 180}   // Gira pra direita pra fugir
    );

    static const MotionSequence MACRO_FUGA_LINHA_DIREITA = MACRO(
        {-100, -100, 150}, // Da re com tudo
        {-100, 100, 180}   // Gira pra esquerda pra fugir
    );

    static const MotionSequence MACRO_CURVINHA_ESQUERDA = MACRO(
        {-100, 100, 70},
        {100, 50, 180}, 
        {100, -100, 180});

    static const MotionSequence MACRO_CURVINHA_DIREITA = MACRO(
        {-100, 100, 70},
        {50, 100, 180}, 
        {-100, 100, 180});

    static const MotionSequence MACRO_CURVAO_ESQUERDA = MACRO(
        {-100, 100, 70},
        {100, 40, 500}
    );

    static const MotionSequence MACRO_CURVAO_DIREITA = MACRO(
        {100, -100, 70},
        {40, 100, 500}
    );

    // Macro vazia: o MotionPlayer pula o saque cego e o robo cai direto no combate.
    static const MotionSequence MACRO_SEM_SAQUE = {nullptr, 0};

    // Placeholder pra calibrar em bancada: dá ré e volta com tudo. Valores
    // (velocidade/duração) ainda não ajustados.
    static const MotionSequence MACRO_REVENGE = MACRO(
        {100, 100, 300},
        {100, -100, 100}, {100, 100, 180}, {100,-100,110});

    // Tabela de macros mapeadas pelo ID que vem do site (0 = Frentão, 1 = Curvão, 2 = Sem Saque, 3 = Frentinha,
    // 4 = Recuo, 5 = Curvinha, 6 = Revenge)
    static const MotionSequence *const TABELA_MACROS_ESQ[] = {
        &MACRO_FRENTAO, &MACRO_CURVAO_ESQUERDA,   &MACRO_SEM_SAQUE, &MACRO_FRENTINHA,
        &MACRO_RECUO,   &MACRO_CURVINHA_ESQUERDA, &MACRO_REVENGE};

    static const MotionSequence *const TABELA_MACROS_DIR[] = {
        &MACRO_FRENTAO, &MACRO_CURVAO_DIREITA,   &MACRO_SEM_SAQUE, &MACRO_FRENTINHA,
        &MACRO_RECUO,   &MACRO_CURVINHA_DIREITA, &MACRO_REVENGE};

    // Sem indentação de propósito: essa string vai inteira pra uma BLE
    // characteristic (BleConfigServer, CMD_GET_PROFILE) e o Bluedroid do
    // ESP32 tem um teto rígido de ESP_GATT_MAX_ATTR_LEN = 600 bytes por
    // characteristic — acima disso setValue() falha em silêncio (só loga
    // erro) e o app lê o valor antigo. Indentado, esse mesmo JSON passava
    // de 600 bytes com 6 macros + 5 buscas. Se for adicionar itens, prefira
    // nomes curtos a reindentar.
    static constexpr const char *UI_PROFILE_JSON = R"({
"robot_name": "Arruela",
"macros": [
{"id": 0, "name": "FRENTÃO"},
{"id": 1, "name": "CURVÃO"},
{"id": 2, "name": "SEM SAQUE"},
{"id": 3, "name": "FRENTINHA"},
{"id": 4, "name": "RECUO"},
{"id": 5, "name": "CURVINHA"},
{"id": 6, "name": "REVENGE"}
],
"searches": [
{"id": 1, "name": "BUSCA PADRÃO"},
{"id": 2, "name": "BUSCA LENTA"},
{"id": 3, "name": "BUSCA POR DISTÂNCIA"},
{"id": 4, "name": "BUSCA GATO PRETO"},
{"id": 5, "name": "BUSCA BALA TENSA"}
],
"has_weapons": false
})";

}