#pragma once

#if defined(ROBOT_CAIPORA)
#include "profiles/caipora.hpp"
#elif defined(ROBOT_SMOKER)
#include "profiles/smoker.hpp"
#elif defined(ROBOT_ARRUELA)
#include "profiles/arruela.hpp"
#elif defined(ROBOT_MAROLA)
#include "profiles/marola.hpp"
#elif defined(ROBOT_SMOKERAUTO)
#include "profiles/smokerAuto.hpp"
#elif defined(ROBOT_FUEGO)
#include "profiles/fuego.hpp"
#elif defined(ROBOT_FUEGUITO)
#include "profiles/fueguito.hpp"
#else
#error "ERRO CRÍTICO: Nenhum perfil de robô foi definido na compilação!"
#endif