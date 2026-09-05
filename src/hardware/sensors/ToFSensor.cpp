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

uint8_t ToFSensor::statusBruto() {
    return (_sensor.readReg(VL53L0X::RESULT_RANGE_STATUS) >> 3) & 0x0F;
}

bool ToFSensor::temOponente(uint16_t thresholdMm) {
    uint16_t dist = leituraRaw();

    // Filtra erros de leitura e timeouts para não gerar "falsos positivos" de ataque
    if(_sensor.timeoutOccurred() || dist > 8000) {
        return false;
    }

    // Lê o status logo após a distância: RESULT_RANGE_STATUS fica travado até o próximo
    // SYSRANGE_START, mas em modo contínuo sem intervalo a próxima leitura já começa
    // durante essa checagem, então status e distância podem, raramente, vir de ciclos
    // adjacentes em vez do mesmo ciclo.
    if(statusBruto() != RANGE_STATUS_VALIDO) {
        return false;
    }

    return dist < thresholdMm;
}

uint16_t ToFSensor::leituraRaw() {
    return _sensor.readRangeContinuousMillimeters();
}