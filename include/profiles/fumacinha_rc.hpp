#pragma once
#include "./RobotTypes.hpp"

/*
    Perfil do Fumacinha RC — placa ESP32-C3 (supermini), build somente-RC.

    Diferente dos outros perfis (ESP32 clássico): este roda num SoC RISC-V que
    NÃO tem o periférico MCPWM. O Drive usa a implementação LEDC (ver o ramo
    CONFIG_IDF_TARGET_ESP32C3 em Drive.cpp). Não há modo AUTO, sensores, WiFi,
    IR nem tira de LED aqui — só a pilha de RC (Bluepad32 + Drive + servos),
    selecionada pelo build_src_filter do env fumacinha_rc no platformio.ini.

    Só 6 GPIOs usadas (as que você fisicamente ligou):
      Ponte H: 5, 6, 7, 8   |   Servos: 20, 21
    Lembrando que o C3 tem exatamente 6 canais LEDC — 4 motores + 2 servos
    ocupam todos, sem folga.
*/
namespace Config {

    static constexpr const char *ROBOT_NAME = "FumacinhaRC";

    // MAC do controle Bluetooth "dono" — allowlist em tempo de compilação (mesmo
    // esquema dos outros perfis). No C3 o controle PRECISA ser BLE (o C3 não tem
    // Bluetooth clássico). Tudo-zero = modo descoberta: aceita o primeiro controle
    // e imprime o MAC no serial (USB) pra você fixar aqui.
    static constexpr uint8_t CONTROLLER_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Ponte H (4 entradas, 2 por motor). Se um motor girar ao contrário, basta
    // trocar os dois pinos daquele motor (padrão do projeto). GPIO 8 é strapping
    // de boot + LED onboard do supermini — se houver problema de boot, mova esse
    // fio pro GPIO 10.
    static constexpr int RIGHT_POS_PIN = 5;
    static constexpr int RIGHT_NEG_PIN = 6;
    static constexpr int LEFT_POS_PIN  = 7;
    static constexpr int LEFT_NEG_PIN  = 8;

    static constexpr int MAX_THROTTLE      = 100;
    static constexpr int TURN_COEFFICIENT  = 83;
    static constexpr int PIVOT_COEFFICIENT = 83;

    // Dois servos no controle manual, acionados pela mesma lógica de arma do
    // RCMode (deploy/retract via botões — ver RCMode::handleWeapons/handleMacros).
    // GPIO 20/21 são UART0 no C3, mas o serial roda por USB nativo, então liberam.
    // Ângulos são placeholders — calibre retract/deploy conforme a mecânica.
    static constexpr int NUM_SERVOS = 2;
    static constexpr ServoConfig SERVOS[] = {
        {20, 0, 90}, // servo 1 | recolhido 0° | acionado 90°
        {21, 0, 90}, // servo 2 | recolhido 0° | acionado 90°
    };

    // Velocidade no motor direito, velocidade no motor esquerdo, tempo (ms)
    static const MotionSequence MACRO_FRENTAO = MACRO({100, 100, 300});

    static const MotionSequence MACRO_DIAGONAL = MACRO({-100, 100, 50}, {100, 100, 100});

}
