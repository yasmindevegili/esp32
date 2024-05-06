#include <WiFi.h>
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEScan.h>

const char* ssid = "FRASANET-CASA";
const char* password = "80808080";
const char* mqtt_server = "192.168.0.33";
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32";

const float distance = 10.0;
const float RSSI = -59;
const float n = 2;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
BLEServer* pBLEServer;
BLEService* pBLEService;
BLEScan* pBLEScan;

class Callbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) override {
        Serial.println("Connected device");
    }

    void onDisconnect(BLEServer* pServer) override {
        Serial.println("Disconected device");
    }
};

float measureDistance(int rssi) {
    return pow(10, (RSSI - rssi) / (10 * n));
}

void setup_wifi() {
  // Conecta-se ao WiFi
  Serial.print("Attempting WiFi connection...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop até que estejamos reconectados
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Tenta conectar
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("MQTT connected");
      // Uma vez conectado, publica uma msg...
      mqttClient.publish(mqtt_topic, "ESP on!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Espera 5 segundos antes de tentar novamente
      delay(5000);
    }
  }
}

void setup_bluetooth() {
  Serial.print("Attempting Bluetooth connection...");
   BLEDevice::init("ESP32");
    pBLEServer = BLEDevice::createServer();
    pBLEServer->setCallbacks(new Callbacks());
    pBLEService = pBLEServer->createService(BLEUUID((uint16_t)0xFFE0));
    pBLEService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pBLEService->getUUID());
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();

    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void scanDevices() {
    BLEScanResults foundDevices = pBLEScan->start(5, false);
    String msg = "Detected devices: ";
    int count = 0;

    for (int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        float meters = measureDistance(device.getRSSI());
        
        if (meters <= distance) {
            count++;
            String MACDevice = device.getAddress().toString().c_str();
            msg += "Device " + String(count) + ": " + 
                        MACDevice + ", RSSI: " + String(device.getRSSI()) + 
                        ", Distance: " + String(meters, 2) + "m / " + 
                        String(meters * 100, 0) + "cm; ";
        }
    }

    msg = "Total de Dispositivos dentro de " + String(distance) + "m: " + 
               String(count) + "; " + msg;
    mqttClient.publish(mqtt_topic, msg.c_str());
    pBLEScan->clearResults();  // Limpa os resultados para o próximo scan
}

void setup() {
  Serial.begin(115200);
  setup_wifi(); // Inicializa a conexão WiFi
  mqttClient.setServer(mqtt_server, mqtt_port); // Configura o servidor MQTT e a porta
  reconnect(); // Conecta ao servidor MQTT
  setup_bluetooth();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop(); // Mantém a conexão MQTT ativa

    mqttClient.loop();
    scanDevices();
    delay(5000);
  }
