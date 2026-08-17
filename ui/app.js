const terminal = document.getElementById('terminal-log');
function printLog(msg, isError = false) {
    terminal.innerText = msg;
    terminal.style.color = isError ? 'var(--alert-red)' : 'var(--sea-green)';
}

// ==========================================
// MUDANÇA DE TELAS (SPA & FULLSCREEN)
// ==========================================
const stratPanel = document.getElementById('strategy-panel');
const joyPanel = document.getElementById('joystick-panel');

function openJoystick() {
    let elem = document.documentElement;
    if (elem.requestFullscreen) elem.requestFullscreen();
    else if (elem.webkitRequestFullscreen) elem.webkitRequestFullscreen();
    
    stratPanel.style.display = 'none';
    joyPanel.style.display = 'block';
    
    setTimeout(() => {
        let saved = localStorage.getItem('sumo_hud_config');
        if (saved) {
            currentLayout = JSON.parse(saved);
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
        stratPanel.style.display = 'block';
        joyPanel.style.display = 'none';
        if(ws && ws.readyState === WebSocket.OPEN) {
            ws.close();
            ws = null;
            printLog("> WS DISCONNECTED // RETURNED TO MENU");
        }
    }
});

// ==========================================
// COMUNICAÇÃO WEBSOCKET (JOYSTICK)
// ==========================================
let ws;
function initWebSocket() {
    const gateway = `ws://${window.location.hostname}/ws-joy`;
    ws = new WebSocket(gateway);
    ws.onopen = () => printLog("> WS CONNECTED // DATA LINK ESTABLISHED");
    ws.onclose = () => { 
        if (document.fullscreenElement) {
            printLog("> WS DISCONNECTED // RETRYING..."); 
            setTimeout(initWebSocket, 2000); 
        }
    };
    ws.onerror = () => printLog("> WS ERROR");
}

let joyX = 0;
let joyY = 0;
let lastSentX = 0;
let lastSentY = 0;

setInterval(() => {
    if(ws && ws.readyState === WebSocket.OPEN) {
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
// LÓGICA DO JOYSTICK E BOTÕES PS4
// ==========================================
let currentLayout = null;
const zoneAnalog = document.getElementById('zone-analog');
const zoneAction = document.getElementById('zone-action');
let selectedZone = null;
let isEditing = false;

function getFactoryDefaultLayout() {
    const w = window.innerWidth, h = window.innerHeight;
    const analogW = 180, analogH = 220, actionW = 220, actionH = 240;
    return {
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

    zoneAnalog.style.opacity = 1; zoneAction.style.opacity = 1;
    updateCenter(); 
}

function saveLayout() { localStorage.setItem('sumo_hud_config', JSON.stringify(currentLayout)); printLog("> CONFIG SAVED"); }
function resetLayout() { currentLayout = getFactoryDefaultLayout(); applyLayout(); saveLayout(); printLog("> LAYOUT RESET"); }

function toggleEditMode() {
    isEditing = !isEditing;
    if (isEditing) {
        document.body.classList.add('editing'); printLog("> EDIT MODE: ACTIVE. DRAG ZONES OR RESIZE.");
    } else {
        document.body.classList.remove('editing');
        if (selectedZone) selectedZone.classList.remove('selected');
        selectedZone = null; saveLayout(); updateCenter();
    }
}

function resizeSelected(amount) {
    if (!selectedZone) return;
    let id = selectedZone.id === 'zone-analog' ? 'analog' : 'action';
    let newScale = currentLayout[id].scale + amount;
    if (newScale < 0.5) newScale = 0.5; if (newScale > 2.0) newScale = 2.0;
    currentLayout[id].scale = newScale; applyLayout();
}

let dragInfo = { active: false, id: null, offsetX: 0, offsetY: 0 };
function startDrag(e, zoneElement, id) {
    if (!isEditing) return;
    e.preventDefault();
    if (selectedZone) selectedZone.classList.remove('selected');
    selectedZone = zoneElement; selectedZone.classList.add('selected');
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

const base = document.getElementById('joystick-base');
const knob = document.getElementById('joystick-knob');
const coords = document.getElementById('joystick-coords');
let joyActive = false, centerX, centerY, radius;

function updateCenter() {
    if(joyPanel.style.display === 'block' && !isEditing && currentLayout) {
        const rect = base.getBoundingClientRect();
        radius = rect.width / 2;
        centerX = rect.left + radius; centerY = rect.top + radius;
    }
}

base.addEventListener('touchstart', (e) => {
    if(isEditing) return;
    joyActive = true; base.classList.add('active'); updateCenter(); joyMove(e);
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
    joyX = Math.round((dx/radius)*100); joyY = Math.round((-dy/radius)*100); 
    coords.innerText = `X: ${joyX} | Y: ${joyY}`;
}

window.addEventListener('touchmove', joyMove, {passive: false});
window.addEventListener('touchend', () => {
    if(joyActive) { 
        joyActive = false; base.classList.remove('active'); 
        joyX = 0; joyY = 0; coords.innerText = `X: 0 | Y: 0`; 
        knob.style.transform = `translate(0px, 0px)`;
    }
});

document.querySelectorAll('.btn-ps4').forEach(btn => {
    const engageButton = () => { if(isEditing) return; btn.style.transform = 'scale(0.9)'; btn.style.backgroundColor = 'currentColor'; btn.style.color = 'var(--void-bg)'; };
    const releaseButton = () => { if(isEditing) return; btn.style.transform = 'scale(1)'; btn.style.backgroundColor = 'transparent'; btn.style.color = ''; };

    btn.addEventListener('touchstart', (e) => {
        if(isEditing) return; e.preventDefault(); if (navigator.vibrate) navigator.vibrate(15);
        printLog(`> UPLINK: MACRO TRIGGER -> [${btn.dataset.key}]`); engageButton(); sendButton(btn.dataset.key, 1);
    });

    btn.addEventListener('touchend', (e) => {
        if(isEditing) return; e.preventDefault(); releaseButton(); sendButton(btn.dataset.key, 0);
    });

    btn.addEventListener('mousedown', (e) => {
        if(isEditing) return; printLog(`> UPLINK: MACRO TRIGGER -> [${btn.dataset.key}]`); engageButton(); sendButton(btn.dataset.key, 1);
    });

    const handleRelease = () => {
        if(btn.style.transform === 'scale(0.9)') { releaseButton(); sendButton(btn.dataset.key, 0); }
    };
    btn.addEventListener('mouseup', handleRelease);
    btn.addEventListener('mouseleave', handleRelease);
});

// ==========================================
// LÓGICA DO PAINEL DE ESTRATÉGIA
// ==========================================
let numSteps = 1;

function addStep() {
    if(numSteps >= 8) return;
    const div = document.createElement('div');
    div.className = 'step-row'; div.dataset.index = numSteps;
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
    if(!d0) { showUiAlert('[!] ERRO', 'Preencha pelo menos a duração do Passo 1.', true); return; }

    let params = `steps=${numSteps}`;
    for(let i = 0; i < numSteps; i++) {
        const l = parseInput(`l${i}`, 0), r = parseInput(`r${i}`, 0), d = parseInput(`d${i}`, 300);
        params += `&l${i}=${l}&r${i}=${r}&d${i}=${d}`;
    }

    printLog('> DISPARANDO MACRO DE TESTE...');
    fetch('/test-macro?' + params)
        .then(res => res.text())
        .then(txt => { printLog('> [OK] MACRO DISPARADA NO ROBÔ'); showUiAlert('MACRO DISPARADA', txt, false); })
        .catch(() => { printLog('> [ERRO] ROBÔ NÃO RESPONDEU', true); showUiAlert('NETWORK ERROR', 'Falha ao disparar macro.<br>Verifique o Wi-Fi.', true); });
}

const stratState = {
    macro: { val: '', label: 'NENHUM' },
    direction: { val: '', label: 'NENHUM' },
    search: { val: '', label: 'NENHUM' },
    weapon: { val: '', label: 'NENHUM' }
};

const modal = document.getElementById('custom-modal');
const mContainer = document.getElementById('modal-container');
const mTitle = document.getElementById('modal-title');
const mBody = document.getElementById('modal-body');
const mActions = document.getElementById('modal-actions');

function closeModal() { modal.style.display = 'none'; }

function showUiAlert(title, text, isError = false) {
    mContainer.className = isError ? "modal-box error-mode" : "modal-box";
    mTitle.innerText = title; mTitle.style.color = isError ? 'var(--alert-red)' : 'var(--sea-green)';
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
        } else {
            document.querySelectorAll(`button[data-category="${category}"]`).forEach(b => b.classList.remove('selected'));
            this.classList.add('selected');
            stratState[category] = { val: val, label: label };
            printLog(`> SEL: ${category.toUpperCase()} -> ${label}`);
        }
    });
});

function openTacticalConfirm() {
    if (!stratState.macro.val) {
        showUiAlert("[!] ERRO CRÍTICO", "Selecione obrigatoriamente a Movimentação Inicial antes de realizar o Uplink.", true);
        printLog("> ERRO: PREENCHIMENTO INCOMPLETO", true); return;
    }
    let reportHtml = `<div class="modal-chip btn-base btn-macro selected">${stratState.macro.label}</div>`;
    if (stratState.direction.val) reportHtml += `<div class="modal-chip btn-base btn-direction selected">${stratState.direction.label}</div>`;
    if (stratState.search.val) reportHtml += `<div class="modal-chip btn-base btn-search selected">${stratState.search.label}</div>`;
    if (stratState.weapon.val) reportHtml += `<div class="modal-chip btn-base btn-weapon selected">${stratState.weapon.label}</div>`;
    reportHtml += `<div class="modal-prompt">Transmitir parâmetros para o robô?</div>`;

    mContainer.className = "modal-box"; mTitle.innerText = "CONFIRMAÇÃO TÁTICA"; mTitle.style.color = 'var(--lemon-chiffon)';
    mBody.innerHTML = reportHtml;
    mActions.className = "modal-actions";
    mActions.innerHTML = `<button class="btn-abort" onclick="closeModal()">ABORTAR</button><button class="btn-confirm" onclick="executeTransmit()">TRANSMITIR</button>`;
    modal.style.display = 'flex';
}

function executeTransmit() {
    if (navigator.vibrate) navigator.vibrate([40, 50, 40]);
    closeModal(); printLog("> EXECUTANDO UPLINK...");
    const params = new URLSearchParams();
    Object.keys(stratState).forEach(key => { if (stratState[key].val !== '') params.append(key, stratState[key].val); });
    const endpoint = '/set-strat?' + params.toString();
    printLog("> TX: " + endpoint);
    fetch(endpoint, { method: 'GET' })
        .then(res => {
            if(res.ok) { showUiAlert("UPLINK SUCCESS", "Configuração salva com sucesso!", false); printLog("> [OK 200] PARÂMETROS TRAVADOS NO ROBÔ"); }
            else { showUiAlert("UPLINK FAILURE", "O robô retornou erro:<br><br>STATUS " + res.status, true); printLog("> [ERRO] STATUS HTTP: " + res.status, true); }
        })
        .catch(err => { showUiAlert("NETWORK ERROR", "Falha crítica de comunicação.", true); printLog("> [ERRO FATAL] SEM RESPOSTA DA REDE", true); });
}
