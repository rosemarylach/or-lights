// Include the AccelStepper Library
#include <AccelStepper.h>


float inputRevs = 0;
String input;
int linActDone = 0;
int yawStepperDone = 0;

#define STEPS_PER_REV 200
#define GEARED_STEPS_PER_REV 10280 //needs to be updated with actual value based on motor


// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper linActuator(motorInterfaceType, 49, 48);
AccelStepper yawStepper(motorInterfaceType, 51, 50);
AccelStepper pitchStepper(motorInterfaceType, 53, 52);

void setup() {
  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  linActuator.setMaxSpeed(1000);
  linActuator.setAcceleration(100);
  linActuator.setSpeed(1000);

  yawStepper.setMaxSpeed(1000);
  yawStepper.setAcceleration(1000);
  yawStepper.setSpeed(1000);

  pitchStepper.setMaxSpeed(1000);
  pitchStepper.setAcceleration(1000);
  pitchStepper.setSpeed(1000);
 
  Serial.begin(9600);
  Serial.println("Setup complete.");
}

void loop() {
  // Change direction once the motor reaches target position
  if(Serial.available() && inputRevs == 0){
    inputRevs = Serial.parseFloat();
    //Serial.println(inputRevs);
    //inputRevs = atof(input);
    //Serial.println(myStepper.currentPosition());
    //Serial.println(STEPS_PER_REV * inputRevs);
    linActuator.moveTo(linActuator.currentPosition() + STEPS_PER_REV * inputRevs);
  }
  if (!linActDone && linActuator.distanceToGo() == 0 && inputRevs != 0) {
      Serial.print("Moved linear actuator ");
      Serial.print(inputRevs);
      Serial.println(" revolutions.");
      linActDone = 1;
      yawStepper.moveTo(yawStepper.currentPosition() + GEARED_STEPS_PER_REV * inputRevs);
      
    }

  if (linActDone && !yawStepperDone && yawStepper.distanceToGo() == 0 && inputRevs != 0) {
      Serial.print("Moved yaw stepper ");
      Serial.print(inputRevs);
      Serial.println(" revolutions.");
      yawStepperDone = 1;
      pitchStepper.moveTo(pitchStepper.currentPosition() + GEARED_STEPS_PER_REV * inputRevs);
    }
  
  if (linActDone && yawStepperDone && pitchStepper.distanceToGo() == 0 && inputRevs != 0) {
      Serial.print("Moved pitch stepper ");
      Serial.print(inputRevs);
      Serial.println(" revolutions.");
      inputRevs = 0;
      linActDone = 0;
      yawStepperDone = 0;
      Serial.println("How many revolutions to turn?");
    }    


    
  linActuator.run();
  pitchStepper.run();
  yawStepper.run();
}
