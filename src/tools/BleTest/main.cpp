// Teste isolado: exemplo oficial WebAppBLEService do Bluepad32, sem NADA do
// nosso projeto (sem I2C, sem AutoMode, sem Receiver). Objetivo: descobrir se
// o problema de "não aparece em nenhum scanner BLE" é do nosso código ou da
// combinação Bluepad32+BLE nesse chip/firmware, independente de nós.
//
// Adaptado do .ino original em
// libraries/Bluepad32_ESP32/examples/WebAppBLEService/WebAppBLEService.ino
// (mesmo pacote framework-arduinoespressif32 do projeto) — só virou .cpp e
// perdeu a lógica de gamepad, que não interessa pra esse teste.
//
// pio run -e ble_test -t upload -t monitor

#include <Arduino.h>
#include <Bluepad32.h>
#include "ble_server.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t *addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %02X:%02X:%02X:%02X:%02X:%02X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    BP32.setup([](ControllerPtr) {}, [](ControllerPtr) {});
    BP32.forgetBluetoothKeys();
    BP32.enableNewBluetoothConnections(false);

    BLE_SERVER_SETUP();
    Serial.println("[BLE_TEST] Advertising 'Delayed' iniciado. Procura no nRF Connect.");
}

void loop() {
    BP32.update();
    delay(150);
}
