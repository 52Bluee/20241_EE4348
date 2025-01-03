#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPping.h>
const char* ssid = "linh";
const char* pass = "linhlinh";

// ThingsBoard URL
const char* URL = "http://104.196.24.70/api/v1/";
const char* URL_remote = "http://c7.hust-2slab.org/api/v1/";
const char* URL_local = "http://172.20.10.8:10000/api/v1/";
IPAddress remote_ip(104, 196, 24, 70);
IPAddress local_ip(172, 20, 10, 2);

// Device Access Token
String LIGHT_AC = "LF3pWNnIiBk3UphraMpG";
String CONTROLLER_AC = "LF3pWNnIiBk3UphraMpG";
 
unsigned long lastTime1 = -5000;
unsigned long lastTime2 = 0;
unsigned long timerDelay = 10000;
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
 
  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  // Information message
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  randomSeed(analogRead(33));  // Use a random seed for the random values
}
 
void loop() {
  // Send an HTTP POST request every 10 seconds
  if (WiFi.status() == WL_CONNECTED){
    Serial.println("Wifi is connected");
    if (Ping.ping("8.8.8.8") > 0){
      Serial.println("Internet is connected");
      if ((millis() - lastTime1) > timerDelay){
        postLight(URL_remote);
      }
      if ((millis() - lastTime2) > timerDelay){
        postController(URL_remote);
      }
    } else{
      Serial.println("Internet connection is error");
      Serial.println("Connecting to controller ... ");
      
        Serial.println("Connected to controller");
        if ((millis() - lastTime1) > timerDelay){
          postLight(URL_local);
        }
        if ((millis() - lastTime2) > timerDelay){
          postController(URL_local);
        }
      
    }
  } else{
    Serial.println("Cannot connect to wifi");

  }
}

void postLight(const char* URL)
{

      HTTPClient http;
 
      // Create the URL to send data to ThingsBoard
      String url = String(URL) + LIGHT_AC + "/telemetry";
      // Create the payload in JSON format
      String payload = "{\"light\":" + String(random(20, 120)) + "}";
 
      // Start the HTTP request
      http.begin(url);
 
      // Set the headers
      http.addHeader("Content-Type", "application/json");
 
      // Send the HTTP POST request
      int httpResponseCode = http.POST(payload);
      Serial.println("Posted " + payload);
      // Check the response code
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        //Serial.print("Response: ");
        //Serial.println(response);
      } else {
        Serial.print("Error in sending POST request: ");
        Serial.println(httpResponseCode);
      }
 
      // Free resources
      http.end();
     
 
    // Update last time
    lastTime1 = millis();
}
 
void postController(const char* URL)
{
    
      HTTPClient http;
 
      // Create the URL to send data to ThingsBoard
      String url = String(URL) + CONTROLLER_AC + "/rpc";
      // Create the payload in JSON format
      String payload = "{\"method\":\"getLightControlSignal\", \"params\":{}}";
 
      // Start the HTTP request
      http.begin(url);
 
      // Set the headers
      http.addHeader("Content-Type", "application/json");
 
      // Send the HTTP POST request
      int httpResponseCode = http.POST(payload);
      Serial.println("Posted " + payload);
      // Check the response code
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response: ");
        Serial.println(response);
      } else {
        Serial.print("Error in sending POST request: ");
        Serial.println(httpResponseCode);
      }
 
      // Free resources
      http.end();
     
 
    // Update last time
    lastTime2 = millis();
}