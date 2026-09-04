#include "strategies/FumacinhaAuto.hpp"
#include "Config.hpp"
#include "hardware/Drive.hpp"
#include "hardware/HardwareCore.hpp"
#include "hardware/WeaponSystem.hpp"
#include <Arduino.h>

// Esta estratégia é compartilhada por mais de um robô (Fumacinha, Fuego e
// Caipora — ver o dispatch no Config.hpp), então o prefixo do log sai do perfil
// em vez de vir cravado no texto. Com dois robôs ligados na mesma bancada, um
// serial que dissesse "[FUMACINHA]" nos dois seria pior que nenhum.
#define LOG_COMBATE(msg) Serial.printf("[%s] %s\n", Config::ROBOT_NAME, msg)

void FumacinhaAuto::init(HardwareCore &hardware) {
    _hardware = &hardware;
    _largou   = false;
    LOG_COMBATE("Estratégia de combate carregada.");
}

void FumacinhaAuto::setCombatProfile(const CombatProfile &profile) {
    combatProfile = profile;
    // Perfil novo = luta nova: a largada tem que rodar de novo.
    _largou = false;
}

void FumacinhaAuto::autoEngage(Drive &motores, WeaponSystem &armas, HardwareCore &hardware) {
    (void)armas;
    _hardware = &hardware;

    if(!_largou) {
        _largou = true;
        _largada();
    }

    _executeCombat(motores);
}

void FumacinhaAuto::_largada() {
    // Asa pro LADO PREFERENCIAL, com três exceções: na CURVA, no RECUO e no
    // DESEMPATE ela abre pro lado OPOSTO ao escolhido. Nas duas o robô varre ou cede terreno numa
    // direção, e a asa do lado de fora fica na frente de quem vier pelo lado que
    // ele está deixando para trás. A escolha é feita AQUI, e não no disparo da
    // macro, pra não mandar o servo pro lado errado e corrigir no frame seguinte
    // — e pra não brigar, a cada frame, com a troca de lado que o recuo de borda
    // faz.
    bool ladoEsq = (combatProfile.preferredSide == Direction::left);
    bool asaInvertida = (combatProfile.openingTactic == OpeningTactic::CURVA) ||
                        (combatProfile.openingTactic == OpeningTactic::RECUO) ||
                        (combatProfile.openingTactic == OpeningTactic::DESEMPATE);
    bool asaEsq = asaInvertida ? !ladoEsq : ladoEsq;
    _hardware->setWing(asaEsq ? WingPosition::LEFT : WingPosition::RIGHT);

    // Rearma o flanco de abertura: sem isto, uma segunda luta sem reboot já
    // começaria com a manobra dada como concluída.
    _flancoGirou = false;
    _flancoVoltando = false;
    _aberturaDisparada = false;
    _aberturaFinalizada = false;

    // Rearma a BUSCA ASA: o lado da asa é herdado da abertura no primeiro frame
    // dela, e nada do contato da luta anterior pode sobreviver — herdar _asaViu
    // faria a luta nova nascer achando que perdeu um alvo, e disparar o recomeço
    // por curva de borda nos primeiros 250 ms.
    _asaIniciada = false;
    _asaViu = false;
    _asaPerdeu = false;
    _asaReabrindo = false;

    // Correção de asa adversária: luta nova, nenhuma rampagem corrigida ainda.
    _corrigiuAsaAdv = false;
    _corrigindoAsaAdv = false;

    // Os contadores partem da largada — sem carimbar, o gatilho de "sem ver
    // linha" já nasceria vencido e o robô arrancaria no primeiro frame.
    unsigned long agora = millis();
    _ultimaLinhaMs = agora;
    _semLinhaMs = agora;

    // Base do relógio da FINALIZAÇÃO POR TEMPO. Tem que ser carimbado aqui, na
    // largada, e não no primeiro frame de busca: o tempo pedido na HUD é de
    // LUTA, e a abertura faz parte dela.
    _largadaMs = agora;
    _finalizando = false;
    _avancando = false;

    // Rearma as escaladas: uma segunda luta sem reboot não pode largar já em
    // carga total nem com a defensiva dada como vencida.
    _cargaTotal = false;
    _defensivaEscalou = false;
    // Sem isto, uma luta encerrada com o oponente na rampa deixaria o latch
    // armado e a largada seguinte dispararia um "S" de reengate no primeiro
    // frame, sem ninguém ter desgrudado.
    _ldrAtacando = false;

    // Rearma a busca: sem isto, uma segunda luta sem reboot herdaria o marco
    // zero e a série de pulsos da luta anterior.
    _buscaIniciada = false;
    _pulsoConcluido = false;

    // Nenhuma carência de linha herdada da luta anterior: o robô larga
    // enxergando a borda desde o primeiro frame.
    _recuandoBorda = false;
    _recuoFimMs = 0;

    LOG_COMBATE("LARGADA! Asa deployada, combate iniciado.");
}

// Define a sequência inline com a macro MACRO()
// Ordem dos campos do MACRO: {esquerda, direita, tempo_ms}.
static const MotionSequence RECUO_BORDA_DIREITA = MACRO(
    {-100, -100, 125}, // recua
    { 100, -100, 120}  // gira
);

static const MotionSequence RECUO_BORDA_ESQUERDA = MACRO(
    {-100, -100, 125}, // recua
    { -100, 100, 120}  // gira
);

// Borda DE FRENTE: os DOIS sensores de linha acusam no mesmo frame, ou seja, o
// robô chegou perpendicular à borda em vez de raspá-la de lado. A manobra dos
// casos laterais é curta demais pra isso — recuando pouco e girando pouco ele
// volta a encostar na borda no frame seguinte, que é o movimento esquisito de
// ficar se debatendo na linha. Aqui a ré e o giro são mais longos.
// O lado do giro segue o LADO PREFERENCIAL da HUD: com os dois sensores acesos
// nenhum deles indica pra onde escapar, então não há lado a deduzir da leitura.
static const MotionSequence RECUO_BORDA_FRONTAL_DIR = MACRO(
    {-100, -100, 150}, // ré mais longa
    { 100, -100, 175}  // giro mais longo, pra direita
);

static const MotionSequence RECUO_BORDA_FRONTAL_ESQ = MACRO(
    {-100, -100, 150}, // ré mais longa
    { -100, 100, 175}  // giro mais longo, pra esquerda
);

