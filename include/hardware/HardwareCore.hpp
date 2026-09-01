#pragma once

#include "HardwareFamily.hpp" // define HW_FAMILY_* antes de declarar os membros
#include "JS40F.hpp"
#include "LDRSensor.hpp"
#include "QRE1113.hpp"
#include "ToFSensor.hpp"
#include "StealthEmitter.hpp"
#include "WingServo.hpp"
#include <Arduino.h>

/**
    @class HardwareCore
    @brief Camada de hardware que isola todos os sensores sensíveis a latência
           (receptores IR frontal/laterais, sensores de linha e o LDR de rampa)
           e os periféricos de estado (transistor do modo furtivo e o servo da
           asa lateral) atrás de estados lógicos limpos.

    update() varre e filtra os sensores e aplica as intenções de atuação
    pendentes. Deve ser chamado uma vez por frame a partir do loop do Core 1
    (mesmo núcleo do combate) — a lógica de combate só lê os getters e escreve
    intenções via setStealth()/setWing(), sem tocar em hardware bruto.

    Por que não roda como task pinada no Core 0?
    O AutoMode sobe um Access Point WiFi (ConfigServer) durante a seleção de
    estratégia e, hoje, ConfigServer::shutdown() é um stub — o stack WiFi/LwIP
    fica residente no Core 0 durante todo o combate (ver Decisões Críticas no
    README). Uma task de alta frequência pinada ali ficaria sujeita a jitter do
    rádio ocioso, o oposto do determinismo que se busca. Como o loop() do Core 1
    já opera bare-metal a >10 kHz sem overhead de scheduler, manter a varredura
    síncrona nesse mesmo núcleo é mais previsível do que uma segunda task.
*/
class HardwareCore {
  public:
    HardwareCore();

    /**
    @brief Inicializa os pinos dos sensores e periféricos. Idempotente.
    */
    void begin();

    /**
    @brief Varre e filtra os sensores e aplica as intenções de atuação pendentes.
           Chamar uma vez por frame a partir do Core 1.
    */
    void update();

    bool isInitialized() const {
        return _initialized;
    }

    // --- Estados limpos lidos pela lógica de combate -----------------------------
    bool frontDetected() const {
        return _frontDetected;
    }
    bool leftDetected() const {
        return _leftDetected;
    }
    bool rightDetected() const {
        return _rightDetected;
    }
    bool lineLeft() const {
        return _lineLeft;
    }
    bool lineRight() const {
        return _lineRight;
    }
    bool opponentOnRamp() const {
        return _opponentOnRamp;
    }
    uint16_t ldrFiltered() const {
        return _ldrFiltered;
    }

    // --- Leituras cruas para diagnóstico de bancada (log ao vivo no celular) ----
    // Valores exatos por trás dos booleanos já filtrados acima. Definidos por
    // família em HardwareCore.cpp: famílias sem um dado sensor retornam neutro
    // (0/false), já que não têm o campo correspondente. jsumo*/ir* só existem de
    // verdade na família FUMACINHA; lineLeftRaw/lineRightRaw existem em FUMACINHA
    // e LEGACY (a NONE não tem sensores de linha).
    uint16_t lineLeftRaw() const;
    uint16_t lineRightRaw() const;
    bool jsumoLeftRaw() const;
    bool jsumoRightRaw() const;
    bool jsumoAsaRaw() const;
    bool irLeftRaw() const;
    bool irRightRaw() const;

    // --- Intenções escritas pela lógica de combate -------------------------------
    void setStealth(bool on) {
        _stealthIntent = on;
    }
    void setWing(WingPosition position) {
        _wingIntent = position;
    }

    /**
    @brief Última posição pedida à asa. Devolve a INTENÇÃO, não a posição
        física: o servo pode ainda estar a caminho, já que update() só dispara o
        movimento e não espera o curso terminar.

        Lê o intent (e não _wing.position()) de propósito — o WingServo só
        existe nas famílias FUMACINHA e LEGACY, então tocá-lo aqui quebraria a
        compilação dos robôs da família NONE.
    */
    WingPosition wing() const {
        return _wingIntent;
    }

  private:
    // Front-end de sensores/atuadores: a COMPOSIÇÃO muda por família de hardware
    // (HW_FAMILY_*, definida em Config.hpp), não por nome de robô. Perfis de
    // famílias sem um dado bloco não precisam definir os pinos dele.
#if defined(HW_FAMILY_FUMACINHA)
    // 2 sensores por lado: o "JSumo" (emissor+receptor, CI inversor, HIGH=alvo),
    // desligável no modo furtivo, e o "IR puro" (mesmo receptor, sem o CI,
    // LOW=alvo), sempre energizado — fallback de detecção lateral quando o JSumo
    // daquele lado perde o emissor. A "asa" (frente) nunca é desligada.
    JS40F          _jsumoLeft;
    JS40F          _jsumoRight;
    JS40F          _jsumoAsa;
    JS40F          _irLeft;
    JS40F          _irRight;
    QRE1113        _lineSensorLeft;
    QRE1113        _lineSensorRight;
    LDRSensor      _ldr; // LDR de rampa: exclusivo desta família
    StealthEmitter _stealth;
    WingServo      _wing;
#elif defined(HW_FAMILY_CAIPORA)
    // 4 VL53L0X (ToF) no lugar dos IR/JSumo do Fumacinha: 2 CENTRAIS, que juntos
    // fazem o papel do sensor frontal (este chassi não tem um dedicado), e 2
    // LATERAIS EXTREMOS, que fazem o papel dos JSumo.
    //
    // Sem StealthEmitter e sem WingServo de propósito: o ToF é sempre ativo, não
    // existe fonte passiva pra alternar, e este robô não tem asa. setStealth() e
    // setWing() continuam existindo e apenas não têm efeito aqui — é o que deixa
    // a FumacinhaAuto rodar sem uma linha de alteração.
    ToFSensor _tofCentroEsq;
    ToFSensor _tofCentroDir;
    ToFSensor _tofLateralEsq;
    ToFSensor _tofLateralDir;
    QRE1113   _lineSensorLeft;
    QRE1113   _lineSensorRight;
    LDRSensor _ldr;
#elif defined(HW_FAMILY_LEGACY)
    // 3 IR simples (frente/esq/dir) + linha + furtivo + asa. Sem LDR de rampa.
    JS40F          _front;
    JS40F          _left;
    JS40F          _right;
    QRE1113        _lineSensorLeft;
    QRE1113        _lineSensorRight;
    StealthEmitter _stealth;
    WingServo      _wing;
#else
    // HW_FAMILY_NONE: robô sem front-end de sensores/atuadores de combate.
    // Nenhum membro de hardware — o perfil não define pino de sensor nenhum.
#endif

    // Estados limpos: sempre presentes (lidos pelos getters), preenchidos por
    // update() apenas nas famílias que têm o sensor correspondente; nas demais
    // ficam no default e o getter devolve neutro.
    bool     _frontDetected  = false;
    bool     _leftDetected   = false;
    bool     _rightDetected  = false;
    bool     _lineLeft       = false;
    bool     _lineRight      = false;
    bool     _opponentOnRamp = false;
    uint16_t _ldrFiltered    = 0;

    bool         _stealthIntent = true;
    WingPosition _wingIntent    = WingPosition::RETRACTED;

    bool _initialized = false;
};
