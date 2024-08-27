#include "Adafruit_CCS811.h"
#include <WiFi.h>                                 
#include <PubSubClient.h> 
#include <ArduinoJson.h>                          
#include <ArduinoJson.hpp>
#include <Wire.h>                                 
#include "DHT.h" 

#define uS_TO_S_FACTOR 1000000  /* Konversionsfaktor von us in s */
#define TIME_TO_SLEEP  5        /* zeitspanne die der ESP schläft in s */

RTC_DATA_ATTR int bootCount = 0; /* Angabe des Bootzykluses */

float Temperature_DHT;
float Humidity_DHT;
int CO2_CCS811;
int TVOC_CCS811;

Adafruit_CCS811 ccs; /* CCS811-Sensor Objekt */

#define DHTTYPE DHT22                             
#define DHTPIN 19

DHT dht(DHTPIN, DHTTYPE);

// int Helligkeitsensor = 4;
// int Helligkeit;

// WLAN/MQTT Setup
const char* ssid = "FRITZ!Box 7530 WY";           
const char* password = "09612677397865897502";    
WiFiClient wifi_client;
                           
const char* mqtt_server = "192.168.178.29";       
const char* topic = "ESP32/SmartTemp";              
PubSubClient client(wifi_client);

/*
Gibt den aufwachgrund im seriellen Monitor aus 
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

// Versetzt den ESP in den tiefschlaf
void goto_Deepsleep(){
  Serial.println("Going to sleep after " + String(millis()) + "ms operating time");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
}

void print_measurments(){
  if(ccs.available()){
    if(!ccs.readData()){

      // abrufen der Daten
      CO2_CCS811 = ccs.geteCO2();
      char co2string[8];
      TVOC_CCS811 = ccs.getTVOC();
      char tvocstring[8];
      Temperature_DHT = dht.readTemperature();           
      Humidity_DHT = dht.readHumidity();
      // Helligkeit = map(analogRead(Helligkeitsensor),1400,4095,0,100);

      // ausgabe in den seriellen Monitor
      Serial.print("Temp: ");
      Serial.print(Temperature_DHT);
      Serial.print(" Humid: ");
      Serial.print(Humidity_DHT);
      // Serial.print(" Helligkeit:");
      // Serial.print(Helligkeit);
      Serial.print(" CO2: ");
      Serial.print(CO2_CCS811);
      Serial.print("ppm, TVOC: ");
      Serial.print(TVOC_CCS811);
      Serial.println("ppb");
    }
    else{
      Serial.println("ERROR!");

      goto_Deepsleep();
    }
  }
}

void Send_Data(){
  StaticJsonDocument<150> doc;                 
  char output[150];

  doc["MeasurmentUpTime"] = millis();
  doc["BootCount"] = bootCount;
  doc["CO2"] = CO2_CCS811;
  doc["TVOC"] = TVOC_CCS811;
  doc["Temperature"] = String(Temperature_DHT, 2);                              
  doc["Humidity"] = String(Humidity_DHT, 2); 
  // doc["Helligkeit"] = Helligkeit;

  serializeJson(doc, output);                   
  Serial.println(output);

  if (!client.connected()) {                      
    reconnect();                                  
  }
  client.publish(topic, output);
}

void setup_Sensor(){
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    // Wenn bei der initialisierung des Sensors ein fehler auftritt dann geht der ESP in den Tiefschlaf ohne Daten zu senden.
    goto_Deepsleep();
  }
  while(!ccs.available());

  dht.begin(); 

  // pinMode(Helligkeitsensor, INPUT);
}

void connect_to_WLAN(){
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
}

void setup_mqtt(){
  client.setServer(mqtt_server, 1883);  
}

void reconnect() {                                
  while (!client.connected()) {                   
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";             
    clientId += String(random(0xffff), HEX);      
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup(){
  Serial.begin(115200);
  while (!Serial);

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  setup_Sensor();

  connect_to_WLAN();
  setup_mqtt();

  print_measurments();
  Send_Data();
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  goto_Deepsleep();
}

void loop(){}