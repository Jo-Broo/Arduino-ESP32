#include "DHT.h"
#define DHTTYPE DHT11                             
#define DHTPIN 4 

DHT dht(DHTPIN, DHTTYPE);

long lastMsg = 0; 
const long intervall = 60000; // in ms 60000 = 60s

void setup() {
  Serial.begin(115200);
  dht.begin();  
  // put your setup code here, to run once:

}

void loop() {
  
  long now = millis();                            
  if (now - lastMsg > intervall) {                
    
    lastMsg = now;   
    float temp = dht.readTemperature();           
    float humidity = dht.readHumidity();          

    Serial.println("==========");
    Serial.print("Temperatur (C): ");
    Serial.println(temp);   
    Serial.print("Luftfeuchtigkeit (%): ");
    Serial.println(humidity);  
    Serial.println("==========");    
  }

}
