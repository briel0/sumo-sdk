#pragma once

#include "CombatStrategy.hpp"
#include "MotionPlayer.hpp"
#include "RobotTypes.hpp"

class Drive;
class WeaponSystem;
class HardwareCore;

/**
    @class FumacinhaAuto
    @brief Combate autônomo do Fumacinha: abertura por macro cega, busca
           selecionável e ataque, tudo despachado a partir do CombatProfile
           escolhido na HUD.

    É uma CombatStrategy como as outras (SmokerAuto, FuegoAuto, ArruelaAuto,
    CaiporaAuto): o AutoMode não sabe qual robô está lutando, só chama
    init() uma vez e autoEngage() a cada frame. A diferença é o payload — esta
    estratégia é a única que lê um CombatProfile, entregue pelo AutoMode via
    setCombatProfile() quando a HUD transmite. As demais ignoram esse hook.

    Dona do próprio MotionPlayer: as aberturas, o recuo de borda e o "S" de
    reengate são macros, e o player não pode ser o mesmo do AutoMode (que toca
    o saque cego legado e o testador de bancada, ambos fora do combate).

    PRIORIDADE DENTRO DE UM FRAME, de cima pra baixo:
      1. macro em curso     — manda no frame inteiro (ponto ÚNICO de execução)
      2. LDR na rampa       — ataque; exceção: não corta um recuo de borda
      3. abertura           — uma vez por luta
      4. linha branca       — recuo de borda (ignorado durante a carga total)
      5. busca escolhida    — ofensiva / pulso / linha / defensiva

    Todo o controle de tempo é não-bloqueante (millis()); nenhum método aqui usa
    delay(). autoEngage() deve ser chamado a cada iteração do loop principal.
*/
class FumacinhaAuto : public CombatStrategy {
  public:
    FumacinhaAuto() = default;

    void init(HardwareCore &hardware) override;
    void autoEngage(Drive &motores, WeaponSystem &armas, HardwareCore &hardware) override;

    /**
    @brief Recebe o pacote tático da HUD. Chamado pelo AutoMode assim que o
           celular transmite, antes da largada.
    */
    void setCombatProfile(const CombatProfile &profile) override;

  private:
    // Perfil tático do round (abertura/busca/ataque/lado). Preenchido por
    // setCombatProfile(); até lá vale o default do struct.
    CombatProfile combatProfile;

    HardwareCore *_hardware = nullptr;

    // Largada: o AutoMode entra em FIGHTING e o primeiro autoEngage() faz o
    // preparo da luta (asa + rearme dos latches). Mesmo padrão do
    // FuegoAuto::_asaAberta — evita um hook a mais na interface.
    bool _largou = false;

    void _largada();

    // Player próprio das macros de combate — ver o doc da classe.
    MotionPlayer _macroPlayer;

    // Ataque por LDR: latch só pro log sair na borda de subida/descida, e não a
    // cada frame afogando o serial. Não participa da decisão de atacar — essa é
    // tomada direto da leitura do sensor, todo frame.
    bool _ldrAtacando = false;

    static constexpr int LDR_ATTACK_PWM = 100; // rampa ocupada: empurra em força máxima

    // --- Correção de asa adversária (CombatProfile::opponentHasWing) --------
    // Com "TEM ASA" marcado nas ESPECIFICAÇÕES DO ADVERSÁRIO, o LDR coberto
    // deixa de significar necessariamente "corpo dele na nossa rampa": pode ser
    // só a ASA dele apoiada ali, com o corpo de lado. O sinal que separa os dois
    // casos é o JSumo lateral — se ele acusa ao mesmo tempo que o LDR, o corpo
    // está para aquele lado, e empurrar reto é empurrar a asa.
    //
    // A correção é gira-pro-lado + avanço curto, uma vez por rampagem. O latch
    // de disparo é rearmado quando o LDR abre; sem ele, com o lateral ainda
    // acusando depois da manobra, a correção redispararia a cada frame e o robô
    // nunca chegaria a empurrar.
    //
    // O segundo bool é o que protege a macro: enquanto ela toca, o empurrão do
    // LDR fica suspenso. Sem isso ele chamaria _macroPlayer.stop() no frame
    // seguinte — a rampa continua ocupada durante a manobra inteira — e a
    // correção morreria no primeiro passo.
    // Os tempos e a potência da manobra ficam no .cpp, em escopo de arquivo, ao
    // lado das MotionSequence que os usam — MACRO() monta um array constexpr em
    // escopo de arquivo e não enxerga membro privado de classe. Mesmo motivo do
    // VEL_FRENTE_LINHA. Ver ASA_ADV_* lá.
    bool _corrigiuAsaAdv = false;   // já corrigiu NESTA rampagem
    bool _corrigindoAsaAdv = false; // a macro em curso é a correção

