#pragma once
#include "./RobotTypes.hpp"

// Fuego usa o MESMO mapa de pinos e thresholds do Fumacinha (mesma família de
// hardware, ver HardwareFamily.hpp) e, desde que a HUD tática virou o padrão dos
// dois, TAMBÉM a mesma estratégia: ActiveAuto = FumacinhaAuto no Config.hpp.
// Os dois robôs lutam igual, então repetir a lógica em duas classes só criava
// duas versões pra manter — a FuegoAuto ficou no repositório, mas fora do
// dispatch (ver Config.hpp).
//
// Se um valor de sensor precisar divergir entre os dois robôs, é aqui que se
// separa — este perfil é uma cópia deliberada, não um include do fumacinha.hpp,
// justamente pra permitir essa divergência sem afetar o outro robô.

namespace Config {

    static constexpr const char *ROBOT_NAME = "Fuego";

    // === Rádio =================================================================
    // Canal WiFi do AP de configuração (2.4GHz), espalhado entre os canais
    // não-sobrepostos (1/6/11) pra dois robôs em bancada não brigarem pelo mesmo
    // espectro na seleção de tática. Mantido em 11 de propósito — o Fumacinha
    // usa o 6, então os dois podem subir o AP ao mesmo tempo.
    static constexpr int WIFI_CHANNEL = 11;

    // MAC do controle Bluetooth "dono" — allowlist em tempo de compilação. Só
    // este controle pareia, eliminando cross-pairing quando dois robôs nossos
    // entram em RC ao mesmo tempo. Tudo-zero = modo descoberta (aceita o
    // primeiro controle e imprime o MAC no serial pra você fixar aqui).
    static constexpr uint8_t CONTROLLER_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static constexpr int RIGHT_POS_PIN = 16;
    static constexpr int RIGHT_NEG_PIN = 17;
    static constexpr int LEFT_POS_PIN = 19;
    static constexpr int LEFT_NEG_PIN = 18;

    // Coeficientes de pilotagem do RC — só afetam o RCMode, não o AUTO. Mantidos
    // nos valores do Fuego, que são do chassi dele.
    static constexpr int MAX_THROTTLE = 100;
    static constexpr int TURN_COEFFICIENT = 83;
    static constexpr int PIVOT_COEFFICIENT = 83;

    static constexpr int NUM_SERVOS = 0;

    // "JSumo": módulo composto emissor+receptor, com CI que inverte o sinal
    // (HIGH = alvo detectado). Os dois laterais têm o emissor (e esse CI)
    // desligável pelo NMOS de PIN_STEALTH_EMITTER, pro modo furtivo — negar ao
    // adversário o próprio sinal infravermelho que ele poderia usar pra nos achar.
    static constexpr int PIN_JSUMO_ESQ = 23;
    static constexpr int PIN_JSUMO_DIR = 27;
    // ATENÇÃO: o Fuego NÃO tem sensor frontal. Este pino é o mesmo do servo da
    // asa (PIN_SERVO_ASA, abaixo) — o JS40F frontal existe só porque a família
    // de hardware declara o membro, e lê o próprio sinal do servo. Ou seja,
    // HardwareCore::frontDetected() é lixo neste robô e a FuegoAuto nunca o
    // consulta: "alvo à frente" é os dois JSumo laterais acusando ao mesmo
    // tempo. Se um dia entrar um sensor frontal de verdade, ele precisa de um
    // pino próprio.
    static constexpr int PIN_JSUMO_ASA = 32;

    // "IR puro": receptor cru do MESMO módulo lateral, sem o CI (LOW = alvo
    // detectado). Fica sempre energizado, mesmo com o emissor desligado no
    // modo furtivo — serve de fallback de detecção lateral nesse caso.
    static constexpr int PIN_IR_ESQ = 5;
    static constexpr int PIN_IR_DIR = 4;

    static constexpr uint8_t PIN_LINHA_ESQ = 39;
    static constexpr uint8_t PIN_LINHA_DIR = 34;
    static constexpr uint16_t LINHA_THRESHOLD = 3800;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Sem servo de arma separado: o servo da asa (PIN_SERVO_ASA, abaixo)
    // acumula os dois papéis nesse robô — dois Servo independentes no mesmo
    // GPIO entrariam em conflito.
    static constexpr ServoConfig SERVOS[] = {};

    // Velocidade no motor esquerdo, velocidade no motor direito, tempo
    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

    // === Núcleo de Hardware ======================================================
    // Família HW_FAMILY_FUMACINHA (ver HardwareFamily.hpp): 5 IR + 2 linha + LDR
    // de rampa + emissor furtivo + servo de asa.
    static constexpr bool USES_HARDWARE_CORE = true;

    // Transistor low-side que chaveia o GND dos emissores IR laterais (modo furtivo).
    static constexpr int PIN_STEALTH_EMITTER = 25; // HIGH = emissores ligados

    // LDR de confirmação de rampa (divisor de tensão com resistor de 10k).
    static constexpr uint8_t PIN_LDR = 36;
    static constexpr uint16_t LDR_THRESHOLD = 200; // abaixo disso: oponente sobre a rampa
    static constexpr uint8_t LDR_FILTER_SIZE = 8;  // janela da média móvel

    // Servo da asa lateral — posições fundamentais.
    static constexpr int PIN_SERVO_ASA = 26;
    static constexpr int ASA_ANGLE_RETRACT = 93; // recolhida
    static constexpr int ASA_ANGLE_LEFT = 11;    // aberta para a esquerda
    static constexpr int ASA_ANGLE_RIGHT = 175;  // aberta para a direita

    // === Modo AUTO ==============================================================
    // true = HUD tática (aberturas, buscas, lado preferencial, diagnóstico de
    // bancada) + payload CombatProfile, o mesmo par que o Fumacinha usa. Esta
    // flag é lida em DOIS lugares e os dois têm que concordar: o ConfigServer
    // escolhe por ela qual página servir, e o AutoMode escolhe por ela qual
    // payload consumir. Ligar aqui só faz sentido com ActiveAuto = FumacinhaAuto
    // no Config.hpp, que é quem de fato lê o CombatProfile — com a FuegoAuto no
    // lugar, a HUD travaria a seleção sem ninguém do outro lado pra usá-la.
    static constexpr bool USES_FUMACINHA_FSM = true;

}