// === Proteção de desengate ======================================================
// Roda de dentro do "S" de reengate. É a mesma velocidade de avanço reto do
// BUSCA LINHA (CRUZEIRO_PWM = 55) — o CRUZEIRO_PWM em si não dá pra usar aqui
// porque é membro privado da classe e este escopo é de arquivo.
static constexpr int VEL_FRENTE_LINHA = 55;

// Disparado na BORDA DE DESCIDA do LDR: o oponente estava embarcado na rampa e
// escapou. Em vez de largar o contato e voltar a procurar do zero, o robô varre
// um "S" curto tentando reencostar nele — primeiro fechando pra um lado, depois
// pro outro. Os tempos são assimétricos (50 ms / 100 ms) de propósito: a segunda
// perna precisa ser mais longa pra cruzar de volta o setor varrido pela primeira.
//
// Não é espelhado pelo LADO PREFERENCIAL: o oponente pode ter escapado pra
// qualquer lado, e a leitura do LDR não diz pra qual. O "S" cobre os dois.
static const MotionSequence DESENGATE_S = MACRO(
    {VEL_FRENTE_LINHA, 100,  50}, // fecha pra um lado
    {100, VEL_FRENTE_LINHA, 100}  // cruza de volta pro outro
);

// === Correção de asa adversária =================================================
// Em escopo de arquivo, e não como membro da classe, pelo mesmo motivo do
// VEL_FRENTE_LINHA: MACRO() monta um array constexpr aqui fora, que não enxerga
// membro privado.
static constexpr unsigned long ASA_ADV_GIRO_MS = 70; // giro pro lado do lateral
static constexpr unsigned long ASA_ADV_AVANCO_MS = 90;
static constexpr int ASA_ADV_PWM = 100;

// Dispara com "TEM ASA" marcado, o LDR coberto e UM lateral acusando: o que
// subiu na rampa é a asa dele, e o corpo está do lado que o lateral vê. Gira pro
// corpo e dá um avanço curto pra reencostar nele já alinhado.
//
// O tempo do giro é um CHUTE a calibrar em pista. A referência é a macro de RC do
// Fumacinha, onde 135 ms a ±100 fecham ~180°; 70 ms é aproximadamente o quarto de
// volta que aponta o robô pro lateral. Se ele passar do ponto ou ficar curto, é
// este número que se mexe.
static const MotionSequence ASA_ADV_ESQ = MACRO(
    {-ASA_ADV_PWM, ASA_ADV_PWM, ASA_ADV_GIRO_MS}, // gira pro lateral esquerdo
    { ASA_ADV_PWM, ASA_ADV_PWM, ASA_ADV_AVANCO_MS}
);

static const MotionSequence ASA_ADV_DIR = MACRO(
    { ASA_ADV_PWM, -ASA_ADV_PWM, ASA_ADV_GIRO_MS}, // gira pro lateral direito
    { ASA_ADV_PWM,  ASA_ADV_PWM, ASA_ADV_AVANCO_MS}
);

// === Macros de abertura =========================================================
// Portadas do projeto smoker (strategies.hpp). Duas conversões em relação às
// matrizes de lá: a escala de velocidade, que naquele projeto é -255..255 e aqui
// é -100..100 (fator 100/255), e o passo terminador {0,0,0}, que não existe aqui
// — MotionSequence carrega o número de passos.

// Nenhuma das matrizes originais é espelhada com exatidão entre esquerda e
// direita — tempos e rodas de dentro diferem de um lado pro outro. Mantive fiel
// ao original em vez de uniformizar: se um lado abrir mais que o outro no robô,
// vem da matriz, não da conversão.

// CURVA, portada da CURVINHA (1/4 do dohyo). Já recalibrada no robô — os valores
// abaixo não são mais a conversão crua da matriz original.
static const MotionSequence CURVA_ESQ = MACRO(
    {-100,  100, 100}, // giro no eixo pra esquerda
    { 100,   40, 144}, // arco aberto, roda de dentro segurando
    { 100, -100, 120}, // giro no eixo de volta pra direita
    {  50,   50,  75}  // avanço curto pra assentar
);

static const MotionSequence CURVA_DIR = MACRO(
    { 100, -100, 100}, // giro no eixo pra direita
    {  50,  100, 144}, // arco aberto, roda de dentro segurando
    {-100,  100, 130}, // giro no eixo de volta pra esquerda
    {  50,   50,  75}  // avanço curto pra assentar
);

// CURVÃO, portado do curvao (3/4 do dohyo): mesma ideia da CURVA com o arco
// muito mais aberto — a roda de dentro cai a 20 e depois a 8 (50 e 20 na matriz
// original), fechando um raio bem maior ao longo de ~470 ms de arco.
static const MotionSequence CURVAO_ESQ = MACRO(
    {-100,  100,  70}, // giro no eixo pra esquerda
    { 100,   20, 170}, // arco abrindo
    { 100,    8, 300}, // arco no raio máximo
    { 100, -100, 120}, // giro no eixo de volta pra direita
    { 100,  100,  30}  // avanço curto pra assentar
);

static const MotionSequence CURVAO_DIR = MACRO(
    { 100, -100,  70}, // giro no eixo pra direita
    {  27,  100, 150}, // arco abrindo
    {  16,  100, 220}, // arco no raio máximo
    {-100,  100, 150}, // giro no eixo de volta pra esquerda
    { 100,  100,  60}  // avanço curto pra assentar
);

// EM V, portado do emV (3/4 do dohyo): avança reto, gira no eixo e avança reto
// de novo — o vértice do V é o giro do meio. Abre com um giro curto que aponta
// a primeira perna pro lado escolhido.
static const MotionSequence EM_V_ESQ = MACRO(
    {-100,  100,  40}, // aponta pra esquerda
    { 100,  100, 155}, // primeira perna do V
    { 100, -100, 190}, // vértice: giro no eixo
    { 100,  100, 170}  // segunda perna do V
);

static const MotionSequence EM_V_DIR = MACRO(
    { 100, -100,  45}, // aponta pra direita
    { 100,  100, 190}, // primeira perna do V
    {-100,  100, 155}, // vértice: giro no eixo
    { 100,  100, 200}  // segunda perna do V
);

