#pragma once

// Build somente-RC do Fumacinha na placa ESP32-C3 (ver profiles/fumacinha_rc.hpp
// e o env fumacinha_rc no platformio.ini). É tratado ANTES do dispatch normal
// porque não usa nada da máquina de AUTO: não define ActiveAuto/ActiveAutoMode
// (o build_src_filter nem compila AutoMode/ConfigServer/estratégias aqui), o
// que evita puxar WiFi/AsyncWebServer/HardwareCore pra dentro do C3.
#if defined(ROBOT_FUMACINHA_RC)
#include "profiles/fumacinha_rc.hpp"

#elif defined(ROBOT_CAIPORA)
#include "profiles/caipora.hpp"
#include "strategies/FumacinhaAuto.hpp"
using ActiveAuto = FumacinhaAuto; // mesma lógica do Fumacinha, sensores ToF (ver HardwareFamily.hpp)
#elif defined(ROBOT_SMOKER)
#include "profiles/smoker.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;
#elif defined(ROBOT_ARRUELA)
#include "profiles/arruela.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;
#elif defined(ROBOT_MAROLA)
#include "profiles/marola.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;
#elif defined(ROBOT_SMOKERAUTO)
#include "profiles/smokerAuto.hpp"
#include "strategies/SmokerAuto.hpp"
using ActiveAuto = SmokerAuto;
#elif defined(ROBOT_FUEGO)
#include "profiles/fuego.hpp"
#include "strategies/FumacinhaAuto.hpp"
// Fuego e Fumacinha são o MESMO robô do ponto de vista do combate: mesma família
// de hardware (HW_FAMILY_FUMACINHA: JSumo laterais + IR puro + 2 sensores de
// linha + LDR de rampa + emissor furtivo + asa) e o mesmo repertório de táticas.
// Duas classes pra isso significavam manter a mesma lógica em dois lugares e ver
// uma delas ficar pra trás — que foi o que aconteceu: só a FumacinhaAuto ganhou
// as aberturas, as quatro buscas e a proteção de desengate.
//
// A strategies/FuegoAuto continua no repositório e continua compilando; ela só
// não está mais no dispatch. Pra voltar atrás basta reverter esta linha e o
// USES_FUMACINHA_FSM do perfil — os dois juntos, nunca um só.
using ActiveAuto = FumacinhaAuto;
#elif defined(ROBOT_FUEGUITO)
#include "profiles/fueguito.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;
#elif defined(ROBOT_FUMACINHA)
#include "profiles/fumacinha.hpp"
#include "strategies/FumacinhaAuto.hpp"
using ActiveAuto = FumacinhaAuto;
#else
#error "ERRO CRÍTICO: Nenhum perfil de robô foi definido na compilação!"
#endif

// Orquestrador do modo AUTO: uma classe só, o AutoMode, para todos os robôs.
// Ele cuida do ciclo em volta da luta (AP de configuração, HUD, largada por IR,
// feedback de LED, diagnóstico de bancada) e delega o combate frame a frame
// para a CombatStrategy escolhida acima em ActiveAuto.
//
// Já existiu um FumacinhaMode paralelo aqui, porque o Fumacinha precisa de um
// payload diferente (CombatProfile em vez de AutoStrategy) e de uma HUD
// própria. Isso agora cabe no AutoMode: ele escolhe o payload pela mesma flag
// Config::USES_FUMACINHA_FSM que o ConfigServer usa pra escolher a página, e
// entrega o pacote à estratégia pelo hook CombatStrategy::setCombatProfile(),
// que é no-op para todas as outras. Resultado: um orquestrador só, e o combate
// do Fumacinha virou uma estratégia como as dos outros robôs.
// (Pulado no build somente-RC do C3, que não tem modo AUTO.)
#if !defined(ROBOT_FUMACINHA_RC)
#include "modes/AutoMode.hpp"
using ActiveAutoMode = AutoMode;
#endif
