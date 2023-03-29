#include <TimerOne.h>
#define USE_SERIAL Serial

unsigned char channel_1 = 32; //channel 1 output on pin D4
unsigned char channel_2 = 34;
unsigned char channel_3 = 36;
unsigned char channel_4 = 38;
unsigned char channel_5 = 40; //channel 1 output on pin D4
unsigned char channel_6 = 42;
unsigned char channel_7 = 44;
unsigned char channel_8 = 46;
unsigned char CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8;
unsigned char clock_tick;
unsigned int delay_time = 50;
int lux;
int outVal = 0;

void setup() {
  pinMode(channel_1, OUTPUT);
  pinMode(channel_2, OUTPUT);
  pinMode(channel_3, OUTPUT);
  pinMode(channel_4, OUTPUT);
  pinMode(channel_5, OUTPUT);
  pinMode(channel_6, OUTPUT);
  pinMode(channel_7, OUTPUT);
  pinMode(channel_8, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(2), zero_crosss_int, RISING);
  Timer1.initialize(83);
  Timer1.attachInterrupt(timerIsr);

  USE_SERIAL.begin(9600); 
  USE_SERIAL.println("Dimmer Program is starting...");
  USE_SERIAL.println("Set value");
  
}

void timerIsr()
{
  clock_tick++;

  if (CH1 == clock_tick)  {
    digitalWrite(channel_1, HIGH);
    delayMicroseconds(8.33);
    digitalWrite(channel_1, LOW);
  }
    if (CH2 == clock_tick)  {
    digitalWrite(channel_2, HIGH);
    delayMicroseconds(8.33);
    digitalWrite(channel_2, LOW);
  }
    if (CH3 == clock_tick)  {
    digitalWrite(channel_3, HIGH);
    delayMicroseconds(8.33);
    digitalWrite(channel_3, LOW);
  }
    if (CH4 == clock_tick)  {
    digitalWrite(channel_4, HIGH);
    delayMicroseconds(8.33);
    digitalWrite(channel_4, LOW);
  }
    if (CH5 == clock_tick)  {
    digitalWrite(channel_5, HIGH);
    delayMicroseconds(8.33);
    digitalWrite(channel_5, LOW);
  }
    if (CH6 == clock_tick)  {
    digitalWrite(channel_6, HIGH);
    delayMicroseconds(8.33);
    digitalWrite(channel_6, LOW);
  }
    if (CH7 == clock_tick)  {
    digitalWrite(channel_7, HIGH);
    delayMicroseconds(8.33);
    digitalWrite(channel_7, LOW);
  }
    if (CH8 == clock_tick)  {
    digitalWrite(channel_8, HIGH);
    delayMicroseconds(8.33);
    digitalWrite(channel_8, LOW);
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
    CH2 = 100-outVal;
    CH3 = 100-outVal;
    CH4 = 100-outVal;
    CH5 = 100-outVal;
    CH6 = 100-outVal;
    CH7 = 100-outVal;
    CH8 = 100-outVal;
  }
  
  if (preVal != outVal)
  {
    USE_SERIAL.print("lampValue -> ");
    USE_SERIAL.print(outVal);
    USE_SERIAL.println("%");
  }

  delay(50);
  

}
