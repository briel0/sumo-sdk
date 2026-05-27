# SUMO-SDK

Framework modular para robôs de Mini Sumô baseados em ESP32. Construído com foco absoluto em determinismo de tempo real, ele combina leitura de registradores em bare-metal e isolamento de rotinas via FreeRTOS para garantir latência zero no dohyo. Sua arquitetura orientada a objetos, guiada por Máquinas de Estados Finitos (FSM) não-bloqueantes, permite compilar um único codebase para múltiplos robôs através de perfis de hardware independentes.

---

## Estrutura de Diretórios

A arquitetura do projeto segue a separação estrita entre definições (headers) e implementações (sources), mantendo o código modular e escalável.

```text
📦 SUMO-SDK
├── 📂 include/              
│   ├── 📂 defs/             
│   │   ├── 📄 Config.hpp 
│   │   └── 📄 RobotTypes.hpp 
│   ├── 📂 hardware/         
│   │   ├── 📂 sensors/
│   │   │   └── 📄 JS40F.hpp 
│   │   ├── 📄 Drive.hpp
│   │   ├── 📄 IRreader.hpp
│   │   ├── 📄 Receiver.hpp
│   │   ├── 📄 ServoMechanism.hpp
│   │   └── 📄 WeaponSystem.hpp
│   ├── 📂 modes/            
│   │   ├── 📄 AutoMode.hpp
│   │   └── 📄 RCMode.hpp
│   ├── 📂 profiles/         
│   │   ├── 📄 arruela.hpp
│   │   ├── 📄 caipora.hpp
│   │   ├── 📄 marola.hpp
│   │   ├── 📄 smoker.hpp
│   │   └── 📄 smokerAuto.hpp
│   └── 📂 services/         
│       ├── 📄 ConfigServer.hpp
│       ├── 📄 MotionPlayer.hpp
│       └── 📄 WebUI.hpp
│
├── 📂 lib/                  
│   └── 📄 README
│
├── 📂 src/                  
│   ├── 📂 hardware/
│   │   ├── 📂 sensors/
│   │   │   └── 📄 JS40F.cpp
│   │   ├── 📄 Drive.cpp
│   │   ├── 📄 IRreader.cpp
│   │   ├── 📄 Receiver.cpp
│   │   ├── 📄 ServoMechanism.cpp
│   │   └── 📄 WeaponSystem.cpp
│   ├── 📂 modes/
│   │   ├── 📄 AutoMode.cpp
│   │   └── 📄 RCMode.cpp
│   ├── 📂 services/
│   │   ├── 📄 ConfigServer.cpp
│   │   └── 📄 MotionPlayer.cpp
│   └── 📄 main.cpp          
│
├── 📂 test/                 
│   ├── 📄 DriveClassTest.cpp
│   └── 📄 README
│
├── 📂 ui/                   
│   └── 📄 index.html
│
├── ⚙️ .clang-format         
├── 👁️ .gitignore            
├── 🛠️ Makefile              
└── ⚙️ platformio.ini        
```

### `include/defs/` — Contratos globais

| Arquivo | O que faz |
|---|---|
| `Config.hpp` | Dispatcher central de perfis. Lê a flag de compilação (`-DROBOT_CAIPORA`, `-DROBOT_SMOKERAUTO`, etc.) e inclui o perfil correto. Todo o código lê de `Config::` sem saber qual robô está compilando. |
| `RobotTypes.hpp` | Define os tipos compartilhados por todo o projeto: `MotionStep`, `MotionSequence`, `ServoConfig`, `AutoStrategy` e a macro `MACRO()` para declarar sequências de movimento. |

### `include/hardware/` — Abstração de peças físicas

| Arquivo | O que faz |
|---|---|
| `Drive.hpp` | Controla os dois motores via MCPWM. Expõe `setSpeed(left, right)` com valores de -100 a 100. Gerencia internamente inversão de sentido, brake e release. |
| `IRreader.hpp` | Lê o controle IR e expõe sinais com semântica clara: `modeRC()`, `modeAuto()`, `start()`. Header-only para evitar problema de definição duplicada da biblioteca IRremote. |
| `Receiver.hpp` | Lê o joystick via Bluepad32. Expõe eixos (`leftStickX`, `rightTrigger`, etc.) e botões com detecção de borda (dispara uma vez por pressão). Gerencia pareamento e segurança por MAC. |
| `ServoMechanism.hpp` | Abstração de um servo físico. Encapsula os ângulos de deploy, retract e relax. Inicializa o timer LEDC e controla a posição do servo. |
| `WeaponSystem.hpp` | Gerencia um conjunto de servos como sistema único. Controla o ciclo `deploy → relax` com timer interno — o `RCMode` só chama `deploy()` e `retract()`, sem saber de timings. |
| `sensors/JS40F.hpp` | Abstração do sensor de presença JS40F. Lê o pino diretamente do registrador GPIO (`GPIO.in`) em vez de `digitalRead()`, garantindo leitura em 1 ciclo de clock. `temAlvo()` retorna `true` quando detecta o adversário. |

