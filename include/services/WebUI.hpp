#pragma once
#include <Arduino.h>

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
            letter-spacing: 1.5px; /* O respiro tático nas letras */
            margin-bottom: 25px;
            white-space: pre-wrap;
            color: var(--lemon-chiffon); 
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
            mBody.innerText = text;
            mActions.className = "modal-actions single-btn";
            mActions.innerHTML = `<button class="btn-confirm" onclick="closeModal()">OK</button>`;
            modal.style.display = 'flex';
        }

        document.querySelectorAll('button[data-category]').forEach(function(btn) {
            btn.addEventListener('click', function() {
                if (navigator.vibrate) navigator.vibrate(20); // Vibração Restaurada
                
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

            const report = 
                "MOVIMENTO : " + stratState.macro.label + "\n" +
                "DIREÇÃO   : " + stratState.direction.label + "\n" +
                "BUSCA     : " + stratState.search.label + "\n" +
                "ARMA      : " + stratState.weapon.label;

            mContainer.className = "modal-box";
            mTitle.innerText = "CONFIRMAÇÃO TÁTICA";
            mTitle.style.color = 'var(--lemon-chiffon)';
            mBody.innerText = report + "\n\nTransmitir parâmetros para o robô?";
            
            mActions.className = "modal-actions";
            mActions.innerHTML = `
                <button class="btn-abort" onclick="closeModal()">ABORTAR</button>
                <button class="btn-confirm" onclick="executeTransmit()">TRANSMITIR</button>
            `;
            modal.style.display = 'flex';
        }

        function executeTransmit() {
            if (navigator.vibrate) navigator.vibrate([40, 50, 40]); // Vibração Restaurada

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
                        showUiAlert("UPLINK SUCCESS", "Configuração salva com sucesso!\nStatus: 200 OK", false);
                        printLog("> [OK 200] PARÂMETROS TRAVADOS NO ROBÔ");
                    } else {
                        showUiAlert("UPLINK FAILURE", "O robô recebeu os dados mas retornou erro: " + response.status, true);
                        printLog("> [ERRO] STATUS HTTP: " + response.status, true);
                    }
                })
                .catch(function(err) {
                    showUiAlert("NETWORK ERROR", "Falha crítica de comunicação.\nVerifique se o celular ainda está conectado no Wi-Fi do robô.", true);
                    printLog("> [ERRO FATAL] SEM RESPOSTA DA REDE", true);
                });
        }
    </script>
</body>
</html>

)rawliteral";