#include <AccelStepper.h>
#include <MultiStepper.h>
#include <TimerOne.h>
#include <SoftwareSerial.h>

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
#define ESP8266 Serial1 //for WiFi
#define DIMMER_DELAY 8.33 //8.33ms delay for 60Hz waveform

//UART PIN INITIALIZATION
const byte rxPin = 2; // Wire this to Tx Pin of ESP8266
const byte txPin = 3; // Wire this to Rx Pin of ESP8266

//WIFI VARIABLE INITIALIZATION
float userTargets[4][3] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
int brightnessIndex = 0;
int calibrate = 0;

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

float zeroPos[4][3] = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};

int moving;

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

  attachInterrupt(digitalPinToInterrupt(2), zero_crosss_int, RISING);

  Timer1.initialize(83);
  Timer1.attachInterrupt(timerIsr);

  //Initialize fan output pins and turn fans on
  pinMode(FAN1_PWM, OUTPUT);
  pinMode(FAN2_PWM, OUTPUT);
  pinMode(FAN3_PWM, OUTPUT);
  
  //Replace with analogWrite() for PWM control of fan speed in future versions
  digitalWrite(FAN1_PWM, HIGH);
  digitalWrite(FAN2_PWM, HIGH);
  digitalWrite(FAN3_PWM, HIGH);

  Serial.begin(9600);
  ESP8266.begin(9600); 
  
  Serial.println("Setup complete.");
  reconnect();

}


void reconnect() {
  String conn_response = "";
  while (conn_response.indexOf("CONNECT") == -1)
  {
    ESP8266.println("AT+CIPSTART=\"TCP\",\"168.5.94.42\",50000");
    delay(2000);
    if (ESP8266.available() > 0)
    {
      conn_response = ESP8266.readStringUntil('\n');
    }
    Serial.println("[ " + conn_response + " ]");
  }
  Serial.println("CONNECTED");  
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
  targetPos[3] = origin[panel][0] + linInch * LIN_STEPS_PER_INCH;
  targetPos[2] = origin[panel][1] + pitchDeg * PITCH_STEPS_PER_DEG;
  targetPos[1] = origin[panel][2] + yawDeg * YAW_STEPS_PER_DEG;
}

void calibratePanels() {
  /**
  void calibratePanels()

  Calibrates the entire assembly by individually driving each panel to its hard limits, then to the define zero position
  */

  //Individually drive panel to hard limits and set zero position
  for (int i = 0; i < 4; i++) {

    //Move panels to hard limits regardless of current position by instructing them to cover full range of motion
    setTargetPos(i, -1.5, 100, 200);
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

int scaleBrightness(int targetIndex) {
  /*
  int scaleBrightness(int targetIndex)

  Scales a target brightness in percent (0-100%) from the user to the corresponding percent for the active brightness range on the panel bulbs

  inputs:
    target - integer from 0-5 corresponding to user-input target brightness percent:
              0 -> 0%
              1 -> 20%
              2 -> 40%
              3 -> 60%
              4 -> 80%
              5 -> 100%

  outputs:
    tickDimVal - integer from 0-100 corresponding to the tick timer at which the digital dimmer outputs should be pulsed (100 - scaled brightness %)
  */

  int realBrightnessVals[] = {1, 37, 45, 55, 65, 75}; //experimentally determined tick values corresponding to target brightness percent

  return 100 - realBrightnessVals[targetIndex];

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

void loop() {
  /**
  //dimVal = scaleBrightness()
  for (int i = 0; i < 4; i++) { 

    //Move panels to hard limits regardless of current position by instructing them to cover full range of motion
    setTargetPos(i, 0.1, 20, 20);
    panels[i].moveTo(targetPos);
  }


  //Run all 4 panels to their zero positions simultaneously
  moving = 1;
  while(moving) {
    panels[0].run();
    panels[1].run();
    panels[2].run();
    panels[3].run();
    if (!panels[0].run() && !panels[1].run() && !panels[2].run() && !panels[3].run()){
      moving = 0;
    } 
  }

    for (int i = 0; i < 4; i++) { 

    //Move panels to hard limits regardless of current position by instructing them to cover full range of motion
    setTargetPos(i, 0, 0, 0);
    panels[i].moveTo(targetPos);
  }
  moving = 1;
  while(moving) {
    panels[0].run();
    panels[1].run();
    panels[2].run();
    panels[3].run();
    if (!panels[0].run() && !panels[1].run() && !panels[2].run() && !panels[3].run()){
      moving = 0;
    } 
  }
  */
  int responseCounter = 0;
  
  if (ESP8266.available() > 0)
  {
    while (ESP8266.available() > 0)
    {
      if (responseCounter == 0)
      {
      
        String response = ESP8266.readStringUntil('\n');

        Serial.println(response);

        /*
         * [[panel1_f, panel1_p, panel1_y]
         *  [panel2_f, panel2_p, panel2_y]
         *  [panel3_f, panel3_p, panel3_y]
         *  [panel4_f, panel4_p, panel4_y]]
          */

        
        if (response.indexOf("+IPD") > -1)
        {
          int b_idx = response.indexOf('b');
          brightnessIndex = response.substring(b_idx+2).toInt();

          int start_idx;
          int end_idx = b_idx;

          String parsedFloatString;
          float parsedFloat;
          

          // for each panel
          for (int p=3; p>=0; p--) {
            // for each userTargets point
            for (int d=2; d>=0; d--) {
              String start = (d==0) ? "f" : (d==1) ? "p" : "y";
              start = start + String(p+1);

              start_idx = response.indexOf(start);
              parsedFloatString = response.substring(start_idx+3, end_idx);
              parsedFloat = parsedFloatString.toFloat();
              
              if (parsedFloat == 0) {
                if (parsedFloatString.indexOf("0.00" < 0)) {
                  parsedFloat = userTargets[p][d];
                } 
              }
              if (d==0) {
                parsedFloat = min(parsedFloat, 1.25);
              }
              if (d==1) {
                parsedFloat = (parsedFloat < 0) ? max(parsedFloat, -50) : min(parsedFloat, 50);
              }
              userTargets[p][d] = parsedFloat;
  
              end_idx = start_idx;
              
            }
          }
          if (calibrate) {
            //calibratePanels();
            calibrate = 0;
          }

          else {
            //set brightness to target
            dimVal = scaleBrightness(brightnessIndex);
            
            //move motors to recieved target position
            moveAllMotors(userTargets);
            /*
            //set individual target positions for each motor
            for (int i = 0; i<4; i++) {
              setTargetPos(i, userTargets[i][0], -userTargets[i][1], userTargets[i][2]); //invert pitchDeg because of sign convention in app script
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
            */

          //set brightness target
        
          }
          
          // just printy for testing purposes - separate to comment out
          Serial.println("focus\tpitch\tyaw\t");
          for (int p=0; p<4; p++) {
            // for each userTargets point
            for (int d=0; d<=2; d++) {
              Serial.print(userTargets[p][d]);
              Serial.print("\t");
            }
            Serial.println();
          }
          
        }
        /*
        else if (response.indexOf("CLOSED") > -1) {
          moveAllMotors(zeroPos);
          reconnect();
        }
        */
      }
    }
    /*
    Serial.println();
    Serial.println("============");
    Serial.println();
    */
  }
  //for interface w rosemary's wifi stuff -- assuming float userTargets 4x3 array of target vals and int 0-5 brightnessIndex
  
  


  delay(delay_time);
}