// VZINHO, portado do vzinho: o mesmo V SEM o giro de entrada — larga reto e o
// primeiro desvio já é o vértice. O lado escolhido só muda pra onde ele vira.
static const MotionSequence VZINHO_ESQ = MACRO(
    { 100,  100, 190}, // primeira perna, reto da largada
    { 100, -100, 180}, // vértice: giro no eixo
    { 100,  100, 180}  // segunda perna do V
);

static const MotionSequence VZINHO_DIR = MACRO(
    { 100,  100, 190}, // primeira perna, reto da largada
    {-100,  100, 155}, // vértice: giro no eixo
    { 100,  100, 190}  // segunda perna do V
);

// VZÃO, portado do vzao: V com giro de entrada mais longo que o do EM V e
// vértice mais fechado.
static const MotionSequence VZAO_ESQ = MACRO(
    {-100,  100,  90}, // aponta pra esquerda
    { 100,  100, 145}, // primeira perna do V
    { 100, -100, 200}, // vértice: giro no eixo
    { 100,  100, 170}  // segunda perna do V
);

static const MotionSequence VZAO_DIR = MACRO(
    { 100, -100,  90}, // aponta pra direita
    { 100,  100, 170}, // primeira perna do V
    {-100,  100, 145}, // vértice: giro no eixo
    { 100,  100, 170}  // segunda perna do V
);

// RECUO: ré curta em arco, uma roda segurando a outra. É a única abertura que
// larga andando pra trás — o robô cede terreno em vez de disputá-lo, e apresenta
// ao adversário a asa, que abre pro lado oposto ao escolhido (ver a regra da asa
// no _largada).
//
// Os nomes são pelo LADO SELECIONADO, não pelo sentido do arco: RECUO_LADO_DIR é
// o que roda com "DIREITA" escolhida na HUD.
static const MotionSequence RECUO_LADO_DIR = MACRO({-60, -100, 180});
static const MotionSequence RECUO_LADO_ESQ = MACRO({-100, -60, 180});

// DESEMPATE: deriva curta pra um lado e giro no próprio eixo, terminando de lado
// em relação a quem larga de frente. Mesma regra de asa do RECUO — ela abre pro
// lado oposto ao escolhido, ou seja, pro lado de onde o robô está saindo.
static const MotionSequence DESEMPATE_LADO_DIR = MACRO(
    {  85,  100, 120}, // deriva: a roda de dentro segura, o robô sai de lado
    {-100,  100, 180}  // giro no próprio eixo, fechando a posição
);

static const MotionSequence DESEMPATE_LADO_ESQ = MACRO(
    { 100,   85, 120}, // espelhado
    { 100, -100, 180}
);

// FRENTÃO, portado do frenteSemDelay: {255, 255, 200} -> {100, 100, 200}.
// Avanço reto de ~3/4 do dohyo em força máxima, sem curva de aceleração.
static const MotionSequence ABERTURA_FRENTAO = MACRO({100, 100, 200});

// FRENTINHO, portado do frentinho: {127, 127, 200} -> {50, 50, 200}.
// Mesmo avanço reto, meia força: ~1/4 do dohyo.
static const MotionSequence ABERTURA_FRENTINHO = MACRO({50, 50, 200});

// Traduz a tática de abertura escolhida na HUD para a macro que a executa,
// já resolvendo o lado. Ponto único dessa correspondência — quem despacha a
// abertura no _executeCombat só pede a sequência e toca.
static const MotionSequence &macroDeAbertura(OpeningTactic tatica, Direction lado) {
    bool esq = (lado == Direction::left);

    switch(tatica) {
        case OpeningTactic::CURVAO:
            return esq ? CURVAO_ESQ : CURVAO_DIR;
        case OpeningTactic::EM_V:
            return esq ? EM_V_ESQ : EM_V_DIR;
        case OpeningTactic::VZINHO:
            return esq ? VZINHO_ESQ : VZINHO_DIR;
        case OpeningTactic::VZAO:
            return esq ? VZAO_ESQ : VZAO_DIR;
        case OpeningTactic::RECUO:
            return esq ? RECUO_LADO_ESQ : RECUO_LADO_DIR;
        case OpeningTactic::DESEMPATE:
            return esq ? DESEMPATE_LADO_ESQ : DESEMPATE_LADO_DIR;
        case OpeningTactic::FRENTAO:
            return ABERTURA_FRENTAO; // reto: não tem lado
        case OpeningTactic::FRENTINHO:
            return ABERTURA_FRENTINHO; // idem
        case OpeningTactic::CURVA:
        default:
            // EDGE_POSITIONING nunca chega aqui (é manobra com sensor, não macro).
            return esq ? CURVA_ESQ : CURVA_DIR;
    }
}

// No _executeCombat, em vez de setSpeed() + delay():

