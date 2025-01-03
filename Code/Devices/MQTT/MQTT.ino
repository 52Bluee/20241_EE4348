#include <WiFi.h>  // Wi-Fi library
#include <PubSubClient.h>
#include <ESPping.h>
const char* ssid = "tlinhdaynay";
const char* pass = "tlinhdayne";

const char* remote_server = "104.196.24.70";
const char* local_server = "172.20.10.8";
const int remote_port = 1883;
const int local_port = 9999;
const char* mqttUsername = "AyYM5ClBZVYZxwXuB6Sa";  // Access Token from ThingsBoard

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  randomSeed(analogRead(33));
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqttUsername, NULL)) {
      Serial.println("Reconnected to MQTT Broker");
    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(client.state());
      Serial.println(" Trying again in 5 seconds");
      delay(2500);
    }
  }
}

void postTemperature(const char* server, const int port){
  client.setServer(server, port);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  int temperature = random(0, 100);
  char payload[100];
  snprintf(payload, sizeof(payload), "{\"temperature\":%d}", temperature);
  Serial.print("Sending temperature: " + String(temperature) + "\n");
  client.publish("v1/devices/me/telemetry", payload);
  delay(2500);
}

void loop() {
  client.loop();
  // Send an MQTT POST request every 10 seconds
  if (WiFi.status() == WL_CONNECTED){
    Serial.println("Wifi is connected");
    if (Ping.ping("8.8.8.8") > 0){
      Serial.println("Internet is connected");
      postTemperature(remote_server, remote_port);
    } else{
      Serial.println("Internet connection is error");
      Serial.println("Connecting to controller ... ");
      Serial.println("Connected to controller");
      postTemperature(local_server, local_port);      
    }
  } else{
    Serial.println("Cannot connect to wifi");
  }
}



void callback(char* topic, byte* payload, unsigned int length) {
  // Process messages from ThingsBoard if needed
}