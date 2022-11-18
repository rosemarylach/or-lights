// Include the AccelStepper Library
#include <AccelStepper.h>

// Define pin connections
const int dirPin = 50;
const int stepPin = 52;

float inputRevs = 0;
String input;

#define STEPS_PER_REV 200


// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);


void setup() {
  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  myStepper.setMaxSpeed(1000);
  myStepper.setAcceleration(100);
  myStepper.setSpeed(1000);
  Serial.begin(9600);
}

void loop() {
  // Change direction once the motor reaches target position
  if(Serial.available() && inputRevs == 0){
    inputRevs = Serial.parseFloat();
    //Serial.println(inputRevs);
    //inputRevs = atof(input);
    //Serial.println(myStepper.currentPosition());
    //Serial.println(STEPS_PER_REV * inputRevs);
    myStepper.moveTo(myStepper.currentPosition() + STEPS_PER_REV * inputRevs);
  }
  if (myStepper.distanceToGo() == 0 && inputRevs != 0) {
      Serial.print("Moved ");
      Serial.print(inputRevs);
      Serial.println(" revolutions.");
      inputRevs = 0;
      Serial.println("How many revolutions to turn?");
    }
    
  myStepper.run();
}
