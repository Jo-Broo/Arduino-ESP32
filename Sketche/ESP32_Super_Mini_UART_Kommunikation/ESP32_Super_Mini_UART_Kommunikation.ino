// Diese Skizze empfängt eine Nachricht über den Seriellen Monitor
// und sendet "Welt" zurück, wenn "Hallo" empfangen wird.

void setup() {
  // Initialisiere die serielle Kommunikation mit 115200 Baud
  Serial.begin(115200);

  // Warte, bis die serielle Verbindung hergestellt ist
  while (!Serial) {
    ; // Warten
  }

  Serial.println("ESP32 bereit. Geben Sie 'Hallo' ein und drücken Sie Enter.");
}

void loop() {
  // Überprüfen, ob Daten verfügbar sind
  if (Serial.available() > 0) {
    // Lese den eingehenden String
    String input = Serial.readStringUntil('\n');

    // Entferne Leerzeichen am Anfang und Ende des Strings
    input.trim();

    // Überprüfe, ob der eingegebene Text "Hallo" ist
    if (input == "Hallo") {
      // Sende "Welt" zurück
      Serial.println("Welt");
    } else {
      // Wenn etwas anderes eingegeben wurde, informiere den Benutzer
      Serial.println("Bitte 'Hallo' eingeben.");
    }
  }
}
