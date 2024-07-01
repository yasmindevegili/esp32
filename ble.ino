#include <WiFi.h>
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEScan.h>
#include <esp_wpa2.h>

const char* ssid = "YOUR-SSID";
const char* password = "YOUR-PASSWORD";
const char* username = "YOUR-USERNAME"; //only if network connection requires
const char* mqtt_server = "IP-MOSQUITTO-SERVER";
const int mqtt_port = 1883;
const char* mqtt_topic = "TOPIC-NAME";

const float distance = 10.0;
const float RSSI = -70;
const float n = 2;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
BLEServer* pBLEServer;
BLEService* pBLEService;
BLEScan* pBLEScan;

class Callbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) override {
    Serial.println("Connected device");
  }

  void onDisconnect(BLEServer* pServer) override {
    Serial.println("Disconnected device");
  }
};

float measureDistance(int rssi) {
  return pow(10, (RSSI - rssi) / (10 * n));
}

void setup_wifi() {
  Serial.print("Attempting WiFi connection...");
  //WiFi.mode(WIFI_STA); uncomment this if network requires username
  //configureWPA2();     uncomment this if network requires username
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void configureWPA2(){

  esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password, strlen(password));
  esp_wifi_sta_wpa2_ent_enable();

}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32")) {
      Serial.println("MQTT connected");
      mqttClient.publish(mqtt_topic, "ESP on!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
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
      msg += "Device " + String(count) + ": " + MACDevice + 
             ", RSSI: " + String(device.getRSSI()) + 
             ", Distance: " + String(meters, 2) + "m / " + 
             String(meters * 100, 0) + "cm; \n";
    }
  }

  msg = "Total de Dispositivos: " + 
        String(count) + ";\n " + msg;
  mqttClient.publish(mqtt_topic, msg.c_str());
  pBLEScan->clearResults();
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  reconnect();
  setup_bluetooth();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  scanDevices();
  delay(5000);
}
