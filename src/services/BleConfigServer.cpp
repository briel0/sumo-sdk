#include "BleConfigServer.hpp"
#include "Config.hpp"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <string.h>

namespace {

    // Mesmo serviço/characteristic da versão anterior (BTstack) — só trocou o
    // stack por baixo, o protocolo do app não muda.
    constexpr char SERVICE_UUID[] = "0000ff10-0000-1000-8000-00805f9b34fb";
    constexpr char CHARACTERISTIC_UUID[] = "0000ff11-0000-1000-8000-00805f9b34fb";

    // Comandos do protocolo — ver a tabela completa na conversa de design
    // (WRITE = 1 byte de comando + payload; READ = resposta do último comando).
    enum BleCommand : uint8_t {
        CMD_GET_PROFILE = 0x01,
        CMD_GET_SENSORS = 0x02,
        CMD_SET_STRATEGY = 0x03,
        CMD_SET_TEST = 0x04,
        CMD_TRIGGER_MACRO = 0x05,
    };

    enum BleAck : uint8_t {
        ACK_OK = 0x00,
        ACK_ERROR = 0x01,
    };

    // Cópia estável do JSON de sensores no instante do comando. Necessária
    // porque _testReadoutJson é uma String mutável, escrita pelo AutoMode::run()
    // enquanto setValue() copia o buffer pro característic — sem essa cópia,
    // um _testReadoutJson sendo reatribuído no meio do strncpy correria risco de
    // leitura inconsistente.
    char s_sensorSnapshot[512];

} // namespace

// A lib BLE nativa entrega o WRITE via uma classe de callback própria (não
// um ponteiro de função C solto como o BTstack) — guarda um ponteiro pra a
// instância dona e repassa pro método público handleWrite().
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
        case CMD_GET_PROFILE:
            // UI_PROFILE_JSON é constexpr em flash — ponteiro direto é seguro,
            // setValue() copia o conteúdo, não guarda o ponteiro.
            characteristic->setValue((uint8_t *)Config::UI_PROFILE_JSON, strlen(Config::UI_PROFILE_JSON));
            return;

        case CMD_GET_SENSORS:
            strncpy(s_sensorSnapshot, _testReadoutJson.c_str(), sizeof(s_sensorSnapshot) - 1);
            s_sensorSnapshot[sizeof(s_sensorSnapshot) - 1] = '\0';
            characteristic->setValue((uint8_t *)s_sensorSnapshot, strlen(s_sensorSnapshot));
            return;

        case CMD_SET_STRATEGY:
            if(bufferSize < 5) {
                ack = ACK_ERROR;
                break;
            }
            _currentAutoStrategy.macro = buffer[1];
            _currentAutoStrategy.direction = (char)buffer[2];
            _currentAutoStrategy.search = buffer[3];
            _currentAutoStrategy.weapon = buffer[4];
            _currentAutoStrategy.isNew = true;
            break;

        case CMD_SET_TEST: {
            if(bufferSize < 3) {
                ack = ACK_ERROR;
                break;
            }
            const bool isMotor = buffer[1] == 1;
            const bool state = buffer[2] == 1;
            if(isMotor) {
                if(_motorTestCallback)
                    _motorTestCallback(state);
            }
            else {
                if(_sensorTestCallback)
                    _sensorTestCallback(state);
            }
            break;
        }

        case CMD_TRIGGER_MACRO: {
            // Payload: numSteps:u8, depois numSteps x (l:i8, r:i8, d:u16 little-endian).
            // Cap de 8 passos casa com o array fixo _macroTestSteps e com o
            // limite que a rota HTTP /test-macro já impunha.
            if(bufferSize < 2) {
                ack = ACK_ERROR;
                break;
            }
            const uint8_t numSteps = buffer[1];
            if(numSteps < 1 || numSteps > 8 || bufferSize < (size_t)(2 + numSteps * 4)) {
                ack = ACK_ERROR;
                break;
            }
            for(uint8_t i = 0; i < numSteps; i++) {
                const uint8_t *step = buffer + 2 + i * 4;
                _macroTestSteps[i].leftSpeed = (int8_t)step[0];
                _macroTestSteps[i].rightSpeed = (int8_t)step[1];
                _macroTestSteps[i].durationMs = (unsigned long)(step[2] | (step[3] << 8));
            }
            if(_macroTestCallback) {
                MotionSequence seq = {_macroTestSteps, numSteps};
                _macroTestCallback(seq);
            }
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

    _characteristic = service->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ |
                                                                             BLECharacteristic::PROPERTY_WRITE);
    _characteristic->setCallbacks(new BleConfigServerCallbacks(this));

    uint8_t initialAck = ACK_OK;
    _characteristic->setValue(&initialAck, 1);

    service->start();

    BLEAdvertising *advertising = server->getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    // Recomendação padrão da lib pra evitar conexão lenta/instável em
    // iPhone e alguns Android — sem isso a primeira conexão às vezes falha.
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);
    advertising->start();

    Serial.printf("[BLE] GATT server ativo, anunciando '%s'\n", Config::ROBOT_NAME);
}

void BleConfigServer::shutdown() {
    // Só para de anunciar (novos pareamentos) — não derruba a conexão ativa.
    // Mesmo raciocínio do ConfigServer HTTP: matar o rádio com uma resposta
    // pendente no meio arrisca estado inconsistente.
    BLEDevice::getAdvertising()->stop();
    Serial.println("[BLE] Parou de anunciar.");
}

void BleConfigServer::update() {
    // A lib processa os callbacks de característica (read/write) na própria
    // task interna do Bluedroid — não precisa de pump manual aqui. Mantido
    // só por simetria de interface com AutoMode, que chama update() todo
    // frame independente do transporte.
}

bool BleConfigServer::consumePayload(AutoStrategy &outStrategy) {
    if(!_currentAutoStrategy.isNew) {
        return false;
    }
    outStrategy = _currentAutoStrategy;
    _currentAutoStrategy.isNew = false;
    return true;
}
