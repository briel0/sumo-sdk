#pragma once
#include "RobotTypes.hpp" // WingPosition
#include "StatusLED.hpp"

/**
    Estados do ciclo de teste de motores. Existem aqui (e não em
    DiagnosticsMode) porque DiagnosticsPanel::showMotionVector() precisa do
    tipo na própria assinatura — evita duplicar o mesmo enum em duas classes.
*/
enum class MotorTestState {
    PARADO,
    FRENTE,
    TRAS,
    ESQUERDA,
    DIREITA
};

/**
    @class DiagnosticsPanel
    @brief Mapeia os 5 LEDs endereçáveis da bancada de diagnóstico pra
           topologia física de sensores:

           LED 0 (Centro)   -> LDR da rampa
           LED 1 (Esquerda) -> Sensor de linha esquerdo
           LED 2 (Esquerda) -> Sensor IR esquerdo
           LED 3 (Direita)  -> Sensor IR direito
           LED 4 (Direita)  -> Sensor de linha direito

    Não possui tira/FastLED próprios: FastLED.addLeds() só pode registrar um
    controller por pino físico (o pino é fixado em tempo de compilação dentro
    de StatusLed::init()). Este painel recebe uma referência a um StatusLed já
    inicializado — a mesma tira de 5 LEDs usada pro feedback de boot/pareamento
    — e desenha nela via setLedRaw()/push(), em vez de criar uma segunda tira
    virtual sobre o mesmo hardware.
*/
class DiagnosticsPanel {
  public:
    explicit DiagnosticsPanel(StatusLed &statusLed);

    /**
    @brief Desenha o estado dos 5 sensores da bancada.
           Verde = detectando (alvo/linha/oponente). Vermelho = repouso.
    */
    void showSensorReadings(bool rampaLdr, bool linhaEsq, bool irEsq, bool irDir, bool linhaDir);

    /**
    @brief Desenha o vetor de movimento do teste de motores.
           PARADO acende os 5 LEDs na cor de repouso. Os outros estados acendem
           só o subconjunto de índices do vetor na cor de destaque; o resto
           apaga, pra o vetor ficar visualmente óbvio contra o fundo escuro.
    */
    void showMotionVector(MotorTestState state);

    /**
    @brief Desenha a posição atual da asa no teste de servo, mapeada na tira:
           RETRACTED acende o LED central (2); LEFT acende o par da esquerda
           (0,1); RIGHT acende o par da direita (3,4) — na cor de destaque, o
           resto apagado, espelhando pra onde a asa aponta fisicamente.
    */
    void showServoState(WingPosition pos);

  private:
    StatusLed &_statusLed;

    static const CRGB DETECT_COLOR;    // sensor detectando (verde)
    static const CRGB REST_COLOR;      // sensor em repouso / PARADO (vermelho)
    static const CRGB HIGHLIGHT_COLOR; // vetor de movimento em destaque (azul)
    static const CRGB OFF_COLOR;       // fora do vetor de movimento (apagado)
};
