#include <TimerOne.h>
#define USE_SERIAL Serial

unsigned char channel_1 = 4; //channel 1 output on pin D4
unsigned char CH1;
unsigned char clock_tick;
unsigned int delay_time = 50;
int lux;
int outVal = 0;

void setup() {
  pinMode(channel_1, OUTPUT);
  attachInterrupt(1, zero_crosss_int, RISING);
  Timer1.initialize(83);
  Timer1.attachInterrupt(timerIsr);

  USE_SERIAL.begin(9600); 
  USE_SERIAL.println("Dimmer Program is starting...");
  USE_SERIAL.println("Set value");
  
}

void timerIsr()
{
  clock_tick++;

  if (CH1 == clock_tick)
  {
    digitalWrite(channel_1, HIGH);
    delayMicroseconds(8.33);
    digitalWrite(channel_1, LOW);
  }
}

void zero_crosss_int(){
  clock_tick = 0;
}

void loop() {
  int preVal = outVal;
  
  if (USE_SERIAL.available())
  {
    int buf = USE_SERIAL.parseInt();
    if (buf != 0) outVal = buf;
    delay(200);
  }
  if (outVal < 100 && outVal > 0) {
    CH1 = 100-outVal;
  }
  
  if (preVal != outVal)
  {
    USE_SERIAL.print("lampValue -> ");
    USE_SERIAL.print(outVal);
    USE_SERIAL.println("%");
  }

  delay(50);
  

}