    // --- Finalização por tempo (AttackTactic::TIME_FINISH) -----------------
    // A alternativa à finalização por LDR, e o oposto dela: aqui o LDR é
    // ignorado a luta INTEIRA — não há ataque de rampa nem "S" de reengate —
    // e quem encerra é o relógio. Vencidos combatProfile.finishTimeS segundos
    // contados da largada, os emissores ligam e a BUSCA OFENSIVA assume o resto
    // da luta no lugar da tática de busca escolhida na HUD.
    //
    // Porta de mão única, como a carga total: uma vez aberta não fecha. Sem o
    // latch, o robô voltaria pra tática original a cada frame em que o relógio
    // fosse recalculado depois de um estouro do millis().
    //
    // O que NÃO muda com a finalização em curso: a borda. A leitura de linha
    // continua acima da busca no _executeCombat, então o robô segue recuando da
    // borda enquanto finaliza — ignorar a linha aqui seria jogá-lo pra fora.
    unsigned long _largadaMs = 0;  // instante da largada, base do relógio
    bool _finalizando = false;     // relógio vencido, busca ofensiva no comando

    /**
    @brief A finalização por tempo já deve estar no comando neste frame?
    @return true quando a tática é TIME_FINISH e o relógio já venceu — daí em
            diante o chamador deve rodar a BUSCA OFENSIVA e ignorar a tática de
            busca da HUD. false para a finalização por LDR, sempre.
    */
    bool _finalizacaoPorTempo(unsigned long agora);

    // --- Manobra de flanco -------------------------------------------------
    // Gira pro lado pedido, avança até achar a borda e volta pro miolo do dohyo.
    // Ajuste os ângulos/tempos nas MotionSequence FLANCO_* do .cpp.
    // Roda UMA vez por luta, como abertura: quando termina, o frame volta pro
    // combate normal. Os dois bools são resetados na largada.
    bool _flancoGirou = false;    // já fez o giro inicial
    bool _flancoVoltando = false; // já achou a borda e disparou o retorno

    static constexpr int FLANK_ADVANCE_PWM = 50;

    /**
    @brief Um frame da manobra de flanco. Chamar a cada frame.
    @param lado Para que lado girar antes de avançar.
    @return true enquanto a manobra está em curso; false quando terminou.
    */
    bool _flanco(Drive &motores, Direction lado);

    // --- Aberturas por macro cega ------------------------------------------
    // Todas as aberturas menos a CURVA DE BORDA: macros de tiro único portadas
    // do projeto smoker. Ao contrário do flanco, não lêem sensor nenhum — são
    // passos de tempo fixo. Um único latch serve a todas, porque só uma abertura
    // é escolhida por luta. Resetado na largada. Qual macro cada tática usa está
    // em macroDeAbertura(), no .cpp.
    bool _aberturaDisparada = false; // a macro de abertura já rodou nesta luta

    // A abertura inteira (macro cega OU curva de borda) já entregou o frame ao
    // combate. Latch separado do _aberturaDisparada porque a CURVA DE BORDA não
    // é macro: quem diz que ela acabou são os bools do flanco. É o que permite à
    // BUSCA ASA rearmar esses bools pra refazer a curva de borda no meio da luta
    // sem que a abertura volte a rodar junto. Resetado na largada.
    bool _aberturaFinalizada = false;

