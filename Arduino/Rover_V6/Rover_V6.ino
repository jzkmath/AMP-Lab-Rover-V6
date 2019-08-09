
/*************************************************************
  LOW COST ROVER V6 - ARDUINO CODE
  By: Jonathan Kayne, Virginia Tech ECE Class of 2021
  jzkmath@vt.edu
  Based off the Low Cost Rover By Will Gerhard

  Uses the Arduino BLE-Nano board from eMakeFun to communicate with
  a smartphone via Bluetooth Low Energy (BLE, or Bluetooth 4.0).
  This requires the use of an App called Blynk (by Sparkfun)
  which is available on iOS and Android.

  Make sure to edit the auth token below so that the
  rover is unique!

  Also, use the setup sketch to configure your device!
   
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc

  Blynk library is licensed under MIT license

 *************************************************************

  You can receive x and y coords for joystick movement within App.

  App project setup:
    HARDWARE MODEL: Arduino Nano
    CONNECTION TYPE: BLE
    
    - BLE widget
    - Two Axis Joystick on V1 in MERGE output mode.
    - Range set from -127 to 127 for both x and y.
   
 *************************************************************/
 
#include <BlynkSimpleSerialBLE.h>
#include <Wire.h>

// ***************************************************
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Max speed for the rover.
// any value between 0 and 255 is acceptable.
// only adjust if you want to make the rover slower!
#define MAX_SPEED 255

// ***************************************************

// Arduino Motor Pins
#define R_MOT_F 5
#define R_MOT_R 6
#define L_MOT_F 10
#define L_MOT_R 9

void setup()
{
  // Pin initialization
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(R_MOT_F, OUTPUT);
  pinMode(R_MOT_R, OUTPUT);
  pinMode(L_MOT_F, OUTPUT);
  pinMode(L_MOT_R, OUTPUT);
  
  // set up serial and change device name
  Serial.begin(9600);
  delay(1000);
  initAccelerometer();
  delay(1000);

  // start up Blynk
  Blynk.begin(Serial, auth);
}

void loop()
{
  Blynk.run();
}


