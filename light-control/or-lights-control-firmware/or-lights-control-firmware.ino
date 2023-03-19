#include <AccelStepper.h>
#include <TimerOne.h>

//PARAMETERS
#define LIN_STEPS_PER_INCH 200*12.7 //200 steps/rev * 12.7rev/inch (2mm pitch)
#define YAW_STEPS_PER_REV 200*17 //200 steps/rev * 17:1 gearbox
#define PITCH_STEPS_PER_REV 200*100 //200 steps/rev * 100:1 gearbox

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

//ACCELSTEPPER INSTANTIATION -- myStepper(motorInterfaceType, stepPin, dirPin);
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


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
