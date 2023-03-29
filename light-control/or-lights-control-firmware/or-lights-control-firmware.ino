#include <AccelStepper.h>
#include <MultiStepper.h>
#include <TimerOne.h>

//PARAMETERS
#define LIN_STEPS_PER_INCH 200 * 12.7        //200 steps/rev * 12.7rev/inch (2mm pitch)
#define YAW_STEPS_PER_DEG 200 * 17 / 360     //200 steps/rev * 17:1 gearbox
#define PITCH_STEPS_PER_DEG 200 * 100 / 360  //200 steps/rev * 100:1 gearbox

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

#define DIM_CH1 32
#define DIM_CH2 34
#define DIM_CH3 36
#define DIM_CH4 38
#define DIM_CH5 40
#define DIM_CH6 42
#define DIM_CH7 48
#define DIM_CH8 50

#define DIM_SYNC 2

#define FAN1_PWM 44
#define FAN1_TACH 35
#define FAN2_PWM 45
#define FAN2_TACH 49
#define FAN3_PWM 6
#define FAN3_TACH 7

//OTHER
#define motorInterfaceType 1
#define USE_SERIAL Serial
#define DIMMER_DELAY 8.33 //8.33ms delay for 60Hz waveform

//DIMMER VARIABLE INITIALIZATION
unsigned char dimVal;
unsigned char clock_tick;
unsigned int delay_time = 50;
int lux;
int outVal = 0;

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

/**
AccelStepper P1Lin(motorInterfaceType, P1_LIN_STEP, P1_LIN_DIR);
AccelStepper P1Pitch(motorInterfaceType, P1_PITCH_STEP, P1_PITCH_DIR);
AccelStepper P1Yaw(motorInterfaceType, P1_YAW_STEP, P1_YAW_DIR);

AccelStepper P2Lin(motorInterfaceType, P2_LIN_STEP, P2_LIN_DIR);
AccelStepper P2Pitch(motorInterfaceType, P2_PITCH_STEP, P2_PITCH_DIR);
AccelStepper P2Yaw(motorInterfaceType, P2_YAW_STEP, P2_YAW_DIR);

AccelStepper P3Lin(motorInterfaceType, P3_LIN_STEP, P3_LIN_DIR);
AccelStepper P3Pitch(motorInterfaceType, P3_PITCH_STEP, P3_PITCH_DIR);
AccelStepper P3Yaw(motorInterfaceType, P3_YAW_STEP, P3_YAW_DIR);

AccelStepper P4Lin(motorInterfaceType, P4_LIN_STEP, P4_LIN_DIR);
AccelStepper P4Pitch(motorInterfaceType, P4_PITCH_STEP, P4_PITCH_DIR);
AccelStepper P4Yaw(motorInterfaceType, P4_YAW_STEP, P4_YAW_DIR);
*/

void setup() {

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

  //Configure dimmer output and input pins
  pinMode(DIM_CH1, OUTPUT);
  pinMode(DIM_CH2, OUTPUT);
  pinMode(DIM_CH3, OUTPUT);
  pinMode(DIM_CH4, OUTPUT);
  pinMode(DIM_CH5, OUTPUT);
  pinMode(DIM_CH6, OUTPUT);
  pinMode(DIM_CH7, OUTPUT);
  pinMode(DIM_CH8, OUTPUT);

  attachInterrupt(digitalPinToINterrupt(2), zero_crosss_int, RISING);

  Timer1.initialize(83);
  Timer1.attachInterrupt(timerIsr);

}

void timerIsr(){
  /**
  void timerIsr()

  Timer interrupt service routing for managing dimmer output values.
  Called when interrupt is triggered on Timer1 by 

  */
  clock_tick ++;

  if (dimVal == clock_tick) {
    digitalWrite(DIM_CH1, HIGH);
    digitalWrite(DIM_CH2, HIGH);
    digitalWrite(DIM_CH3, HIGH);
    digitalWrite(DIM_CH4, HIGH);
    digitalWrite(DIM_CH5, HIGH);
    digitalWrite(DIM_CH6, HIGH);
    digitalWrite(DIM_CH7, HIGH);
    digitalWrite(DIM_CH8, HIGH);

    delayMicroseconds(DIMMER_DELAY);
    
    digitalWrite(DIM_CH1, HIGH);
    digitalWrite(DIM_CH2, HIGH);
    digitalWrite(DIM_CH3, HIGH);
    digitalWrite(DIM_CH4, HIGH);
    digitalWrite(DIM_CH5, HIGH);
    digitalWrite(DIM_CH6, HIGH);
    digitalWrite(DIM_CH7, HIGH);
    digitalWrite(DIM_CH8, HIGH);
  }
  
}

void zero_crosss_int(){
  /**
  void zero_crosss_int()

  Resets clock tick when interrupt triggered by ZC/SYNC pin from dimmer
  */
  clock_tick = 0;
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
  targetPos[1] = origin[panel][0] + linInch * LIN_STEPS_PER_INCH;
  targetPos[2] = origin[panel][1] + pitchDeg * PITCH_STEPS_PER_DEG;
  targetPos[3] = origin[panel ][2] + yawDeg * YAW_STEPS_PER_DEG;
}

void calibrate() {
  /**
  void calibrate()

  Calibrates the entire assembly by individually driving each panel to its hard limits, then to the define zero position
  */


  //Individually drive panel to hard limits and set zero position
  for (int i = 0; i < 4; i++) {

    //Move panels to hard limits regardless of current position by instructing them to cover full range of motion
    setTargetPos(i, -1.5, 80, 200);
    panels[i].moveTo(targetPos);
    panels[i].runSpeedToPosition();
    
    //Set temporary zero
    for (int j = 0; j < 3; j++) {
      origin[i][j] = drivers[i][j].currentPosition();
    }

    //Set panels to move from hard limit position to the calibrated zero position
    setTargetPos(i, 0, -40, -100);
    panels[i].moveTo(targetPos);
  }

  //Run all 4 panels to their zero positions simultaneously
  while(panels[0].run() || panels[1].run() || panels[2].run() || panels[3].run()) {}

  //Set final zero position
  for (int i = 0; i < 4; i++) {    
    for (int j = 0; j < 3; j++) {
      origin[i][j] = drivers[i][j].currentPosition();
    }
  }
}

void loop() {

  delay(delay_time);
}
