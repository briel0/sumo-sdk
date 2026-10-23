# AGENTS.md

Instruções pra agentes de IA (Claude Code, Codex, Cursor...) trabalhando no `sumo-sdk`.

## O projeto

Firmware ESP32 (Arduino + PlatformIO) dos robôs de sumô da TamanduTech: Caipora, Smoker, Arruela e Marola. Um só código, um perfil por robô em `include/profiles/`, escolhido pela flag `-DROBOT_*` do env. Todo o código lê de `Config::` sem saber qual robô está compilando.

O `README.md` está parcialmente desatualizado (ainda fala de `ConfigServer`/WiFi, que virou `BleConfigServer`/BLE). Na dúvida, o código manda.

## Build

- Um env por robô e por função no `platformio.ini`:
  - `<robo>`: firmware AUTO
  - `<robo>_rc`: firmware RC (`src/tools/RCFirmware/main.cpp`)
  - `<robo>_sensores`: teste de bancada dos sensores
- AUTO e RC são firmwares separados (duas stacks Bluetooth não convivem). Trocar de modo é reflash.
- Compilar: `pio run -e <env>`. Gravar: `pio run -e <env> -t upload`. Atalhos no `Makefile`.
- `platformio.local.ini` (ignorado pelo git, ver `.example`) guarda ajustes da máquina, como `core_dir = C:/pio` no Windows.
- Não trocar `lib_ldf_mode = chain` por `chain+` nem mexer na partição `huge_app.csv`. Os motivos estão comentados no `platformio.ini`.

## Regras do hardware

- **Motor com lado trocado ou invertido se corrige pelo `MotorPolarity`** (NVS, segurar Start no RC ou pelo site), **nunca mudando os pinos do perfil**. Os pinos são fixos por robô e as macros/estratégias foram calibradas em cima deles.
- `Drive::setSpeed` recebe -100..100. O driver é A4950 em slow decay: velocidade 0 freia, e `release()` solta.
- Mexer na escala do `Drive` muda o comportamento de todo `VEL_*`, macro e estratégia. Avise que vai precisar recalibrar.
- Nada de `delay()` no caminho quente de pilotagem. Macros e sequências usam o `MotionPlayer` (não bloqueante). Leituras lentas (ToF/I2C) são espaçadas por intervalo, não feitas todo frame.

## Código

- Comentários em português, explicando o porquê (é o estilo do repo, pode ser longo quando a decisão não é óbvia).
- Formatação pelo `.clang-format` (LLVM, indentação 4, coluna 120).
- `#ifdef ROBOT_*` só quando um recurso existe em um perfil e não nos outros. Prefira constante no perfil.

## Git

- Mensagens de commit em inglês, no formato `tipo(escopo): descrição`. Tipos usados: `feat`, `fix`, `tweak` (ajuste de valores/calibração), `refactor`, `build`, `revert`. Escopo é o robô ou o módulo (`arruela`, `smoker`, `auto`, `rc`, `drive`, `ui`, `pio`...). Corpo explica o porquê.
- Trabalho novo vai em branch (`feat/...`, `build/...`, `experimental/...`) e entra na `main` por merge/PR. Não fazer push direto na `main` sem combinar.
- Antes de dar push, `git pull` pra não criar merge "Merge branch 'main' of ..." à toa.

## UI

`ui/index.html` é o site de configuração (Web Bluetooth, fala com o `BleConfigServer`). É publicado no GitHub Pages pelo workflow `.github/workflows/deploy-ui-pages.yml`.

## Pendências

Drive / RC:
- Recalibrar os `VEL_*`, edge escape, PINBALL e macros depois da troca pra slow decay (`feat/drive-slow-decay`).
- Migrar o `Drive` pro MCPWM v2 (driver novo do ESP-IDF).
- Descobrir por que o RC parece pior que o do Briga. Suspeitas: o fast decay a 20 kHz (resolvido pelo slow decay) e a mistura `throttle ± steer`, que inverte a roda de dentro em curva com gatilho parcial. Alternativa: só reduzir a roda de dentro quando há throttle.
- Avaliar a troca da biblioteca do controle (Bluepad32 vs. PS4Controller do Briga).
- Lista de controles (MACs) aceitos. Hoje o `Receiver` aceita o primeiro que conectar no pareamento.

Arquitetura:
- Reunificar AUTO e RC num firmware só (em avaliação). Foram separados porque Bluedroid (BLE do site) e BTstack (Bluepad32) disputavam o controller Bluetooth. Ver `src/tools/RCFirmware/main.cpp`. O trabalho de modo de boot na NVS do Giovanni está dentro da branch `feat/fumacinha-ble`.
- Programação orientada a componentes. Definir o objetivo concreto antes: o repo já tem composição (`Mechanism`, `WeaponSystem`) e dois refactors grandes nessa linha foram abandonados.
- Simplificar removendo constantes que ninguém usa. Cuidado: as constantes dos perfis são a configuração dos robôs.

Site / sensores:
- LEDs indicando qual sensor está vendo.
- Calibração de pinos e thresholds pelo site, como no Fuego. Pinos pela NVS exigem tirar o bind do MCPWM do construtor global do `Drive` (ver `MotorPolarity.hpp`) e validar os GPIOs aceitos.

Versão:
- Bump de versão do firmware (SemVer + tag `vX.Y.Z` no git), mostrando a versão no serial e no site.
