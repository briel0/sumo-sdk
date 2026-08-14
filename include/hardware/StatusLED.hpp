#pragma once

#include <FastLED.h>

/**
    @class StatusLed
    @brief Manages the onboard debug LED and the addressable LED strip for boot status indication.

    On init, all addressable LEDs are set to red.
    Each call to confirmStep() turns the next LED green, indicating a successful setup stage.
*/
class StatusLed {
  public:
    /**
    @brief Initializes the debug LED pin and the FastLED strip.
    @param debugPin GPIO pin of the single debug LED (LED_BUILTIN).
    @param dataPin  GPIO pin of the addressable LED strip data line.
    @param numLeds  Number of LEDs in the addressable strip.
    */
    void init(int debugPin, int dataPin, int numLeds);

    /**
    @brief Turns the next LED in the strip from red to green, confirming a setup step passed.
           Blinks the debug LED once as a pulse.
    */
    void confirmStep();

    /**
    @brief Sets all addressable LEDs to a solid color. Useful for error indication.
    @param color CRGB color to apply.
    */
    void setAll(CRGB color);

    /**
    @brief Blinks the debug LED a given number of times. Blocking.
    @param times     Number of blinks.
    @param intervalMs Duration of each on/off phase in milliseconds.
    */
    void blinkDebug(int times, int intervalMs);

    /**
    @brief Sets all addressable LEDs to a solid color to reflect the current robot state.
        Safe to call repeatedly in loop(). Idempotent.
    @param color CRGB color to apply.
    */
    void setState(CRGB color);

    /**
    @brief Non-blocking heartbeat pulse on the debug LED. Call once per loop() iteration.
        Blinks slowly to indicate the robot is alive and waiting.
    */
    void heartbeat();

    /**
    @brief Non-blocking wave animation: green base with one LED pulsing orange.
        Call once per loop() iteration while waiting for controller pairing.
    */
    void pairingWave();

    /**
    @brief Non-blocking ping-pong wave animation on LEDs 1–4 (indexes 1 to 4).
        Green base with one LED traveling back and forth in orange.
        Call once per loop() iteration while waiting for strategy configuration.
    */
    void strategyWave();

    /**
    @brief Define a cor de um LED específico sem enviar pro hardware ainda.
        Use push() depois de uma ou mais chamadas pra aplicar as mudanças de
        uma vez. Existe pra permitir controle individual por índice (ex: painel
        de diagnóstico) sem duplicar o addLeds() do FastLED — só pode haver um
        registro por pino físico.
    @param index Índice do LED (0 a numLeds-1 passado em init()). Fora da faixa: no-op.
    @param color Cor a aplicar.
    */
    void setLedRaw(int index, CRGB color);

    /**
    @brief Envia o buffer atual pra tira física. Chame depois de setLedRaw().
    */
    void push();

  private:
    int _debugPin = 0;
    int _numLeds = 0;
    int _currentStep = 0;
    unsigned long _lastHeartbeatMs = 0;
    unsigned long _lastWaveMs = 0;
    int _waveIndex = 0;
    bool _pingpongForward = true;

    // FastLED exige array estático de tamanho conhecido em compile time.
    // O limite de 8 cobre qualquer configuração razoável do projeto.
    static constexpr int MAX_LEDS = 8;
    CRGB _leds[MAX_LEDS];
};