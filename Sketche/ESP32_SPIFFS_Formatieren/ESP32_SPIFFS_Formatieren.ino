#include "SPIFFS.h"

bool format_SPIFFS = true;


bool isempty = true;

void setup() {
  Serial.begin(115200);

  delay(2000);
  Serial.println("Process Start");

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // list SPIFFS contents
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      isempty = false;
      Serial.print("  DIR : ");
      Serial.println(file.name());
    } else {
      isempty = false;
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }

  if (isempty == true) {
    Serial.println("SPIFFS is empty");
  }

  if (format_SPIFFS == true && isempty == false) {
    Serial.print("Start Formatting...");
    SPIFFS.format();
    Serial.println("Finished");
  }
  Serial.println("Process End");
}

void loop() {
  // put your main code here, to run repeatedly:
}
