#include "IRreader.hpp"
#include <IRremote.hpp>

void IRReader::init(int pin) {
    IrReceiver.begin(pin, DISABLE_LED_FEEDBACK);
}

void IRReader::update() {
    _modeRC = false;
    _modeAuto = false;
    _ready = false;
    _start = false;
    _stop = false;

    if(!IrReceiver.decode())
        return;

    uint32_t code = IrReceiver.decodedIRData.decodedRawData;
    IrReceiver.resume();

    Serial.printf("[IR] Código: 0x%X\n", code);

    if(code == 0x80) { // Botão 1: ready
        _ready = true;
    }
    if(code == 0x81) { // Botão 2: largada
        _start = true;
    }
    if(code == 0x82) { // Botão 3: stop
        _stop = true;
    }
    if(code == 0x87) { // Botão 8: modo auto
        _modeAuto = true;
    }
    if(code == 0x88) { // Botão 9: modo RC
        _modeRC = true;
    }
}

void IRReader::shutdown() {
    IrReceiver.stop();
}