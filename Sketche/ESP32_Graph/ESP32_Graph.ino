#include <DHT22.h>
#include <Wire.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Hilfsvariablen
unsigned long mytime = 0;
const int INTERVAL = 1; // Angabe in s wird im Code in ms umgerechnet
int Interval_ms;
const int DHTPIN = 16;

// Graph
const int Graph_left_offset = 20;
const int Graph_height = display.height() -15;
const int History_length = 80;
const int max_Temp = 30;
const int min_Temp = -10;

DHT22 dht(DHTPIN);

struct Wetter_eintrag{
  float temperature;
  float humidity;
};

Wetter_eintrag eintrag;

int History[History_length];

bool Messungen(){
  eintrag.temperature = dht.getTemperature();
  Rightshift_Array(History,History_length);
  History[0] = int(eintrag.temperature);

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

void Draw_History(){
  display.setCursor(0, 0);     
  display.println(max_Temp);

  display.setCursor(0, (Graph_height-5));     
  display.println(min_Temp);

  int nullinie = map(0,min_Temp,max_Temp,Graph_height-1,1);

  display.setCursor(display.width()-20, (nullinie+2));     
  display.println(String((History_length*INTERVAL))+"s");

  display.drawLine(Graph_left_offset, 0, Graph_left_offset, Graph_height, SSD1306_WHITE);
  display.drawLine(Graph_left_offset, nullinie, display.width()-1, nullinie, SSD1306_WHITE);

  int start_x = Graph_left_offset + 2;
  for(int i=0;i<History_length;i++){
    if(History[i] == -1){continue;}
    int datapoint = map(History[i],min_Temp,max_Temp,Graph_height-1,1);
    display.drawPixel((start_x + i), datapoint, SSD1306_WHITE);
  }

  display.setCursor(0, (Graph_height + 5));     
  display.println(String(eintrag.temperature)+" C|" + String(eintrag.humidity) + " %");
}

void Rightshift_Array(int Array[], int size){

  for(int i=size-1;i>0;i--){
    Array[i] = Array[i-1];
  }

}

void Initialize_Array(int Array[], int size){
  
  for(int i=0;i<size;i++){
    Array[i] = -1;
  }

}

void setup() {  
  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(400000L);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();

  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE); 
  display.cp437(true);   
  
  Initialize_Array(History,History_length);

  Interval_ms = INTERVAL * 1000;
}

void loop() {
  if((millis()-mytime) > Interval_ms){
    Messungen();
    display.clearDisplay();

    Draw_History();

    display.display();
    mytime = millis();
  }
}