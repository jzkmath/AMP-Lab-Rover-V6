/*************************************************************
 ROVER V6 CONFIGURATION

 This is where we can set necessary variables 
 for the Low Cost Rover.
 This includes: 
 - the auth token from Blynk,
 - Device Name
 - Max Speed
*************************************************************/

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Put the device name here
// We need to do this so that we can identify our Rover
#define DEV_NAME "Rover-Bilbo-Baggins"

// Max speed for the rover.
// any value between 0 and 255 is acceptable.
// only adjust if you want to make the rover slower!
#define MAX_SPEED 255
