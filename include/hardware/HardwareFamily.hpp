#pragma once

// Família de hardware: descreve a FORMA do front-end de sensores/atuadores que o
// HardwareCore deve instanciar, desacoplada do NOME do robô. O HardwareCore
// chaveia por capacidade (FUMACINHA / LEGACY / NONE), nunca por ROBOT_X — então
// colocar um robô novo numa família é UMA linha aqui, não uma edição espalhada
// dentro do HardwareCore.
//
// Fica num header próprio (e não no Config.hpp) de propósito: o HardwareCore.hpp
// precisa destes macros já definidos ao declarar seus membros, independente da
// ordem de include. Depende só de ROBOT_X (que vem do -D no build), então não
// inclui nada e não corre risco de include circular com Config.hpp.
//
//   NONE     : sem front-end de sensores (só-RC ou AUTO cego). O perfil não
//              define nenhum pino de sensor/atuador de combate.
//   LEGACY   : 3 IR (frente/esq/dir) + 2 linha + emissor furtivo + servo de asa.
//              Sem LDR de rampa.
//   FUMACINHA: 5 IR (jsumo×3 + ir puro×2) + 2 linha + LDR de rampa + furtivo + asa.
//   CAIPORA  : 4 VL53L0X (2 centrais + 2 laterais extremos) + 2 linha + LDR de
//              rampa. SEM emissor furtivo e SEM asa: o ToF é sempre ativo (não
//              tem modo passivo pra alternar) e este chassi não tem asa.
#if defined(ROBOT_FUMACINHA) || defined(ROBOT_FUEGO)
#define HW_FAMILY_FUMACINHA
#elif defined(ROBOT_CAIPORA)
#define HW_FAMILY_CAIPORA
#elif defined(ROBOT_SMOKERAUTO)
#define HW_FAMILY_LEGACY
#else
#define HW_FAMILY_NONE
#endif
