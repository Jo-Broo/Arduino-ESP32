#include <HTTPClient.h>
#include <WiFi.h>
#include <DHT22.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3c

// Stammpfad des Servers wo die verschiedenen .php Dateien liegen
String serverName = "http://192.168.178.33:8080/insert_temp.php";


// Hilfsvariablen
unsigned long mytime = 0;
const int INTERVAL = 60; // Angabe in s wird im Code in ms umgerechnet
int Interval_ms;
const int DHTPIN = 16;
String IP = "";
int httpResponseCode = 0;

const char* ssid = "ESP 32";
const char* password = "13579";

DHT22 dht(DHTPIN);
SSD1306AsciiWire oled;

struct Wetter_eintrag{
  float temperature;
  float humidity;
};

Wetter_eintrag eintrag;

void initWiFi() {
  WiFi.mode(WIFI_STA); // Der ESP wählt sich als Client in ein Netzwerk ein
  WiFi.begin("FRITZ!Box 7530 WY", "09612677397865897502");  
  Serial.print("Connecting to WiFi network");
  oled.print("Connecting to WiFi network");
  while (WiFi.status() != WL_CONNECTED) {
    oled.print(".");
    Serial.print('.');
    delay(2000);
  }
  oled.clear();
  oled.print("IP: ");oled.println(WiFi.localIP());
  Serial.println(WiFi.localIP());
}

bool Messungen(){
  eintrag.temperature = dht.getTemperature();
  eintrag.humidity = dht.getHumidity();

  Serial.print("t=");Serial.print(eintrag.temperature,1);Serial.print("\t");
  Serial.print("h=");Serial.println(eintrag.humidity,1);

  if (dht.getLastError() != dht.OK) {
    Serial.print("last error :");
    Serial.println(dht.getLastError());
    return false;
  }
  
  if(isnan(eintrag.temperature)||isnan(eintrag.humidity)){
    return false;
  }
  else{
    return true;
  }
}

void Insert(float Temperature = 0.00, float Humidity = 0.00){    
  
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      String serverPath = serverName + "?Temperature=" + Temperature + "&Humidity=" + Humidity;
      //Serial.println(serverName + "?Temperature=" + Temperature + "&Pressure=" + Pressure + "&Humidity=" + Humidity);
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
            
      // Send HTTP GET request
      httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");      
    }    
}

void setup() {  
  Serial.begin(115200);    

  Wire.begin();
  Wire.setClock(400000L);
  oled.setFont(TimesNewRoman13); 
  oled.begin(&Adafruit128x64, I2C_ADDRESS);

  initWiFi();
  Interval_ms = INTERVAL * 1000;
}

void loop() {
  if((millis()-mytime) > Interval_ms){
    Messungen();
    Insert(eintrag.temperature,eintrag.humidity);

    oled.clear(); 
    oled.print("IP: ");oled.println(WiFi.localIP());
    oled.print("T: ");oled.print(eintrag.temperature);
    oled.print(" C | H: ");oled.print(eintrag.humidity);oled.println(" %");
    oled.print("HTML Response: ");oled.println(httpResponseCode);

    mytime = millis();
  }
}
