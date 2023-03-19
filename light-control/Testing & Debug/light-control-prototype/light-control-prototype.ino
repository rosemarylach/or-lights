// Include the AccelStepper Library
#include <AccelStepper.h>

//int linActDone = 0;
//int yawStepperDone = 0;

int i = 0;
char inputOptions[3][13] = {"pitch angle:", "yaw angle:", "focus height:"};
float params[3];
float origin[3];
int moving = 0;

int pitchSteps, yawSteps, linSteps;

#define STEPS_PER_REV 200*27
#define PITCH_STEPS_PER_REV 200*27
#define YAW_STEPS_PER_REV 200*27

//#define STEPS_PER_REV 200
//#define PITCH_STEPS_PER_REV 10280 //gearbox ratio + 200 steps per rev
//#define YAW_STEPS_PER_REV 51400 //50:10 design ratio + gearbox ratio + 200 steps per rev


// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
//AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);
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

  origin[0] = pitchStepper.currentPosition();
  origin[1] = yawStepper.currentPosition();
  origin[2] = linActuator.currentPosition();
 
  Serial.begin(9600);
  Serial.println("Setup complete.");

  i = 0;
  Serial.print("Enter target ");
  Serial.println(inputOptions[i]);
}

void loop() {
  // Change direction once the motor reaches target position
  if(Serial.available() && moving == 0){
    params[i] = Serial.parseFloat();
    Serial.println(params[i]);
    while(Serial.available()){ 
      Serial.read();
    }
    if (i==2) {
      i = 0;
      moving = 1;

      pitchSteps = params[0] * PITCH_STEPS_PER_REV / 360;
      yawSteps = params[1] * YAW_STEPS_PER_REV / 360;
      

      //CHANGE THIS FOR REAL MODEL THIS IS FOR TESTING MOTOR NOT LIN ACT
      linSteps = params[2] * STEPS_PER_REV / 360;
      //linSteps = params[2] * 12.7 * STEPS_PER_REV; //2mm pitch = 12.7 revs/in

      pitchStepper.moveTo(origin[0] + pitchSteps);
      yawStepper.moveTo(origin[1] + yawSteps);
      linActuator.moveTo(origin[2] + linSteps);
    }
    else {
      i++;
      Serial.print("Enter target ");
      Serial.println(inputOptions[i]);
    } 
  }
  
  if (moving) {
    if (linActuator.distanceToGo() == 0 && yawStepper.distanceToGo() == 0 && pitchStepper.distanceToGo() == 0) {
      moving = 0;

      i = 0;
      Serial.println("\nMOVED TO TARGET POSITION\n");
      Serial.println("-----------------------------------------");
      Serial.println("\nDEFINE NEXT TARGET POSITION\n");
      Serial.print("Enter target ");
      Serial.println(inputOptions[i]);
    }
  }
  /**
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
    **/


    
  linActuator.run();
  pitchStepper.run();
  yawStepper.run();
}
