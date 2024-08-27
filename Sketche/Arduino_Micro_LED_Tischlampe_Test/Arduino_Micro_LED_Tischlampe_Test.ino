#include <IRremote.h>

const uint8_t warm_ground = 9;
const uint8_t cold_ground = 10;

uint8_t active_pin = cold_ground;
uint8_t Brightness = 100;
bool On = true;

const uint8_t EmpfaengerPin = 7;

void setup() {
  pinMode(warm_ground, OUTPUT);
  pinMode(cold_ground, OUTPUT);
  IrReceiver.begin(EmpfaengerPin);
  Serial.begin(9600);
}

void loop() {
  analogWrite(active_pin, On ? map(Brightness, 0, 100, 0, 255) : 0);

  if (IrReceiver.decode()) {
    delay(200);
    IrReceiver.resume();

    uint8_t command = IrReceiver.decodedIRData.command;
    Serial.println(command);
    if (command > 0 && command < 95) {
      
      switch (command) {
        case 64:
          On = !On;
          break;
        case 68:
          analogWrite(active_pin, 0);
          active_pin = (active_pin == warm_ground) ? cold_ground : warm_ground;
          break;
        case 72:
          Brightness -= 10;
          break;
        case 76:
          Brightness += 10;
          break;
      }
      Brightness = ((Brightness % 101) + 101) % 101;
    }
  }
  else{
    delay(200);
  }
}
