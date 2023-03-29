#include <TimerOne.h>

unsigned char channel_1 = 4;  // Output to Opto Triac pin, channel 1
unsigned char channel_2 = 5;  // Output to Opto Triac pin, channel 2
unsigned char channel_3 = 6;  // Output to Opto Triac pin, channel 3
unsigned char channel_4 = 7;  // Output to Opto Triac pin, channel 4
unsigned char channel_5 = 8;  // Output to Opto Triac pin, channel 5
unsigned char channel_6 = 9;  // Output to Opto Triac pin, channel 6
unsigned char channel_7 = 10; // Output to Opto Triac pin, channel 7
unsigned char channel_8 = 11; // Output to Opto Triac pin, channel 8
unsigned char CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8;
unsigned char CHANNEL_SELECT;
unsigned char i = 0;
unsigned char clock_tick; // variable for Timer1
unsigned int delay_time = 50;
unsigned int delay_time2 = 100;
unsigned char low = 75; // luce massima
unsigned char high = 55; // luce minima
unsigned char off = 95; // totalmente accesa
int lux = 25; // limite luce minima
int lux2 = 75; // limite luce massima

unsigned char CH[] = {CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8};

void setup() {

  //CH1=CH2=CH3=CH4=CH5=CH6=CH7=CH8=high;

  pinMode(channel_1, OUTPUT);// Set AC Load pin as output
  
  pinMode(channel_2, OUTPUT);// Set AC Load pin as output
  pinMode(channel_3, OUTPUT);// Set AC Load pin as output
  pinMode(channel_4, OUTPUT);// Set AC Load pin as output
  pinMode(channel_5, OUTPUT);// Set AC Load pin as output
  pinMode(channel_6, OUTPUT);// Set AC Load pin as output
  pinMode(channel_7, OUTPUT);// Set AC Load pin as output
  pinMode(channel_8, OUTPUT);// Set AC Load pin as output
  
  attachInterrupt(digitalPinToInterrupt(3), zero_crosss_int, RISING);
  Timer1.initialize(83); // set a timer of length 100 microseconds for 50Hz or 83 microseconds for 60Hz;
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here

}

void timerIsr()
{
  clock_tick++;

  if (CH1 == clock_tick) {
    digitalWrite(channel_1, HIGH); // triac firing
    delayMicroseconds(8.33); // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(channel_1, LOW); // triac Off
  }

  

/*
  if (CH2 == clock_tick)
  {
    digitalWrite(channel_2, HIGH); // triac firing
    delayMicroseconds(5); // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(channel_2, LOW); // triac Off
  }

  if (CH3 == clock_tick)
  {
    digitalWrite(channel_3, HIGH); // triac firing
    delayMicroseconds(5); // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(channel_3, LOW); // triac Off
  }

  if (CH4 == clock_tick)
  {
    digitalWrite(channel_4, HIGH); // triac firing
    delayMicroseconds(5); // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(channel_4, LOW); // triac Off
  }

  if (CH5 == clock_tick)
  {
    digitalWrite(channel_5, HIGH); // triac firing
    delayMicroseconds(5); // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(channel_5, LOW); // triac Off
  }

  if (CH6 == clock_tick)
  {
    digitalWrite(channel_6, HIGH); // triac firing
    delayMicroseconds(5); // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(channel_6, LOW); // triac Off
  }

  if (CH7 == clock_tick)
  {
    digitalWrite(channel_7, HIGH); // triac firing
    delayMicroseconds(5); // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(channel_7, LOW); // triac Off
  }

  if (CH8 == clock_tick)
  {
    digitalWrite(channel_8, HIGH); // triac firing
    delayMicroseconds(5); // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(channel_8, LOW); // triac Off
  }
  */


}



void zero_crosss_int() // function to be fired at the zero crossing to dim the light
{
  // Every zerocrossing interrupt: For 50Hz (1/2 Cycle) => 10ms ; For 60Hz (1/2 Cycle) => 8.33ms
  // 10ms=10000us , 8.33ms=8330us

  clock_tick = 0;
}



void loop() {


  // CICLO 3 ********************************************************


  for (i = lux; i < lux2; i++)
  {
    CH1 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH1 = i;
    delay(delay_time);
  }


/*
  for (i = lux; i < lux2; i++)
  {
    CH2 = i;
    delay(delay_time);

  }

  for (i = lux2; i > lux; i--)
  {
    CH2 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH3 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH3 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH4 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH4 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH5 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH5 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH6 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH6 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH7 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH7 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH8 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH8 = i;
    delay(delay_time);
  }
  */



  // ****************************************************************


  //  CICLO 4 *******************************************************


/*

  for (i = lux; i < lux2; i++)
  {
    CH7 = i;
    delay(delay_time);

  }

  for (i = lux2; i > lux; i--)
  {
    CH7 = i;
    delay(delay_time);
  }




  for (i = lux2; i < 75; i++)
  {
    CH6 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH6 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH5 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH5 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH4 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH4 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH3 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH3 = i;
    delay(delay_time);
  }



  for (i = lux; i < lux2; i++)
  {
    CH2 = i;
    delay(delay_time);
  }

  for (i = lux2; i > lux; i--)
  {
    CH2 = i;
    delay(delay_time);
  }
  */



  // ****************************************************************



}