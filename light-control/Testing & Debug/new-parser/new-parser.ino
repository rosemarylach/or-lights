#include <SoftwareSerial.h>

const byte rxPin = 2; // Wire this to Tx Pin of ESP8266
const byte txPin = 3; // Wire this to Rx Pin of ESP8266
 
// We'll use a software serial interface to connect to ESP8266
//SoftwareSerial ESP8266 (rxPin, txPin); // for Uno
#define ESP8266 Serial1 // for Mega
 
void setup() {
  Serial.begin(9600);
  ESP8266.begin(9600); 

  reconnect();
}

// 4 panels, 3 data points per (focus, pitch, yaw)
float data[4][3];
int b;
 
void loop() {

  int responseCounter = 0;
  
  if (ESP8266.available() > 0)
  {
    while (ESP8266.available() > 0)
    {
      if (responseCounter == 0)
      {
      
        String response = ESP8266.readStringUntil('\n');

        /*
         * [[panel1_f, panel1_p, panel1_y]
         *  [panel2_f, panel2_p, panel2_y]
         *  [panel3_f, panel3_p, panel3_y]
         *  [panel4_f, panel4_p, panel4_y]]
          */

        if (response.indexOf("+IPD") > -1)
        {
          int b_idx = response.indexOf('b');
          b = response.substring(b_idx+2).toInt();

          int start_idx;
          int end_idx = b_idx;


          // for each panel
          for (int p=3; p>=0; p--) {
            // for each data point
            for (int d=2; d>=0; d--) {
              String start = (d==0) ? "f" : (d==1) ? "p" : "y";
              start = start + String(p);

              start_idx = response.indexOf(start);
              data[p][d] = response.substring(start_idx+3, end_idx).toFloat();

              end_idx = start_idx;
              
            }
          }

          // just printy for testing purposes - separate to comment out
          Serial.println("focus\tpitch\tyaw\t");
          for (int p=0; p<4; p++) {
            // for each data point
            for (int d=0; d<=2; d++) {
              Serial.print(data[p][d]);
              Serial.print("\t");
            }
            Serial.println();
          }
        }
      }
    }
    Serial.println();
    Serial.println("============");
    Serial.println();
  }
 
  
}

void reconnect() {
  String conn_response = "";
  while (conn_response.indexOf("CONNECT") == -1)
  {
    ESP8266.println("AT+CIPSTART=\"TCP\",\"168.4.178.23\",50000");
    delay(2000);
    if (ESP8266.available() > 0)
    {
      conn_response = ESP8266.readStringUntil('\n');
    }
    Serial.println("[ " + conn_response + " ]");
  }
  Serial.println("CONNECTED");  
}
