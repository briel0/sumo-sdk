#pragma once
#include <Arduino.h>

// HUD legada (AutoMode + CombatStrategy): macro/direção/busca/arma.
// Fonte editável em ui/index.html — após editar, atualizar a string abaixo.
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(

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

    <div id="terminal-log">> SYS STANDBY // UPLINK READY</div>

    <div id="custom-modal" class="modal-overlay">
        <div id="modal-container" class="modal-box">
            <div id="modal-title" class="modal-title">CONFIRMAÇÃO TÁTICA</div>
            <div id="modal-body" class="modal-body"></div>
            <div id="modal-actions" class="modal-actions"></div>
        </div>
    </div>

    <script>
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

            document.querySelectorAll('.tune-input').forEach(function(inp) {
                if (inp.value !== '') {
                    params.append(inp.dataset.key, inp.value);
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

// HUD Fumacinha (FumacinhaMode + Fumacinha_FSM): abertura/busca/ataque/arma.
// Fonte editável em ui/fumacinha.html — após editar, atualizar a string abaixo.
const char FUMACINHA_DASHBOARD_HTML[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>FUMACINHA AUTO</title>
    <style>
        :root {
            --sea-green: #2c9246;
            --dark-spruce: #124925;
            --lemon-chiffon: #eeecc5;
            --grey-olive: #847f79;
            --dim-grey: #6e6965;
            --void-bg: #05140a;
            --alert-red: #ff3333;
            --signal-blue: #3b82c4;
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
        .grid-3 { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 10px; }

        button {
            background-color: transparent;
            font-size: 0.8rem;
            font-weight: bold;
            text-transform: uppercase;
            width: 100%;
            cursor: pointer;
            transition: 0.1s;
            padding: 16px 5px;
        }

        button:active { filter: brightness(1.5); }

        .btn-opening {
            border: 2px solid var(--sea-green);
            color: var(--lemon-chiffon);
        }
        .btn-opening.selected {
            background-color: var(--sea-green);
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

        .btn-attack {
            border: 2px solid var(--dim-grey);
            color: var(--lemon-chiffon);
        }
        .btn-attack.selected {
            background-color: var(--dim-grey);
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

        .btn-diag {
            border: 2px solid var(--alert-red);
            color: var(--alert-red);
        }
        .btn-diag.active {
            background-color: var(--alert-red);
            color: var(--void-bg);
        }

        .btn-direction {
            border: 2px solid var(--signal-blue);
            color: var(--lemon-chiffon);
        }
        .btn-direction.selected {
            background-color: var(--signal-blue);
            color: var(--void-bg);
        }

        .btn-emitter {
            border: 2px solid var(--lemon-chiffon);
            color: var(--lemon-chiffon);
        }
        .btn-emitter.selected {
            background-color: var(--lemon-chiffon);
            color: var(--void-bg);
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

        #test-readout {
            display: none;
            margin-top: 10px;
            padding: 10px;
            background-color: #000;
            border: 2px solid var(--alert-red);
            font-size: 0.75rem;
            color: var(--sea-green);
            white-space: pre-wrap;
            word-break: break-word;
        }

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

        .modal-chip {
            display: block;
            width: 100%;
            padding: 12px;
            margin-bottom: 8px;
            text-align: center;
            font-size: 1rem;
            font-weight: bold;
            text-transform: uppercase;
            pointer-events: none;
        }

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

        /* Construtor de macro (testador ao vivo) */
        .step-row {
            display: flex;
            align-items: center;
            gap: 8px;
            margin-bottom: 10px;
            background-color: rgba(44, 146, 70, 0.05);
            padding: 10px;
            border-left: 3px solid var(--sea-green);
        }
        .step-row span {
            font-weight: bold;
            color: var(--lemon-chiffon);
            min-width: 58px;
            font-size: 0.8rem;
        }
        .step-input {
            flex: 1;
            width: 100%;
            background-color: transparent;
            border: 1px solid var(--dark-spruce);
            color: var(--sea-green);
            padding: 10px 4px;
            font-size: 0.9rem;
            font-weight: bold;
            text-align: center;
        }
        .step-input:focus { outline: none; border-color: var(--sea-green); background-color: rgba(44, 146, 70, 0.15); }
        .step-input::placeholder { color: var(--dim-grey); font-size: 0.7rem; }

        .btn-step { border: 2px solid var(--grey-olive); color: var(--lemon-chiffon); }
        .btn-step.selected, .btn-step:active { background-color: var(--grey-olive); color: var(--void-bg); }

        .btn-validar {
            margin-top: 12px;
            background-color: transparent;
            border: 3px solid var(--signal-blue);
            color: var(--signal-blue);
            font-size: 1rem;
            padding: 16px;
            letter-spacing: 2px;
        }
    </style>
</head>
<body>

    <h1>FUMACINHA AUTO</h1>

    <h2>DIAGNÓSTICO DE BANCADA</h2>
    <div class="grid-3">
        <button id="btn-test-sensor" class="btn-diag" onclick="toggleTest('sensor')">TESTE SENSORES</button>
        <button id="btn-test-motor" class="btn-diag" onclick="toggleTest('motor')">TESTE MOTORES</button>
        <button id="btn-test-servo" class="btn-diag" onclick="toggleTest('servo')">TESTE SERVO</button>
    </div>
    <div id="test-readout"></div>

    <h2>LADO PREFERENCIAL</h2>
    <div class="grid">
        <button class="btn-direction" data-category="direction" data-val="0" data-label="ESQUERDA">ESQUERDA</button>
        <button class="btn-direction" data-category="direction" data-val="1" data-label="DIREITA">DIREITA</button>
    </div>

    <h2>ABERTURA (OPENING)</h2>
    <div class="grid-3">
        <button class="btn-opening" data-category="openingTactic" data-val="0" data-label="ARCO EVASIVO">ARCO EVASIVO</button>
        <button class="btn-opening" data-category="openingTactic" data-val="1" data-label="CURVA DE BORDA">CURVA DE BORDA</button>
        <button class="btn-opening" data-category="openingTactic" data-val="2" data-label="ARRANCADA RETA">ARRANCADA RETA</button>
    </div>

    <h2>BUSCA (SEARCHING)</h2>
    <div class="grid-3">
        <button class="btn-search" data-category="searchTactic" data-val="0" data-label="ZIGUEZAGUE IR">ZIGUEZAGUE IR</button>
        <button class="btn-search" data-category="searchTactic" data-val="1" data-label="PULSO PERIÓDICO">PULSO PERIÓDICO</button>
        <button class="btn-search" data-category="searchTactic" data-val="2" data-label="AVANÇO CEGO">AVANÇO CEGO</button>
    </div>

    <h2>EMISSOR NA BUSCA</h2>
    <div class="grid">
        <button class="btn-emitter" data-category="searchEmitters" data-val="0" data-label="FURTIVO">FURTIVO</button>
        <button class="btn-emitter" data-category="searchEmitters" data-val="1" data-label="ATIVO">ATIVO</button>
    </div>

    <h2>ATAQUE (ATTACKING)</h2>
    <div class="grid">
        <button class="btn-attack" data-category="attackTactic" data-val="0" data-label="EMPUXO CORRIGIDO">EMPUXO CORRIGIDO</button>
        <button class="btn-attack" data-category="attackTactic" data-val="1" data-label="EMPUXO CEGO">EMPUXO CEGO</button>
    </div>

    <h2>EMISSOR NO ATAQUE</h2>
    <div class="grid">
        <button class="btn-emitter" data-category="attackEmitters" data-val="0" data-label="FURTIVO">FURTIVO</button>
        <button class="btn-emitter" data-category="attackEmitters" data-val="1" data-label="ATIVO">ATIVO</button>
    </div>

    <button class="btn-submit" onclick="openTacticalConfirm()">[SUBMETER]</button>
    <button class="btn-validar" onclick="salvarValidada()">[VALIDAR ESTRATÉGIA]</button>

    <h2>CONSTRUTOR DE MACRO (TESTE AO VIVO)</h2>
    <div id="macro-steps">
        <div class="step-row" data-index="0">
            <span>Passo 1</span>
            <input type="text" class="step-input" inputmode="numeric" placeholder="DIR %" id="r0">
            <input type="text" class="step-input" inputmode="numeric" placeholder="ESQ %" id="l0">
            <input type="text" class="step-input" inputmode="numeric" placeholder="MS" id="d0">
        </div>
    </div>
    <div class="grid" style="margin-bottom: 10px;">
        <button class="btn-step" onclick="addStep()">ADICIONAR PASSO</button>
        <button class="btn-step" onclick="removeStep()">REMOVER PASSO</button>
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
        const stratState = {
            direction: { val: '', label: 'NENHUM' },
            openingTactic: { val: '', label: 'NENHUM' },
            searchTactic: { val: '', label: 'NENHUM' },
            searchEmitters: { val: '', label: 'NENHUM' },
            attackTactic: { val: '', label: 'NENHUM' },
            attackEmitters: { val: '', label: 'NENHUM' }
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

        // Toggles de diagnóstico de bancada. Ao vivo, sem passar pelo modal de
        // confirmação — não fazem parte do pacote tático (stratState) e não
        // bloqueiam a seleção/transmissão de tática abaixo. Mutuamente
        // exclusivos: ligar um desliga os outros (mesmos 5 LEDs / atuadores).
        const testState = { sensor: false, motor: false, servo: false };
        const testLabels = { sensor: 'SENSORES', motor: 'MOTORES', servo: 'SERVO' };
        const readoutBox = document.getElementById('test-readout');
        let readoutTimer = null;

        // Log ao vivo dos valores lidos/enviados: enquanto QUALQUER teste está
        // ativo, faz polling de /get-test-data e mostra os campos crus, sem
        // presumir o formato (sensor e motor devolvem JSONs diferentes).
        function fetchReadout() {
            fetch('/get-test-data')
                .then(function(response) { return response.json(); })
                .then(function(data) {
                    const keys = Object.keys(data);
                    if (keys.length === 0) {
                        readoutBox.innerText = '> aguardando leitura...';
                        return;
                    }
                    readoutBox.innerText = keys.map(function(key) {
                        return '> ' + key.toUpperCase() + ': ' + data[key];
                    }).join('\n');
                })
                .catch(function() {
                    readoutBox.innerText = '> [ERRO] FALHA AO LER /get-test-data';
                });
        }

        function startReadoutPolling() {
            readoutBox.style.display = 'block';
            if (readoutTimer) return;
            readoutTimer = setInterval(fetchReadout, 300);
            fetchReadout();
        }

        function stopReadoutPolling() {
            if (readoutTimer) {
                clearInterval(readoutTimer);
                readoutTimer = null;
            }
            readoutBox.style.display = 'none';
            readoutBox.innerText = '';
        }

        function toggleTest(which) {
            if (navigator.vibrate) navigator.vibrate(20);

            const turningOn = !testState[which];
            testState.sensor = (which === 'sensor') ? turningOn : false;
            testState.motor  = (which === 'motor')  ? turningOn : false;
            testState.servo  = (which === 'servo')  ? turningOn : false;

            document.getElementById('btn-test-sensor').classList.toggle('active', testState.sensor);
            document.getElementById('btn-test-motor').classList.toggle('active', testState.motor);
            document.getElementById('btn-test-servo').classList.toggle('active', testState.servo);

            const params = new URLSearchParams();
            params.append('sensorTest', testState.sensor ? '1' : '0');
            params.append('motorTest', testState.motor ? '1' : '0');
            params.append('servoTest', testState.servo ? '1' : '0');

            fetch('/set-test?' + params.toString(), { method: 'GET' })
                .then(function(response) {
                    if (response.ok) {
                        printLog('> TESTE DE ' + testLabels[which] + ': ' + (turningOn ? 'ATIVO' : 'DESLIGADO'));
                        if (testState.sensor || testState.motor || testState.servo) {
                            startReadoutPolling();
                        } else {
                            stopReadoutPolling();
                        }
                    } else {
                        printLog('> [ERRO] FALHA AO ALTERAR MODO DE TESTE (' + response.status + ')', true);
                    }
                })
                .catch(function() {
                    printLog('> [ERRO FATAL] SEM RESPOSTA DA REDE AO ALTERAR TESTE', true);
                });
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
            if (!stratState.openingTactic.val) {
                showUiAlert("[!] ERRO CRÍTICO", "Selecione obrigatoriamente a tática de Abertura antes de realizar o Uplink.", true);
                printLog("> ERRO: PREENCHIMENTO INCOMPLETO", true);
                return;
            }

            let reportHtml = "";

            if (stratState.direction.val) {
                reportHtml += `<div class="modal-chip btn-direction selected">${stratState.direction.label}</div>`;
            }

            reportHtml += `<div class="modal-chip btn-opening selected">${stratState.openingTactic.label}</div>`;

            if (stratState.searchTactic.val) {
                reportHtml += `<div class="modal-chip btn-search selected">${stratState.searchTactic.label}</div>`;
            }

            if (stratState.searchEmitters.val) {
                reportHtml += `<div class="modal-chip btn-emitter selected">EMISSOR BUSCA: ${stratState.searchEmitters.label}</div>`;
            }

            if (stratState.attackTactic.val) {
                reportHtml += `<div class="modal-chip btn-attack selected">${stratState.attackTactic.label}</div>`;
            }

            if (stratState.attackEmitters.val) {
                reportHtml += `<div class="modal-chip btn-emitter selected">EMISSOR ATAQUE: ${stratState.attackEmitters.label}</div>`;
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

        // ===================================================================
        // CONSTRUTOR DE MACRO (testador ao vivo — dispara /test-macro na hora)
        // ===================================================================
        let numSteps = 1;

        function addStep() {
            if (numSteps >= 8) return;
            const div = document.createElement('div');
            div.className = 'step-row';
            div.dataset.index = numSteps;
            div.innerHTML =
                '<span>Passo ' + (numSteps + 1) + '</span>' +
                '<input type="text" class="step-input" inputmode="numeric" placeholder="DIR %" id="r' + numSteps + '">' +
                '<input type="text" class="step-input" inputmode="numeric" placeholder="ESQ %" id="l' + numSteps + '">' +
                '<input type="text" class="step-input" inputmode="numeric" placeholder="MS" id="d' + numSteps + '">';
            document.getElementById('macro-steps').appendChild(div);
            numSteps++;
        }

        function removeStep() {
            if (numSteps <= 1) return;
            numSteps--;
            const rows = document.querySelectorAll('.step-row');
            rows[rows.length - 1].remove();
        }

        function parseInput(id, fallback) {
            const el = document.getElementById(id);
            const v = el ? el.value : '';
            return v !== '' ? parseInt(v) : fallback;
        }

        function dispararMacro() {
            if (navigator.vibrate) navigator.vibrate(20);
            let params = 'steps=' + numSteps;
            for (let i = 0; i < numSteps; i++) {
                // l = motor esquerdo, r = motor direito, d = tempo (ms)
                params += '&l' + i + '=' + parseInput('l' + i, 0) +
                          '&r' + i + '=' + parseInput('r' + i, 0) +
                          '&d' + i + '=' + parseInput('d' + i, 300);
            }
            printLog('> DISPARANDO MACRO...');
            fetch('/test-macro?' + params)
                .then(function (res) { return res.text(); })
                .then(function (txt) { printLog('> [OK] ' + txt); })
                .catch(function () { printLog('> [ERRO] ROBÔ NÃO RESPONDEU', true); });
        }

        // ===================================================================
        // VALIDAR: salva/restaura a estratégia inteira no navegador (localStorage)
        // ===================================================================
        const VALIDADA_KEY = 'fumacinhaValidada';

        function coletarConfigAtual() {
            const macro = [];
            for (let i = 0; i < numSteps; i++) {
                macro.push({
                    r: (document.getElementById('r' + i) || {}).value || '',
                    l: (document.getElementById('l' + i) || {}).value || '',
                    d: (document.getElementById('d' + i) || {}).value || ''
                });
            }
            return { strat: stratState, macro: macro };
        }

        function salvarValidada() {
            if (navigator.vibrate) navigator.vibrate([30, 40, 30]);
            localStorage.setItem(VALIDADA_KEY, JSON.stringify(coletarConfigAtual()));
            printLog('> ESTRATÉGIA VALIDADA SALVA NO CELULAR');
            showUiAlert('VALIDADA SALVA', 'A estratégia atual (táticas + macro) foi salva neste navegador.<br><br>Ela volta sozinha ao recarregar a página.', false);
        }

        function restaurarValidada() {
            const raw = localStorage.getItem(VALIDADA_KEY);
            if (!raw) return;
            let saved;
            try { saved = JSON.parse(raw); } catch (e) { return; }

            if (saved.strat) {
                Object.keys(saved.strat).forEach(function (cat) {
                    const item = saved.strat[cat];
                    if (!item || !item.val) return;
                    const btn = document.querySelector('button[data-category="' + cat + '"][data-val="' + item.val + '"]');
                    if (btn) {
                        document.querySelectorAll('button[data-category="' + cat + '"]').forEach(function (b) { b.classList.remove('selected'); });
                        btn.classList.add('selected');
                        stratState[cat] = { val: item.val, label: item.label };
                    }
                });
            }
            if (saved.macro && saved.macro.length) {
                while (numSteps > 1) removeStep();
                for (let i = 0; i < saved.macro.length; i++) {
                    if (i > 0) addStep();
                    const s = saved.macro[i];
                    const r = document.getElementById('r' + i), l = document.getElementById('l' + i), d = document.getElementById('d' + i);
                    if (r) r.value = s.r; if (l) l.value = s.l; if (d) d.value = s.d;
                }
            }
            printLog('> ESTRATÉGIA VALIDADA RESTAURADA');
        }

        restaurarValidada();
    </script>
</body>
</html>

)rawliteral";