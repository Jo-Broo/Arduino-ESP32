#include <WiFi.h>
#include <time.h>
#include "Credentials.h"

// NTP-Server und Zeitzone
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600; // Offset für Zeitzone (GMT+1 für Mitteleuropäische Zeit)
const int   daylightOffset_sec = 3600; // Sommerzeit

void setup() {
  Serial.begin(115200);
  
  // Verbinde dich mit dem WLAN
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Verbinde mit WLAN...");
  }
  Serial.println(WiFi.localIP());
  Serial.println("Verbunden mit WLAN");

  // Initialisiere die NTP-Zeit
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void loop() {
  delay(10000); // Alle 10 Sekunden Zeit ausgeben
  printLocalTime();
}

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Fehler beim Abrufen der Zeit");
    return;
  }
  Serial.println(&timeinfo, "%d %m %Y %H:%M:%S");
}
