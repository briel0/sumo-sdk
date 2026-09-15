#pragma once
#include <Preferences.h>

/**
    @class MotorPolarity
    @brief Corrige motor invertido / lados trocados sem nunca tocar nos pinos
           do Drive — persiste um transform pequeno (inverter esquerda,
           inverter direita, trocar os lados) na NVS.

    Por que não guarda os pinos: Drive é um objeto global, construído antes do
    setup() (e portanto antes da NVS estar garantidamente pronta) — não dá pra
    ler pino salvo a tempo de bindar o MCPWM com ele. Guardar só o transform e
    aplicar em cima do leftSpeed/rightSpeed que já vai pro Drive::setSpeed()
    existente evita esse problema inteiro: os 4 GPIOs continuam exatamente os
    fixos do profile, sempre.

    Por que sobrevive a reflash AUTO<->RC: todos os envs do platformio.ini usam
    o partition table padrão do esp32dev (nenhum define board_build.partitions
    próprio) — a partição NVS fica no mesmo offset/tamanho em qualquer
    firmware, e o upload normal (`pio run -t upload`) não mexe nela, só
    reescreve bootloader+partições+app. Só um `erase_flash` completo apaga.
*/
class MotorPolarity {
  public:
    static constexpr uint8_t INVERT_LEFT = 1 << 0;
    static constexpr uint8_t INVERT_RIGHT = 1 << 1;
    static constexpr uint8_t SWAP_SIDES = 1 << 2;
    static constexpr uint8_t NUM_CONFIGS = 8; // 3 bits independentes

    // Abre a NVS (namespace "motorpol") e carrega o índice salvo. Precisa
    // rodar depois que o framework Arduino inicializou (ver comentário no
    // .cpp sobre o main.cpp global Drive) — chame no começo do setup(), não
    // em construtor de objeto global.
    void init();

    // Aplica o transform atual em cima de um comando de velocidade.
    void apply(int &leftSpeed, int &rightSpeed) const;

    // Avança pro próximo índice (0..7, dá a volta) e persiste na NVS.
    uint8_t cycle();

    uint8_t current() const {
        return _index;
    }

  private:
    Preferences _prefs;
    uint8_t _index = 0;
};