// ***************************************************
// MOTOR DRIVER
// This function takes the Joystick values from Blynk
// and runs the motors based on the coordinates.
// This was ultimately ported from Will's Python code
// that was used on the Onion Omegas in previous versions.
// It seems a bit complex, because it has differential steering (like a tank)
// ***************************************************
BLYNK_WRITE(V1) 
{
  // get the Joystick coordinates from Blynk
  int nJoyX = param[0].asInt();
  int nJoyY = param[1].asInt();

  // set some varaibles
  int fPivYLimit = 32;
  int fPivScale = 0;
  int nMotPremixL = 0;
  int nMotPremixR = 0;
  
  // Calculate Drive Turn output due to Joystick X input
  if (nJoyY >= 0) // *** FORWARD ***
  {
    if (nJoyX >= 0) 
    {
      // Right
      nMotPremixL = 127;
      nMotPremixR = (127 - nJoyX);
    }
    else 
    {
      // Left
      nMotPremixL = (127 + nJoyX);
      nMotPremixR = 127;
    }
  }
  else // *** REVERSE ***
  {
    if (nJoyX >= 0) 
    {
      // Right
      nMotPremixL = (127 - nJoyX);
      nMotPremixR = 127;
    }
    else 
    {
      // Left
      nMotPremixL = 127;
      nMotPremixR = (127 + nJoyX);
    }
  }

  // Scale Drive output due to Joystick Y input (throttle)
  nMotPremixL = nMotPremixL * nJoyY/128;
  nMotPremixR = nMotPremixR * nJoyY/128;

  // Now calculate pivot amount
  // pivot strength (nPivSpeed) is based on Joystick X
  // pivot and drive blending (fPivScale) is based on Joystick Y
  int nPivSpeed = nJoyX;
  if (abs(nJoyY) > fPivYLimit)
  {
    fPivScale = 0;
  }
  else
  {
    fPivScale = (1 - abs(nJoyY)/fPivYLimit);
  }

  // Calculate final mix of drive and pivot
  int nMotMixL = (1 - fPivScale)*nMotPremixL + fPivScale*(nPivSpeed);
  int nMotMixR = (1 - fPivScale)*nMotPremixR + fPivScale*((-1)*nPivSpeed);

  // Scale the values
  int Rg = map(nMotMixL, -127, 127, ((-1)*MAX_SPEED), MAX_SPEED);
  int Lg = map(nMotMixR, -127, 127, ((-1)*MAX_SPEED), MAX_SPEED);

  // Now, we can set the motor speeds based on Rg and Lg
  if ((Rg == 0) && (Lg == 0)) // *** STOPPED ***
  {
    digitalWrite(R_MOT_F, LOW);
    digitalWrite(R_MOT_R, LOW);
    digitalWrite(L_MOT_F, LOW);
    digitalWrite(L_MOT_R, LOW);
  }
  else if ((Rg < -250) && (Lg < -250)) // *** FULL SOUTH *** 
  {
    digitalWrite(R_MOT_F, HIGH);
    digitalWrite(R_MOT_R, LOW);
    digitalWrite(L_MOT_F, HIGH);
    digitalWrite(L_MOT_R, LOW);
  }
  else if ((Rg < -250) && (Lg < 0)) // *** SOUTHWEST *** 
  {
    digitalWrite(R_MOT_F, HIGH);
    digitalWrite(R_MOT_R, LOW);
    analogWrite(L_MOT_F, abs(Lg));
    digitalWrite(L_MOT_R, LOW);
  }
  else if ((Rg < 0) && (Lg < -250)) // *** SOUTHEAST *** 
  {
    analogWrite(R_MOT_F, abs(Rg));
    digitalWrite(R_MOT_R, LOW);
    digitalWrite(L_MOT_F, HIGH);
    digitalWrite(L_MOT_R, LOW);
  }
  else if ((Rg < 0) && (Lg < 0)) // *** SOUTH *** 
  {
    analogWrite(R_MOT_F, abs(Rg));
    digitalWrite(R_MOT_R, LOW);
    analogWrite(L_MOT_F, abs(Lg));
    digitalWrite(L_MOT_R, LOW);
  }
  else if ((Rg > 250) && (Lg < 0)) // *** FULL EAST *** 
  {
    digitalWrite(R_MOT_R, LOW);
    digitalWrite(R_MOT_F, HIGH);
    analogWrite(L_MOT_R, abs(Lg));
    digitalWrite(L_MOT_F, LOW);
  }
  else if ((Rg > 0) && (Lg < 0)) // *** EAST *** 
  {
    digitalWrite(R_MOT_R, LOW);
    analogWrite(R_MOT_F, Rg);
    analogWrite(L_MOT_R, abs(Lg));
    digitalWrite(L_MOT_F, LOW);
  }
  else if ((Rg < 0) && (Lg > 250)) // *** FULL WEST *** 
  {
    analogWrite(R_MOT_R, abs(Rg));
    digitalWrite(R_MOT_F, LOW);
    digitalWrite(L_MOT_R, LOW);
    digitalWrite(L_MOT_F, HIGH);
  }
  else if ((Rg < 0) && (Lg > 0)) // *** WEST *** 
  {
    analogWrite(R_MOT_R, abs(Rg));
    digitalWrite(R_MOT_F, LOW);
    digitalWrite(L_MOT_R, LOW);
    analogWrite(L_MOT_F, Lg);
  }
  else if ((Rg > 250) && (Lg > 250)) // *** FULL NORTH *** 
  {
    digitalWrite(R_MOT_F, HIGH);
    digitalWrite(R_MOT_R, LOW);
    digitalWrite(L_MOT_F, LOW);
    digitalWrite(L_MOT_R, HIGH);
  }
  else // *** NORTH ***
  {
    digitalWrite(R_MOT_F, LOW);
    analogWrite(R_MOT_R, Lg);
    digitalWrite(L_MOT_F, LOW);
    analogWrite(L_MOT_R, Rg);
  }
  
}



// ***************************************************
// Initializes the MPU-6050 / GY-521 Accelerometer.
// This is done by sending I2C commands so that when a bump is detected,
// a pulse will be sent to the 74HC123D via the interrupt pin.
// ***************************************************
void initAccelerometer()
{
  Wire.begin();
  
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x68);
  Wire.write(0x07);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x37);
  Wire.write(0x82);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x01);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x1F);
  Wire.write(0x1A);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x20);
  Wire.write(0x05);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x69);
  Wire.write(0x15);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x38);
  Wire.write(0x40);
  Wire.endTransmission();
}
