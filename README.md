# or-lights

/or-lights/light-control contains all firmware and Arduino code for manual operation

/or-lights/zoildef-app contains Python code and additional resource for the touchscreen app

TO SET UP ARDUINO AND HARDWARE:
1) Once system physical system has been assembled and mounted, upload /or-lights/light-control/manual-calibration/manual-calirbation.ino to the Arduino Mega
2) Follow the prompts in serial monitor to complete manual calibration of the lights
3) Repeat step 2 as many times as needed if the lights are noticeably in the wrong position
4) Upload /or-lights/light-control/or-lights-control-firmware/or-lights-control-firmware.ino to the Mega
5) Wait for app to connect and display "CONNECTED" in serial monitor
6) Disconnect debug cable and run wirelessly from app!

TO SET UP APP:
