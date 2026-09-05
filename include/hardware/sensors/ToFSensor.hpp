#pragma once
#include <Arduino.h>
#include <VL53L0X.h>
#include <Wire.h>

/**
 * @class ToFSensor
 * @brief Driver encapsulado para o sensor de distância VL53L0X (Time-of-Flight).
 *
 * Utiliza a biblioteca Pololu VL53L0X. O driver injeta o modo "High Speed"
 * diminuindo o tempo de leitura para 20ms e usa leitura contínua, ideal
 * para rastreamento agressivo em combate.
 */
class ToFSensor {
  public:
    /**
     * @brief Construtor do sensor ToF.
     * @param xshutPin Pino XSHUT para desligar o sensor. Essencial se houver múltiplos sensores no mesmo barramento
     * I2C. (Use -1 se não houver).
     * @param address Endereço I2C customizado (padrão é 0x29).
     * @param signalRateLimitMcps Taxa mínima de sinal de retorno (Mcps) aceita como leitura válida. O VL53L0X não
     * tem ROI real de hardware; subir esse valor descarta reflexos fracos/oblíquos (fora do eixo do sensor) e
     * aproxima o efeito de um cone de detecção mais estreito. Padrão da lib Pololu é 0.25.
     */
    explicit ToFSensor(int8_t xshutPin = -1, uint8_t address = 0x29, float signalRateLimitMcps = 0.25f);

    /**
     * @brief Desliga o sensor (coloca o pino XSHUT em LOW).
     */
    void disable();

    /**
     * @brief Inicializa o pino XSHUT, define o endereço I2C e configura a velocidade.
     * @return true se o sensor respondeu corretamente no barramento, false se falhou.
     */
    bool init();

    /**
     * @brief Retorna true se o oponente foi detectado dentro da zona de combate.
     * @param thresholdMm Distância máxima em milímetros (ex: 400mm = 40cm).
     *
     * Só aceita uma leitura como válida se pelo menos INTERVALO_MIN_MS tiver
     * passado desde a última aceita — garante que veio de um ciclo de medição
     * novo, sem depender de reler RESULT_RANGE_STATUS separado da distância
     * (que corre risco de pegar o ciclo seguinte em modo contínuo sem intervalo).
     */
    bool temOponente(uint16_t thresholdMm = 400);

    /**
     * @brief Retorna a distância pura lida em milímetros.
     */
    uint16_t leituraRaw();

    /**
     * @brief Ajusta em runtime a taxa mínima de sinal de retorno aceita (Mcps).
     * @return false se limitMcps estiver fora da faixa aceita pelo sensor (0 a 511.99).
     */
    bool setSignalRateLimit(float limitMcps);

  private:
    // > orçamento de medição (20000us, ver init()): garante que passou tempo
    // suficiente pra um ciclo de medição completo novo antes de confiar na leitura.
    static constexpr unsigned long INTERVALO_MIN_MS = 25;

    VL53L0X _sensor;
    int8_t _pinXshut;
    uint8_t _address;
    float _signalRateLimitMcps;
    unsigned long _ultimaLeituraMs = 0;
};