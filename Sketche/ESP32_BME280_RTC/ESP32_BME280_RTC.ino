// Enstanden am 17.08.2024 von 14:00 bis 17:00
//              18.08.2024 von 8:00 bis 14:00

// SDA = 21
// SCL = 22

// === Librarys ===
#include <Seeed_BME280.h> // BME280 Sensor

#include <RtcDS1302.h> // Real-Time-Clock Modul (DS1302)

#include <Adafruit_SSD1306.h> // für das LCD Display
#include <Wire.h>

#include <sqlite3.h> // für die SQLite Datenbank

#include <SPI.h>  // ===
#include <FS.h>   // für die Kommunikation mit der SD-Karte
#include "SD.h"   // ===
// ===

// === Variablen und Konstanten ===
// LCD Einstellungen
#define SCREEN_WIDTH 128 // OLED display Breite in Pixeln
#define SCREEN_HEIGHT 64 // OLED display Höhe in Pixeln
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RTC-Modul Einstellungen
ThreeWire myWire(25, 27, 32); // DATA, CLOCK, RESET
RtcDS1302<ThreeWire> Rtc(myWire);
RtcDateTime now;

// SQLite EInstellungen
sqlite3 *db1;
int rc;
char *zErrMsg = 0;

// BME Einstellungen
BME280 bme280; // Sensor
// Globale Variablen für die Sensorwerte
float temperature;
float pressure;
uint32_t humidity;

// Flags zur Synchronisierung der Daten
volatile bool dataReady = false; // zeigt dem aktualisierungsthread wann neue Daten bereit stehen
// ===

// === SQLite Methoden ===
const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}

int openDb(const char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
       Serial.printf("Opened database successfully\n");
   }
   return rc;
}

int db_exec(sqlite3 *db, const char *sql) {
   Serial.println(sql);
   long start = micros();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   Serial.print(F("Time taken:"));
   Serial.println(micros()-start);
   return rc;
}
// ===

// Setup Methoden
void SetupRTC()
{
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  Rtc.SetDateTime(compiled);
  
  // Wenn das RTC Modul über eine Stunde von der compilierten Zeit abweicht dann wird die Zeit aktualisiert
  if(abs(compiled.Hour() - now.Hour()) > 1)
  {
    Serial.println("RTC was over an Hour off! (Updating DateTime)");
    Rtc.SetDateTime(compiled);
    delay(250);
  }
}

void SetupSensors() {
  // es wird nach einem bme280 gesucht
  if (!bme280.init()) {
      Serial.println("Fehler beim Initialisieren des BME280 Sensors!");
      for(;;); // bei einem fehler geht der esp in einen unendlichen loop
  }
}

void SetupLCD(){
  Wire.begin();
  Wire.setClock(400000L);

  // es wird nach einem Display gesucht
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Fehler beim Initialisieren des SSD1306 Sensors!"));
    for(;;); // bei einem fehler geht der esp in einen unendlichen loop
  }
  display.clearDisplay();
  display.display();

  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE); 
  display.cp437(true); 
}
// ===

// LCD Methoden
// eine Methode zum einfachen schreiben auf das Display
void PrintToLCD(String string, int x, int y){
  display.setCursor(x, y);  
  display.println(string);
}

// Task auf Kern 0
unsigned long lastMillis = 0;
unsigned long interval = 60000; // 1000 = 1s
void MeasureData(void *pvParameters) {
  for (;;) {
    unsigned long currentMillis = millis();     
    if (currentMillis - lastMillis >= interval) 
    {
      // Sensordaten erfassen 
      temperature = bme280.getTemperature();
      pressure = bme280.getPressure();
      humidity = bme280.getHumidity();
      now = Rtc.GetDateTime();

      // Daten als bereit kennzeichnen
      dataReady = true;
      lastMillis = currentMillis;     
    }
    
    // Verzögerung, um die CPU nicht zu blockieren 
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Verzögerung von 1 Sekunde
  }
}

// Task auf Kern 1
void PrintData(void *pvParameters) {
  for (;;) {
    // Wenn Daten bereit sind, ausgeben
    if (dataReady) {

      // hier wird die sql abfrage erstellt
      String dt = String(now.Day()) + "." + String(now.Month()) + "." + String(now.Year()) + " " + String(now.Hour()) + ":" + String(now.Minute()) + ":" + String(now.Second());
      String sql = "Insert into sensor_data(Temperatur,Humidity,Pressure,DateTime,RTC_Confident) Values(" + String(temperature) + "," + String(humidity) +"," + String(pressure / 100000.0) +",'" + dt + "'," + String(Rtc.IsDateTimeValid()) + ");";

      rc = db_exec(db1, sql.c_str());
      if (rc != SQLITE_OK) {
          sqlite3_close(db1);
          for(;;);
      }

      display.clearDisplay();
      PrintToLCD(dt, 0, 0);
      PrintToLCD("T (C): " + String(temperature), 0, 10);
      PrintToLCD("P (bar): " + String((pressure / 100000.0)), 0, 20);
      PrintToLCD("H (%): " + String(humidity), 0, 30);
      PrintToLCD("RC (SQLite): " + String(rc), 0, 40);
      display.display();

      // Daten auf "nicht bereit" setzen
      dataReady = false;
  }

  // Verzögerung, um die CPU nicht zu blockieren 
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Verzögerung von 1 Sekunde
  }
}

void setup() {
    Serial.begin(115200); // Die  Serielle Verbindung wird gestartet 

    SPI.begin(); // Das Serial Peripheral Interface wird gestartet
    if (!SD.begin())  // Die verbindung zur SD Karte wird aufgebaut
    {
      Serial.println("Card Mount Failed"); // Wenn das fehlschlägt dann geht der esp in einen unendlichen loop
      for(;;);
    }

    sqlite3_initialize(); // sqlite wird initialisiert

    if (openDb("/sd/Sensor.sq3", &db1)) // es wird versucht eine existierende Datenbank zu öffnen sonst wird eine neue angelegt
    {
      Serial.println("SQLite Fehler"); // wenn es hier fehler gibt dann geht der esp in einen unendlichen loop
      for(;;);
    }
    // hier wird eine Tabelle angelegt sofern sie nicht schon existiert
    rc = db_exec(db1, "CREATE TABLE IF NOT EXISTS sensor_data (id INTEGER PRIMARY KEY,Temperatur REAL,Humidity INTEGER,Pressure REAL,DateTime DATETIME,RTC_Confident INTEGER);");
    if (rc != SQLITE_OK) {
        sqlite3_close(db1); // // wenn es hier fehler gibt dann geht der esp in einen unendlichen loop
        for(;;);
    }

    // diverse Setup Methoden
    SetupRTC(); 
    SetupSensors();
    SetupLCD();
    display.clearDisplay();
    display.display();

    // Erstellen der Aufgaben und Zuweisen zu den Kernen
    xTaskCreatePinnedToCore(
        MeasureData,      // Task-Funktion
        "MeasureData",    // Name des Tasks
        10000,            // Stack-Größe
        NULL,             // Task-Parameter
        1,                // Priorität
        NULL,             // Task-Handle
        0);               // Kern 0

    xTaskCreatePinnedToCore(
        PrintData,        // Task-Funktion
        "PrintData",      // Name des Tasks
        10000,            // Stack-Größe
        NULL,             // Task-Parameter
        1,                // Priorität
        NULL,             // Task-Handle
        1);               // Kern 1
}

void loop() {} // wird nicht benötigt da es ja die Tasks gibt