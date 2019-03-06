#include <AsyncMqttClient.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>

#define WIFI_SSID "Solomaha"
#define WIFI_PASSWORD "solomakha21"

#define MQTT_HOST IPAddress(192, 168, 1, 76)
#define MQTT_PORT 1883
#define MQTT_ID "room1-light"
#define MQTT_PASSWORD "sdfnkjhdhjkvcfhjkvsdhjk324bkwerfsdw23r2345"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

const int relayPin = D1;

void connectToWifi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP &event) {
    Serial.println("Connected to Wi-Fi.");
    connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &event) {
    Serial.println("Disconnected from Wi-Fi.");
    mqttReconnectTimer.detach();
    wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool) {
    Serial.println("Connected to MQTT.");

    // Subscribe to topics:
    mqttClient.subscribe("switch/room1-light/set", 0);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.print("Disconnected from MQTT. Reason: ");
    Serial.println((int) reason);

    if (WiFi.isConnected()) {
        mqttReconnectTimer.once(2, connectToMqtt);
    }
}

void onMqttMessage(char *topic, char *payload,
                   AsyncMqttClientMessageProperties properties, size_t len,
                   size_t index, size_t total) {
    if (strcmp(topic, "switch/room1-light/set") == 0) {
        if (strcmp(payload, "{\"state\":true}") == 0) {
            digitalWrite(relayPin, 0);  // Turn on
        } else if (strcmp(payload, "{\"state\":false}") == 0) {
            digitalWrite(relayPin, 1);  // Turn off
        } else {
            Serial.println("Unknown payload:");
            Serial.println(payload);
        }

        mqttClient.publish("switch/room1-light", 0, false, payload);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    pinMode(relayPin, OUTPUT);

    // Turn off by default
    digitalWrite(relayPin, 1);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setClientId(MQTT_ID);
    mqttClient.setCredentials("device", MQTT_PASSWORD);

    connectToWifi();
}

void loop() {}