#include "ToFSensor.hpp"

ToFSensor::ToFSensor(int8_t xshutPin, uint8_t address, float signalRateLimitMcps)
    : _pinXshut(xshutPin), _address(address), _signalRateLimitMcps(signalRateLimitMcps) {}

void ToFSensor::disable() {
    if(_pinXshut >= 0) {
        pinMode(_pinXshut, OUTPUT);
        digitalWrite(_pinXshut, LOW);
    }
}

bool ToFSensor::init() {
    // 1. Hardware Reset (Garante que o sensor acorde limpo e pronto para aceitar endereço)
    if(_pinXshut >= 0) {
        pinMode(_pinXshut, OUTPUT);
        digitalWrite(_pinXshut, LOW);
        delay(10);
        digitalWrite(_pinXshut, HIGH);
        delay(10);
    }

    // 2. Configura o endereço I2C da placa e o timeout para não travar o ESP32
    _sensor.setAddress(_address);
    _sensor.setTimeout(500);

    // 3. Tenta inicializar a comunicação
    if(!_sensor.init()) {
        Serial.printf("[ERRO] VL53L0X nao detectado no I2C: 0x%02X\n", _address);
        return false;
    }

    // 4. Overclock Tático: Reduz de 33ms (padrão) para 20ms o tempo de resposta
    _sensor.setMeasurementTimingBudget(20000);

    // 5. Sem ROI de hardware no VL53L0X: eleva o limite mínimo de sinal de retorno pra
    // descartar reflexos fracos/oblíquos (fora do eixo do sensor), aproximando o efeito
    // de um cone de detecção mais estreito.
    _sensor.setSignalRateLimit(_signalRateLimitMcps);

    // 6. Entra em modo contínuo sem intervalo (0) entre as leituras
    _sensor.startContinuous();

    return true;
}

bool ToFSensor::setSignalRateLimit(float limitMcps) {
    _signalRateLimitMcps = limitMcps;
    return _sensor.setSignalRateLimit(limitMcps);
}

bool ToFSensor::temOponente(uint16_t thresholdMm) {
    unsigned long agora = millis();
    uint16_t dist = leituraRaw();

    // Filtra timeout, medição inválida (LEITURA_INVALIDA_MM) e "sem alvo" do
    // sensor (~8190) para não gerar "falsos positivos" de ataque
    if(dist > 8000) {
        return false;
    }

    // Só aceita a leitura se veio de um ciclo de medição novo o bastante desde a
    // última aceita — em vez de reler RESULT_RANGE_STATUS à parte da distância
    // (que corre risco de pegar o ciclo seguinte em modo contínuo sem intervalo).
    if(agora - _ultimaLeituraMs < INTERVALO_MIN_MS) {
        return false;
    }
    _ultimaLeituraMs = agora;

    return dist < thresholdMm;
}

uint16_t ToFSensor::leituraRaw() {
    // Espera a amostra nova do modo contínuo, igual ao
    // readRangeContinuousMillimeters() da lib.
    unsigned long inicio = millis();
    while((_sensor.readReg(VL53L0X::RESULT_INTERRUPT_STATUS) & 0x07) == 0) {
        if(millis() - inicio > _sensor.getTimeout()) {
            _ultimoStatus = STATUS_TIMEOUT;
            return LEITURA_INVALIDA_MM;
        }
    }

    // Status e distância saem do mesmo bloco de resultado, numa transação I2C
    // só e antes de limpar a interrupção — os dois são garantidamente do mesmo
    // ciclo. A lib lê só os 2 bytes da distância (offset 10) e ignora o status.
    uint8_t resultado[12];
    _sensor.readMulti(VL53L0X::RESULT_RANGE_STATUS, resultado, sizeof(resultado));
    _sensor.writeReg(VL53L0X::SYSTEM_INTERRUPT_CLEAR, 0x01);

    if(_sensor.last_status != 0) {
        _ultimoStatus = STATUS_TIMEOUT;
        return LEITURA_INVALIDA_MM;
    }

    _ultimoStatus = (resultado[0] & 0x78) >> 3;
    if(_ultimoStatus != STATUS_VALIDO) {
        return LEITURA_INVALIDA_MM;
    }

    return ((uint16_t)resultado[10] << 8) | resultado[11];
}