

#include <ArduinoJson.h>                          // for data to .json conversion
#include <ArduinoJson.hpp>  
#include <WiFi.h>                                 // wifi librarys
#include <PubSubClient.h>                         // mqtt-client librarys
#include <Wire.h>                                 // library for one-wire bus communication
#include "DHT.h"                                  // dht-library
#define DHTTYPE DHT11                             // what is the exact type of dht-Sensor 
#define DHTPIN 4                                  // where is the dht connected with the esp    

// Network-settings
const char* ssid = "FRITZ!Box 7530 WY";           // WiFi-SSID 
const char* password = "09612677397865897502";    // WiFi-Password
const char* mqtt_server = "192.168.178.30";       // IP of MQTT-Broker
const char* topic = "/home/sensors";              // the topic to wich the esp will post
const char* client_id = "Livingroom";             // Name of the mqtt-client  

WiFiClient wifi_client;                           // definition of a WiFi-client Object
PubSubClient client(wifi_client);                 // definition of a mqtt-client Object
DHT dht(DHTPIN, DHTTYPE);                         // definition of a DHT-Sensor Object

long lastMsg = 0;                                 // holds the time of the last send Message
const long intervall = 60000;                     // Measuring Intervall in ms, 60.000ms = measure after 1 min
const int send = 23;                              // pin-Defention for a external LED
const int error = 19;                             // pin-Defention for a external LED

const bool useMQTT = false;                       // this is for debugging purposes of the Temperature Sensor

void setup() {
  pinMode(send, OUTPUT);                          // configure the send and error Pin 
  pinMode(error, OUTPUT);                         // to output-mode so we can drive some Led's

  digitalWrite(send, HIGH);                       // Both Led's will be on during the setup part 
  digitalWrite(error, HIGH);                      // to visualy indicate the setup state

  Serial.begin(115200);                           // start Serial communication for debuging-purposes
  while (!Serial);                                // and wait until it is set up

  if(useMQTT == true)
  {
    Serial.println();                               //
    Serial.print("Connecting to ");                 // Some Debug-Information at the beginning
    Serial.println(ssid);                           //

    WiFi.begin(ssid, password);                     // start of the Wifi-connection

    while (WiFi.status() != WL_CONNECTED) 
    {         
      delay(500);                                   // wait until the connection is established
      Serial.print(".");                            //
    }                                               //

    Serial.println("");                             //
    Serial.println("WiFi connected");               // and print the recievded IP to the Serial-Monitor
    Serial.println("IP address: ");                 //
    Serial.println(WiFi.localIP());                 //

    client.setServer(mqtt_server, 1883);            // connect to the MQTT-Broker on the standardport 
  }

  dht.begin();                                    // startup the Sensor

  digitalWrite(send, LOW);                        // turn off the Led's
  digitalWrite(error, LOW);                       // to visualy indicate the end of the setup state
}

void loop() {

  if (!client.connected() && useMQTT == true) {   // check if the MQTT-client is still connected
    digitalWrite(error, HIGH);                    // if not then turn on the error LED and try to reconnect 
    reconnect();                                  //
    digitalWrite(error, LOW);                     //
  }

  long now = millis();                            // get the current time of the ESP
  if (now - lastMsg > intervall) {                // if the intervall is reached the esp will measure else it will continue without measurment
    
    digitalWrite(send, HIGH);                     // turn on to indicate the start of the measuring/sending process
    StaticJsonDocument<80> doc;                   // create a new Json-DOcument with a bufferlength of 80
    char output[80];                              // create a char Array with the same bufferlength

    lastMsg = now;                                // update the variable for the next intervall
    float temp = dht.readTemperature();           // read the temperature and the humidity from the sensor
    float humidity = dht.readHumidity();          //
    
    doc["id"] = client_id;                        //
    doc["t"] = temp;                              // set the Key|Value pairs in the Json-document
    doc["h"] = humidity;                          //

    serializeJson(doc, output);                   // convert the Json to a char array for transportation
    Serial.println(output);                       // print for Debug
    if(useMQTT == true){
      client.publish(topic, output);              // and Post the Data under the specified Topic
    }
    digitalWrite(send, LOW);                      // and turn the Led off at the end
  }
    
}

void reconnect() {                                // function for the mqtt-reconnection 
  // Loop until we're reconnected
  while (!client.connected()) {                   // this will run until the connection is established again, maybe i will put a counter here :|
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";             // the esp will create a new unique ClientId eachtime it trys to reconnect
    clientId += String(random(0xffff), HEX);      //
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}