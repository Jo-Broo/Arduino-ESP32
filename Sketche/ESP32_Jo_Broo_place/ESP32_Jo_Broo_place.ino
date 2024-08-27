// Jo-Broo/place
// Mit diesem Sketch wird dein ESP32 zu einem Künstler :D

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// NeoPixel setup
#define PIN       14 
#define NUMPIXELS 5 
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "FRITZ!Box 7530 WY";            
const char* password = "09612677397865897502";
WiFiServer server(80);


void setup() {
  Serial.begin(115200);

  // NeoPixel setup
  pixels.begin(); 
  pixels.setBrightness(100);

  // WiFi Setup
  Serial.println();                               
  Serial.print("Connecting to ");                 
  Serial.println(ssid);                           

  WiFi.begin(ssid, password);                     

  while (WiFi.status() != WL_CONNECTED) {         
    delay(500);                                   
    Serial.print(".");                            
  }                                               

  Serial.println("");                             
  Serial.println("WiFi connected");               
  Serial.println("IP address: ");                 
  Serial.println(WiFi.localIP());  

  server.begin();
}

void loop() {
  // Client handling
  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        String received_data = client.readStringUntil('\n');
        
        StaticJsonDocument<50> doc; // Puffergröße anpassen je nach Größe deines JSON
        DeserializationError error = deserializeJson(doc, received_data);
        if (error) {
          Serial.print("Fehler beim Parsen: ");
          Serial.println(error.c_str());
          return;
        }
        Serial.print("Received data: ");
        Serial.println((int)doc["led"]);
        Serial.println((int)doc["r"]);
        Serial.println((int)doc["g"]);
        Serial.println((int)doc["b"]);
        pixels.setPixelColor((int)doc["led"], pixels.Color((int)doc["r"], (int)doc["g"], (int)doc["b"]));
        pixels.show();
      }
    }
    client.stop();
  }
}
