#pragma once

#if defined(ROBOT_TRATOR)
#include "profiles/trator.hpp"
#else
#error "ERRO CRÍTICO: Nenhum perfil de robô foi definido na compilação!"
#endif
