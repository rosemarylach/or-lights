#include <AccelStepper.h>

float input_steps;

#define motorInterfaceType 1

AccelStepper testMotor(motorInterfaceType, 31, 30); //15,14

void setup() {
  // put your setup code here, to run once:
  testMotor.setMaxSpeed(1000);
  testMotor.setAcceleration(1000);
  testMotor.setSpeed(1000);
  Serial.begin(9600);
  Serial.println("Setup complete.");

  Serial.print("Enter target ");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    input_steps = Serial.parseFloat();
    Serial.println(input_steps);
    while(Serial.available()){ 
      Serial.read();
    }
    testMotor.moveTo(testMotor.currentPosition() + input_steps);
  }
  if (testMotor.distanceToGo() == 0) { 
    Serial.println("next input:");
  }
  testMotor.run();
}
