#pragma once

#include "RobotTypes.hpp"

/**
    @brief Guarda na NVS em qual RobotState o robô deve subir no próximo boot.

    Existe porque o modo de inicialização era uma constante no main.cpp: trocar
    entre IDLE/RC/AUTO exigia recompilar e regravar. O BootModeSelector escreve
    aqui a escolha feita por senha IR, e o main.cpp lê no setup().

    Conflito entre as duas fontes de verdade
    ----------------------------------------
    A partir do momento que existe um valor gravado, ele venceria pra sempre o
    valor compilado — inclusive depois de você editar o default no código e
    regravar o firmware, o que seria surpresa garantida na bancada. Por isso o
    save() carimba junto QUAL era o default compilado na hora da gravação, e o
    load() compara: se o default do firmware mudou desde então, quem manda é o
    código (a senha antiga é descartada). Se não mudou, quem manda é a senha.
    Ou seja, a escolha por IR é permanente até uma nova senha OU até você
    mudar o default no firmware.
*/
namespace BootModeStore {

    /**
    @brief Lê o modo de boot gravado.
    @param compiledDefault Valor definido no firmware (BOOT_DEFAULT do main.cpp).
    @return O modo gravado por senha IR, ou compiledDefault se não há gravação
            válida ou se o default do firmware mudou desde a gravação.
    */
    RobotState load(RobotState compiledDefault);

    /**
    @brief Grava o modo de boot escolhido, carimbando o default compilado atual.
    @param mode Modo que deve valer nos próximos boots.
    @param compiledDefault Default do firmware em vigor agora (o carimbo).
    */
    void save(RobotState mode, RobotState compiledDefault);

    /**
    @brief Apaga a gravação, devolvendo o controle ao default compilado.
    */
    void clear();

    /**
    @brief Nome legível do modo, para log.
    */
    const char *name(RobotState mode);

}
