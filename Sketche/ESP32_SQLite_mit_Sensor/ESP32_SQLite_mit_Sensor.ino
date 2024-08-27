#include <sqlite3.h>
#include "SPIFFS.h"
#include "DHT.h"
#define DHTTYPE DHT11                             
#define DHTPIN 4      

#define FORMAT_SPIFFS_IF_FAILED true

const char* DatabasePath = "/spiffs/Sensor.sq3";
const char* Tablename = "sensor_data";

DHT dht(DHTPIN, DHTTYPE);
long lastMsg = 0; 
const long intervall = 60000; // in ms 60000 = 60s   
const int send = 23;
const int error = 19; 

bool ListSPIFFSContents() {
  bool isempty = true;

  // Öffne das Verzeichnis
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("- Fehler beim Öffnen des Verzeichnisses");
    return false; // Rückgabe von false bei einem Fehler
  }
  if (!root.isDirectory()) {
    Serial.println(" - Kein Verzeichnis");
    return false; // Rückgabe von false bei einem Fehler
  }

  // Durchlaufe die Dateien im Verzeichnis
  File file = root.openNextFile();
  while (file) {
    isempty = false;
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file.close(); // Schließe die Datei
    file = root.openNextFile();
  }

  // Rückgabe, ob das Verzeichnis leer ist oder nicht
  if (isempty) {
    Serial.println("Verzeichnis ist leer.");
  }

  return isempty;
}

const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName) {
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}

int db_open(const char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
       Serial.printf("Opened database successfully\n");
   }
   return rc;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql) {
   Serial.println(sql);
   long start = micros();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       digitalWrite(error, HIGH);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   Serial.print(F("Time taken: "));
   Serial.println(micros()-start);
   return rc;
}

sqlite3 *db;
int rc;

void setup() {
  Serial.begin(115200);
  Serial.println("Begin of Setup");

  pinMode(send, OUTPUT);    
  digitalWrite(send, HIGH);                        
  pinMode(error, OUTPUT);       
  
  // SPIFFS
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) 
  {
    Serial.println("Failed to mount file system");
    return;
  }

  ListSPIFFSContents();
  // ===

  sqlite3_initialize();
  db_open(DatabasePath, &db);

  // SQL-Befehl zum Erstellen einer Tabelle
  String sqlCreateTable = "CREATE TABLE IF NOT EXISTS " + String(Tablename) + "(id INTEGER PRIMARY KEY,Temperatur REAL,Humidity INTEGER,Intervall INTEGER);";
  rc = db_exec(db, sqlCreateTable.c_str());
  if (rc != SQLITE_OK) {
    sqlite3_close(db);
    return;
  }
  // Einfügen einer Platzhalter Zeile zum abtrennen verschiedener Messzyklen
  String sqlInsertPlaceholder = "INSERT INTO " + String(Tablename) + "(Temperatur ,Humidity ,Intervall) VALUES(-1.00, -1, -1);";
  rc = db_exec(db, sqlInsertPlaceholder.c_str());
  if (rc != SQLITE_OK) {
    sqlite3_close(db);
    return;
  }
  // ===

  dht.begin();  
  digitalWrite(send, LOW);      
  Serial.println("End of Setup");
}

void loop() {
  long now = millis();                            
  if (now - lastMsg > intervall) {                
    
    digitalWrite(send, HIGH);                     
    lastMsg = now;                                
    float temp = dht.readTemperature();           
    float humidity = dht.readHumidity();          

    Serial.println("==========");
    Serial.print("Temperatur (C): ");
    Serial.println(temp);   
    Serial.print("Luftfeuchtigkeit (%): ");
    Serial.println(humidity);    

    String sqlInsert = "INSERT INTO " + String(Tablename) + "(Temperatur ,Humidity ,Intervall) VALUES("+ temp + ", " + humidity + ", " + intervall +");";
    rc = db_exec(db, sqlInsert.c_str());
    if (rc != SQLITE_OK) {
      sqlite3_close(db);
      return;
    }
    Serial.println("==========");
    digitalWrite(send, LOW);                      
  }
}