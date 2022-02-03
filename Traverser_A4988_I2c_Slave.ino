/*
 * Simple demo, should work with any driver board
 *
 * Connect STEP, DIR as indicated
 *
 * Copyright (C)2015-2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */
//#include <Arduino.h>
#include <A4988.h>
#include <SPI.h>
#include <Wire.h>
#include <i2cSimpleTransfer.h>

// Setup the i2c data exchange
#define i2c_slave_address 17

// You can add more variables into the struct, but the default limit for transfer size in the Wire library is 32 bytes
// structure of the data
struct SLAVE_DATA{
    int Scommand;
    long Strack;
};

SLAVE_DATA traverser_request;

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 120

// Acceleration and deceleration values are always in FULL steps / s^2
#define MOTOR_ACCEL 500
#define MOTOR_DECEL 500


// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 1

// All the wires needed for full functionality
#define DIR 2
#define STEP 3
//Uncomment line to use enable/disable functionality
#define SLEEP 9
#define MS1 6
#define MS2 7
#define MS3 8


// 2-wire basic config, microstepping is hardwired on the driver
//BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

//Uncomment line to use enable/disable functionality
//BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP, SLEEP);

//A4988 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MS1, MS2, MS3);
A4988 stepper(MOTOR_STEPS, DIR, STEP, SLEEP);

//constants to hold input pin assignments.
byte limitforward = 15; //forward limit microswitches.
byte limitbackward = 14; // reverse limit microswitch 

/*constants to hold steps for each position.
  Adjust to tune traverser.
  1mm = 25 steps (200 steps per revolution.
  8mm pitch leadscrew and NEMA 17 1.8 deg/step motor).
  44mm required steps between tracks
  pitch = (44/8)*25 = 137.5 - say 138
  pitch = (44*25) = 1100
  */
const int backlash = 50; //steps to take during anti-backlash manouver.
const int pitch = 1115; // Pitch in steps between tracks.  Original value 1100
const int homed = 190; // distance from home to first position.  Changed from 200
//define positions to go to for each track. adjust +/-0 for fine tuning.
const int TrackPosition[] = {0 * pitch, 1 * pitch, 2 * pitch,  3 * pitch, 4 * pitch, 5 * pitch, 6 * pitch, 7 * pitch, 8 * pitch, 9 * pitch};

//variables to store current table and servo position and remember previous position.
//Used for anti-backlash and change detection.
long TargetPosition = 0; //initially zero for after the traverser is homed in setup.
long CurrentPosition = 0;
int CurrentTrack =0;
long StepsToMove = 0;
bool TraverserRunning = false; 
bool BackLashNeeded = false;
long IncompletedSteps = 0;

void MoveTheStepper(long howfar) {

     stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
    Serial.print("Moving the stepper - ");
    //Serial.println(howfar);

    if (BackLashNeeded){
      howfar = howfar - backlash; // The value is negative so deduct the backlash
    }
    Serial.println(howfar);
    stepper.enable();
    delay(10);
    stepper.move(howfar);
    //stepper.startMove(howfar);
    //delay(10);
    //stepper.disable();
    
}
void setup() {
  
   Serial.begin(9600);
   Serial.println("A4988 Slave Running");

   Wire.begin(i2c_slave_address);    // i2c Slave address
   Wire.onRequest(requestEvent);     // when the Master makes a request, run this function
   Wire.onReceive(receiveEvent);    // when the Master sends us data, run this function
    
// Start the stepper object   
   stepper.begin(RPM, MICROSTEPS);
    
   // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
   stepper.setEnableActiveState(LOW);

     /*
     * Set LINEAR_SPEED (accelerated) profile.
     */
   stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
   Serial.println("START");
   

  //define limit microwswitches as inputs with internal pullup resistors used.
  pinMode(limitforward, INPUT_PULLUP);
  pinMode(limitbackward, INPUT_PULLUP);

  digitalWrite(limitforward, HIGH);
  digitalWrite(limitbackward, HIGH);
// digitalWrite(MS1, LOW);
//  digitalWrite(MS2, LOW);
 // digitalWrite(MS3, LOW);
  
  //home motor
  //Travel backwards till you trigger the home microswitch.
  //Works by setting stepPin high, then low with delays. A bit noisy when running!

  
  stepper.enable();
  Serial.println("Seeking Home Switch");
  while (digitalRead(limitbackward)) {
    stepper.move(-1);
    delay(5);
  }
  
  Serial.println("Backing Away");
  //Back away from the microswitch slowly till it goes off.
  //As above, but longer delays to slow it down.
  while (!digitalRead(limitbackward)) {
    stepper.move(1);
    delay(10);
  } 
  Serial.println("Traverser Initialised");
  
   // Move to the first track
  Serial.println("Move to home track");
  //stepper.move(homed);
  MoveTheStepper(homed);
  
  Serial.println("Stepper at Track 1");
  
  CurrentTrack = 1;
  CurrentPosition = 0;  // Position = CurrentTrack - 1 as the array starts at 0
  
  //stepper.disable();

  Serial.print("Current Position - ");
  Serial.println(CurrentPosition);

  Serial.println("End of Setup");
//  delay(2000);


}

