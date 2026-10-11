/*
    Teste de sensores do Marola — bancada, saida no monitor serial.

    Env dedicado (marola_sensores no platformio.ini): sobe SEM WiFi, SEM
    Bluetooth, SEM maquina de estados e SEM motores. Sem VL53L0X e sem JS40F
    aqui — o Marola usa so tres LDR (analogico) e dois QRE1113 (linha,
    analogico) — entao nao ha bring-up de I2C, so leitura direta dos pinos.

    Os pinos vem do proprio Config:: (perfil ativo via -DROBOT_MAROLA, ver
    include/profiles/marola.hpp), entao o que este teste valida e o que o
    robo usa em combate — nao uma copia que envelhece sozinha.

        make sensores-marola
        pio run -e marola_sensores -t upload && pio device monitor -b 115200
*/

#include "Config.hpp"
#include "LDR.hpp"
#include "QRE1113.hpp"
#include <Arduino.h>

static constexpr unsigned long PERIODO_MS = 200;
static constexpr int LINHAS_POR_CABECALHO = 20;

static LDR ldrEsq(Config::PIN_LDR_ESQ);
static LDR ldrDir(Config::PIN_LDR_DIR);
static LDR ldrFrontal(Config::PIN_LDR_FRONT);
static QRE1113 linhaEsq(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ);
static QRE1113 linhaDir(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR);

static int linhasImpressas = 0;

static void imprimirCabecalho() {
    Serial.println();
    Serial.println("  LDREsq  LDRDir LDRFront |  LinEsq  LinDir");
    Serial.println("  -----------------------------------------");
    linhasImpressas = 0;
}

void setup() {
    // Liga o transistor de alimentação dos sensores antes de qualquer outra
    // coisa: quanto mais cedo ele sobe, mais tempo de estabilização o sensor
    // tem até a leitura. (Mesma placa do Arruela, por isso PIN_JS_POWER.)
    pinMode(Config::PIN_JS_POWER, OUTPUT);
    digitalWrite(Config::PIN_JS_POWER, HIGH);

    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=========================================================");
    Serial.println("  TESTE DE SENSORES - MAROLA");
    Serial.println("=========================================================");

    Serial.println("\n-- LDR (analogico) --");
    Serial.printf("  Alimentação pino %d (transistor) -> HIGH\n", Config::PIN_JS_POWER);
    ldrEsq.init();
    ldrDir.init();
    ldrFrontal.init();
    Serial.printf("  Esquerda    pino %d\n", Config::PIN_LDR_ESQ);
    Serial.printf("  Direita     pino %d\n", Config::PIN_LDR_DIR);
    Serial.printf("  Frontal     pino %d\n", Config::PIN_LDR_FRONT);

    Serial.println("\n-- QRE1113 (linha, analogico) --");
    linhaEsq.init();
    linhaDir.init();
    Serial.printf("  Linha Esq   pino %d  threshold %u\n", Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ);
    Serial.printf("  Linha Dir   pino %d  threshold %u\n", Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR);

    Serial.println("\nLDR e Linha: valor ADC bruto (0-4095).");
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

    Serial.printf("   %4d    %4d    %4d   |   %4u    %4u\n", ldrEsq.readRaw(), ldrDir.readRaw(), ldrFrontal.readRaw(),
                  linhaEsq.leituraRaw(), linhaDir.leituraRaw());

    linhasImpressas++;
}
