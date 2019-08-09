/*
 * ROVER SETUP
 * By: Jonathan Kayne, Virginia Tech ECE 2019
 * 
 * This program allows you to configure the BLE+Nano board so that it works.
 * Upload this code to the board, then open a serial monitor
 * (Tools > Serial Monitor, or CTRL+Shift+M)
 * and set the monitor to "Both NL and CR" at 9600 baud.
 * 
 * Set up the board by typing in the given commands to name the device and allow for communication.
 */


byte comdata;

void setup()
 {
     Serial.begin(9600);
     Serial.println("BLE Test");
     Serial.println("Put in AT+NAME=First-Last to set up your name.");
     Serial.println("Next, type AT+PASS=123456");
     Serial.println("Next, type AT+AUTH=0");
     Serial.println("Next, type AT+BLEUSB=3");
 }

void loop()
 {
     while (Serial.available() > 0)  
     {
         comdata = Serial.read();
         delay(2);
         Serial.write(comdata);
     }
 }
