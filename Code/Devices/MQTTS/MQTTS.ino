#include <WiFi.h>  // Wi-Fi library
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESPping.h>
const char* ssid = "tlinhdaynay";
const char* pass = "tlinhdayne";

const char* remote_server = "104, 196, 24, 70";
const char* local_server = "";
const int remote_port = 8883;
const int local_port = 9999;
const char* mqttUsername = "dQBlc0MgCyDz86j6LYpQ";  // Access Token from ThingsBoard

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb\n" \
"MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\n" \
"GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj\n" \
"YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL\n" \
"MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n" \
"BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM\n" \
"GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP\n" \
"ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua\n" \
"BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe\n" \
"3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4\n" \
"YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR\n" \
"rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm\n" \
"ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU\n" \
"oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v\n" \
"QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t\n" \
"b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF\n" \
"AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q\n" \
"GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz\n" \
"Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2\n" \
"G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi\n" \
"l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3\n" \
"smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==\n" \
"-----END CERTIFICATE-----\n";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  espClient.setCACert(ca_cert);
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
      delay(5000);
    }
  }
}

void postTemperature(const char* server, const int port){
  client.setServer(server, port);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  int temperature = random(0, 20);
  char payload[100];
  snprintf(payload, sizeof(payload), "{\"temperature\":%d}", temperature);
  Serial.print("Sending temperature: " + String(temperature));
  client.publish("v1/devices/me/telemetry", payload);
  delay(10000);
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