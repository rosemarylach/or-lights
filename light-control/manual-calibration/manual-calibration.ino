#include <AccelStepper.h>
#include <MultiStepper.h>


//PARAMETERS
#define LIN_STEPS_PER_INCH 200 * 12.7 / 5       //200 steps/rev * 12.7 rev/in (2mm pitch) / 5 pitch/rev
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

/*
#define YAW_LIM 200
#define FOCUS_LIM -1.5
*/
#define YAW_LIM 200
#define FOCUS_LIM -1.3


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
  Serial.println("\nStarting calibration procedure.");  


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

  //begin one-time manual calibration procedure on user serial input
  Serial.println("Enter '1' when ready to begin automatic focus and yaw calibration:");
  
  //wait for user to input "1"
  while(1){
    if (Serial.available()) {
      if (Serial.parseInt() == 1){
        break;
      }
      else {
        Serial.println("Invalid input. Enter '1' when ready to begin automatic focus and yaw calibration:");
      }
      while(Serial.available()){
          Serial.read();
        }
    }
  }
  Serial.println("Calibrating focus and yaw.");

  //automatically calibrate focus and yaw using hard limits
  calibrateYawFocus();

  Serial.println("Focus and yaw calibrated.");
  delay(2000); //pause for dramatic effect

  Serial.println("Moving yaw 90 degrees for manual pitch calibration.");
  
  //clear serial input
  while(Serial.available()){
    Serial.read();
  }

  //move yaw to avoid unintended contact while calibrating pitch
  float pitchCalPos[4][3] = {{0, 0, -90}, {0, 0, 90}, {0, 0, 90}, {0, 0, -90}};
  moveAllMotors(pitchCalPos);

  //manually calibrate pitch for each panel individually
  for (int i; i<4; i++){
    calibratePitch(i);
  }


  Serial.println("\n-----------------------------------------------------\n\nCALIBRATION COMPLETE");
  
  //return to origin at end of calibration so or-lights-control-firmware can accurately measure zero position on startup
  float zeroPos[4][3] = {{0, 0, 0},{0, 0, 0},{0, 0, 0},{0, 0, 0}};
  moveAllMotors(zeroPos);

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
  targetPos[0] = origin[panel][0] + linInch * LIN_STEPS_PER_INCH;
  targetPos[1] = origin[panel][1] + pitchDeg * PITCH_STEPS_PER_DEG;
  targetPos[2] = origin[panel][2] + yawDeg * YAW_STEPS_PER_DEG;
}


void calibrateYawFocus() {
  /**
  void calibratePanels()


  Calibrates the entire assembly by individually driving each panel to its hard limits, then to the define zero position
  */

  float limitPos[4][3] = {{FOCUS_LIM, 0, YAW_LIM}, {FOCUS_LIM, 0, YAW_LIM}, {FOCUS_LIM, 0, YAW_LIM}, {FOCUS_LIM, 0, YAW_LIM}}; //target position for panels to hit yaw and focus hard limits
  float calZeroPos[4][3] = {{0, 0, -100}, {0, 0, -100}, {0, 0, -100}, {0, 0, -100}}; //target position for panels to move to calibrated yaw/focus zero position


  //Move panels to hard limits regardless of current position by instructing them to cover full range of motion
  moveAllMotors(limitPos);


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
  /*
  void calibratePitch(int panel)

  Runs manual calibration of a single panel by using serial input to command small angle movements 
  
  inputs:
    - panel: integer for panel number to be calibrated
  */

  //initialize variables for automatic pitch calibration
  int calibrated = 0;
  int pitchPos = 0;
  int response;

  //formatting for user input on manual calibration
  Serial.print("\n-----------------------------------------------------\nMANUAL CALIBRATION OF PANEL ");
  Serial.println(panel+1);
  Serial.println("\nEnter '1' to move 10deg. Enter 0 to finish calibration.");
  
  //loop through calibration procedure until calibration is complete
  while(!calibrated) {
    
    //read serial input from user
    if (Serial.available()) {
      response = Serial.parseInt();
      while (Serial.available()) {
        Serial.read();
      }
      Serial.println(response);
      
      //move pitch +10 degrees if commanded
      if (response == 1) {
        pitchPos += 10;

        drivers[panel][1].move(10 * PITCH_STEPS_PER_DEG);
        drivers[panel][1].runToPosition();
        
        Serial.println("Enter '1' to move 10deg. Enter 0 to finish calibration.");
      }

      //return pitch to zero by moving -50 degrees if commanded that hard limit is hit
      else if (response == 0) {
        origin[panel][1] = drivers[panel][1].currentPosition(); //set origin at current position (hard limit)
        drivers[panel][1].moveTo(origin[panel][1] - 50 * PITCH_STEPS_PER_DEG); //move -50 from current position to true origin
        drivers[panel][1].runToPosition();

        origin[panel][1] = drivers[panel][1].currentPosition(); //set origin at current position (true origin)
        
        //end panel calibration
        Serial.print("Calibration complete for panel ");
        Serial.println(panel+1);
        calibrated = 1;  
      }

      //call for input again if not 0 or 1
      else {
        Serial.println("Invalid input. 1 to move further, 0 to finish calibration.");
      }
    }
   
  }
 
}


void loop() {
  // put your main code here, to run repeatedly:


}