void FumacinhaAuto::_executeCombat(Drive &motores) {
    if(!_hardware)
        return;

    // Sem macro tocando não existe manobra de recuo em curso. Fica no topo pra
    // que o LDR logo abaixo já enxergue o estado atualizado — senão ele adiaria
    // o ataque por um frame depois que o recuo acabou.
    if(!_macroPlayer.isPlaying()) {
        _recuandoBorda = false;
        _corrigindoAsaAdv = false;
    }

    // Prioridade máxima: LDR acusou oponente embarcado na rampa -> ataca,
    // cancelando a macro em curso (MotionPlayer::stop) — com o adversário em
    // cima da rampa, parar pra manobrar entregaria a única posição vantajosa da
    // luta. A abertura (flanco/arco) cai nessa regra. O recuo de borda NÃO: ver
    // a exceção logo abaixo.
    // FINALIZAÇÃO POR TEMPO ignora o LDR desde o primeiro frame da luta. Zerar
    // a leitura aqui, e não em cada uso, desliga o bloco inteiro de uma vez: o
    // ataque de rampa, o latch de log e o "S" de reengate (que depende da borda
    // de descida de _ldrAtacando, e sem subida nunca há descida).
    bool rampaOcupada = (combatProfile.attackTactic == AttackTactic::LDR_FINISH) &&
                        _hardware->opponentOnRamp();

    if(rampaOcupada && !_ldrAtacando) {
        LOG_COMBATE("LDR: oponente na rampa -> ATAQUE.");
        _ldrAtacando = true;
    }
    else if(!rampaOcupada && _ldrAtacando) {
        // BORDA DE DESCIDA: o oponente estava embarcado e desgrudou. Em vez de
        // soltar o contato e voltar a procurar do zero, varre um "S" curto
        // tentando reencostar nele — ver DESENGATE_S.
        //
        // Não precisa de trava pra ser abortado: se o LDR voltar a acusar no
        // meio do "S", o ramo de cima deste mesmo bloco chama _macroPlayer.stop()
        // e retoma o ataque no mesmo frame. Se o "S" terminar com o LDR ainda
        // falso, o frame seguinte simplesmente segue pro fluxo normal (abertura,
        // borda, busca) — que é o comportamento de antes.
        LOG_COMBATE("LDR: rampa livre -> 'S' de reengate.");
        _ldrAtacando = false;
        _macroPlayer.play(DESENGATE_S);
    }

    // ÚNICA exceção à prioridade do LDR: o recuo de borda termina antes. Abortar
    // ré+giro no meio deixava o robô meia-manobra em cima da linha e mandava 100
    // pra frente — se o LDR oscila perto da borda, isso vira o vai e vem que
    // acaba jogando o robô pra fora. A rampa continua ocupada quando a manobra
    // acabar, e aí o ataque assume no frame seguinte; o que se perde é só o
    // resto do recuo. O latch de log acima já subiu, então o ataque não fica
    // mudo enquanto espera.
    // Rampa livre rearma a correção: a próxima rampagem é um caso novo e merece
    // sua própria checagem de asa.
    if(!rampaOcupada) {
        _corrigiuAsaAdv = false;
    }

    // CORREÇÃO DE ASA ADVERSÁRIA. Roda ACIMA do empurrão porque é justamente ele
    // que ela substitui neste frame — ver o doc no header.
    //
    // UM lateral só, não "algum": com os DOIS acusando o corpo dele está centrado
    // à nossa frente, que é o caso em que empurrar reto já é o certo. Corrigir aí
    // giraria pra fora do alvo.
    if(combatProfile.opponentHasWing && rampaOcupada && !_recuandoBorda && !_corrigiuAsaAdv) {
        bool viuEsq = _hardware->leftDetected();
        bool viuDir = _hardware->rightDetected();

        if(viuEsq != viuDir) {
            _corrigiuAsaAdv = true;
            _corrigindoAsaAdv = true;
            _macroPlayer.stop();
            _macroPlayer.play(viuEsq ? ASA_ADV_ESQ : ASA_ADV_DIR);
            _macroPlayer.update(motores);
            _ultimoLado = viuEsq ? Direction::left : Direction::right;
            LOG_COMBATE("LDR + lateral com ADVERSÁRIO C/ ASA -> girando pro corpo dele.");
            return;
        }
    }

    if(rampaOcupada && !_recuandoBorda && !_corrigindoAsaAdv) {
        _macroPlayer.stop();
        motores.setSpeed(LDR_ATTACK_PWM, LDR_ATTACK_PWM);
        return;
    }

    // PONTO ÚNICO de execução de macro em combate. Fica logo abaixo do LDR e
    // ACIMA da abertura de propósito: enquanto uma macro toca, ela manda no
    // frame inteiro e mais nada roda — nem o flanco, nem a leitura de linha,
    // nem a busca. Se algum método chamado depois daqui repetir a checagem de
    // isPlaying(), ele passa a interceptar macros que não são dele e essa
    // contabilidade abaixo para de acontecer (foi o que aconteceu com o
    // _flanco: ele engolia os recuos de borda e a carência nunca armava).
    if(_macroPlayer.isPlaying()) {
        // Manobra de recuo em curso: a linha fica ignorada. Reempurrar o carimbo
        // a cada frame faz ele terminar valendo o INSTANTE EM QUE A MANOBRA
        // ACABOU, que é de onde a carência de LINHA_IGNORE_MS conta.
        if(_recuandoBorda) {
            _recuoFimMs = millis();
        }
        _macroPlayer.update(motores);
        return;
    }

    // Abertura: despachada pelo selecionável ABERTURA da HUD. Roda uma vez por
    // luta e, quando termina, devolve o frame pro combate normal abaixo. Fica
    // depois do LDR de propósito — rampa ocupada interrompe a abertura.
    if(!_aberturaFinalizada) {
        if(combatProfile.openingTactic == OpeningTactic::EDGE_POSITIONING) {
            // CURVA DE BORDA: única abertura que lê sensor — o flanco pro LADO
            // PREFERENCIAL, validado no robô. Todas as outras são macro cega.
            if(_flanco(motores, combatProfile.preferredSide)) {
                return;
            }
        }
        else if(_aberturaCega(macroDeAbertura(combatProfile.openingTactic, combatProfile.preferredSide))) {
            return;
        }

        // Passou reto pelos dois: a abertura entregou o frame ao combate e não
        // roda mais nesta luta. O latch é o que deixa a BUSCA ASA rearmar os
        // bools do flanco pra refazer a curva de borda no meio da luta sem
        // ressuscitar a abertura junto — as duas usam o MESMO _flanco.
        _aberturaFinalizada = true;
    }

    // BUSCA ASA em recomeço: a curva de borda que ela refaz ao perder o alvo roda
    // AQUI, no lugar da abertura, e não lá embaixo junto com a busca. O motivo é
    // o bloco de linha, que fica entre os dois: esta manobra precisa ENCOSTAR na
    // borda pra terminar, e disparada abaixo da leitura de linha ela levaria um
    // recuo de borda em cima a cada tentativa — nunca acharia o que foi procurar,
    // e o robô ficaria se debatendo na linha.
    //
    // O lado passado é o INVERTIDO do lado da asa, de propósito: o _flanco abre a
    // asa pro lado oposto ao que recebe (regra da abertura — o arco varre pra um
    // lado e a asa cobre o outro). Como aqui o objetivo é TERMINAR com a asa onde
    // ela já está, inverter na entrada é o que preserva o lado.
    if(_asaReabrindo) {
        if(_flanco(motores, _asaLado == WingPosition::LEFT ? Direction::right : Direction::left)) {
            return;
        }
        _asaReabrindo = false;
    }

    // Marco zero da BUSCA: primeiro frame depois que a abertura entregou o
    // controle. É a referência de tempo da série de pulsos — por isso é
    // carimbado aqui, e não na largada, senão a abertura comeria os pulsos.
    if(!_buscaIniciada) {
        _buscaIniciada = true;
        _buscaStartMs  = millis();
    }

    bool linhaEsq = !_hardware->lineLeft();
    bool linhaDir = !_hardware->lineRight();
    bool jsumoEsq = _hardware->jsumoLeftRaw();
    bool jsumoDir = _hardware->jsumoRightRaw();
    bool jsumoFrente = _hardware->frontDetected();
    unsigned long agora = millis();

    // Carência pós-manobra: a linha vista logo depois de um recuo é ignorada.
    // Subtração de unsigned — imune ao estouro do millis().
    bool ignorandoLinha = (agora - _recuoFimMs) < LINHA_IGNORE_MS;

    // A carga total (etapa 2 do BUSCA LINHA) ignora a borda de propósito: é uma
    // investida sem freio, e recuar no meio dela a anularia. Depois de armada,
    // só o botão 3 do controle encerra — ver _cargaTotal no header.
    if((linhaEsq || linhaDir) && !ignorandoLinha && !_cargaTotal) {
        _ultimaLinhaMs = agora; // realimenta o gatilho da arrancada (etapa 1)
        _semLinhaMs    = agora; // contato REAL: é este que a etapa 2 e a defensiva leem
        _recuandoBorda = true;  // a macro que vai tocar agora é um recuo de borda

        if(linhaEsq && linhaDir) {
            // Batida de frente na borda: manobra longa (ver RECUO_BORDA_FRONTAL_*).
            // Precisa ser testado ANTES dos casos laterais — com os dois sensores
            // acesos, o `if(linhaDir)` sozinho vencia e aplicava a manobra curta.
            _macroPlayer.play(combatProfile.preferredSide == Direction::left ? RECUO_BORDA_FRONTAL_ESQ
                                                                            : RECUO_BORDA_FRONTAL_DIR);
        }
        else if(linhaDir) {
            _macroPlayer.play(RECUO_BORDA_DIREITA);
        }
        else {
            _macroPlayer.play(RECUO_BORDA_ESQUERDA);
        }
        _macroPlayer.update(motores);
        if(_hardware->wing() == WingPosition::LEFT){
            _hardware->setWing(WingPosition::RIGHT);
        }
        else{
            _hardware->setWing(WingPosition::LEFT);
        }
        return;
    }

    // FINALIZAÇÃO POR TEMPO vencida: a tática de busca escolhida na HUD deixa de
    // valer e a BUSCA OFENSIVA assume o resto da luta. Fica ACIMA do switch de
    // propósito — é uma sobreposição, não uma quinta opção de busca.
    if(_finalizacaoPorTempo(agora)) {
        _buscaOfensiva(motores);
        return;
    }

    // Busca: despachada pelo selecionável BUSCA da HUD. Tudo que vem antes deste
    // ponto (LDR na rampa, abertura, recuo de borda) tem prioridade sobre a
    // tática escolhida e vale igual pras três — quem chega aqui já passou por
    // essas travas.
    switch(combatProfile.searchTactic) {
        case SearchTactic::PULSED_SEARCH:
            // Série de pulsos: enquanto ela não se cumpre, manda no frame.
            if(_pulsoPeriodico(motores, agora)) {
                return;
            }
            // Série cumprida: o busca linha assume o resto da luta. Rearma o
            // relógio da arrancada de desencalhe pra ele começar do zero aqui —
            // sem isto os ~4,6 s parado da série já teriam estourado o
            // SEM_LINHA_MS e a arrancada dispararia no primeiro frame.
            if(!_pulsoConcluido) {
                _pulsoConcluido = true;
                _ultimaLinhaMs  = agora;
                LOG_COMBATE("BUSCA PULSADA cumprida -> busca linha.");
            }
            _buscaLinha(motores, agora);
            break;

        case SearchTactic::OFFENSIVE_SEARCH:
            // BUSCA OFENSIVA: busca e ataque guiados pelos laterais.
            _buscaOfensiva(motores);
            break;

        case SearchTactic::DEFENSIVE_SEARCH:
            // BUSCA DEFENSIVA: rasteja e só alinha; escala pro BUSCA LINHA.
            _buscaDefensiva(motores, agora);
            break;

        case SearchTactic::WING_SEARCH:
            // BUSCA ASA: gira no eixo mantendo o alvo no setor varrido pela asa.
            _buscaAsa(motores, agora);
            break;

        case SearchTactic::LINE_SEARCH:
        default:
            // BUSCA LINHA, a busca validada no robô.
            _buscaLinha(motores, agora);
            break;
    }

    /*
    // Prioridade 3: inimigo lateral — centraliza devagar
    if(jsumoEsq && !jsumoDir) {
        _hardware->setWing(WingPosition::LEFT);
        motores.setSpeed(-25, 25); // giro lento para esquerda
        return;
    }
    if(jsumoDir && !jsumoEsq) {
        _hardware->setWing(WingPosition::RIGHT);
        motores.setSpeed(25, -25); // giro lento para direita
        return;
    }

    // Prioridade 4: cegueira total — busca lenta
    _hardware->setWing(combatProfile.preferredSide == Direction::left ? WingPosition::LEFT : WingPosition::RIGHT);
    motores.setSpeed(25, 25); // avança devagar
    */
}

