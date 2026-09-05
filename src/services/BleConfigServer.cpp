#include "BleConfigServer.hpp"

// Guarda de arquivo inteiro: só compila de verdade pro Fuego/Fumacinha
// (únicos robôs que usam ActiveConfigServer = BleConfigServer, ver
// Config.hpp). Os outros envs (caipora/smoker/arruela/marola/...) não têm
// build_src_filter próprio — herdam o +<*> do [env] — então este arquivo
// entraria na varredura deles também, e <BLEDevice.h> não existe no fork da
// Bluepad32 que esses envs ainda usam (Bluedroid desligado na compilação
// desse fork). Mesmo padrão de auto-guarda que main_rc_c3.cpp já usa nesta
// branch pro build do C3.
#if defined(ROBOT_FUEGO) || defined(ROBOT_FUMACINHA)

#include "Config.hpp"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <string.h>

namespace {

// Mesmo serviço/characteristic da feat/bluetooth-config — protocolo interno,
// não precisa divergir por robô.
constexpr char SERVICE_UUID[] = "0000ff10-0000-1000-8000-00805f9b34fb";
constexpr char CHARACTERISTIC_UUID[] = "0000ff11-0000-1000-8000-00805f9b34fb";

enum BleCommand : uint8_t {
    CMD_GET_ROBOT_NAME = 0x01, // antigo /robot — sem UI_PROFILE_JSON aqui, a HUD do Fumacinha é fixa
    CMD_GET_SENSORS = 0x02,
    CMD_SET_STRATEGY = 0x03,
    CMD_SET_TEST = 0x04,
    CMD_TRIGGER_MACRO = 0x05,
};

// CMD_SET_STRATEGY carrega um dos dois formatos abaixo, indicado pelo
// primeiro byte do payload (depois do byte de comando) — mesma dualidade que
// o /set-strat HTTP resolvia olhando quais parâmetros vinham no request.
enum StrategyPayloadType : uint8_t {
    PAYLOAD_LEGACY = 0x01,         // AutoStrategy: macro, direction, search, weapon
    PAYLOAD_COMBAT_PROFILE = 0x02, // CombatProfile (HUD do Fumacinha)
};

// CMD_SET_TEST: primeiro byte diz QUAL teste (os três são mutuamente
// exclusivos — mesma regra do ConfigServer::/set-test).
enum TestChannel : uint8_t {
    TEST_SENSOR = 0x00,
    TEST_MOTOR = 0x01,
    TEST_SERVO = 0x02,
};

enum BleAck : uint8_t {
    ACK_OK = 0x00,
    ACK_ERROR = 0x01,
};

// Cópia estável do JSON de sensores no instante do comando — ver o porquê no
// header (mesmo bug de "sensor piscando" já resolvido na feat/bluetooth-config).
char s_sensorSnapshot[512];
portMUX_TYPE s_readoutMux = portMUX_INITIALIZER_UNLOCKED;

} // namespace

// A lib BLE nativa entrega o WRITE via uma classe de callback própria (não um
// ponteiro de função C solto) — guarda um ponteiro pra instância dona e
// repassa pro método público handleWrite().
class BleConfigServerCallbacks : public BLECharacteristicCallbacks {
  public:
    explicit BleConfigServerCallbacks(BleConfigServer *owner) : _owner(owner) {}

    void onWrite(BLECharacteristic *characteristic) override {
        _owner->handleWrite(characteristic);
    }

  private:
    BleConfigServer *_owner;
};

