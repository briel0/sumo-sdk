// ARQUIVO GERADO AUTOMATICAMENTE. NÃO EDITE DIRETAMENTE.
// Modifique os arquivos .html, .css e .js na pasta ui/ e recompile o projeto.
#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>SUMO AUTO</title>
    <style>
        :root {
            --sea-green: #2c9246;
            --dark-spruce: #124925;
            --lemon-chiffon: #eeecc5;
            --grey-olive: #847f79;
            --dim-grey: #6e6965;
            --void-bg: #05140a; 
            --alert-red: #ff3333;
        }

        * { box-sizing: border-box; font-family: 'Courier New', Courier, monospace; }
        
        body { 
            background-color: var(--void-bg); 
            color: var(--lemon-chiffon); 
            margin: 0; 
            padding: 15px;
            padding-bottom: 80px; 
            user-select: none; 
            -webkit-user-select: none;
        }
        
        h1 { 
            color: var(--sea-green); 
            font-size: 1.2rem; 
            text-align: center; 
            border-bottom: 2px solid var(--dark-spruce); 
            padding-bottom: 10px; 
            margin-top: 5px; 
        }
        
        h2 { 
            font-size: 0.9rem; 
            color: var(--lemon-chiffon); 
            margin-top: 25px; 
            margin-bottom: 10px; 
            border-left: 4px solid var(--sea-green); 
            padding-left: 8px; 
        }
        
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
        
        button {
            background-color: transparent;
            font-size: 0.85rem;
            font-weight: bold;
            text-transform: uppercase;
            width: 100%;
            cursor: pointer;
            transition: 0.1s;
            padding: 18px 5px;
        }
        
        button:active { filter: brightness(1.5); }

        .btn-macro {
            border: 2px solid var(--sea-green);
            color: var(--lemon-chiffon);
        }
        .btn-macro.selected { 
            background-color: var(--sea-green); 
            color: var(--void-bg);
        }

        .btn-direction { 
            border: 2px solid var(--dim-grey);
            color: var(--lemon-chiffon);
        }
        .btn-direction.selected { 
            background-color: var(--dim-grey); 
            color: var(--void-bg); 
        }

        .btn-search { 
            border: 2px solid var(--grey-olive);
            color: var(--lemon-chiffon); 
        }
        .btn-search.selected { 
            background-color: var(--grey-olive); 
            color: var(--void-bg); 
        }

        .btn-weapon { 
            border: 2px solid var(--lemon-chiffon);
            color: var(--lemon-chiffon);
        }
        .btn-weapon.selected { 
            background-color: var(--lemon-chiffon); 
            color: var(--dark-spruce); 
        }

        .btn-submit {
            margin-top: 30px;
            background-color: var(--dark-spruce);
            border: 3px solid var(--sea-green);
            color: var(--lemon-chiffon);
            font-size: 1.2rem;
            padding: 20px;
            letter-spacing: 2px;
        }
        
        /* TERMINAL DE LOG FIXO NO RODAPÉ */
        #terminal-log {
            position: fixed;
            bottom: 0;
            left: 0;
            right: 0;
            padding: 10px;
            background-color: #000;
            border-top: 2px solid var(--dark-spruce);
            font-size: 0.7rem;
            color: var(--sea-green);
            overflow-x: auto;
            white-space: nowrap;
            z-index: 99;
        }

        /* MODAL BRUTALISTA DE CONFIRMAÇÃO / ERRO */
        .modal-overlay {
            display: none;
            position: fixed;
            top: 0; left: 0; width: 100%; height: 100%;
            background-color: rgba(5, 20, 10, 0.95);
            z-index: 100;
            padding: 20px;
            justify-content: center;
            align-items: center;
        }
        
        .modal-box {
            background-color: #000;
            border: 3px solid var(--sea-green);
            width: 100%;
            max-width: 400px;
            padding: 20px;
        }

        .modal-box.error-mode {
            border-color: var(--alert-red);
        }

        .modal-title {
            font-size: 1.1rem;
            font-weight: bold;
            text-align: center;
            letter-spacing: 2px;
            margin-bottom: 20px;
            border-bottom: 2px solid var(--dim-grey);
            padding-bottom: 8px;
        }

        .modal-body {
            font-size: 1.1rem; 
            font-weight: bold; 
            line-height: 1.5;
            letter-spacing: 1.5px;
            margin-bottom: 25px;
            color: var(--lemon-chiffon); 
        }
        
        /* ESTILO DOS "MINI-BOTÕES" NO MODAL */
        .modal-chip {
            display: block;
            width: 100%;
            padding: 12px;
            margin-bottom: 8px;
            text-align: center;
            font-size: 1rem;
            font-weight: bold;
            text-transform: uppercase;
            pointer-events: none; /* Impede clique */
        }
        
        /* Texto final antes dos botões de ação */
        .modal-prompt {
            text-align: center;
            margin-top: 20px;
            font-size: 0.9rem;
            color: var(--grey-olive);
        }

        .modal-actions {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
        }

        .modal-actions.single-btn {
            grid-template-columns: 1fr;
        }

        .btn-abort { border: 2px solid var(--alert-red); color: var(--alert-red); letter-spacing: 1px; }
        .btn-abort:active { background-color: var(--alert-red); color: #fff; }
        
        .btn-confirm { border: 2px solid var(--sea-green); color: var(--sea-green); letter-spacing: 1px; }
        .btn-confirm:active { background-color: var(--sea-green); color: #000; }

        /* ESTILIZAÇÃO DO CONSTRUTOR DE MACROS */
    .step-row {
        display: flex;
        align-items: center;
        gap: 8px;
        margin-bottom: 12px;
        background-color: rgba(44, 146, 70, 0.05);
        padding: 10px;
        border-left: 3px solid var(--sea-green);
    }

    .step-row span {
        font-weight: bold;
        color: var(--lemon-chiffon);
        min-width: 65px;
        font-size: 0.85rem;
    }

    .step-input {
        flex: 1;
        width: 100%;
        background-color: transparent;
        border: 1px solid var(--dark-spruce);
        color: var(--sea-green);
        padding: 10px 5px;
        font-size: 0.9rem;
        font-weight: bold;
        text-align: center;
        border-radius: 0;
        appearance: textfield; /* Remove setinhas em alguns navegadores */
    }

    .step-input:focus {
        outline: none;
        border-color: var(--sea-green);
        background-color: rgba(44, 146, 70, 0.15);
    }

    .step-input::placeholder {
        color: var(--dim-grey);
        font-size: 0.75rem;
    }

    </style>
</head>
<body>

    <h1>SUMO AUTO</h1>

    <h2>MOVIMENTAÇÃO INICIAL</h2>
    <div class="grid">
        <button class="btn-macro" data-category="macro" data-val="0" data-label="FRENTÃO">FRENTÃO</button>
        <button class="btn-macro" data-category="macro" data-val="1" data-label="FRENTÃO [FAST]">FRENTÃO [FAST]</button>
        <button class="btn-macro" data-category="macro" data-val="2" data-label="FRENTINHO">FRENTINHO</button>
        <button class="btn-macro" data-category="macro" data-val="3" data-label="CURVÃO">CURVÃO</button>
        <button class="btn-macro" data-category="macro" data-val="4" data-label="CURVINHA">CURVINHA</button>
        <button class="btn-macro" data-category="macro" data-val="5" data-label="EM V">EM V</button>
        <button class="btn-macro" data-category="macro" data-val="6" data-label="COSTAS">COSTAS</button>
        <button class="btn-macro" data-category="macro" data-val="7" data-label="DEFENSIVO">DEFENSIVO</button>
    </div>

    <h2>DIREÇÃO</h2>
    <div class="grid">
        <button class="btn-direction" data-category="direction" data-val="E" data-label="ESQUERDA">ESQUERDA</button>
        <button class="btn-direction" data-category="direction" data-val="D" data-label="DIREITA">DIREITA</button>
    </div>

    <h2>BUSCA E TÁTICA</h2>
    <div class="grid">
        <button class="btn-search" data-category="search" data-val="1" data-label="BUSCA PADRÃO">BUSCA PADRÃO</button>
        <button class="btn-search" data-category="search" data-val="2" data-label="BUSCA LENTA">BUSCA LENTA</button>
    </div>

    <h2>SISTEMA DE ARMAS</h2>
    <div class="grid">
        <button class="btn-weapon" data-category="weapon" data-val="0" data-label="DESARMAR">DESARMAR</button>
        <button class="btn-weapon" data-category="weapon" data-val="1" data-label="NÃO DESARMAR">NÃO DESARMAR</button>
    </div>

    <button class="btn-submit" onclick="openTacticalConfirm()">[SUBMETER]</button>

    <h2>TESTE DE MACRO</h2>
    <div id="macro-steps">
        <div class="step-row" data-index="0">
            <span>Passo 1</span>
            <input type="text" class="step-input" placeholder="ESQ %" id="l0" min="-100" max="100">
            <input type="text" class="step-input" placeholder="DIR %" id="r0" min="-100" max="100">
            <input type="text" class="step-input" placeholder="MS" id="d0" min="50" max="5000">
        </div>
    </div>
    <div class="grid" style="margin-bottom: 10px;">
        <button class="btn-macro" onclick="addStep()">ADICIONAR PASSO</button>
        <button class="btn-macro" onclick="removeStep()">REMOVER PASSO</button>
    </div>
    <button class="btn-submit" onclick="dispararMacro()">[DISPARAR MACRO]</button>

    <div id="terminal-log">> SYS STANDBY // UPLINK READY</div>

    <div id="custom-modal" class="modal-overlay">
        <div id="modal-container" class="modal-box">
            <div id="modal-title" class="modal-title">CONFIRMAÇÃO TÁTICA</div>
            <div id="modal-body" class="modal-body"></div>
            <div id="modal-actions" class="modal-actions"></div>
        </div>
    </div>

    <script>
        // =================================================
        // LÓGICA DO TESTADOR DE MACRO
        // =================================================
        let numSteps = 1;

        function addStep() {
            if(numSteps >= 8) return;
            const div = document.createElement('div');
            div.className = 'step-row';
            div.dataset.index = numSteps;
            div.innerHTML = `
                <span>Passo ${numSteps + 1}</span>
                <input type="text" class="step-input" placeholder="ESQ %" id="l${numSteps}">
                <input type="text" class="step-input" placeholder="DIR %" id="r${numSteps}">
                <input type="text" class="step-input" placeholder="MS" id="d${numSteps}">
            `;
            document.getElementById('macro-steps').appendChild(div);
            numSteps++;
        }

        function removeStep() {
            if(numSteps <= 1) return;
            numSteps--;
            const rows = document.querySelectorAll('.step-row');
            rows[rows.length - 1].remove();
        }

        function parseInput(id, fallback) {
            const val = document.getElementById(id).value;
            return val !== '' ? parseInt(val) : fallback;
        }

        function dispararMacro() {
            const d0 = document.getElementById('d0').value;
            if(!d0) {
                showUiAlert('[!] ERRO', 'Preencha pelo menos a duração do Passo 1.', true);
                return;
            }

            let params = `steps=${numSteps}`;
            for(let i = 0; i < numSteps; i++) {
                const l = parseInput(`l${i}`, 0);
                const r = parseInput(`r${i}`, 0);
                const d = parseInput(`d${i}`, 300);
                params += `&l${i}=${l}&r${i}=${r}&d${i}=${d}`;
            }

            printLog('> DISPARANDO MACRO DE TESTE...');
            fetch('/test-macro?' + params)
                .then(res => res.text())
                .then(txt => {
                    printLog('> [OK] MACRO DISPARADA NO ROBÔ');
                    showUiAlert('MACRO DISPARADA', txt, false);
                })
                .catch(() => {
                    printLog('> [ERRO] ROBÔ NÃO RESPONDEU', true);
                    showUiAlert('NETWORK ERROR', 'Falha ao disparar macro.<br>Verifique o Wi-Fi.', true);
        });
}

        // =================================================
        // LÓGICA PRINCIPAL DA INTERFACE
        // =================================================
        const stratState = {
            macro: { val: '', label: 'NENHUM' },
            direction: { val: '', label: 'NENHUM' },
            search: { val: '', label: 'NENHUM' },
            weapon: { val: '', label: 'NENHUM' }
        };

        const logTerm = document.getElementById('terminal-log');
        const modal = document.getElementById('custom-modal');
        const mContainer = document.getElementById('modal-container');
        const mTitle = document.getElementById('modal-title');
        const mBody = document.getElementById('modal-body');
        const mActions = document.getElementById('modal-actions');

        function printLog(msg, isError = false) {
            logTerm.innerText = msg;
            logTerm.style.color = isError ? 'var(--alert-red)' : 'var(--sea-green)';
        }

        function closeModal() {
            modal.style.display = 'none';
        }

        function showUiAlert(title, text, isError = false) {
            mContainer.className = isError ? "modal-box error-mode" : "modal-box";
            mTitle.innerText = title;
            mTitle.style.color = isError ? 'var(--alert-red)' : 'var(--sea-green)';
            mBody.innerHTML = `<div style="text-align:center;">${text}</div>`;
            mActions.className = "modal-actions single-btn";
            mActions.innerHTML = `<button class="btn-confirm" onclick="closeModal()">OK</button>`;
            modal.style.display = 'flex';
        }

        document.querySelectorAll('button[data-category]').forEach(function(btn) {
            btn.addEventListener('click', function() {
                if (navigator.vibrate) navigator.vibrate(20); 
                
                const category = this.getAttribute('data-category');
                const val = this.getAttribute('data-val');
                const label = this.getAttribute('data-label');
                
                if (this.classList.contains('selected')) {
                    this.classList.remove('selected');
                    stratState[category] = { val: '', label: 'NENHUM' };
                    printLog(`> CATEGORIA ${category.toUpperCase()} DESMARCADA`);
                }
                else {
                    document.querySelectorAll(`button[data-category="${category}"]`).forEach(function(b) {
                        b.classList.remove('selected');
                    });
                    this.classList.add('selected');
                    stratState[category] = { val: val, label: label };
                    printLog(`> SEL: ${category.toUpperCase()} -> ${label}`);
                }
            });
        });

        function openTacticalConfirm() {
            if (!stratState.macro.val) {
                showUiAlert("[!] ERRO CRÍTICO", "Selecione obrigatoriamente a Movimentação Inicial antes de realizar o Uplink.", true);
                printLog("> ERRO: PREENCHIMENTO INCOMPLETO", true);
                return;
            }

            // Construção do Relatório Visual com Caixinhas Coloridas
            let reportHtml = "";
            
            reportHtml += `<div class="modal-chip btn-macro selected">${stratState.macro.label}</div>`;
            
            if (stratState.direction.val) {
                reportHtml += `<div class="modal-chip btn-direction selected">${stratState.direction.label}</div>`;
            }
            
            if (stratState.search.val) {
                reportHtml += `<div class="modal-chip btn-search selected">${stratState.search.label}</div>`;
            }
            
            if (stratState.weapon.val) {
                reportHtml += `<div class="modal-chip btn-weapon selected">${stratState.weapon.label}</div>`;
            }

            reportHtml += `<div class="modal-prompt">Transmitir parâmetros para o robô?</div>`;

            mContainer.className = "modal-box";
            mTitle.innerText = "CONFIRMAÇÃO TÁTICA";
            mTitle.style.color = 'var(--lemon-chiffon)';
            mBody.innerHTML = reportHtml;
            
            mActions.className = "modal-actions";
            mActions.innerHTML = `
                <button class="btn-abort" onclick="closeModal()">ABORTAR</button>
                <button class="btn-confirm" onclick="executeTransmit()">TRANSMITIR</button>
            `;
            modal.style.display = 'flex';
        }

        function executeTransmit() {
            if (navigator.vibrate) navigator.vibrate([40, 50, 40]);

            closeModal();
            printLog("> EXECUTANDO UPLINK...");

            const params = new URLSearchParams();
            Object.keys(stratState).forEach(function(key) {
                if (stratState[key].val !== '') {
                    params.append(key, stratState[key].val);
                }
            });

            const endpoint = '/set-strat?' + params.toString();
            printLog("> TX: " + endpoint);
            
            fetch(endpoint, { method: 'GET' })
                .then(function(response) {
                    if(response.ok) {
                        showUiAlert("UPLINK SUCCESS", "Configuração salva com sucesso!<br><br>Status: 200 OK", false);
                        printLog("> [OK 200] PARÂMETROS TRAVADOS NO ROBÔ");
                    } else {
                        showUiAlert("UPLINK FAILURE", "O robô recebeu os dados mas retornou erro:<br><br>STATUS " + response.status, true);
                        printLog("> [ERRO] STATUS HTTP: " + response.status, true);
                    }
                })
                .catch(function(err) {
                    showUiAlert("NETWORK ERROR", "Falha crítica de comunicação.<br><br>Verifique se o celular ainda está conectado no Wi-Fi do robô.", true);
                    printLog("> [ERRO FATAL] SEM RESPOSTA DA REDE", true);
                });
        }
    </script>
</body>
</html>
)rawliteral";

const char JOYSTICK_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>SUMO AUTO // TACTICAL JOYSTICK</title>
    <style>
        :root {
            --void-bg: #05140a; 
            --sea-green: #2c9246;
            --dark-spruce: #124925;
            --lemon-chiffon: #eeecc5;
            --grey-olive: #847f79;
            --dim-grey: #6e6965;
            --alert-red: #ff3333;
            --edit-yellow: #f1c40f;
        }

        * { 
            box-sizing: border-box; 
            margin: 0; 
            padding: 0; 
            font-family: 'Courier New', Courier, monospace; 
        }
        
        body { 
            background-color: var(--void-bg); 
            color: var(--lemon-chiffon); 
            width: 100vw;
            height: 100dvh; 
            overflow: hidden; 
            position: fixed; 
            user-select: none; 
            -webkit-user-select: none;
            touch-action: none; 
        }

        #btn-engage {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            background-color: var(--dark-spruce);
            border: 3px solid var(--sea-green);
            color: var(--lemon-chiffon);
            font-size: 1.2rem;
            font-weight: bold;
            padding: 20px 30px;
            letter-spacing: 2px;
            cursor: pointer;
            z-index: 100;
        }
        #btn-engage:active { background-color: var(--sea-green); color: var(--void-bg); }

        #combat-hud { display: none; width: 100%; height: 100%; position: relative; }
        #combat-hud.active { display: block; }

        h2 {
            font-size: 0.8rem; 
            color: var(--lemon-chiffon); 
            text-align: center;
            margin-bottom: 10px;
            letter-spacing: 1px;
            font-weight: bold;
            pointer-events: none;
        }

        /* ================= BARRA DE FERRAMENTAS SUPERIOR ================= */
        #top-toolbar {
            position: absolute;
            top: 10px;
            left: 50%;
            transform: translateX(-50%);
            display: flex;
            gap: 10px;
            z-index: 90;
        }

        .tool-btn {
            background: transparent; border: 2px solid var(--dim-grey); color: var(--dim-grey);
            padding: 8px 15px; font-weight: bold; font-size: 0.8rem; cursor: pointer;
        }
        .tool-btn:active { background: var(--dim-grey); color: #000; }

        #edit-controls { display: none; gap: 10px; }
        body.editing #btn-edit { display: none; }
        body.editing #edit-controls { display: flex; }

        /* ================= ZONAS DE CONTROLE (DRAGGABLE) ================= */
        .control-zone {
            position: absolute; display: flex; flex-direction: column; align-items: center;
            transform-origin: center center; padding: 10px;
            /* Invisível até que o JS calcule a posição correta */
            opacity: 0; transition: opacity 0.2s;
        }

        body.editing .control-zone {
            border: 2px dashed var(--edit-yellow); background: rgba(241, 196, 15, 0.05); cursor: move;
        }
        body.editing .control-zone.selected {
            border: 2px solid var(--edit-yellow); background: rgba(241, 196, 15, 0.15);
        }

        /* ================= JOYSTICK & BOTÕES PS4 ================= */
        #joystick-base {
            width: 160px; height: 160px; border: 4px solid var(--dark-spruce);
            border-radius: 50%; background: rgba(44, 146, 70, 0.05); position: relative;
        }
        #joystick-knob {
            width: 60px; height: 60px; background-color: var(--sea-green); border-radius: 50%;
            position: absolute; top: 50%; left: 50%; margin-top: -30px; margin-left: -30px; 
            pointer-events: none; transition: transform 0.05s linear;
        }
        #joystick-base:not(.active) #joystick-knob {
            transition: transform 0.2s ease-out; transform: translate(0px, 0px) !important;
        }
        #joystick-coords { margin-top: 10px; color: var(--grey-olive); font-size: 0.8rem; font-weight: bold; }

        .ps4-pad {
            display: grid;
            grid-template-columns: 65px 65px 65px;
            grid-template-rows: 65px 65px 65px;
            gap: 8px; justify-content: center;
        }
        .btn-ps4 {
            background-color: transparent; border-radius: 50%; font-size: 1.4rem; font-weight: bold; border-width: 3px;
            display: flex; align-items: center; justify-content: center; cursor: pointer;
            transition: transform 0.05s; /* Suaviza o impacto do toque */
        }
        .btn-tri { grid-column: 2; grid-row: 1; border-color: var(--sea-green); color: var(--sea-green); }
        .btn-sq  { grid-column: 1; grid-row: 2; border-color: var(--lemon-chiffon); color: var(--lemon-chiffon); }
        .btn-cir { grid-column: 3; grid-row: 2; border-color: var(--alert-red); color: var(--alert-red); }
        .btn-x   { grid-column: 2; grid-row: 3; border-color: var(--dim-grey); color: var(--dim-grey); }

        #terminal-log {
            position: fixed; bottom: 0; left: 0; right: 0; padding: 8px 15px; background-color: #000;
            border-top: 2px solid var(--dark-spruce); font-size: 0.7rem; color: var(--sea-green);
            white-space: nowrap; z-index: 99;
        }
    </style>
