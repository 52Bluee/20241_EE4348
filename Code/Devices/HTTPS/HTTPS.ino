#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESPping.h>
const char* ssid = "linh";
const char* pass = "linhlinh";
 
// ThingsBoard URL

const char* URL_remote = "https://104.196.24.70/api/v1/";
const char* URL_local = "https://172.20.10.2:10000/api/v1/";
IPAddress remote_ip(104, 196, 24, 70);
IPAddress local_ip(172, 20, 10, 2);

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

// Device Access Token
String LIGHT_AC = "fXhK5jIGaAnllN7lkqib";
String CONTROLLER_AC = "AyYM5ClBZVYZxwXuB6Sa";
 
unsigned long lastTime1 = -6000;
unsigned long lastTime2 = 0;
unsigned long timerDelay = 12000;
WiFiClientSecure client;
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
  client.setCACert(ca_cert);
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
      /*if ((millis() - lastTime2) > timerDelay){
        postController(URL_remote);
      }*/
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

      HTTPClient https;
 
      // Create the URL to send data to ThingsBoard
      String url = String(URL) + LIGHT_AC + "/telemetry";
      // Create the payload in JSON format
      String payload = "{\"light\":" + String(random(20, 120)) + "}";
 
      // Start the HTTP request
      https.begin(url);
 
      // Set the headers
      https.addHeader("Content-Type", "application/json");
 
      // Send the HTTP POST request
      int httpsResponseCode = https.POST(payload);
      Serial.println("Posted " + payload);
      // Check the response code
      if (httpsResponseCode > 0) {
        String response = https.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpsResponseCode);
        //Serial.print("Response: ");
        //Serial.println(response);
      } else {
        Serial.print("Error in sending POST request: ");
        Serial.println(httpsResponseCode);
      }
 
      // Free resources
      https.end();
     
 
    // Update last time
    lastTime1 = millis();
}
 
void postController(const char* URL)
{
    
      HTTPClient https;
 
      // Create the URL to send data to ThingsBoard
      String url = String(URL) + CONTROLLER_AC + "/rpc";
      // Create the payload in JSON format
      String payload = "{\"method\":\"getLightControlSignal\", \"params\":{}}";
 
      // Start the HTTP request
      https.begin(url);
 
      // Set the headers
      https.addHeader("Content-Type", "application/json");
 
      // Send the HTTP POST request
      int httpsResponseCode = https.POST(payload);
      Serial.println("Posted " + payload);
      // Check the response code
      if (httpsResponseCode > 0) {
        String response = https.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpsResponseCode);
        Serial.print("Response: ");
        Serial.println(response);
      } else {
        Serial.print("Error in sending POST request: ");
        Serial.println(httpsResponseCode);
      }
 
      // Free resources
      https.end();
     
 
    // Update last time
    lastTime2 = millis();
}