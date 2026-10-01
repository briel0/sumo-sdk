/*
    Teste de sensores da Arruela — bancada, saida no monitor serial.

    Env dedicado (arruela_sensores no platformio.ini): sobe SEM WiFi, SEM
    Bluetooth, SEM maquina de estados e SEM motores. Le os tres JS40F
    (digital), o VL53L0X unico (I2C, endereco padrao 0x29, sem XSHUT) e os
    dois QRE1113 de linha (analogico, pinos 26 e 32).

    Os pinos vem do proprio Config:: (perfil ativo via -DROBOT_ARRUELA, ver
    include/profiles/arruela.hpp), entao o que este teste valida e o que o
    robo usa em combate — nao uma copia que envelhece sozinha.

        make sensores-arruela
        pio run -e arruela_sensores -t upload && pio device monitor -b 115200
*/

#include "Config.hpp"
#include "JS40F.hpp"
#include "QRE1113.hpp"
#include "ToFSensor.hpp"
#include <Arduino.h>
#include <Wire.h>

static constexpr unsigned long PERIODO_MS = 200;
static constexpr int LINHAS_POR_CABECALHO = 20;

static JS40F sensorEsq(Config::PIN_JS_ESQ);
static JS40F sensorDir(Config::PIN_JS_DIR);
static JS40F sensorFrontal(Config::PIN_JS_FRONT);
// Mesmo construtor do ArruelaAuto: sem XSHUT, endereco padrao.
static ToFSensor sensorDistancia;
static QRE1113 linhaEsq(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ);
static QRE1113 linhaDir(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR);

static bool toFOk = false;
static int linhasImpressas = 0;

// Varre o barramento e lista quem responde: o esperado e so 0x29.
static void varrerI2C() {
    Serial.print("  Respondendo: ");
    int achados = 0;
    for(uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if(Wire.endTransmission() == 0) {
            Serial.printf("0x%02X ", addr);
            achados++;
        }
    }
    if(achados == 0) {
        Serial.print("(barramento vazio)");
    }
    Serial.println();
}

static void imprimirCabecalho() {
    Serial.println();
    Serial.println("   Esq   Dir  Front |  ToF mm |  LinEsq  LinDir");
    Serial.println("  ----------------------------------------------");
    linhasImpressas = 0;
}

void setup() {
    // Liga o transistor dos JS40F antes de qualquer outra coisa: quanto mais
    // cedo ele sobe, mais tempo de estabilização o sensor tem até a leitura.
    pinMode(Config::PIN_JS_POWER, OUTPUT);
    digitalWrite(Config::PIN_JS_POWER, HIGH);

    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=========================================================");
    Serial.println("  TESTE DE SENSORES - ARRUELA");
    Serial.println("=========================================================");

    Serial.println("\n-- JS40F (digital) --");
    Serial.printf("  Alimentação pino %d (transistor) -> HIGH\n", Config::PIN_JS_POWER);
    sensorEsq.init();
    sensorDir.init();
    sensorFrontal.init();
    Serial.printf("  Esquerda    pino %d\n", Config::PIN_JS_ESQ);
    Serial.printf("  Direita     pino %d\n", Config::PIN_JS_DIR);
    Serial.printf("  Frontal     pino %d\n", Config::PIN_JS_FRONT);

    Serial.println("\n-- VL53L0X (I2C) --");
    Wire.begin();
    varrerI2C();
    Serial.print("  Subindo em 0x29 ... ");
    toFOk = sensorDistancia.init();
    Serial.println(toFOk ? "OK" : "FALHOU");

    Serial.println("\n-- QRE1113 (linha, analogico) --");
    linhaEsq.init();
    linhaDir.init();
    Serial.printf("  Linha Esq   pino %d  threshold %u\n", Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ);
    Serial.printf("  Linha Dir   pino %d  threshold %u\n", Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR);

    Serial.println("\nJS40F: 1 = alvo detectado, 0 = livre. ToF: mm; off = sensor nao subiu.");
    Serial.println("Linha: valor ADC bruto (0-4095).");
    imprimirCabecalho();
}

void loop() {
    static unsigned long ultima = 0;
    unsigned long agora = millis();
    if(agora - ultima < PERIODO_MS) {
        return;
    }
    ultima = agora;

    if(linhasImpressas >= LINHAS_POR_CABECALHO) {
        imprimirCabecalho();
    }

    Serial.printf("    %d     %d     %d  |", sensorEsq.temAlvo() ? 1 : 0, sensorDir.temAlvo() ? 1 : 0,
                  sensorFrontal.temAlvo() ? 1 : 0);
    if(toFOk) {
        Serial.printf("  %5u  |", sensorDistancia.leituraRaw());
    }
    else {
        Serial.print("    off  |");
    }
    Serial.printf("   %4u    %4u\n", linhaEsq.leituraRaw(), linhaDir.leituraRaw());

    linhasImpressas++;
}