    /**
    @brief Dispara uma macro de abertura, se nenhuma tiver rodado nesta luta.
    @param seq Sequência a tocar — normalmente vinda de macroDeAbertura().
    @return true no frame em que dispara a macro; false quando a abertura já
            aconteceu e o frame pertence ao combate normal.
    */
    bool _aberturaCega(const MotionSequence &seq);

    // --- Recuo de borda ------------------------------------------------------
    // Enquanto a manobra de ré+giro está em curso, a linha é IGNORADA: o frame é
    // todo da macro e nenhuma leitura nova pode reiniciá-la no meio. _recuoFimMs
    // é reempurrado a cada frame da manobra, então quando ela termina o carimbo
    // marca o fim dela e ainda restam LINHA_IGNORE_MS de carência — sem essa
    // sobra o robô relê a linha no frame seguinte, ainda em cima dela, e
    // redispara o recuo em loop (o movimento de "se debater" na borda).
    // CUIDADO ao aumentar LINHA_IGNORE_MS: nessa janela o robô anda cego pra
    // borda, então tempo demais aqui é risco de cair pra fora do dohyo.
    bool _recuandoBorda = false;   // a macro em curso é um recuo de borda
    unsigned long _recuoFimMs = 0; // fim da última manobra de recuo

    static constexpr unsigned long LINHA_IGNORE_MS = 200; // carência depois da manobra

    // --- Busca ---------------------------------------------------------------
    // Último lado em que o oponente foi visto. Serve de palpite quando os dois
    // laterais ficam cegos: gira pra onde ele estava, em vez de escolher ao acaso.
    Direction _ultimoLado = Direction::left;

    static constexpr int VEL_BUSCA_GIRO = 60;
    static constexpr int VEL_ATAQUE_MAX = 100;      // lado alinhado / alvo centrado
    static constexpr int VEL_ATAQUE_REDUZIDA = 50;  // roda de dentro, fechando o arco

    /**
    @brief Gira procurando o oponente: pro lado que o enxergou, ou pro último
           lado visto quando os dois laterais estão cegos.
    */
    void _busca(Drive &motores, bool viuEsq, bool viuDir);

    /**
    @brief Um frame da BUSCA OFENSIVA (busca OFFENSIVE_SEARCH). Estrutura portada do
           FuegoAuto: os dois JSumo laterais decidem tudo — viu alguém, ataca em
           arco; ninguém à vista, gira no eixo pro último lado visto. Não lê a
           linha nem o LDR, que já rodam antes no _executeCombat.
    */
    void _buscaOfensiva(Drive &motores);

    /**
    @brief Alvo à vista em pelo menos um lateral. Dois laterais = alvo centrado,
           avanço reto em força máxima; um só = arco fechando pra aquele lado
           (avança E gira, pra não depender dos dois feixes se cruzarem).
    */
    void _ataqueArco(Drive &motores, bool viuEsq, bool viuDir);

    // Arrancada cega periódica: o ciclo é ESPERA_MS girando na busca seguidos de
    // AVANCO_MS indo pra1 frente, e recomeça. O carimbo marca o início do ciclo.
    // O avanço é feito com setSpeed direto, NÃO com MotionPlayer: macro sequestra
    // o frame e deixaria o robô cego pra linha justamente enquanto ele avança.
    // Arrancada de desencalhe: passar SEM_LINHA_MS sem cruzar a linha é sinal de
    // que o robô ficou preso no meio do dohyo. Fora da arrancada ele segue em
    // frente devagar, em CRUZEIRO_PWM.
    unsigned long _ultimaLinhaMs = 0; // última vez que a linha foi vista
    unsigned long _avancoMs = 0;      // início da arrancada em curso
    bool _avancando = false;

    static constexpr unsigned long SEM_LINHA_MS = 4000; // tempo sem ver a linha que dispara a arrancada
    static constexpr unsigned long AVANCO_MS = 90;      // duração da arrancada
    static constexpr int AVANCO_PWM = 100;              // potência da arrancada
    static constexpr int CRUZEIRO_PWM = 55;             // avanço normal, fora da arrancada