// === Buscas =====================================================================

void FumacinhaAuto::_buscaLinha(Drive &motores, unsigned long agora) {
    // === ETAPA 2: carga total ===============================================
    // A MESMA sequência sem tocar a linha passou de CARGA_TOTAL_MS: os trancos
    // curtos da etapa 1 não resolveram, então o robô para de negociar e vai pra
    // cima em potência máxima. Conta de _semLinhaMs (contato REAL com a linha),
    // e não de _ultimaLinhaMs, que a etapa 1 rearma a cada tranco.
    //
    // Porta de mão única: uma vez de pé, nada aqui a desarma, e o bloco de linha
    // do _executeCombat passa a ser ignorado. Quem encerra é o botão 3 do
    // controle, tratado no loop() do main.cpp — ele para os motores e reinicia.
    if(!_cargaTotal && agora - _semLinhaMs >= CARGA_TOTAL_MS) {
        _cargaTotal = true;
        LOG_COMBATE("BUSCA LINHA: 7 s sem tocar a linha -> CARGA TOTAL. Só o botão 3 encerra.");
    }

    if(_cargaTotal) {
        motores.setSpeed(CARGA_TOTAL_PWM, CARGA_TOTAL_PWM);
        return;
    }

    // === ETAPA 1: arrancada de desencalhe ===================================
    // Arrancada de desencalhe: dispara quando passa SEM_LINHA_MS sem cruzar a
    // linha. Como isto roda DEPOIS do bloco de linha do _executeCombat e usa
    // setSpeed em vez de macro, a borda continua sendo checada a cada frame,
    // inclusive durante a arrancada.
    if(!_avancando && agora - _ultimaLinhaMs >= SEM_LINHA_MS) {
        _avancando = true;
        _avancoMs = agora;
    }

    if(_avancando) {
        if(agora - _avancoMs < AVANCO_MS) {
            motores.setSpeed(AVANCO_PWM, AVANCO_PWM);
        }
        else {
            // Arrancada cumprida: rearma o contador e volta pro cruzeiro.
            _avancando = false;
            _ultimaLinhaMs = agora;
            motores.setSpeed(CRUZEIRO_PWM, CRUZEIRO_PWM);
        }
    }
    else {
        // Fora da arrancada: avanço normal. O ramo NÃO pode ficar sem comando —
        // o Drive segura o último PWM, então sem isto o robô continuaria na
        // velocidade da arrancada anterior (o mesmo motivo do setSpeed(0,0) no
        // fim do MotionPlayer::update).
        motores.setSpeed(CRUZEIRO_PWM, CRUZEIRO_PWM);
        //_busca(motores, jsumoEsq, jsumoDir);
        //DESATIVA O JSUMO
    }
}

