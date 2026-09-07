/*
    Teste de sensores do Smoker — bancada, saida no monitor serial.

    Env dedicado (smoker_sensores no platformio.ini): sobe SEM WiFi, SEM
    Bluetooth, SEM maquina de estados e SEM motores. Sem VL53L0X aqui — o
    Smoker usa so JS40F (digital, arco frontal) e QRE1113 (linha, analogico)
    — entao nao ha bring-up de I2C, so leitura direta dos pinos.

    Os pinos vem do proprio Config:: (perfil ativo via -DROBOT_SMOKER, ver
    include/profiles/smoker.hpp), entao o que este teste valida e o que o
    robo usa em combate — nao uma copia que envelhece sozinha.

        pio run -e smoker_sensores -t upload && pio device monitor -b 115200
*/

#include "Config.hpp"
#include "JS40F.hpp"
#include "QRE1113.hpp"
#include <Arduino.h>

static constexpr unsigned long PERIODO_MS = 200;
static constexpr int LINHAS_POR_CABECALHO = 20;

static JS40F sensorEsq(Config::PIN_JS_ESQ);
static JS40F sensorDir(Config::PIN_JS_DIR);
static JS40F sensorFrontal(Config::PIN_JS_FRONT);
static QRE1113 linhaEsq(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ);
static QRE1113 linhaDir(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR);

static int linhasImpressas = 0;

static void imprimirCabecalho() {
    Serial.println();
    Serial.println("   Esq   Dir  Front |  LinEsq  LinDir");
    Serial.println("  -------------------------------------");
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
    Serial.println("  TESTE DE SENSORES - SMOKER");
    Serial.println("=========================================================");

    Serial.println("\n-- JS40F (digital, arco frontal) --");
    Serial.printf("  Alimentação pino %d (transistor) -> HIGH\n", Config::PIN_JS_POWER);
    sensorEsq.init();
    sensorDir.init();
    sensorFrontal.init();
    Serial.printf("  Esquerda    pino %d\n", Config::PIN_JS_ESQ);
    Serial.printf("  Direita     pino %d\n", Config::PIN_JS_DIR);
    Serial.printf("  Frontal     pino %d\n", Config::PIN_JS_FRONT);

    Serial.println("\n-- QRE1113 (linha, analogico) --");
    linhaEsq.init();
    linhaDir.init();
    Serial.printf("  Linha Esq   pino %d  threshold %u\n", Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ);
    Serial.printf("  Linha Dir   pino %d  threshold %u\n", Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR);

    Serial.println("\nJS40F: 1 = alvo detectado, 0 = livre. Linha: valor ADC bruto (0-4095).");
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

    Serial.printf("    %d     %d     %d  |   %4u    %4u\n", sensorEsq.temAlvo() ? 1 : 0, sensorDir.temAlvo() ? 1 : 0,
                  sensorFrontal.temAlvo() ? 1 : 0, linhaEsq.leituraRaw(), linhaDir.leituraRaw());

    linhasImpressas++;
}
