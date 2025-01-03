#include <ESPping.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "tlinhdayne";
const char* password = "12345678";

void setup()
{
  Serial.begin(115200);
  delay(100);

  WiFi.begin(ssid, password);

  // attente connexion
  Serial.print("\nConnection");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnection OK, IP: ");
  Serial.print(WiFi.localIP());
  Serial.print(" Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.println("You can try to ping me ;-)");

}

void loop()
{
  
  // Ping IP
  const IPAddress remote_ip(192, 168, 4, 4);
  Serial.print(remote_ip);
  if (Ping.ping(WiFi.localIP()) > 0){
    Serial.printf(" response time : %d/%.2f/%d ms\n", Ping.minTime(), Ping.averageTime(), Ping.maxTime());
    Serial.printf(" Internet connection is activated");
  } else {
    Serial.println(" Error !");
  }
  delay(1000);
  

  
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Process messages from ThingsBoard if needed
}
