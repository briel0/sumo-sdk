#pragma once

// Build somente-RC do Fumacinha na placa ESP32-C3 (ver profiles/fumacinha_rc.hpp
// e o env fumacinha_rc no platformio.ini). É tratado ANTES do dispatch normal
// porque não usa nada da máquina de AUTO: não define ActiveAuto/ActiveAutoMode
// (o build_src_filter nem compila AutoMode/FumacinhaMode/ConfigServer aqui), o
// que evita puxar WiFi/AsyncWebServer/HardwareCore pra dentro do C3.
#if defined(ROBOT_FUMACINHA_RC)
#include "profiles/fumacinha_rc.hpp"

#elif defined(ROBOT_CAIPORA)
#include "profiles/caipora.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;
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
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;
#elif defined(ROBOT_FUEGUITO)
#include "profiles/fueguito.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;
#elif defined(ROBOT_FUMACINHA)
#include "profiles/fumacinha.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto; // FumacinhaMode não usa CombatStrategy; ActiveAuto fica inerte aqui.
#else
#error "ERRO CRÍTICO: Nenhum perfil de robô foi definido na compilação!"
#endif

// Orquestrador do modo AUTO: qual classe recebe o comando do loop() principal
// quando o robô entra em RobotState::AUTO. FumacinhaMode substitui inteiramente
// o par MotionPlayer + CombatStrategy do AutoMode legado pela Fumacinha_FSM
// (fases OPENING/SEARCHING/ATTACKING autocontidas). Dispatch separado do de
// cima porque é ortogonal ao perfil físico — em teoria qualquer robô poderia
// usar qualquer um dos dois motores de combate.
// (Pulado no build somente-RC do C3, que não tem modo AUTO.)
#if !defined(ROBOT_FUMACINHA_RC)
#if defined(ROBOT_FUMACINHA)
#include "modes/FumacinhaMode.hpp"
using ActiveAutoMode = FumacinhaMode;
#else
#include "modes/AutoMode.hpp"
using ActiveAutoMode = AutoMode;
#endif
#endif