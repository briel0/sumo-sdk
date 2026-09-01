#pragma once

/**
    @class IRReader
    @brief Traduz o controle IR do juiz nos eventos que o robô entende.

    O controle do juiz manda um SURTO de quatro tramas por toque — medido no
    robô em 2026-08-14, apertando o botão 8:

        NEC   addr=0x7F00 cmd=0x02  raw=0xFD027F00
        NEC   addr=0x08   cmd=0x9A  raw=0x659AF708
        NEC2  addr=0x03   cmd=0x9A  raw=0x659AFC03
        RC5   addr=0x00   cmd=0x08  raw=0x1808

    Só a última carrega a tecla: RC5, endereço 0, comando igual ao dígito
    (teclado RC5 padrão manda comando N pro dígito N). As três primeiras têm
    endereço e comando fixos, sem relação com a tecla, e são descartadas.

    Isso explica o bug de "o IR parou de funcionar": o código comparava
    `decodedRawData` contra 0x80..0x88, valores que NENHUMA dessas tramas
    produz. Comparar o `command` da trama RC5 é estável — não depende do
    empacotamento do raw nem da versão do IRremote (que inclusive renumera o
    enum decode_type_t entre versões).
*/
class IRReader {
  public:
    void init(int pin);

    void update();

    bool modeRC() const {
        return _modeRC;
    }
    bool modeAuto() const {
        return _modeAuto;
    }
    bool ready() const {
        return _ready;
    }
    bool start() const {
        return _start;
    }
    bool stop() const {
        return _stop;
    }

    /**
    @brief Dígito do teclado numérico neste frame: 1 a 9, ou 0 se não veio nada.

        Mesmo aperto que alimenta modeAuto()/stop()/etc., só que em forma
        numérica — a senha de boot precisa dos dígitos 4 a 9, e dois deles (8 e
        9) já são "modo AUTO" e "modo RC".

        Diferente dos eventos semânticos, este passa por debounce: com a tecla
        presa o RC5 reenvia a cada ~114ms, e sem filtrar um único aperto do "4"
        entraria como vários dígitos e derrubaria a senha na hora.
    */
    int digit() const {
        return _digit;
    }

    /**
    @brief Descarta tudo que foi decodificado neste frame.

        Chamado por quem "comeu" o aperto — hoje, a senha de boot: sem isso, o
        último dígito de 4568/4569 engataria AUTO/RC no mesmo instante em que a
        senha fecha. Vale só pro frame atual.
    */
    void consumeEvent();

    void shutdown();

  private:
    // Janela em que o mesmo dígito é considerado eco do mesmo toque (RC5
    // reenvia a cada ~114ms com a tecla presa, mais margem).
    static constexpr unsigned long DIGIT_DEBOUNCE_MS = 300;

    bool _modeRC = false;
    bool _modeAuto = false;
    bool _ready = false;
    bool _start = false;
    bool _stop = false;
    int _digit = 0;

    int _lastDigit = 0;
    unsigned long _lastDigitMs = 0;
    unsigned long _lastFrameMs = 0;
};
