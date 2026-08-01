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
     */
    explicit ToFSensor(int8_t xshutPin = -1, uint8_t address = 0x29);

    /**
     * @brief Inicializa o pino XSHUT, define o endereço I2C e configura a velocidade.
     * @return true se o sensor respondeu corretamente no barramento, false se falhou.
     */
    bool init();

    /**
     * @brief Retorna true se o oponente foi detectado dentro da zona de combate.
     * @param thresholdMm Distância máxima em milímetros (ex: 400mm = 40cm).
     */
    bool temOponente(uint16_t thresholdMm = 400);

    /**
     * @brief Retorna a distância pura lida em milímetros.
     */
    uint16_t leituraRaw();

  private:
    VL53L0X _sensor;
    int8_t _pinXshut;
    uint8_t _address;
};