    // --- Escalada de 2 estágios do BUSCA LINHA -----------------------------
    // Etapa 1 (SEM_LINHA_MS, acima): arrancada curta de desencalhe, e o relógio
    // dela REARMA a cada tranco — por isso ela se repete indefinidamente.
    // Etapa 2 (CARGA_TOTAL_MS): se a MESMA sequência sem tocar a linha chegar
    // a 7 s, o robô entende que os trancos não resolveram e vai pra cima em
    // potência máxima, sem freio.
    //
    // Por isso existe um segundo carimbo: _semLinhaMs só é reescrito por contato
    // REAL com a linha (e na largada), nunca pelo fim da arrancada. Se as duas
    // etapas dividissem o _ultimaLinhaMs, a etapa 2 nunca venceria — o rearme da
    // etapa 1 zeraria o relógio a cada ~4 s.
    //
    // A carga total é uma porta de mão única: nada no combate a desarma. O
    // único freio é o botão 3 do controle, tratado no loop() do main.cpp, que
    // para os motores e reinicia o ESP. Enquanto ela estiver de pé, o
    // _executeCombat ignora até a linha da borda — é deliberado, e é o que
    // torna essa escalada um recurso de fim de luta, não uma tática qualquer.
    unsigned long _semLinhaMs = 0; // último contato REAL com a linha
    bool _cargaTotal = false;      // etapa 2 disparada; só o botão 3 encerra

    static constexpr unsigned long CARGA_TOTAL_MS = 7000; // sem tocar a linha -> carga total
    static constexpr int CARGA_TOTAL_PWM = 100;           // potência da carga total

    /**
    @brief Um frame do BUSCA LINHA (busca LINE_SEARCH): cruzeiro em linha reta
           com a arrancada periódica de desencalhe (etapa 1) e a carga total
           (etapa 2). Chamar a cada frame de busca.
    */
    void _buscaLinha(Drive &motores, unsigned long agora);

    // --- Busca defensiva (busca DEFENSIVE_SEARCH) --------------------------
    // Portada do "Modo Linha" do TSUNAMI. É a irmã cautelosa da BUSCA OFENSIVA:
    // os mesmos sensores laterais decidem, mas ela NUNCA carrega a partir deles
    // — com um lateral só ela apenas alinha, e cega ou com os dois acesos ela
    // rasteja pra frente, o que a mantém móvel sem se comprometer.
    //
    // Emissores DESLIGADOS (furtivo, IR puro): é o oposto da BUSCA OFENSIVA e o
    // ponto da tática — não anunciar a própria posição. Custa alcance, já que o
    // IR puro enxerga bem menos longe que o JSumo.
    //
    // A agressão vem de fora: o LDR (rampa ocupada), que roda antes no
    // _executeCombat, ou a estagnação. Passar DEFENSIVA_PACIENCIA_MS sem tocar
    // a linha significa "estou parado no meio do dohyo sem fazer nada" — aí ela
    // entrega o comando ao BUSCA LINHA e não retoma.
    bool _defensivaEscalou = false; // já entregou o comando ao BUSCA LINHA

    static constexpr unsigned long DEFENSIVA_PACIENCIA_MS = 10000; // estagnação que escala
    static constexpr int DEFENSIVA_CRUZEIRO_PWM = 20;              // rastejo (45/255 do original)
    static constexpr int DEFENSIVA_GIRO_PWM = 40;                  // giro de alinhamento (100/255)

    /**
    @brief Um frame da BUSCA DEFENSIVA. Chamar a cada frame de busca.
    */
    void _buscaDefensiva(Drive &motores, unsigned long agora);

