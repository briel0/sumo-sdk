#pragma once

#include "RobotTypes.hpp"

class StatusLed;

/**
    @class BootModeSelector
    @brief Senha por IR que reprograma em qual RobotState o robô sobe no próximo
           boot, sem cabo e sem recompilar.

    Senha de quatro dígitos, prefixo comum 456 + o dígito do modo:
        4567 -> IDLE    4568 -> AUTO    4569 -> RC

    Tem que ser digitada exata e em sequência. Qualquer dígito fora da ordem
    derruba a digitação de volta ao começo (e é engolido — um erro de digitação
    não pode acabar engatilhando um modo por acidente).

    Feedback na tira (LEDs numerados de 1 a 5, índices 0 a 4):
      - ao aceitar o 4, apaga tudo e acende só o LED 2 em roxo;
      - cada dígito correto seguinte acende o próximo (3, 4 e enfim o 5);
      - com os quatro certos, o LED 1 acende na cor do modo escolhido —
        verde RC, vermelho AUTO, laranja IDLE;
      - dois segundos depois, grava na NVS e reinicia o ESP.

    Enquanto está digitando, este objeto é dono exclusivo da tira (StatusLed::
    lock()), senão as animações do main e o painel de diagnóstico do AUTO
    repintariam por cima do roxo.

    Quem decide QUANDO a senha pode ser digitada é o main.cpp — só nas janelas
    de espera (seleção de modo no IDLE, pareamento do controle no RC, seleção de
    tática no AUTO). Fechada a janela, o main chama cancel().
*/
class BootModeSelector {
  public:
    /**
    @param compiledDefault Default de boot do firmware, repassado ao
           BootModeStore no momento da gravação (ver o carimbo lá).
    */
    void begin(RobotState compiledDefault);

    /**
    @brief Alimenta a FSM com o dígito IR do frame.
    @param digit 1 a 9, ou 0 se não veio nada (IRReader::digit()).
    @return true se o dígito foi consumido pela senha — o chamador deve então
            descartar os eventos semânticos do frame (IRReader::consumeEvent()).
            false quando o aperto não interessa à senha e segue o fluxo normal.
    */
    bool feed(int digit);

    /**
    @brief Desenha o progresso e dispara a gravação + reboot quando o tempo de
           confirmação vence. Chamar uma vez por loop().
    */
    void update(StatusLed &leds);

    /**
    @brief Aborta a digitação em curso e devolve a tira (a janela fechou).
           No-op depois da senha completa: aí o reboot já está contratado.
    */
    void cancel(StatusLed &leds);

    /**
    @brief true enquanto há digitação em curso — a tira está sob nosso controle.
    */
    bool isActive() const {
        return _progress > 0;
    }

  private:
    static constexpr int SEQUENCE_LENGTH = 4;
    static constexpr unsigned long CONFIRM_HOLD_MS = 2000;

    RobotState _compiledDefault = RobotState::IDLE;
    RobotState _pendingMode = RobotState::IDLE;
    int _progress = 0;        // dígitos corretos até agora (0 a 4)
    int _paintedProgress = -1; // último progresso desenhado na tira
    unsigned long _confirmedMs = 0;
    bool _owningLeds = false;

    void paint(StatusLed &leds);
    void release(StatusLed &leds);
    void commit();

    /**
    @brief Traduz o quarto dígito no modo correspondente.
    @return false se o dígito não fecha nenhuma senha válida.
    */
    static bool modeForFinalDigit(int digit, RobotState &out);
};
