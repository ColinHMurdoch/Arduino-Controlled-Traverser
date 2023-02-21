/*
 * This module actually drives the stepper motor using the A4988 driver
 *
 * Connect STEP, DIR as indicated
 *
 * Copyright (C)2015-2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */

#include <A4988.h>
#include <TFT_ILI9341.h>

#include "display.h"
#include "filestruct.h"
#include "stepper.h"


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
#define DIR 10
#define STEP 12
//Uncomment line to use enable/disable functionality
#define SLEEP 14

A4988 stepper(MOTOR_STEPS, DIR, STEP, SLEEP);

//constants to hold input pin assignments.
byte limitforward = LIMITFORWARD_PIN; //forward and reverse limit microswitches.
byte limitbackward = LIMITBACKWARD_PIN; // reverse limit microswitch 


/*constants to hold steps for each position.
  Adjust to tune traverser.
  1mm = 25 steps (200 steps per revolution.
  8mm pitch leadscrew and NEMA 17 1.8 deg/step motor).
  44mm required steps between tracks
  pitch = (44/8)*25 = 137.5 - say 138
  pitch = (44*25) = 1100
  */
const int backlash = 50; //steps to take during anti-backlash manouver.
const int pitch = 1115; // Pitch in steps between tracks. orginally 1000
const int homed = 190; // distance from home to first position. originally 200
//define positions to go to for each track. adjust +/-0 for fine tuning.
const int TrackPosition[] = {0 * pitch + 0, 1 * pitch + 0, 2 * pitch + 0,  3 * pitch + 0, 4 * pitch + 0, 5 * pitch + 0, 6 * pitch + 0, 7 * pitch + 0, 8 * pitch + 0, 9 * pitch + 0};

//variables to store current table and servo position and remember previous position.
//Used for anti-backlash and change detection.
long TargetPosition = 0; //initially zero for after the traverser is homed in setup.
long CurrentPosition = 0;
int CurrentTrack =0;
long StepsToMove = 0;
long StepsRemaining = 0;
bool TraverserRunning = false; 
bool BackLashNeeded = false;
long IncompletedSteps = 0;

void MoveTheStepper(long howfar) {

     stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
    Serial.print("Moving the stepper - ");
    Serial.println(howfar);
    stepper.enable();
    delay(10);
    //stepper.move(howfar);  // This isthe blocking command
    if (BackLashNeeded){
      howfar = howfar - backlash; // The value is negative so deduct the backlash
    }
    stepper.startMove(howfar); // Non blocking command
    TraverserRunning = true;
    //delay(10);
    //stepper.disable();
    
}

void StopTheTraverser(){
    // Emergency stop so no braking
    IncompletedSteps = stepper.stop();
    TraverserRunning = false;   
    TraverserStopped = true;   
    showProgressMessage("STOPPED");
}

void RestartTheTraverser(){

    TraverserRunning = true;
    MoveTheStepper(IncompletedSteps);
    showProgressMessage("RUNNING");

}

void RunTraverser(int x) {

  TargetPosition = x - 1;

  Serial.print("Current Position - ");
  Serial.print(CurrentPosition);
  Serial.print("  Step -  ");
  Serial.println(TrackPosition[CurrentPosition]);
  
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
  MoveTheStepper(StepsToMove);   

  Serial.println("Stepper Running");
  CurrentPosition = TargetPosition;
  Serial.print("New Position - ");
  Serial.println(CurrentPosition);

  TraverserRunning = true;   
  StepsRemaining = CurrentPosition * 1100; // approximate calculation
  
  // display running message
  //showProgressMessage("RUNNING");
     
}

void ResetTrack() {
  UpLine = selectedupline;
  DownLine = UpLine + 1;
  ShowCurrentTracks();
  //delay(200); // To avoid double or triple press
} 


void GetProgress() {


  StepsRemaining = stepper.getStepsRemaining();
  if (StepsRemaining != 0) {
    stepper.nextAction();
  }
  else {
    if (BackLashNeeded == true) {
      stepper.move(backlash);
    }
    stepper.disable();
  }

}

void SetupTraverser() {
  
   Serial.println("A4988 Slave Running");
// Start the stepper object   
   stepper.begin(RPM, MICROSTEPS);
    
   // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
   stepper.setEnableActiveState(LOW);
     /*
     * Set LINEAR_SPEED (accelerated) profile.
     */
   stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
   Serial.println("START");

    // Seek to the first track
    HomeTheDeck();  

  Serial.println("End of Stepper Setup");


}

void HomeTheDeck() {

//home motor
  //Travel backwards till you trigger the home microswitch.
  //Works by setting stepPin high, then low with delays. A bit noisy when running!

  
  stepper.enable();

  Serial.println("Seeking Home Switch");

  while (digitalRead(limitbackward)) {
    stepper.move(-1);
    delay(10);
  }
  
  Serial.println("Backing Away");
  //Back away from the microswitch slowly till it goes off.
  //As above, but longer delays to slow it down.
  while (!digitalRead(limitbackward)) {
    stepper.move(1);
    delay(20);
  } 
  Serial.println("Traverser Initialised");
  
   // Move to the first track
  Serial.println("Move to home track");
  stepper.move(homed);
  
  Serial.println("Stepper at Track 1");
  
  CurrentTrack = 1;
  CurrentPosition = 0;  // Position = CurrentTrack - 1 as the array starts at 0
  
  stepper.disable();

  Serial.print("Current Position - ");
  Serial.println(CurrentPosition);

}


      