    // --- Busca asa (busca WING_SEARCH) -------------------------------------
    // Tática de asa: o robô não procura o oponente pelo dohyo, ele gira no
    // próprio eixo mantendo o alvo dentro do setor que a ASA cobre — quem manda
    // é o sensor da asa (frontDetected(), emissor próprio, nunca desligado).
    //
    // O ciclo tem três estados e todos giram no eixo em ASA_GIRO_PWM:
    //   varredura  — nenhum contato ainda com este lado: gira PRO lado da asa;
    //   engajado   — a asa vê: continua girando PRO lado da asa;
    //   voltando   — a asa perdeu depois de ter visto: gira pro lado CONTRÁRIO,
    //                desfazendo o que acabou de varrer, até reencontrar.
    //
    // Passar ASA_REAQUISICAO_MS na volta sem reencontrar significa que o alvo
    // saiu do alcance de vez. Aí, em vez de oscilar no lugar, o robô refaz a
    // CURVA DE BORDA (o mesmo _flanco da abertura) preservando o lado da asa e
    // recomeça a varredura do zero.
    //
    // O JSumo lateral do lado OPOSTO ao da asa é o único outro sensor que conta:
    // ele significa "o oponente está do lado que a asa não cobre", e manda
    // trocar a asa de lado. Por isso esta tática liga os emissores — sem eles a
    // leitura lateral cai pro IR puro, curto demais pra essa troca acontecer.
    bool _asaIniciada = false;   // o lado da asa já foi herdado da abertura
    WingPosition _asaLado = WingPosition::LEFT; // lado atual da asa
    bool _asaViu = false;        // já houve contato da asa com este lado
    bool _asaPerdeu = false;     // viu e perdeu: está na volta de reaquisição
    unsigned long _asaPerdaMs = 0;  // instante em que perdeu o contato
    unsigned long _asaTrocaMs = 0;  // instante da última troca de lado da asa
    bool _asaReabrindo = false;  // curva de borda de recomeço em curso

    static constexpr int ASA_GIRO_PWM = 25;                  // giro no eixo, os três estados
    static constexpr unsigned long ASA_REAQUISICAO_MS = 250; // volta sem reencontrar -> recomeça
    // Permanência mínima de um lado da asa. Existe pelo servo, não pela tática:
    // com os dois laterais piscando alternado, sem isto o servo bateria de um
    // extremo ao outro a cada frame. É da ordem do tempo de curso dele.
    static constexpr unsigned long ASA_TROCA_MIN_MS = 300;

    /**
    @brief Um frame da BUSCA ASA. Chamar a cada frame de busca.
    */
    void _buscaAsa(Drive &motores, unsigned long agora);

    /**
    @brief Giro no próprio eixo em ASA_GIRO_PWM, usado pelos três estados da
           BUSCA ASA.
    @param asaEsq   Lado atual da asa (true = esquerda).
    @param voltando true na volta de reaquisição, que gira pro lado contrário.
    */
    void _giroAsa(Drive &motores, bool asaEsq, bool voltando);

    // --- Busca pulsada (busca PULSED_SEARCH) -------------------------------
    // Robô parado a maior parte do tempo, dando um avanço curto a cada
    // PULSO_INTERVALO_MS. Cumpridos PULSO_QTD pulsos, o busca linha assume o
    // resto da luta. Quem quebra o ritmo antes disso são as travas que rodam
    // ANTES da busca no _executeCombat: o LDR (oponente na rampa) e a borda.
    // O tempo é contado de _buscaStartMs, carimbado no primeiro frame de busca
    // — a série é uma só por luta e não rearma depois do busca linha começar.
    unsigned long _buscaStartMs = 0;
    bool _buscaIniciada = false;  // _buscaStartMs já foi carimbado nesta luta
    bool _pulsoConcluido = false; // série cumprida, busca linha no comando

    static constexpr unsigned long PULSO_INTERVALO_MS = 1500; // período do ciclo (pulso + espera parada)
    static constexpr unsigned long PULSO_AVANCO_MS = 60;      // duração do avanço de cada pulso
    static constexpr int PULSO_PWM = 100;                     // potência do pulso (as duas rodas)
    static constexpr int PULSO_QTD = 4;                       // pulsos antes de cair no busca linha

    /**
    @brief Um frame da série de pulsos. Chamar a cada frame de busca.
    @return true enquanto a série está em curso (o frame é dela); false quando
            os PULSO_QTD pulsos já foram dados e o busca linha deve assumir.
    */
    bool _pulsoPeriodico(Drive &motores, unsigned long agora);
    /**
    @brief Um frame de combate. Despacha a cadeia de prioridades do doc da classe.
    */
    void _executeCombat(Drive &motores);
};
