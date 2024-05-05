#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "FRASANET-CASA";
const char* password = "80808080";
const char* mqtt_server = "192.168.0.33";
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup_wifi() {
  delay(10);
  // Conecta-se ao WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
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
      Serial.println("connected");
      // Uma vez conectado, publica uma mensagem...
      mqttClient.publish(mqtt_topic, "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Espera 5 segundos antes de tentar novamente
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi(); // Inicializa a conexão WiFi
  mqttClient.setServer(mqtt_server, mqtt_port); // Configura o servidor MQTT e a porta
  reconnect(); // Conecta ao servidor MQTT
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop(); // Mantém a conexão MQTT ativa

  // Exemplo: Publica uma mensagem a cada 10 segundos
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    // Publica uma nova mensagem
    mqttClient.publish(mqtt_topic, "Hello from ESP");
  }
}