### `include/modes/` — Lógica de operação

| Arquivo | O que faz |
|---|---|
| `RCMode.hpp` | Cérebro do modo manual. Lê o joystick, aplica expo no steer, mistura throttle + steer para os motores, gerencia o sistema de armas e executa macros pré-programadas. |
| `AutoMode.hpp` | Cérebro do modo autônomo. Máquina de estados com 6 sub-estados: seleção de estratégia via HUD, desconexão do WiFi, espera de largada, execução do saque, busca e ataque. |

### `include/profiles/` — Configuração por robô

Cada arquivo representa um robô físico. Define pinos de hardware, parâmetros de controle e sequências de movimento. **É o único arquivo que muda entre robôs.**

| Arquivo | Robô |
|---|---|
| `caipora.hpp` | Caipora — RC com 1 servo |
| `smoker.hpp` | Smoker — RC com 2 servos |
| `smokerAuto.hpp` | SmokerAuto — RC + autônomo com 1 servo |
| `arruela.hpp` | Arruela — RC sem servo |
| `marola.hpp` | Marola — RC com 1 servo |

### `include/services/` — Ferramentas dos modos

| Arquivo | O que faz |
|---|---|
| `MotionPlayer.hpp` | Executor de sequências de movimento não-bloqueante. Recebe uma `MotionSequence` (array de passos com velocidade e duração), executa passo a passo usando `millis()` sem travar o loop. Compartilhado entre RC (macros) e AUTO (estratégias). |
| `ConfigServer.hpp` | Servidor web assíncrono que sobe um WiFi AP durante a fase de configuração do modo autônomo. Serve a HUD de seleção de estratégia e expõe a rota `/set-strat` que recebe o payload do celular. |
| `WebUI.hpp` | O HTML da HUD compilado como string C++ para ser servido diretamente da memória do ESP32. Gerado a partir de `ui/index.html`. |

### `src/` — Implementações

Espelha a estrutura do `include/`. Cada `.cpp` implementa o contrato definido no `.hpp` correspondente. O `main.cpp` instancia todos os objetos, gerencia a FSM principal e chama os modos no loop.

### `test/`

| Arquivo | O que faz |
|---|---|
| `DriveClassTest.cpp` | Teste manual dos motores. Grava no robô e observa o comportamento físico para validar direções, velocidades e inversão de sentido. |

### `ui/`

| Arquivo | O que faz |
|---|---|
| `index.html` | Código-fonte editável da HUD de configuração autônoma. Interface web com seleção de saque, direção de busca e sistema de armas. Após editar, o conteúdo deve ser atualizado em `WebUI.hpp`. |

### Arquivos raiz

| Arquivo | O que faz |
|---|---|
| `.clang-format` | Define as regras de formatação automática do código (indentação, espaçamento, alinhamento). Garante estilo consistente em todo o time. |
| `.gitignore` | Lista de arquivos ignorados pelo Git: binários compilados, cache do PlatformIO, arquivos de IDE. |
| `Makefile` | Atalhos para os comandos mais comuns: `make caipora`, `make smokerauto`, `make monitor`. Evita ter que lembrar os comandos do PlatformIO. |
| `platformio.ini` | Configuração de build do projeto. Define um environment por robô com a flag `-DROBOT_X` correspondente, bibliotecas e parâmetros de compilação. |

## Máquina de Estados Principal

Uma vez ligado, o estado inicial do robô é `IDLE`. A transição ocorre via sinal IR:

```text
IDLE ── (IR RC) ──► RCMode
```
ou
```text
IDLE ── (IR AUTO) ──► AutoMode
```

Uma vez ativado, o modo é selado na memória. A saída exige reset físico para garantir um estado limpo dos periféricos.

### Máquina de estados do Modo Autônomo

