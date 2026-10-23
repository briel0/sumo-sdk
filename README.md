# SUMO-SDK

Firmware modular para os robôs de Mini Sumô da **TamanduTech**, baseados em ESP32. Um único codebase compila para vários robôs (Caipora, Smoker, Arruela e Marola) através de perfis de hardware independentes: cada robô é um arquivo de configuração, e o resto do código lê de `Config::` sem saber qual robô está compilando.

Cada robô tem dois firmwares: **AUTO** (autônomo, configurado pelo celular via Bluetooth Low Energy antes do round) e **RC** (pilotado com controle de PS4/Xbox via Bluepad32). Toda a lógica de combate roda em máquinas de estados não bloqueantes.

---

## Robôs

| Robô | Sensores | Arma |
|---|---|---|
| **Caipora** | 4× VL53L0X (ToF), LDR frontal | 1 servo |
| **Smoker** | 3× JS40F, TSSP4038 (IR), 2× QRE1113 (linha) | 2 servos |
| **Arruela** | 3× JS40F, VL53L0X, 2× QRE1113 (linha) | sem arma |
| **Marola** | 3× LDR, 2× QRE1113 (linha) | 1 servo |

## Como compilar e gravar

Requer [PlatformIO](https://platformio.org/). Cada robô tem três *environments*:

| Env | O que é |
|---|---|
| `<robo>` | Firmware AUTO (`src/main.cpp`) |
| `<robo>_rc` | Firmware RC (`src/tools/RCFirmware/main.cpp`) |
| `<robo>_sensores` | Teste de bancada: lê todos os sensores e imprime no serial |

```sh
pio run -e arruela -t upload        # AUTO da Arruela
pio run -e arruela_rc -t upload     # RC da Arruela
pio device monitor -b 115200
```

O `Makefile` tem atalhos (`make caipora`, `make sensores-smoker`, `make monitor`...).

**Windows:** copie `platformio.local.ini.example` para `platformio.local.ini`. Ele aponta o `core_dir` do PlatformIO para `C:/pio`, porque o caminho padrão estoura o limite de 260 caracteres e quebra com espaço ou acento no nome do usuário.

> AUTO e RC usam frameworks diferentes: o Arduino-ESP32 padrão no AUTO (precisa do Bluedroid para o BLE) e o fork do Bluepad32 no RC (BTstack). Alternar entre envs AUTO e RC faz o PlatformIO trocar o framework instalado, então o primeiro build depois da troca demora mais.

## Usando o robô

### Modo AUTO

1. Ligue o robô. Os LEDs ficam em onda enquanto ele espera a configuração.
2. Abra o site de configuração (`ui/index.html`, publicado no GitHub Pages) num navegador com Web Bluetooth (Chrome/Edge) e conecte no robô pelo nome.
3. Escolha a estratégia de saque, o lado, a distância, o atraso de largada e se a arma abre. O site também testa motores, macros e sensores ao vivo.
4. Envie. O robô desliga o rádio e fica em `READY`, esperando o IR do juiz.

Controle IR:

| Botão | Ação |
|---|---|
| 1 | Ready |
| 2 | Largada |
| 3 | Stop (para os motores e reinicia o robô) |

### Modo RC

Ligue o robô e pareie o controle (o primeiro controle que conectar é aceito). Gatilho direito acelera, esquerdo dá ré, stick esquerdo vira. X é acelerador digital a 100%. Os botões disparam macros e controlam a arma conforme o perfil do robô.

Segurar **Start** por 0,7 s alterna a polaridade dos motores (ver `MotorPolarity` abaixo).

## Estrutura

```text
📦 SUMO-SDK
├── 📂 include/            Headers (contratos)
│   ├── 📂 defs/           Config.hpp (escolhe o perfil), RobotTypes.hpp (tipos compartilhados)
│   ├── 📂 hardware/       Drive, MotorPolarity, Receiver, IRreader, StatusLED, servos e armas
│   │   └── 📂 sensors/    JS40F, LDR, QRE1113, ToFSensor (VL53L0X), TSSP4038
│   ├── 📂 modes/          AutoMode, RCMode
│   ├── 📂 profiles/       Um arquivo por robô: pinos, velocidades, macros, limiares
│   ├── 📂 services/       BleConfigServer (site via BLE), MotionPlayer (macros)
│   └── 📂 strategies/     CombatStrategy (interface) + uma estratégia por robô
├── 📂 src/                Implementações, espelhando o include/
│   └── 📂 tools/          Firmware RC, testes de sensores e teste de BLE
├── 📂 ui/                 Site de configuração (Web Bluetooth)
├── 📂 test/               Teste manual dos motores
├── 📂 brand/              Identidade visual da TamanduTech
├── 🛠️ Makefile
└── ⚙️ platformio.ini      Um env por robô e por firmware
```

### Peças principais

| Componente | O que faz |
|---|---|
| `Config.hpp` | Lê a flag `-DROBOT_*` do env e inclui o perfil e a estratégia do robô. |
| `profiles/*.hpp` | Tudo que muda entre robôs: pinos, coeficientes de pilotagem, servos, limiares de sensor e tabelas de macros. |
| `Drive` | Controla os dois motores (ponte H A4950) via MCPWM a 20 kHz. `setSpeed(esq, dir)` de -100 a 100, com freio ativo em 0 e alívio automático na inversão de sentido. |
| `MotorPolarity` | Corrige motor invertido ou lados trocados sem mexer nos pinos. Guarda a correção na NVS, que sobrevive a reflash. |
| `MotionPlayer` | Executa sequências de movimento (`{vel esq, vel dir, ms}`) sem bloquear o loop. Usado nas macros do RC e nos saques do AUTO. |
| `CombatStrategy` | Interface das estratégias autônomas: `init()`, `configure()` (recebe o que veio do site) e `autoEngage()` (um frame de combate). Cada robô implementa a sua. |
| `BleConfigServer` | Servidor GATT que o site usa para configurar o AUTO e rodar os testes de bancada. |
| `Receiver` | Lê o controle via Bluepad32: eixos com zona morta e botões com detecção de borda. |
| `WeaponSystem` | Agrupa os servos da arma e cuida do ciclo armar → relaxar sozinho. |

## Máquina de estados do AUTO

```text
SELECTING_ESTRATEGIA ── site conectado por BLE: testes de motor/macro/sensor
    │ (configuração recebida)
    ▼
DISCONNECTING_WIFI ── 500 ms para o rádio responder, depois desliga
    ▼
READY ── espera o IR (1 = ready, 2 = largada)
    │ (largada)
    ▼
EXECUTING_ESTRATEGIA ── atraso de largada + macro de saque escolhida no site
    │ (macro terminou)
    ▼
FIGHTING ── CombatStrategy::autoEngage() do robô: busca, ataque e fuga de borda
```

O IR 3 para tudo e reinicia o robô em qualquer estado.

## Como adicionar um novo robô

1. Copie um perfil de `include/profiles/` e ajuste pinos, coeficientes, servos e macros.
2. Crie a estratégia em `include/strategies/` + `src/strategies/` implementando `CombatStrategy`.
3. Registre o robô em `include/defs/Config.hpp`:
   ```cpp
   #elif defined(ROBOT_MEUROBO)
   #include "profiles/meurobo.hpp"
   #include "strategies/MeuRoboAuto.hpp"
   using ActiveAuto = MeuRoboAuto;
   ```
4. Adicione os envs no `platformio.ini`, copiando os de um robô existente (`meurobo`, `meurobo_rc` e, se quiser, `meurobo_sensores`). Lembre de excluir as estratégias dos outros robôs no `build_src_filter` do AUTO.

## Decisões de arquitetura

**Por que AUTO e RC são firmwares separados?**
O BLE do site usa o Bluedroid; o controle usa o Bluepad32, que roda sobre o BTstack. As duas stacks disputavam o mesmo controlador Bluetooth (o BLE nunca aparecia em nenhum scanner), e o fork do Bluepad32 é compilado sem o Bluedroid. Separar os dois resolveu isso. O custo é que trocar de modo exige reflash.

**Por que o motor invertido não se corrige trocando pino?**
Os pinos de cada robô são fixos no perfil, e todas as macros e estratégias foram calibradas em cima deles. O `MotorPolarity` aplica a correção (inverter esquerda, inverter direita, trocar lados) sobre a velocidade, logo antes do `Drive`, e guarda na NVS. O `Drive` é um objeto global criado antes do `setup()`, quando a NVS ainda não está garantida, por isso ele salva a correção e não os pinos.

**Por que o JS40F não usa `digitalRead()`?**
O sensor é lido direto dos registradores `GPIO.in`/`GPIO.in1` do ESP32, sem a validação de pino e o overhead do `digitalRead()` do Arduino.

**Por que nada de `delay()` no combate?**
Macros, saques e leituras lentas (como os VL53L0X no I2C) são cadenciados por `millis()` dentro das máquinas de estado. Assim o loop de combate nunca fica parado esperando, e o robô reage aos sensores e ao IR a cada volta.

## Dependências

Gerenciadas pelo PlatformIO, com versões fixas no `platformio.ini`:

* **[Bluepad32](https://github.com/ricardoquesada/bluepad32)**: controles PS4/Xbox via Bluetooth (só no firmware RC).
* **[IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote)**: leitura do controle IR do juiz.
* **[ESP32Servo](https://github.com/madhephaestus/ESP32Servo)**: PWM dos servos da arma.
* **[FastLED](https://github.com/FastLED/FastLED)**: fita de LEDs de status.
* **[VL53L0X](https://github.com/pololu/vl53l0x-arduino)** (Pololu): sensores de distância ToF.

Meus parabéns se chegou até aqui!
