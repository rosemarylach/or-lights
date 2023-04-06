#include <AccelStepper.h>
#include <MultiStepper.h>

//PARAMETERS
#define LIN_STEPS_PER_INCH 200 * 12.7        //200 steps/rev * 12.7rev/inch (2mm pitch)
#define YAW_STEPS_PER_DEG 200 * 19.19 * 30 / (12*360)     //200 steps/rev * 19.19:1 gearbox * 30:12 gear ratio
#define PITCH_STEPS_PER_DEG 200 * 99.05 / 360  //200 steps/rev * 99.05:1 gearbox

//PIN CONFIGURATION
#define P1_LIN_STEP 13
#define P1_LIN_DIR 12
#define P1_YAW_STEP 8
#define P1_YAW_DIR 9
#define P1_PITCH_STEP 11
#define P1_PITCH_DIR 10

#define P2_LIN_STEP 14
#define P2_LIN_DIR 15
#define P2_YAW_STEP 22
#define P2_YAW_DIR 23
#define P2_PITCH_STEP 17
#define P2_PITCH_DIR 16

#define P3_LIN_STEP 55
#define P3_LIN_DIR 54
#define P3_YAW_STEP 26
#define P3_YAW_DIR 24
#define P3_PITCH_STEP 30
#define P3_PITCH_DIR 28

#define P4_LIN_STEP 53
#define P4_LIN_DIR 52
#define P4_YAW_STEP 58
#define P4_YAW_DIR 57
#define P4_PITCH_STEP 66
#define P4_PITCH_DIR 65

#define motorInterfaceType 1

//ACCELSTEPPER INSTANTIATION -- myStepper(motorInterfaceType, stepPin, dirPin);
AccelStepper drivers[4][3] = {
  { AccelStepper(motorInterfaceType, P1_LIN_STEP, P1_LIN_DIR),
    AccelStepper(motorInterfaceType, P1_PITCH_STEP, P1_PITCH_DIR),
    AccelStepper(motorInterfaceType, P1_YAW_STEP, P1_YAW_DIR) },
  { AccelStepper(motorInterfaceType, P2_LIN_STEP, P2_LIN_DIR),
    AccelStepper(motorInterfaceType, P2_PITCH_STEP, P2_PITCH_DIR),
    AccelStepper(motorInterfaceType, P2_YAW_STEP, P2_YAW_DIR) },
  { AccelStepper(motorInterfaceType, P3_LIN_STEP, P3_LIN_DIR),
    AccelStepper(motorInterfaceType, P3_PITCH_STEP, P3_PITCH_DIR),
    AccelStepper(motorInterfaceType, P3_YAW_STEP, P3_YAW_DIR) },
  { AccelStepper(motorInterfaceType, P4_LIN_STEP, P4_LIN_DIR),
    AccelStepper(motorInterfaceType, P4_PITCH_STEP, P4_PITCH_DIR),
    AccelStepper(motorInterfaceType, P4_YAW_STEP, P4_YAW_DIR) }
};

MultiStepper panels[4] = {MultiStepper(), MultiStepper(), MultiStepper(), MultiStepper() };

long origin[4][3];
long targetPos[3];

int moving;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting calibration procedure.");  

  //Set stepper speed limits and build MultiSteppers for each panel
  for (int panel = 0; panel < 4; panel++) {
    for (int motor = 0; motor < 3; motor++) { 
      drivers[panel][motor].setMaxSpeed(1000);
      drivers[panel][motor].setSpeed(1000);
      drivers[panel][motor].setAcceleration(100);
      
      panels[panel].addStepper(drivers[panel][motor]);
    }
  }

  //Set initial zero position at arbitrary starting point -- highly recommend running calibration sequence on bootup
  for (int i = 0; i < 4; i++) {    
    for (int j = 0; j < 3; j++) {
      origin[i][j] = drivers[i][j].currentPosition();
    }
  }

  Serial.println("Calibrating focus and yaw.");

  calibrateYawFocus();

  Serial.println("Focus and yaw calibrated.");
  Serial.println("Moving yaw 90 degrees for manual pitch calibration.");

  float pitchCalPos[4][3] = {{0, 0, 90}, {0, 0, 90}, {0, 0, 90}, {0, 0, 90}};
  moveAllMotors(pitchCalPos);

  for (int i; i<4; i++){
    calibratePitch(i);
  }

  Serial.println("CALIBRATION COMPLETE");

}


