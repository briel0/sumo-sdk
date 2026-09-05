/*
    Teste de sensores da Caipora — bancada, saida no monitor serial.

    Env dedicado (caipora_sensores no platformio.ini): sobe SEM WiFi, SEM
    Bluetooth, SEM maquina de estados e SEM motores. O barramento I2C fica so
    para os quatro VL53L0X, que e exatamente onde o reenderecamento por XSHUT
    costuma falhar e onde o painel web (que divide o loop com o servidor async)
    nao consegue te ajudar.

    Os pinos vem do proprio CaiporaAuto, entao o que este teste valida e o que o
    robo usa em combate — nao uma copia que envelhece sozinha.

        make sensores          (compila, grava e abre o monitor)
        pio run -e caipora_sensores -t upload && pio device monitor -b 115200
*/

#include "CaiporaAuto.hpp"
#include "LDR.hpp"
#include "ToFSensor.hpp"
#include <Arduino.h>
#include <Wire.h>

// Enderecos I2C definitivos, iguais aos do construtor de CaiporaAuto.
// (Se mudarem la, mudam aqui — vale hoistar para o header algum dia.)
static constexpr uint8_t ADDR_FRENTE_ESQ = 0x30;
static constexpr uint8_t ADDR_LATERAL_ESQ = 0x31;
static constexpr uint8_t ADDR_FRENTE_DIR = 0x32;
static constexpr uint8_t ADDR_LATERAL_DIR = 0x33;

static constexpr unsigned long PERIODO_MS = 200;
static constexpr int LINHAS_POR_CABECALHO = 20;

static LDR ldr(CaiporaAuto::PIN_LDR);
static ToFSensor vlFrenteEsq(CaiporaAuto::PIN_XSHUT_FRENTE_ESQ, ADDR_FRENTE_ESQ);
static ToFSensor vlLateralEsq(CaiporaAuto::PIN_XSHUT_LATERAL_ESQ, ADDR_LATERAL_ESQ);
static ToFSensor vlFrenteDir(CaiporaAuto::PIN_XSHUT_FRENTE_DIR, ADDR_FRENTE_DIR);
static ToFSensor vlLateralDir(CaiporaAuto::PIN_XSHUT_LATERAL_DIR, ADDR_LATERAL_DIR);

static bool okFrenteEsq = false;
static bool okLateralEsq = false;
static bool okFrenteDir = false;
static bool okLateralDir = false;

static int linhasImpressas = 0;

// Varre o barramento e lista quem responde. E o diagnostico mais direto do
// reenderecamento: depois do bring-up voce deve ver 0x30..0x33 e nenhum 0x29.
static void varrerI2C(const char *rotulo) {
    Serial.printf("  %s ", rotulo);
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

static void subirToF(const char *nome, int indice, int pinoXshut, uint8_t addr, ToFSensor &sensor, bool &destino) {
    Serial.printf("  [%d/4] %-12s xshut=%-3d addr=0x%02X ... ", indice, nome, pinoXshut, addr);
    destino = sensor.init();
    Serial.println(destino ? "OK" : "FALHOU");
}

// Imprime a distancia, ou ---- quando nao ha retorno (fora de alcance/timeout).
static void imprimirDistancia(bool ok, ToFSensor &sensor) {
    if(!ok) {
        Serial.print("   off");
        return;
    }
    uint16_t mm = sensor.leituraRaw();
    if(mm > 8000) {
        Serial.print("  ----");
    }
    else {
        Serial.printf(" %5u", mm);
    }
}

static void imprimirCabecalho() {
    Serial.println();
    Serial.println("   FrEsq  LatEsq   FrDir  LatDir |  LDR");
    Serial.println("  -------------------------------------");
    linhasImpressas = 0;
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=========================================================");
    Serial.println("  TESTE DE SENSORES - CAIPORA");
    Serial.println("=========================================================");

    Wire.begin();
    Serial.println("\n-- I2C --");
    varrerI2C("antes do bring-up:");

    // Todos desligados antes de subir um a um: com XSHUT solto os quatro
    // respondem em 0x29 ao mesmo tempo e o enderecamento se atropela.
    Serial.println("\n-- VL53L0X (XSHUT + reenderecamento) --");
    vlFrenteEsq.disable();
    vlLateralEsq.disable();
    vlFrenteDir.disable();
    vlLateralDir.disable();
    delay(20);

    subirToF("FRENTE_ESQ", 1, CaiporaAuto::PIN_XSHUT_FRENTE_ESQ, ADDR_FRENTE_ESQ, vlFrenteEsq, okFrenteEsq);
    subirToF("LATERAL_ESQ", 2, CaiporaAuto::PIN_XSHUT_LATERAL_ESQ, ADDR_LATERAL_ESQ, vlLateralEsq, okLateralEsq);
    subirToF("FRENTE_DIR", 3, CaiporaAuto::PIN_XSHUT_FRENTE_DIR, ADDR_FRENTE_DIR, vlFrenteDir, okFrenteDir);
    subirToF("LATERAL_DIR", 4, CaiporaAuto::PIN_XSHUT_LATERAL_DIR, ADDR_LATERAL_DIR, vlLateralDir, okLateralDir);

    Serial.println();
    varrerI2C("depois do bring-up:");
    Serial.println("  (esperado: 0x30 0x31 0x32 0x33, e nenhum 0x29)");

    Serial.println("\n-- Analogicos --");
    ldr.init();
    Serial.printf("  LDR         pino %d\n", CaiporaAuto::PIN_LDR);

    Serial.println("\nDistancias em mm; ---- = sem retorno; off = sensor nao subiu.");
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

    Serial.print(" ");
    imprimirDistancia(okFrenteEsq, vlFrenteEsq);
    imprimirDistancia(okLateralEsq, vlLateralEsq);
    imprimirDistancia(okFrenteDir, vlFrenteDir);
    imprimirDistancia(okLateralDir, vlLateralDir);

    Serial.printf(" | %4d\n", ldr.readRaw());

    linhasImpressas++;
}
