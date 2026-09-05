#pragma once

#if defined(ROBOT_CAIPORA)
#include "profiles/caipora.hpp"
#include "strategies/CaiporaAuto.hpp"
using ActiveAuto = CaiporaAuto;

#elif defined(ROBOT_CAIPORA_RC)
// Firmware RC (ver src/tools/RCFirmware/main.cpp) não roda nenhuma
// estratégia autônoma — sem ActiveAuto aqui, então não puxa
// strategies/CaiporaAuto.hpp (e os sensores/dependências dele) à toa.
#include "profiles/caiporarc.hpp"

#elif defined(ROBOT_BRIGA)
#include "profiles/briga.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;

#elif defined(ROBOT_SMOKER)
#include "profiles/smoker.hpp"
#include "strategies/SmokerAuto.hpp"
using ActiveAuto = SmokerAuto;
#elif defined(ROBOT_ARRUELA)
#include "profiles/arruela.hpp"
#include "strategies/ArruelaAuto.hpp"
using ActiveAuto = ArruelaAuto;
#elif defined(ROBOT_MAROLA)
#include "profiles/marola.hpp"
#include "strategies/MarolaAuto.hpp"
using ActiveAuto = MarolaAuto;
#elif defined(ROBOT_FUEGO)
#include "profiles/fuego.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;
#elif defined(ROBOT_FUEGUITO)
#include "profiles/fueguito.hpp"
#include "strategies/NoAuto.hpp"
using ActiveAuto = NoAuto;
#else
#error "ERRO CRÍTICO: Nenhum perfil de robô foi definido na compilação!"
#endif