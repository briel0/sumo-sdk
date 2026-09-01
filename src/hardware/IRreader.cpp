#include "IRreader.hpp"
#include <Arduino.h>
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
    _digit = 0;

    if(!IrReceiver.decode())
        return;

    uint32_t code = IrReceiver.decodedIRData.decodedRawData;
    uint16_t address = IrReceiver.decodedIRData.address;
    uint8_t command = IrReceiver.decodedIRData.command;
    decode_type_t protocolo = IrReceiver.decodedIRData.protocol;
    bool repeticao = (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) != 0;

    // Log completo de propósito: quando o controle "para de funcionar", é este
    // print que diz se o problema é não chegar nada (nenhuma linha), decodificar
    // como outro protocolo, ou vir com endereço/comando diferentes do esperado.
    // dt = ms desde a trama anterior, pra dar pra ver a estrutura do surto.
    unsigned long agora = millis();
    Serial.printf("[IR] %s addr=0x%X cmd=0x%X raw=0x%lX rep=%d dt=%lu\n", IrReceiver.getProtocolString(), address,
                  command, (unsigned long)code, repeticao ? 1 : 0, agora - _lastFrameMs);
    _lastFrameMs = agora;

    IrReceiver.resume();

    // Número do botão apertado (1 a 9), ou 0 se o aperto não interessa.
    //
    // Cada toque no controle do juiz gera um SURTO de quatro tramas: três em
    // NEC/NEC2 (com endereços e comandos que não têm relação com a tecla) e uma
    // em RC5 que carrega a tecla de verdade. Só a RC5 vale; as outras são
    // ignoradas de propósito. No teclado RC5 o dígito N manda o comando N, daí
    // a identidade.
    int botao = 0;
    if(protocolo == RC5 && command >= 1 && command <= 9) {
        botao = command;
    }
    else if(code >= 0x80 && code <= 0x88) {
        // Fallback pro empacotamento cru antigo, caso apareça um controle que
        // ainda entregue os valores que estavam escritos direto no código.
        botao = (int)(code - 0x80) + 1;
    }

    if(botao == 0) {
        return;
    }

    // Eventos semânticos: sem filtro de repetição, como sempre foi. São
    // idempotentes (ready/start) ou disparam reinício (stop), então a trama
    // repetida do SIRC não muda nada.
    if(botao == 1) { // Botão 1: ready
        _ready = true;
    }
    if(botao == 2) { // Botão 2: largada
        _start = true;
    }
    if(botao == 3) { // Botão 3: stop
        _stop = true;
    }
    if(botao == 8) { // Botão 8: modo auto
        _modeAuto = true;
    }
    if(botao == 9) { // Botão 9: modo RC
        _modeRC = true;
    }

    // Dígito: aqui a repetição IMPORTA (um toque não pode virar dois dígitos da
    // senha), e o filtro é SÓ por tempo. Não dá pra usar a flag de repetição da
    // lib: a trama RC5 chega colada nas NEC do mesmo surto, então ela vem
    // marcada rep=1 mesmo num toque novo — confiar na flag descartaria
    // justamente a única trama que interessa.
    bool ecoDoMesmoToque = (botao == _lastDigit && (agora - _lastDigitMs) < DIGIT_DEBOUNCE_MS);
    if(!ecoDoMesmoToque) {
        _digit = botao;
    }
    _lastDigit = botao;
    _lastDigitMs = agora; // tecla presa continua sendo o mesmo toque
}

void IRReader::consumeEvent() {
    _modeRC = false;
    _modeAuto = false;
    _ready = false;
    _start = false;
    _stop = false;
    _digit = 0;
}

void IRReader::shutdown() {
    IrReceiver.stop();
}
