#include "IRreader.hpp"
#include <IRremote.hpp>

void IRReader::init(int pin) {
    IrReceiver.begin(pin, DISABLE_LED_FEEDBACK);
}

void IRReader::update() {
    _modeRC = false;
    _modeAuto = false;
    _start = false;
    _stop = false;

    if(!IrReceiver.decode())
        return;

    uint32_t code = IrReceiver.decodedIRData.decodedRawData;
    IrReceiver.resume();

    Serial.printf("[IR] Código: 0x%X\n", code);

    if(code == 0x87) {
        _modeAuto = true;
    }
    if(code == 0x88) {
        _modeRC = true;
    }
    if(code == 0x81) {
        _start = true;
    }
    if(code == 0x82) {
        _stop = true;
    }
}

void IRReader::shutdown() {
    IrReceiver.stop();
}