void BleConfigServer::handleWrite(BLECharacteristic *characteristic) {
    uint8_t *buffer = characteristic->getData();
    size_t bufferSize = characteristic->getLength();

    if(bufferSize == 0) {
        return;
    }

    const uint8_t cmd = buffer[0];
    uint8_t ack = ACK_OK;

    switch(cmd) {
        case CMD_GET_ROBOT_NAME:
            characteristic->setValue((uint8_t *)Config::ROBOT_NAME, strlen(Config::ROBOT_NAME));
            return;

        case CMD_GET_SENSORS:
            portENTER_CRITICAL(&s_readoutMux);
            strncpy(s_sensorSnapshot, _testReadoutJson.c_str(), sizeof(s_sensorSnapshot) - 1);
            s_sensorSnapshot[sizeof(s_sensorSnapshot) - 1] = '\0';
            portEXIT_CRITICAL(&s_readoutMux);
            characteristic->setValue((uint8_t *)s_sensorSnapshot, strlen(s_sensorSnapshot));
            return;

        case CMD_SET_STRATEGY: {
            if(bufferSize < 2) {
                ack = ACK_ERROR;
                break;
            }
            const uint8_t payloadType = buffer[1];

            if(payloadType == PAYLOAD_LEGACY) {
                if(bufferSize < 6) {
                    ack = ACK_ERROR;
                    break;
                }
                _currentAutoStrategy.macro = buffer[2];
                _currentAutoStrategy.direction = (char)buffer[3];
                _currentAutoStrategy.search = buffer[4];
                _currentAutoStrategy.weapon = buffer[5];
                _currentAutoStrategy.isNew = true;
            }
            else if(payloadType == PAYLOAD_COMBAT_PROFILE) {
                // A HUD do Fumacinha nunca manda CombatTuning (o dashboard não
                // tem controle pra isso, ver RobotTypes.hpp) — só os 9 campos
                // por-luta. Tuning fica sempre no default do struct, igual o
                // comportamento atual via HTTP.
                if(bufferSize < 12) {
                    ack = ACK_ERROR;
                    break;
                }
                _currentCombatProfile.openingTactic = static_cast<OpeningTactic>(constrain(buffer[2], 0, 9));
                _currentCombatProfile.searchTactic = static_cast<SearchTactic>(constrain(buffer[3], 0, 4));
                _currentCombatProfile.attackTactic = static_cast<AttackTactic>(constrain(buffer[4], 0, 1));
                uint16_t finishTimeS = (uint16_t)(buffer[5] | (buffer[6] << 8));
                _currentCombatProfile.finishTimeS = (uint16_t)constrain(finishTimeS, 0, 1023);
                _currentCombatProfile.preferredSide = buffer[7] ? Direction::right : Direction::left;
                _currentCombatProfile.searchEmitters = buffer[8] != 0;
                _currentCombatProfile.attackEmitters = buffer[9] != 0;
                _currentCombatProfile.opponentHasWing = buffer[10] != 0;
                _currentCombatProfile.weapon = buffer[11] != 0;
                _currentCombatProfile.isNew = true;
            }
            else {
                ack = ACK_ERROR;
            }
            break;
        }

        case CMD_SET_TEST: {
            if(bufferSize < 3) {
                ack = ACK_ERROR;
                break;
            }
            const uint8_t which = buffer[1];
            const bool state = buffer[2] != 0;

            switch(which) {
                case TEST_SENSOR:
                    _sensorTestActive = state;
                    if(state) {
                        _motorTestActive = false;
                        _servoTestActive = false;
                    }
                    break;
                case TEST_MOTOR:
                    _motorTestActive = state;
                    if(state) {
                        _sensorTestActive = false;
                        _servoTestActive = false;
                    }
                    break;
                case TEST_SERVO:
                    _servoTestActive = state;
                    if(state) {
                        _sensorTestActive = false;
                        _motorTestActive = false;
                    }
                    break;
                default:
                    ack = ACK_ERROR;
                    break;
            }

            if(!_sensorTestActive && !_motorTestActive && !_servoTestActive) {
                setTestReadout("{}"); // nenhum teste ativo: não há leitura pra mostrar
            }
            break;
        }

        case CMD_TRIGGER_MACRO: {
            // Payload: numSteps:u8, depois numSteps x (l:i8, r:i8, d:u16 little-endian).
            if(bufferSize < 2) {
                ack = ACK_ERROR;
                break;
            }
            const uint8_t numSteps = buffer[1];
            if(numSteps < 1 || numSteps > MAX_MACRO_STEPS || bufferSize < (size_t)(2 + numSteps * 4)) {
                ack = ACK_ERROR;
                break;
            }
            for(uint8_t i = 0; i < numSteps; i++) {
                const uint8_t *step = buffer + 2 + i * 4;
                _testMacroSteps[i].leftSpeed = (int8_t)step[0];
                _testMacroSteps[i].rightSpeed = (int8_t)step[1];
                _testMacroSteps[i].durationMs = (unsigned long)(step[2] | (step[3] << 8));
            }
            _testMacroCount = numSteps;
            _testMacroPending = true;
            break;
        }

        default:
            ack = ACK_ERROR;
    }

    characteristic->setValue(&ack, 1);
}

void BleConfigServer::begin() {
    BLEDevice::init(Config::ROBOT_NAME);

    BLEServer *server = BLEDevice::createServer();
    BLEService *service = server->createService(SERVICE_UUID);

    _characteristic = service->createCharacteristic(CHARACTERISTIC_UUID,
                                                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    _characteristic->setCallbacks(new BleConfigServerCallbacks(this));

    uint8_t initialAck = ACK_OK;
    _characteristic->setValue(&initialAck, 1);

    service->start();

    BLEAdvertising *advertising = server->getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    // Recomendação padrão da lib pra evitar conexão lenta/instável em
    // iPhone e alguns Android.
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);
    advertising->start();

    Serial.printf("[BLE] GATT server ativo, anunciando '%s'\n", Config::ROBOT_NAME);
}

void BleConfigServer::shutdown() {
    // Só para de anunciar (novos pareamentos) — não derruba a conexão ativa
    // com uma resposta pendente no meio.
    BLEDevice::getAdvertising()->stop();
    Serial.println("[BLE] Parou de anunciar.");
}

void BleConfigServer::update() {
    // A lib processa os callbacks de característica (read/write) na própria
    // task interna do Bluedroid — não precisa de pump manual aqui. Mantido
    // só por simetria de interface com o ConfigServer/AutoMode.
}

void BleConfigServer::setTestReadout(const String &json) {
    portENTER_CRITICAL(&s_readoutMux);
    _testReadoutJson = json;
    portEXIT_CRITICAL(&s_readoutMux);
}

bool BleConfigServer::consumePayload(AutoStrategy &outStrategy) {
    if(!_currentAutoStrategy.isNew) {
        return false;
    }
    outStrategy = _currentAutoStrategy;
    _currentAutoStrategy.isNew = false;
    return true;
}

bool BleConfigServer::consumeCombatProfile(CombatProfile &outProfile) {
    if(!_currentCombatProfile.isNew) {
        return false;
    }
    outProfile = _currentCombatProfile;
    _currentCombatProfile.isNew = false;
    return true;
}

bool BleConfigServer::consumeMacroTest(MotionStep *dst, int dstCap, int &outCount) {
    if(!_testMacroPending) {
        outCount = 0;
        return false;
    }
    int n = _testMacroCount < dstCap ? _testMacroCount : dstCap;
    for(int i = 0; i < n; i++) {
        dst[i] = _testMacroSteps[i];
    }
    outCount = n;
    _testMacroPending = false;
    return true;
}

void BleConfigServer::clearTests() {
    _sensorTestActive = false;
    _motorTestActive = false;
    _servoTestActive = false;
    setTestReadout("{}");
}

#endif // ROBOT_FUEGO || ROBOT_FUMACINHA