bool FumacinhaAuto::_pulsoPeriodico(Drive &motores, unsigned long agora) {
    // Duração da série inteira: o último pulso é o de índice (PULSO_QTD - 1) e
    // termina PULSO_AVANCO_MS depois de começar. Não esperamos a janela parada
    // do último ciclo acabar — seria só tempo morto antes do busca linha.
    constexpr unsigned long SERIE_MS = (PULSO_QTD - 1) * PULSO_INTERVALO_MS + PULSO_AVANCO_MS;

    unsigned long decorrido = agora - _buscaStartMs;
    if(decorrido >= SERIE_MS) {
        return false; // série cumprida — quem chamou assume o frame
    }

    // Cada ciclo de PULSO_INTERVALO_MS abre com um avanço reto e curto e passa
    // todo o resto do tempo parado. Em que ponto do ciclo estamos é função pura
    // do tempo decorrido desde o início da busca — não precisa de contador.
    bool pulsando = (decorrido % PULSO_INTERVALO_MS) < PULSO_AVANCO_MS;
    motores.setSpeed(pulsando ? PULSO_PWM : 0, pulsando ? PULSO_PWM : 0);
    return true;
}

bool FumacinhaAuto::_finalizacaoPorTempo(unsigned long agora) {
    if(combatProfile.attackTactic != AttackTactic::TIME_FINISH) {
        return false;
    }

    // Porta de mão única: depois de aberta, nem o relógio é consultado de novo.
    if(_finalizando) {
        return true;
    }

    // Subtração de unsigned, imune ao estouro do millis(). O cast é o que
    // importa aqui: finishTimeS é uint16_t e 1023 * 1000 estoura int de 16 bits
    // — sem o UL a conta viraria lixo justamente nos tempos mais longos.
    if(agora - _largadaMs < (unsigned long)combatProfile.finishTimeS * 1000UL) {
        return false;
    }

    _finalizando = true;

    // "Ativar todos os sensores" é isto: o emissor lateral é o único periférico
    // de sensor desligável do robô, e ligá-lo troca a detecção lateral do IR
    // puro (passivo, curto) pro JSumo (ativo, longo). A _buscaOfensiva refaz
    // esta chamada a cada frame; aqui ela existe pra que o log e a mudança de
    // alcance aconteçam no MESMO frame em que a finalização abre.
    _hardware->setStealth(true);
    LOG_COMBATE("FINALIZAÇÃO POR TEMPO: relógio vencido -> BUSCA OFENSIVA até o fim.");
    return true;
}

void FumacinhaAuto::_buscaOfensiva(Drive &motores) {
    // Estrutura portada do FuegoAuto::autoEngage: os dois laterais decidem tudo
    // — viu alguém, ataca em arco; não viu ninguém, gira no eixo procurando.
    // Não há aqui o bloco de LDR nem o de asa do Fuego: o LDR já roda com
    // prioridade lá no topo do _executeCombat e a asa é da abertura.
    //
    // Emissores LIGADOS pelo mesmo motivo do FuegoAuto::init(): o JSumo lateral
    // só é confiável com o emissor alimentado, e desligado o HardwareCore cai no
    // IR puro, que é bem mais curto. Isto IGNORA o selecionável "EMISSOR NA
    // BUSCA" da HUD, que ainda não está ligado neste caminho.
    _hardware->setStealth(true);

    bool viuEsq = _hardware->leftDetected();
    bool viuDir = _hardware->rightDetected();

    if(viuEsq) {
        _ultimoLado = Direction::left;
    }
    else if(viuDir) {
        _ultimoLado = Direction::right;
    }

    if(viuEsq || viuDir) {
        _ataqueArco(motores, viuEsq, viuDir);
    }
    else {
        _busca(motores, viuEsq, viuDir);
    }
}