</head>
<body>

    <button id="btn-engage" onclick="initCombatMode()">[ ENGAGE FULLSCREEN ]</button>

    <div id="combat-hud">
        <div id="top-toolbar">
            <button id="btn-edit" class="tool-btn" onclick="toggleEditMode()">[ EDIT LAYOUT ]</button>
            <div id="edit-controls">
                <button class="tool-btn" onclick="resizeSelected(0.1)">[ + SCALE ]</button>
                <button class="tool-btn" onclick="resizeSelected(-0.1)">[ - SCALE ]</button>
                <button class="tool-btn" style="border-color: var(--alert-red); color: var(--alert-red);" onclick="resetLayout()">[ RESET ]</button>
                <button class="tool-btn" style="border-color: var(--edit-yellow); color: var(--edit-yellow);" onclick="toggleEditMode()">[ SAVE CONFIG ]</button>
            </div>
        </div>

        <div id="zone-analog" class="control-zone">
            <div id="joystick-base">
                <div id="joystick-knob"></div>
            </div>
            <div id="joystick-coords">X: 0 | Y: 0</div>
        </div>

        <div id="zone-action" class="control-zone">
            <div class="ps4-pad">
                <button class="btn-ps4 btn-tri" data-key="TRI">▲</button>
                <button class="btn-ps4 btn-sq" data-key="SQR">■</button>
                <button class="btn-ps4 btn-cir" data-key="CIR">●</button>
                <button class="btn-ps4 btn-x" data-key="X">✖</button>
            </div>
        </div>
    </div>

    <div id="terminal-log">> SYS STANDBY // TACTICAL UI V3 LOADED</div>

    <script>
        const terminal = document.getElementById('terminal-log');
        const hud = document.getElementById('combat-hud');
        function printLog(msg) { terminal.innerText = msg; }

        // ==========================================
        // COMUNICAÇÃO WEBSOCKET
        // ==========================================
        let ws;
        function initWebSocket() {
            const gateway = `ws://${window.location.hostname}/ws-joy`;
            ws = new WebSocket(gateway);
            ws.onopen = () => printLog("> WS CONNECTED // DATA LINK ESTABLISHED");
            ws.onclose = () => { 
                printLog("> WS DISCONNECTED // RETRYING..."); 
                setTimeout(initWebSocket, 2000); 
            };
            ws.onerror = () => printLog("> WS ERROR");
        }
        
        // Só inicia o WebSocket quando ativar o modo de combate
        // para não ficar tentando conectar enquanto edita layout.
        
        // Estado do joystick para envio periódico
        let joyX = 0;
        let joyY = 0;
        let lastSentX = 0;
        let lastSentY = 0;

        // Loop de envio do joystick a ~20Hz (50ms)
        setInterval(() => {
            if(ws && ws.readyState === WebSocket.OPEN) {
                // Só envia se houve mudança para economizar banda
                if(joyX !== lastSentX || joyY !== lastSentY) {
                    ws.send(`J:${joyX},${joyY}`);
                    lastSentX = joyX;
                    lastSentY = joyY;
                }
            }
        }, 50);

        function sendButton(btnKey, state) {
            if(ws && ws.readyState === WebSocket.OPEN) {
                ws.send(`B:${btnKey},${state}`);
            }
        }

        // ==========================================
        // SISTEMA INTELIGENTE DE LAYOUT
        // ==========================================
        let currentLayout = null;
        const zoneAnalog = document.getElementById('zone-analog');
        const zoneAction = document.getElementById('zone-action');
        let selectedZone = null;
        let isEditing = false;

        // Calcula matematicamente o centro perfeito dos quadrantes esquerdo e direito
        function getFactoryDefaultLayout() {
            const w = window.innerWidth;
            const h = window.innerHeight;
            
            // Dimensões aproximadas dos blocos
            const analogW = 180, analogH = 220;
            const actionW = 220, actionH = 240;

            return {
                // 20% da tela para a esquerda, 80% para a direita. Alinhamento vertical central absoluto.
                analog: { x: (w * 0.20) - (analogW / 2), y: (h / 2) - (analogH / 2), scale: 1 },
                action: { x: (w * 0.80) - (actionW / 2), y: (h / 2) - (actionH / 2), scale: 1 }
            };
        }

        function applyLayout() {
            if(!currentLayout) return;
            zoneAnalog.style.left = currentLayout.analog.x + 'px';
            zoneAnalog.style.top = currentLayout.analog.y + 'px';
            zoneAnalog.style.transform = `scale(${currentLayout.analog.scale})`;

            zoneAction.style.left = currentLayout.action.x + 'px';
            zoneAction.style.top = currentLayout.action.y + 'px';
            zoneAction.style.transform = `scale(${currentLayout.action.scale})`;

            // Revela os controles suavemente após aplicar a posição para evitar flashes na tela
            zoneAnalog.style.opacity = 1;
            zoneAction.style.opacity = 1;
            updateCenter(); // Re-trava o ponto morto do joystick
        }

        function saveLayout() {
            localStorage.setItem('sumo_hud_config', JSON.stringify(currentLayout));
            printLog("> CONFIG SAVED TO LOCAL MEMORY");
        }

        function resetLayout() {
            currentLayout = getFactoryDefaultLayout();
            applyLayout();
            saveLayout();
            printLog("> LAYOUT RESET TO FACTORY DEFAULTS");
        }

        // ==========================================
        // MODO FULLSCREEN & BOOT DO LAYOUT
        // ==========================================
        function initCombatMode() {
            let elem = document.documentElement;
            if (elem.requestFullscreen) elem.requestFullscreen();
            else if (elem.webkitRequestFullscreen) elem.webkitRequestFullscreen();
            
            document.getElementById('btn-engage').style.display = 'none';
            hud.classList.add('active');
            
            // Dá 300ms pro navegador deitar a tela e recalcular os pixels antes de aplicar a matemática
            setTimeout(() => {
                let saved = localStorage.getItem('sumo_hud_config');
                if (saved) {
                    currentLayout = JSON.parse(saved);
                    // Proteção: se a tela virou e a configuração antiga quebrou a altura, reseta.
                    if(currentLayout.analog.y > window.innerHeight) currentLayout = getFactoryDefaultLayout();
                } else {
                    currentLayout = getFactoryDefaultLayout();
                }
                applyLayout();
                if (!ws || ws.readyState !== WebSocket.OPEN) {
                    initWebSocket();
                }
                if (navigator.vibrate) navigator.vibrate([50, 100, 50]);
                printLog("> UPLINK READY // AESTHETICS LOCKED");
            }, 300); 
        }

        document.addEventListener('fullscreenchange', () => {
            if (!document.fullscreenElement) {
                document.getElementById('btn-engage').style.display = 'block';
                hud.classList.remove('active');
            }
        });

        // ==========================================
        // MODO DE EDIÇÃO (DRAG & DROP)
        // ==========================================
        function toggleEditMode() {
            isEditing = !isEditing;
            if (isEditing) {
                document.body.classList.add('editing');
                printLog("> EDIT MODE: ACTIVE. DRAG ZONES OR RESIZE.");
            } else {
                document.body.classList.remove('editing');
                if (selectedZone) selectedZone.classList.remove('selected');
                selectedZone = null;
                saveLayout();
                updateCenter();
            }
        }

        function resizeSelected(amount) {
            if (!selectedZone) return;
            let id = selectedZone.id === 'zone-analog' ? 'analog' : 'action';
            let newScale = currentLayout[id].scale + amount;
            if (newScale < 0.5) newScale = 0.5;
            if (newScale > 2.0) newScale = 2.0;
            currentLayout[id].scale = newScale;
            applyLayout();
        }

        let dragInfo = { active: false, id: null, offsetX: 0, offsetY: 0 };

        function startDrag(e, zoneElement, id) {
            if (!isEditing) return;
            e.preventDefault();
            if (selectedZone) selectedZone.classList.remove('selected');
            selectedZone = zoneElement;
            selectedZone.classList.add('selected');
            let clientX = e.touches ? e.touches[0].clientX : e.clientX;
            let clientY = e.touches ? e.touches[0].clientY : e.clientY;
            dragInfo.active = true; dragInfo.id = id;
            dragInfo.offsetX = clientX - currentLayout[id].x;
            dragInfo.offsetY = clientY - currentLayout[id].y;
        }

        function doDrag(e) {
            if (!isEditing || !dragInfo.active) return;
            e.preventDefault();
            let clientX = e.touches ? e.touches[0].clientX : e.clientX;
            let clientY = e.touches ? e.touches[0].clientY : e.clientY;
            currentLayout[dragInfo.id].x = clientX - dragInfo.offsetX;
            currentLayout[dragInfo.id].y = clientY - dragInfo.offsetY;
            applyLayout();
        }

        function endDrag() { dragInfo.active = false; }

        zoneAnalog.addEventListener('touchstart', (e) => startDrag(e, zoneAnalog, 'analog'), {passive: false});
        zoneAnalog.addEventListener('mousedown', (e) => startDrag(e, zoneAnalog, 'analog'));
        zoneAction.addEventListener('touchstart', (e) => startDrag(e, zoneAction, 'action'), {passive: false});
        zoneAction.addEventListener('mousedown', (e) => startDrag(e, zoneAction, 'action'));

        window.addEventListener('touchmove', doDrag, {passive: false});
        window.addEventListener('mousemove', doDrag);
        window.addEventListener('touchend', endDrag);
        window.addEventListener('mouseup', endDrag);

        // ==========================================
        // LÓGICA VETORIAL DO JOYSTICK
        // ==========================================
        const base = document.getElementById('joystick-base');
        const knob = document.getElementById('joystick-knob');
        const coords = document.getElementById('joystick-coords');
        let joyActive = false, centerX, centerY, radius;

        function updateCenter() {
            if(hud.classList.contains('active') && !isEditing && currentLayout) {
                const rect = base.getBoundingClientRect();
                radius = rect.width / 2;
                centerX = rect.left + radius;
                centerY = rect.top + radius;
            }
        }

        base.addEventListener('touchstart', (e) => {
            if(isEditing) return;
            joyActive = true; base.classList.add('active');
            updateCenter(); joyMove(e);
        }, {passive: false});

        function joyMove(e) {
            if (!joyActive || isEditing) return;
            e.preventDefault(); 
            let clientX = e.touches ? e.touches[0].clientX : e.clientX;
            let clientY = e.touches ? e.touches[0].clientY : e.clientY;
            let dx = clientX - centerX, dy = clientY - centerY;
            let dist = Math.sqrt(dx * dx + dy * dy);
            if (dist > radius) { dx = (dx/dist)*radius; dy = (dy/dist)*radius; }
            knob.style.transform = `translate(${dx}px, ${dy}px)`;
            joyX = Math.round((dx/radius)*100);
            joyY = Math.round((-dy/radius)*100); 
            coords.innerText = `X: ${joyX} | Y: ${joyY}`;
        }

        window.addEventListener('touchmove', joyMove, {passive: false});
        window.addEventListener('touchend', () => {
            if(joyActive) { 
                joyActive = false; 
                base.classList.remove('active'); 
                joyX = 0; 
                joyY = 0; 
                coords.innerText = `X: 0 | Y: 0`; 
            }
        });

        // ==========================================
        // LÓGICA DOS BOTÕES PS4 (FEEDBACK RESTAURADO E BLINDADO)
        // ==========================================
        document.querySelectorAll('.btn-ps4').forEach(btn => {
            
            // Função para pintar o botão
            const engageButton = () => {
                if(isEditing) return;
                btn.style.transform = 'scale(0.9)';
                btn.style.backgroundColor = 'currentColor';
                btn.style.color = 'var(--void-bg)';
            };

            // Função para resetar a cor do botão
            const releaseButton = () => {
                if(isEditing) return;
                btn.style.transform = 'scale(1)';
                btn.style.backgroundColor = 'transparent';
                btn.style.color = ''; 
            };

            // Eventos de Toque (Mobile)
            btn.addEventListener('touchstart', (e) => {
                if(isEditing) return;
                e.preventDefault(); 
                if (navigator.vibrate) navigator.vibrate(15);
                printLog(`> UPLINK: MACRO TRIGGER -> [${btn.dataset.key}]`);
                engageButton();
                sendButton(btn.dataset.key, 1);
            });

            btn.addEventListener('touchend', (e) => {
                if(isEditing) return;
                e.preventDefault();
                releaseButton();
                sendButton(btn.dataset.key, 0);
            });

            // Fallback para clicks no Mouse (Laboratório)
            btn.addEventListener('mousedown', (e) => {
                if(isEditing) return;
                printLog(`> UPLINK: MACRO TRIGGER -> [${btn.dataset.key}]`);
                engageButton();
                sendButton(btn.dataset.key, 1);
            });

            const handleRelease = () => {
                if(btn.style.transform === 'scale(0.9)') { // Verifica se estava pressionado
                    releaseButton();
                    sendButton(btn.dataset.key, 0);
                }
            };
            btn.addEventListener('mouseup', handleRelease);
            btn.addEventListener('mouseleave', handleRelease);
        });
    </script>
</body>
</html>
)rawliteral";

