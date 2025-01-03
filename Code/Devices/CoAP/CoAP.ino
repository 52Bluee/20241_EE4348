#include <WiFi.h>
#include <coap-simple.h>
#include <ESPping.h>
const char* ssid = "tlinhdaynay";
const char* pass = "tlinhdayne";

// ThingsBoard URL
int remote_port = 5683;
int local_port = 10000;
IPAddress remote_ip(104, 196, 24, 70);
IPAddress local_ip(172, 20, 10, 8);

// Device Access Token
String HUMIDITY_AC = "AyYM5ClBZVYZxwXuB6Sa";
String CONTROLLER_AC = "AyYM5ClBZVYZxwXuB6Sa";

unsigned long lastTime1 = -6000;
unsigned long lastTime2 = 0;
unsigned long timerDelay = 12000;

WiFiUDP udp;
Coap coap(udp);

void callback_response(CoapPacket &packet, IPAddress ip, int port);

void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap/RPC Response got]");
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  Serial.println(p);
}

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
  Serial.println("Setup Response Callback");
  coap.response(callback_response);
  coap.start();
}

void postHumidity(IPAddress server, int port) {
  int humidity = random(0, 101); // Random value between 0 and 100
  String url = "api/v1/" + HUMIDITY_AC + "/telemetry";
  String payload = String("{\"humidity\":") + String(humidity) + String("}");

  Serial.println("Posted " + payload);
    
  int responseCode = coap.post(server, port, url.c_str(), payload.c_str());
  Serial.print("Response Code: ");
  Serial.println(responseCode);

  lastTime1 = millis();
}

void postController(IPAddress server, int port) {
  String url = "api/v1/" + CONTROLLER_AC + "/rpc";
  String payload = "{\"method\":\"getHumidityControlSignal\", \"params\":{}}";

  Serial.print("Sending RPC: Get moisture control signal");
  Serial.println(payload);
    
  int responseCode = coap.post(server, port, url.c_str(), payload.c_str());
    
  Serial.print("Response Code: ");
  Serial.println(responseCode);
  lastTime2 = millis();
}

void loop() {
  coap.loop();
  // Send an CoAP POST request every 10 seconds
  if (WiFi.status() == WL_CONNECTED){
    Serial.println("Wifi is connected");
    if (Ping.ping("8.8.8.8") > 0){
      Serial.println("Internet is connected");
      if ((millis() - lastTime1) > timerDelay){
        postHumidity(remote_ip, remote_port);
      }
      if ((millis() - lastTime2) > timerDelay){
        postController(remote_ip, remote_port);
      }
    } else{
      Serial.println("Internet connection is error");
      Serial.println("Connecting to controller ... ");
      
        Serial.println("Connected to controller");
        if ((millis() - lastTime1) > timerDelay){
          postHumidity(local_ip, local_port);
        }
        if ((millis() - lastTime2) > timerDelay){
          postController(local_ip, local_port);
        }
      
    }
  } else{
    Serial.println("Cannot connect to wifi");
  }
}