void FumacinhaAuto::_buscaDefensiva(Drive &motores, unsigned long agora) {
    // Emissores DESLIGADOS: o oposto da BUSCA OFENSIVA e o ponto desta tática.
    // Sem emissor o HardwareCore troca a detecção lateral pro IR puro, passivo —
    // enxerga menos longe, mas não anuncia a posição do robô.
    //
    // EXCEÇÃO: "TEM ASA" nas especificações do adversário força o emissor ligado
    // e cancela a furtividade desta busca. A correção de asa adversária lê os
    // laterais no frame em que o LDR fecha, e o JSumo só é confiável com o
    // emissor já alimentado — ligá-lo naquele instante chegaria tarde. Marcar
    // TEM ASA é, portanto, abrir mão do furtivo na BUSCA DEFENSIVA.
    _hardware->setStealth(combatProfile.opponentHasWing);

    // Escalada por estagnação: passar DEFENSIVA_PACIENCIA_MS sem NENHUM contato
    // com a linha significa que o robô está parado no meio do dohyo sem nada
    // acontecer. Aí a cautela deixa de valer a pena e o BUSCA LINHA assume o
    // resto da luta — inclusive a própria escalada de 2 estágios dele.
    if(!_defensivaEscalou && agora - _semLinhaMs >= DEFENSIVA_PACIENCIA_MS) {
        _defensivaEscalou = true;
        // Zera os DOIS relógios: sem isto o BUSCA LINHA nasceria com 10 s de
        // atraso acumulado e pularia direto pra carga total, engolindo a etapa 1.
        _ultimaLinhaMs = agora;
        _semLinhaMs    = agora;
        LOG_COMBATE("BUSCA DEFENSIVA estagnada -> entregando pro BUSCA LINHA.");
    }

    if(_defensivaEscalou) {
        _buscaLinha(motores, agora);
        return;
    }

    bool viuEsq = _hardware->leftDetected();
    bool viuDir = _hardware->rightDetected();

    if(viuEsq) {
        _ultimoLado = Direction::left;
    }
    else if(viuDir) {
        _ultimoLado = Direction::right;
    }

    // A diferença central pra BUSCA OFENSIVA: com UM lateral só, aqui o robô
    // apenas GIRA pra encarar o alvo — não fecha o arco pra cima dele. E com os
    // dois acesos (alvo centrado) ele rasteja em vez de carregar. Nenhuma leitura
    // lateral autoriza ataque nesta tática; quem autoriza é o LDR (rampa) ou a
    // escalada acima.
    if(viuEsq && !viuDir) {
        motores.setSpeed(-DEFENSIVA_GIRO_PWM, DEFENSIVA_GIRO_PWM);
    }
    else if(viuDir && !viuEsq) {
        motores.setSpeed(DEFENSIVA_GIRO_PWM, -DEFENSIVA_GIRO_PWM);
    }
    else {
        // Cego, ou alvo centrado: rasteja pra frente. Ficar imóvel faria do robô
        // um alvo parado; o rastejo o mantém se reposicionando sem se comprometer.
        motores.setSpeed(DEFENSIVA_CRUZEIRO_PWM, DEFENSIVA_CRUZEIRO_PWM);
    }
}

// Giro que acompanha a TROCA DE LADO da asa na BUSCA ASA. Dispara no mesmo frame
// em que o servo recebe o comando: enquanto a asa cruza pro outro lado, o chassi
// gira pro mesmo lado, levando o setor que ela cobre até o alvo que o lateral
// acabou de acusar. Sem ele, o robô só reposiciona a asa e continua varrendo a
// 25 — o servo chega ao lado novo apontado para o lugar errado.
//
// Curto e em força máxima de propósito: é um tranco de reorientação, não uma
// varredura. Os 80 ms são da ordem do curso do servo, então os dois terminam
// juntos.
static const MotionSequence ASA_TROCA_GIRO_ESQ = MACRO({-100, 100, 80});
static const MotionSequence ASA_TROCA_GIRO_DIR = MACRO({100, -100, 80});

void FumacinhaAuto::_giroAsa(Drive &motores, bool asaEsq, bool voltando) {
    // Giro no próprio eixo: rodas em sentidos opostos, mesma potência. A volta de
    // reaquisição é o mesmo giro ao contrário — por isso ela desfaz exatamente o
    // setor que a varredura acabou de cobrir, em vez de procurar em outro lugar.
    bool paraEsq = voltando ? !asaEsq : asaEsq;
    motores.setSpeed(paraEsq ? -ASA_GIRO_PWM : ASA_GIRO_PWM,
                     paraEsq ? ASA_GIRO_PWM : -ASA_GIRO_PWM);
}

void FumacinhaAuto::_buscaAsa(Drive &motores, unsigned long agora) {
    // Emissores LIGADOS. Esta tática depende de dois sensores: o da asa, que tem
    // emissor próprio e nunca é desligado, e o JSumo lateral do lado oposto, que
    // é quem manda trocar a asa de lado. O lateral só é JSumo com o emissor
    // alimentado — desligado, o HardwareCore cai no IR puro, curto demais pra
    // essa troca chegar a acontecer.
    _hardware->setStealth(true);

    // Primeira entrada: o lado da asa é o que a ABERTURA deixou. É este o "lado
    // atual" de que trata o resto da tática. RETRACTED não deveria chegar aqui
    // (largada e aberturas sempre abrem a asa), mas se chegar o lado preferencial
    // decide — melhor que girar pra um lado indefinido.
    if(!_asaIniciada) {
        _asaIniciada = true;
        WingPosition daAbertura = _hardware->wing();
        if(daAbertura == WingPosition::RETRACTED) {
            daAbertura = (combatProfile.preferredSide == Direction::left) ? WingPosition::LEFT
                                                                         : WingPosition::RIGHT;
        }
        _asaLado = daAbertura;
        _hardware->setWing(_asaLado);
        _asaTrocaMs = agora;
    }

    bool viuAsa = _hardware->frontDetected();

    // Troca de lado: o lateral do lado OPOSTO ao da asa viu alguém, ou seja, o
    // oponente está justamente no setor que a asa não cobre. A asa muda de lado e
    // o giro passa a ser o equivalente pro lado novo.
    //
    // Só vale com a asa CEGA: o sensor da asa é o principal desta tática, e um
    // lateral não tira o robô de um alvo que ela já está varrendo. O
    // ASA_TROCA_MIN_MS é a outra trava, e essa é mecânica: os dois laterais
    // piscando alternado mandariam o servo bater de um extremo ao outro a cada
    // frame.
    if(!viuAsa && (agora - _asaTrocaMs) >= ASA_TROCA_MIN_MS) {
        bool asaEsq = (_asaLado == WingPosition::LEFT);
        bool viuOposto = asaEsq ? _hardware->rightDetected() : _hardware->leftDetected();
        if(viuOposto) {
            _asaLado = asaEsq ? WingPosition::RIGHT : WingPosition::LEFT;
            _hardware->setWing(_asaLado);
            _asaTrocaMs = agora;
            _ultimoLado = asaEsq ? Direction::right : Direction::left;

            // Lado novo, varredura nova: o contato que valia era do lado antigo, e
            // mantê-lo faria a volta de reaquisição procurar por um alvo que nunca
            // esteve no setor que a asa cobre agora.
            _asaViu = false;
            _asaPerdeu = false;

            // Giro de acompanhamento, pro lado NOVO da asa — que é o lado em que
            // o lateral viu alguém. Tocado e atualizado no mesmo frame, igual ao
            // recuo de borda; dos próximos frames em diante quem toca os passos é
            // o ponto único de macro do _executeCombat, e o frame inteiro é dele.
            // O servo continua andando durante a macro porque quem o move é o
            // HardwareCore, no Core 0, alheio a quem está comandando os motores.
            _macroPlayer.play(_asaLado == WingPosition::LEFT ? ASA_TROCA_GIRO_ESQ
                                                            : ASA_TROCA_GIRO_DIR);
            _macroPlayer.update(motores);
            return;
        }
    }

    bool esq = (_asaLado == WingPosition::LEFT);

    // Asa enxergando: gira PRO lado dela, mantendo o alvo dentro do setor.
    if(viuAsa) {
        _asaViu = true;
        _asaPerdeu = false;
        _giroAsa(motores, esq, false);
        return;
    }

    // Nenhum contato ainda com este lado da asa: é a varredura de entrada, o
    // "modo defensivo que só gira". Não há alvo perdido pra reencontrar, então o
    // relógio dos 250 ms não corre aqui — se corresse, o robô refaria a curva de
    // borda a cada 250 ms sem nunca ter visto ninguém.
    if(!_asaViu) {
        _giroAsa(motores, esq, false);
        return;
    }

    // Viu e perdeu: começa a volta de reaquisição e marca a hora.
    if(!_asaPerdeu) {
        _asaPerdeu = true;
        _asaPerdaMs = agora;
    }

    // A volta inteira sem reencontrar: o alvo saiu do alcance de vez. Em vez de
    // seguir oscilando no lugar, o robô refaz a CURVA DE BORDA preservando o lado
    // da asa e recomeça a varredura do zero. Quem toca a manobra é o bloco de
    // recomeço do _executeCombat, acima da leitura de linha.
    if(agora - _asaPerdaMs >= ASA_REAQUISICAO_MS) {
        _asaReabrindo = true;
        _flancoGirou = false;
        _flancoVoltando = false;
        _asaViu = false;
        _asaPerdeu = false;
        LOG_COMBATE("BUSCA ASA: volta sem reencontrar -> nova curva de borda.");
        return;
    }

    _giroAsa(motores, esq, true);
}

