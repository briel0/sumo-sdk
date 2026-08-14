#pragma once
#include "./RobotTypes.hpp"

namespace Config {

    static constexpr const char *ROBOT_NAME = "Fumacinha";

    // === Rádio =================================================================
    // Canal WiFi do AP de configuração (2.4GHz), espalhado entre os canais
    // não-sobrepostos (1/6/11) pra dois robôs em bancada não brigarem pelo mesmo
    // espectro na seleção de tática. O AP morre na largada (ConfigServer::
    // shutdown()), então isso só importa durante a configuração.
    static constexpr int WIFI_CHANNEL = 6;

    // MAC do controle Bluetooth "dono" — allowlist em tempo de compilação. Só
    // este controle pareia, eliminando cross-pairing quando dois robôs nossos
    // entram em RC ao mesmo tempo. Tudo-zero = modo descoberta (aceita o
    // primeiro controle e imprime o MAC no serial pra você fixar aqui).
    static constexpr uint8_t CONTROLLER_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static constexpr int RIGHT_POS_PIN = 18;
    static constexpr int RIGHT_NEG_PIN = 19;
    static constexpr int LEFT_POS_PIN = 17;
    static constexpr int LEFT_NEG_PIN = 16;

    static constexpr int MAX_THROTTLE = 100;
    static constexpr int TURN_COEFFICIENT = 83;
    static constexpr int PIVOT_COEFFICIENT = 83;

    static constexpr int NUM_SERVOS = 0;

    // "JSumo": módulo composto emissor+receptor, com CI que inverte o sinal
    // (HIGH = alvo detectado). Os dois laterais têm o emissor (e esse CI)
    // desligável pelo NMOS de PIN_STEALTH_EMITTER, pro modo furtivo — negar ao
    // adversário o próprio sinal infravermelho que ele poderia usar pra nos achar.
    static constexpr int PIN_JSUMO_ESQ = 23;
    static constexpr int PIN_JSUMO_DIR = 14;
    static constexpr int PIN_JSUMO_ASA =
        32; // frente — emissor nunca é desligado, sempre confiável — teste: trocado com PIN_SERVO_ASA

    // "IR puro": receptor cru do MESMO módulo lateral, sem o CI (LOW = alvo
    // detectado). Fica sempre energizado, mesmo com o emissor desligado no
    // modo furtivo — serve de fallback de detecção lateral nesse caso.
    static constexpr int PIN_IR_ESQ = 5;
    static constexpr int PIN_IR_DIR = 4;

    // GPIO 23 pedido pra linha esquerda não tem ADC no ESP32 (QRE1113 exige
    // analogRead) — realocado pro 39 (ADC1, livre, sem o problema de ADC2+WiFi).
    static constexpr uint8_t PIN_LINHA_ESQ = 39;
    // GPIO27 é ADC2, que fica instável com o WiFi ativo — e ConfigServer::shutdown()
    // não desliga o rádio de verdade, então isso valeria pra luta inteira, não só
    // pra bancada. Realocado pro 5 (ADC1, livre).
    static constexpr uint8_t PIN_LINHA_DIR = 34;
    static constexpr uint16_t LINHA_THRESHOLD = 3800;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Sem servo de arma separado: o servo da asa (PIN_SERVO_ASA, abaixo)
    // acumula os dois papéis nesse robô — dois Servo independentes no mesmo
    // GPIO entrariam em conflito.
    static constexpr ServoConfig SERVOS[] = {};

    // Velocidade no motor direito, velocidade no motor esquerdo, tempo
    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

    // === Núcleo de Hardware ======================================================
    // Inicializa o HardwareCore (sensores + periféricos de estado) quando este robô entra em AUTO.
    // Fumacinha_FSM depende diretamente do HardwareCore; FumacinhaMode::init() o
    // inicializa incondicionalmente, sem consultar esta flag (ela existe aqui só
    // por completude/documentação — quem realmente consulta é o AutoMode legado).
    static constexpr bool USES_HARDWARE_CORE = true;

    // Transistor low-side que chaveia o GND dos emissores IR laterais (modo furtivo).
    static constexpr int PIN_STEALTH_EMITTER = 25; // HIGH = emissores ligados

    // LDR de confirmação de rampa (divisor de tensão com resistor de 10k).
    // GPIO22 pedido não tem ADC — realocado pro 34, ADC1 livre. Com isso, nenhum
    // sensor analógico do Fumacinha depende mais de ADC2 (ver PIN_LINHA_DIR).
    static constexpr uint8_t PIN_LDR = 36;
    static constexpr uint16_t LDR_THRESHOLD = 200; // abaixo disso: oponente sobre a rampa
    static constexpr uint8_t LDR_FILTER_SIZE = 8;  // janela da média móvel

    // Servo da asa lateral — posições fundamentais.
    static constexpr int PIN_SERVO_ASA = 26;     // teste: trocado com PIN_JSUMO_ASA
    static constexpr int ASA_ANGLE_RETRACT = 90; // recolhida
    static constexpr int ASA_ANGLE_LEFT = 0;     // aberta para a esquerda
    static constexpr int ASA_ANGLE_RIGHT = 180;  // aberta para a direita

    // === Fumacinha_FSM (modo AUTO) ==============================================
    // Habilita o FumacinhaMode como ActiveAutoMode (ver dispatch em Config.hpp) e
    // faz o ConfigServer servir a HUD de abertura/busca/ataque em vez da legada.
    static constexpr bool USES_FUMACINHA_FSM = true;

}
