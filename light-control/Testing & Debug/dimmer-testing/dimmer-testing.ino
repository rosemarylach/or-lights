#include <RBDdimmer.h>//

//#define USE_SERIAL  SerialUSB //Serial for boards whith USB serial port
#define USE_SERIAL  Serial
#define outputPin  3 
#define zerocross  2 // for boards with CHANGEBLE input pins

//dimmerLamp dimmer(outputPin, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
dimmerLamp dimmer(outputPin); //initialase port for dimmer for MEGA, Leonardo, UNO, Arduino M0, Arduino Zero

int outVal = 0;

void setup() {
  USE_SERIAL.begin(9600); 
  dimmer.begin(NORMAL_MODE, ON); //dimmer initialisation: name.begin(MODE, STATE) 
  USE_SERIAL.println("Dimmer Program is starting...");
  USE_SERIAL.println("Set value");
}

void printSpace(int val)
{
  if ((val / 100) == 0) USE_SERIAL.print(" ");
  if ((val / 10) == 0) USE_SERIAL.print(" ");
}

void loop() {
  int preVal = outVal;

  if (USE_SERIAL.available())
  {
    int buf = USE_SERIAL.parseInt();
    if (buf != 0) outVal = buf;
    delay(200);
  }
  dimmer.setPower(outVal); // setPower(0-100%);

  if (preVal != outVal)
  {
    USE_SERIAL.print("lampValue -> ");
    print0ace(dimmer.getPower());
    USE_SERIAL.print(dimmer.getPower());
    USE_SERIAL.println("%");

  }
  delay(50);

}