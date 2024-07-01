# How to use an ESP32 to scan bluetooth devices nearby:

1. Install and configure an broker like Mosquitto Broker and set it's IP adress at line 12:
    
    ```jsx
    const char* mqtt_server = "IP-MOSQUITTO-SERVER";
    ```
    
2. Now you can provide your network configuration settings for SSID, password and username (if requires).
    
    ```jsx
    const char* ssid = "YOUR-SSID";
    const char* password = "YOUR-PASSWORD";
    const char* username = "YOUR-USERNAME"; //only if network connection requires
    ```
    
3. If your are going to use an network that requires username, remember no uncomment lines 42 and 43:
    
    ```jsx
    //WiFi.mode(WIFI_STA); uncomment this if network requires username
    //configureWPA2();     uncomment this if network requires username
    ```
    
4. This way you are ready to go, you can use an MQTT client like MQTT Explorer for Windows or MyMQTT for smartphones to receive the messages.
