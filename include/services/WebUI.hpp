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
    <div class="grid" id="macro-grid">
        <!-- Renderizado dinamicamente -->
    </div>

    <h2>DIREÇÃO</h2>
    <div class="grid">
        <button class="btn-direction" data-category="direction" data-val="E" data-label="ESQUERDA">ESQUERDA</button>
        <button class="btn-direction" data-category="direction" data-val="D" data-label="DIREITA">DIREITA</button>
    </div>

    <h2>BUSCA E TÁTICA</h2>
    <div class="grid" id="search-grid">
        <!-- Renderizado dinamicamente -->
    </div>

    <h2>SISTEMA DE ARMAS</h2>
    <div class="grid" id="weapon-grid">
        <!-- Renderizado dinamicamente -->
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

        // Event Delegation para lidar com botões criados dinamicamente
        document.body.addEventListener('click', function(e) {
            const btn = e.target.closest('button[data-category]');
            if(!btn) return;

            if (navigator.vibrate) navigator.vibrate(20); 
            
            const category = btn.getAttribute('data-category');
            const val = btn.getAttribute('data-val');
            const label = btn.getAttribute('data-label');
            
            if (btn.classList.contains('selected')) {
                btn.classList.remove('selected');
                stratState[category] = { val: '', label: 'NENHUM' };
                printLog(`> CATEGORIA ${category.toUpperCase()} DESMARCADA`);
            }
            else {
                document.querySelectorAll(`button[data-category="${category}"]`).forEach(function(b) {
                    b.classList.remove('selected');
                });
                btn.classList.add('selected');
                stratState[category] = { val: val, label: label };
                printLog(`> SEL: ${category.toUpperCase()} -> ${label}`);
            }
        });

        // Construtor dinâmico do painel
        function loadProfile() {
            fetch('/api/profile')
                .then(res => res.json())
                .then(data => renderProfile(data))
                .catch(() => {
                    // MOCK: Caso o usuário abra o HTML direto no PC para testar
                    console.log("Servidor não encontrado. Usando Mock do Arruela.");
                    renderProfile({
                        robot_name: "Arruela (Local Mock)",
                        macros: [
                            {id: 0, name: "FRENTÃO"},
                            {id: 1, name: "CURVÃO"}
                        ],
                        searches: [
                            {id: 1, name: "BUSCA PADRÃO"}
                        ],
                        has_weapons: false
                    });
                });
        }

        function renderProfile(profile) {
            document.querySelector('h1').innerText = "SUMO AUTO // " + profile.robot_name.toUpperCase();
            
            const macroGrid = document.getElementById('macro-grid');
            macroGrid.innerHTML = profile.macros.map(m => 
                `<button class="btn-macro" data-category="macro" data-val="${m.id}" data-label="${m.name}">${m.name}</button>`
            ).join('');

            const searchGrid = document.getElementById('search-grid');
            searchGrid.innerHTML = profile.searches.map(s => 
                `<button class="btn-search" data-category="search" data-val="${s.id}" data-label="${s.name}">${s.name}</button>`
            ).join('');

            const weaponGrid = document.getElementById('weapon-grid');
            if(profile.has_weapons) {
                weaponGrid.innerHTML = `
                    <button class="btn-weapon" data-category="weapon" data-val="0" data-label="DESARMAR">DESARMAR</button>
                    <button class="btn-weapon" data-category="weapon" data-val="1" data-label="NÃO DESARMAR">NÃO DESARMAR</button>
                `;
            } else {
                weaponGrid.innerHTML = `<button class="btn-weapon selected" data-category="weapon" data-val="0" data-label="SEM ARMAS" disabled style="opacity: 0.5;">SEM ARMAS</button>`;
                stratState.weapon = { val: '0', label: 'SEM ARMAS' }; // Trava default
            }
            printLog(`> PERFIL ${profile.robot_name.toUpperCase()} CARREGADO`);
        }

        // Iniciar
        loadProfile();

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