void loop() {
   // put your main code here, to run repeatedly:

  //overtravel protection. Stops while either limit switch is activated.
  //while (digitalRead(limitforward) && digitalRead(limitbackward)) {

    //Check each button in turn.

    if (digitalRead(limitforward) == LOW) {
      stepper.stop();
      IncompletedSteps = stepper.getStepsRemaining();
      StepsToMove = 0;
      Serial.println("Table has run too far out");
      Serial.print("Remaining STeps - ");
      Serial.println(IncompletedSteps);
      delay(1000);
     }
     
     if (digitalRead(limitbackward) == LOW) {
      stepper.stop();
      Serial.println("Table has run too far in");
      delay(1000);
      }  

      if (StepsToMove != 0) {
        Serial.print("TraverserRunning = ");
        Serial.println(TraverserRunning);
        Serial.print("Steps to move = ");
        Serial.println(StepsToMove);
        MoveTheStepper(StepsToMove); 
        delay(10);
        StepsToMove = 0;
      }
      else {
        TraverserRunning = false;
        if (BackLashNeeded == true){
          stepper.move(backlash);
          BackLashNeeded = false;
        }
        stepper.disable();
      }
} // End of loop

void RunTraverser() {

  Serial.print("Current Position - ");
  Serial.print(CurrentPosition);
  Serial.print("  Step -  ");
  Serial.println(TrackPosition[CurrentPosition]);
  
  //TargetPosition = selectedupline - 1;
  //StepsToMove = TrackPosition[CurrentPosition] - TrackPosition[TargetPosition];
  StepsToMove =  TrackPosition[TargetPosition] - TrackPosition[CurrentPosition];
  Serial.print("Target Position - ");
  Serial.print(TargetPosition);
   Serial.print("  Step -  ");
  Serial.println(TrackPosition[TargetPosition]);
  
  Serial.print("Steps to move - ");
  Serial.println(StepsToMove);

  if (StepsToMove < 0 ){  // Negative movement - moving in - enable backlash
      BackLashNeeded = true;
      Serial.println("Backlash Requested");
  }
  else {
      BackLashNeeded = false;
  }
 
  TraverserRunning = true;  

  Serial.println("Stepper Running");
  CurrentPosition = TargetPosition;
  Serial.print("New Position - ");
  Serial.println(CurrentPosition);
     
}

void StopTraverser(){

}

void ReStartTraverser(){
  
}

void requestEvent() {

    //long StepsRemaining = stepper.getStepsRemaining();
    
    //traverser_request.Scommand = 3;
    //traverser_request.Strack = StepsRemaining;
    //i2cSimpleWrite(traverser_request);            // Send the Master the sensor data
    
}

void receiveEvent() {
    int RunCommand = 0;
    long TrackSelection = 0;
    
    i2cSimpleRead(traverser_request); 

    RunCommand = traverser_request.Scommand;
    TrackSelection = traverser_request.Strack;

    Serial.print("Command Received - ");
    Serial.print(RunCommand);
    Serial.print(" to track ");
    Serial.println(TrackSelection);

    if (RunCommand == 1){
      TargetPosition = TrackSelection - 1;
      Serial.print("Target Position - ");
      Serial.println(TargetPosition);
      Serial.println("Running Traverser");
      RunTraverser();
    }

    if (RunCommand == 9){
      TargetPosition = TrackSelection - 1;
      Serial.print("Target Position - ");
      Serial.println(TargetPosition);
      Serial.println("Running Traverser");
      StopTraverser();
      
    }
  
}