void FumacinhaAuto::_ataqueArco(Drive &motores, bool viuEsq, bool viuDir) {
    // Os dois laterais juntos = alvo centrado à frente: força bruta reta.
    if(viuEsq && viuDir) {
        motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_MAX);
        return;
    }

    // Um lateral só: arco fechando pra aquele lado — reduz a roda de dentro em
    // vez de girar parado, pra avançar e alinhar ao mesmo tempo. Girar parado
    // aqui dependeria dos dois feixes se cruzarem pra sair do lugar.
    if(viuEsq) {
        motores.setSpeed(VEL_ATAQUE_REDUZIDA, VEL_ATAQUE_MAX);
        return;
    }
    motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_REDUZIDA);
}

void FumacinhaAuto::_busca(Drive &motores, bool viuEsq, bool viuDir) {
    if(viuEsq) {
        _ultimoLado = Direction::left;
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
        return;
    }
    if(viuDir) {
        _ultimoLado = Direction::right;
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
        return;
    }
    // Cegueira total: gira pro último lado visto.
    if(_ultimoLado == Direction::right) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    }
    else {
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
    }
}

bool FumacinhaAuto::_aberturaCega(const MotionSequence &seq) {
    // Tiro único: dispara no primeiro frame de combate e não volta a rodar nesta
    // luta, seja qual for a macro. Quem toca os passos é o ponto único de macro
    // do _executeCombat — por isso aqui só resta soltar o play() e sair de cena.
    if(_aberturaDisparada) {
        return false;
    }

    _aberturaDisparada = true;
    _macroPlayer.play(seq);
    return true;
}

// === Manobra de flanco ==========================================================
// Ordem dos campos do MACRO: {esquerda, direita, tempo_ms}. Giro no próprio eixo
// = sinais opostos. Ajuste as durações até fechar o ângulo desejado.

static const MotionSequence FLANCO_GIRO_ESQ = MACRO({-90, 90, 65});
static const MotionSequence FLANCO_GIRO_DIR = MACRO({90, -90, 65});

// Achou a borda: recua um tico e gira pro lado OPOSTO ao do flanco — é pra lá
// que fica o miolo do dohyo.
static const MotionSequence FLANCO_RETORNO_ESQ = MACRO({-100, -100, 80}, {90, -90, 170});
static const MotionSequence FLANCO_RETORNO_DIR = MACRO({-100, -100, 80}, {-90, 90, 170});

bool FumacinhaAuto::_flanco(Drive &motores, Direction lado) {
    if(!_hardware) {
        return false;
    }

    bool esq = (lado == Direction::left);

    // Quem toca a macro em curso é o ponto único do _executeCombat, ANTES deste
    // método ser chamado — não repita a checagem de isPlaying() aqui. Já foi
    // assim, e era um bug: como o _flanco roda antes desse ponto único, ele
    // engolia QUALQUER macro em curso, inclusive os recuos de borda, e a
    // contabilidade da carência de linha (_recuandoBorda/_recuoFimMs) nunca
    // rodava. O sequenciamento abaixo continua valendo: um frame só chega aqui
    // quando não há macro tocando, que é exatamente quando a etapa deve avançar.

    // O retorno já tocou e acabou: manobra concluída.
    if(_flancoVoltando) {
        return false;
    }

    // Começo: gira pro lado pedido, com a asa recolhida.
    if(!_flancoGirou) {
        _flancoGirou = true;
        _hardware->setWing(WingPosition::RETRACTED);
        _macroPlayer.play(esq ? FLANCO_GIRO_ESQ : FLANCO_GIRO_DIR);
        return true;
    }

    // Achou a borda: abre a asa e volta pro miolo.
    if(!_hardware->lineLeft() || !_hardware->lineRight()) {
        _flancoVoltando = true;
        _hardware->setWing(esq ? WingPosition::RIGHT : WingPosition::LEFT);
        _macroPlayer.play(esq ? FLANCO_RETORNO_ESQ : FLANCO_RETORNO_DIR);
        return true;
    }

    // Entre o giro e a borda: só avança.
    motores.setSpeed(FLANK_ADVANCE_PWM, FLANK_ADVANCE_PWM);
    return true;
}