void setTargetPos(int panel, float linInch, float pitchDeg, float yawDeg) {
  /**
  void setTargetPos(int panel, float linInch, float pitchDeg, float yawDeg)

  Converts target positions in inches/degrees to target positions in steps for a particular panel, storing the result in targetPos

  Inputs:
     int panel - integer corresponding to the panel being driven (note: this is needed for selecting the correct zero position corresponding to this panel)
     float linInch - float representing the distance and direction to set the linear actuator
     float pitchDeg - float representing the angle and direction to set the pitch motor to
     float yawDeg - float representing the angle and direction to set the yaw motor to

  Outputs:
    none
    updates variable targetPos[3] - array of long integers representing the target position in steps for each motor on the panel to move to ({linSteps, pitchSteps, yawSteps})
  */
  targetPos[3] = origin[panel][0] + linInch * LIN_STEPS_PER_INCH;
  targetPos[2] = origin[panel][1] + pitchDeg * PITCH_STEPS_PER_DEG;
  targetPos[1] = origin[panel][2] + yawDeg * YAW_STEPS_PER_DEG;
}

void calibrateYawFocus() {
  /**
  void calibratePanels()

  Calibrates the entire assembly by individually driving each panel to its hard limits, then to the define zero position
  */

  //Individually drive panel to hard limits and set zero position

  float limitPos[4][3] = {{-1.5, 0, 200}, {-1.5, 0, 200}, {-1.5, 0, 200}, {-1.5, 0, 200}}; //target position for panels to hit yaw and focus hard limits
  float calZeroPos[4][3] = {{0, -40, -100}, {0, -40, -100}, {0, -40, -100}, {0, -40, -100}}; //target position for panels to move to calibrated yaw/focus zero position

  //Move panels to hard limits regardless of current position by instructing them to cover full range of motion
  moveAllMotors(calZeroPos);

  for (int i = 0; i < 4; i++) {
    //Set temporary zero
    for (int j = 0; j < 3; j++) {
      origin[i][j] = drivers[i][j].currentPosition();
    }
  }

  moveAllMotors(calZeroPos);

  //Set final zero position
  for (int i = 0; i < 4; i++) {    
    for (int j = 0; j < 3; j++) {
      origin[i][j] = drivers[i][j].currentPosition();
    }
  }
}

void moveAllMotors(float targetVals[4][3]) {
  //set target position for each panel
  for (int i = 0; i<4; i++) {
    setTargetPos(i, targetVals[i][0], -targetVals[i][1], targetVals[i][2]); //invert pitchDeg because of sign convention in app script
      panels[i].moveTo(targetPos);
  }
    
  //raise flag for active motion
  moving = 1;
  
  //move panels until all target positions are reached
  while(moving) {
    panels[0].run(); panels[1].run(); panels[2].run(); panels[3].run();
    
    //lower flag for active motion once target positions reached
    if (!panels[0].run() && !panels[1].run() && !panels[2].run() && !panels[3].run()){
      moving = 0;
    }
  } 
}

void calibratePitch(int panel) {
  int calibrated = 0;
  int yawPos = 0;
  char response;
  Serial.print("MANUAL CALIBRATION OF PANEL ");
  Serial.println(panel);
  Serial.println("\nEnter '1' to move 10deg. Enter 0 to finish calibration.");
  while(!calibrated) {
    if (Serial.available()) {
      response = Serial.read();
      if (response == "1") {
        yawPos += 10;
        drivers[panel][1].moveTo(origin[panel][1] + yawPos * YAW_STEPS_PER_DEG);
        while(drivers[panel][1].run()) {
          drivers[panel][1].run();
        }
        Serial.println("Enter '1' to move 10deg. Enter 0 to finish calibration.");
      }
      else if (response == "0") {
        origin[panel][1] = drivers[panel][1].currentPosition();
        drivers[panel][1].moveTo(origin[panel][1] - 40 * YAW_STEPS_PER_DEG);
        while(drivers[panel][1].run()) {
          drivers[panel][1].run();
        }
        origin[panel][1] = drivers[panel][1].currentPosition();
        Serial.print("Calibration complete for panel");
        Serial.println(panel);
        calibrated = 1;


      }
    }
    
  }
  
}


void loop() {
  // put your main code here, to run repeatedly:

}