```text
SELECTING_ESTRATEGIA
    │ (payload recebido via HUD WiFi)
    ▼
DISCONNECTING_WIFI ── delay tático para resposta HTTP (mantém rádio no Core 0)
    │ (Conexão encerrada, FSM segue)
    ▼
READY
    │ (Sinal IR de largada)
    ▼
EXECUTING_ESTRATEGIA ── roda a macro
    │ (Macro finalizada)
    ▼
BUSCA ── varredura reativa ou giro para a última posição conhecida
    │ (Sensor frontal crava alvo)
    ▼
ATTACKING ── força bruta com correção inercial em arco
    │ (Perdeu contato tático)
    ▼
BUSCA ── varredura reativa ou giro para a última posição conhecida
```

## Dependências

O projeto utiliza o gerenciador de pacotes do PlatformIO. As bibliotecas abaixo são injetadas automaticamente no tempo de compilação (ver `platformio.ini`):

* **[Bluepad32_ESP32](https://github.com/ricardoquesada/bluepad32):** Recebimento de inputs de controle PS4/Xbox via Bluetooth (isolado em background no Core 0).
* **[IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote) (z3t0):** Leitura dos sinais infravermelhos de largada/parada do juiz via interrupção de hardware.
* **[ESP32Servo](https://github.com/madhephaestus/ESP32Servo) (madhephaestus):** Geração de PWM estável via hardware (LEDC do ESP32) para o sistema de armas, sem conflito de timers.
* **[ESPAsyncWebServer](https://github.com/mathieucarbou/ESPAsyncWebServer) (mathieucarbou):** Servidor web assíncrono blindado para rodar exclusivamente no Core 0, evitando falhas de LwIP.
* **DNSServer:** Biblioteca nativa do ecossistema ESP32 utilizada para levantar o *Captive Portal* da HUD Tática.

## Como adicionar um novo robô

1. Copie um perfil existente (ex: `caipora.hpp`) dentro de `include/profiles/` e renomeie.
2. Ajuste a pinagem física, compensações de motor e macros de saque.
3. Adicione um novo *environment* no `platformio.ini`:

```ini
[env:meurobo]
build_flags =
    ${env.build_flags}
    -DROBOT_MEUROBO

```

4. Registre o `#elif defined(ROBOT_MEUROBO)` no arquivo `include/defs/Config.hpp`.


## Seleção de modo autônomo (HUD Tática)

O `AutoMode` levanta um Access Point WiFi provisório para injeção da tática pré-round:

1. Selecione a rotina AUTO com o controle remoto.
2. Conecte no WiFi do robô (Senha: `sumo1234`).
3. Acesse `4.3.2.1` no navegador.
4. Defina a macro de saque, direção de varredura e armamento.
5. Pressione **[SUBMETER]**. O robô responderá, desconectará o cliente e isolará o hardware de rede no Core 0.
6. A FSM entra em estado de letargia `READY` aguardando apenas a luz do sinal IR.

## Estratégia Autônoma de Tempo Real

Em produção.

## Decisões Críticas de Arquitetura

**Por que a classe JS40F não usa `digitalRead()`?**
O `digitalRead` do framework Arduino adiciona *overhead* de software para validar pinos e timers. Na classe `JS40F`, lemos diretamente os registradores `GPIO.in` e `GPIO.in1.val` do silício do ESP32. Retorno atômico em 1 ciclo de clock. Latência virtualmente zero.

**Por que não desligamos o hardware do WiFi (WIFI_OFF) no AutoMode?**
Bibliotecas web assíncronas falham catastroficamente ao sofrer *teardown* de rede de forma abrupta, resultando em *Double Exception* no Core 0 (LwIP). Nossa solução tática é a expulsão de clientes (`softAPdisconnect`) e o isolamento total do processo web no Core 0. O combate ocorre no Core 1, sem preempção ou perda de determinismo causados pelo stack de rede ocioso. Se precisarmos do Core 0 no futuro vamos ter um problema :)

**Construtores Explícitos (`explicit`)**
O hardware exige clareza. Instâncias físicas (Sensores, Ponte H) são inicializadas antes mesmo da função `setup()` via *Initializer Lists* de C++, impedindo instanciar objetos "fantasmas" sem pinos físicos designados.

**Ausência de `delay()`**
É proibido o uso de `delay()` no Core 1. Toda a inteligência artificial, contagem de saques e leituras ocorrem através do clock do `millis()` gerenciado pela máquina de estados, assegurando um loop de verificação contínuo a mais de 10.000 Hz.

Meus parabéns se chegou até aqui!