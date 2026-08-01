#include "ToFSensor.hpp"

ToFSensor::ToFSensor(int8_t xshutPin, uint8_t address) : _pinXshut(xshutPin), _address(address) {}

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

    // 5. Entra em modo contínuo sem intervalo (0) entre as leituras
    _sensor.startContinuous();

    return true;
}

bool ToFSensor::temOponente(uint16_t thresholdMm) {
    uint16_t dist = leituraRaw();

    // Filtra erros de leitura e timeouts para não gerar "falsos positivos" de ataque
    if(_sensor.timeoutOccurred() || dist > 8000) {
        return false;
    }

    return dist < thresholdMm;
}

uint16_t ToFSensor::leituraRaw() {
    return _sensor.readRangeContinuousMillimeters();
}