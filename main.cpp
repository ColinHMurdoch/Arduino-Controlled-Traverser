#include <Arduino.h>
#include <SPI.h>
#include <EasyButton.h>
//#include <RF24.h>

#include "display.h"
#include "filestruct.h"
#include "stepper.h"

//#include "radio.h"


// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define SW 4 // used as a button pin

// Define the button pins
#define ESTOP_BUTTON_PIN 6
#define LIMITFORWARD_PIN 18
#define LIMITBACKWARD_PIN 16


// Instance of the Emergency Stop Button 
EasyButton ESTOP(ESTOP_BUTTON_PIN);
bool ESTOPbuttonpressed = false;
bool ledon = false;

// Now define the button to move the traverser
EasyButton TRAVEL(SW);
bool TravelButtonPressed = false;

// Now the limit switches
EasyButton LIMITFORWARD(LIMITFORWARD_PIN);
bool LimitForwardPressed = false;

EasyButton LIMITBACKWARD(LIMITBACKWARD_PIN);
bool LimitBackwardPressed = false;

bool LIMITFORWARDbuttonpressed = false;
bool LIMITBACKWARDbuttonpressed = false;

int loopcount = 0;

int counter = 1;
int currentStateCLK = 0;
int lastStateCLK = 0;
String currentDir ="";
unsigned long lastButtonPress = 0;


// Variables to hold selected tracks
int selectedupline = 1;
int selecteddownline = 2; //start position 
int previousupline = 1;
int UpLine = 1;
int DownLine = 2;
bool TraverserStopped =false;
//bool TraverserRunning = false;
//long StepsToMove = 0;

unsigned long NextActionTime = 0;

// You can add more variables into the struct, but the default limit for transfer size in the Wire library is 32 bytes
// structure of the data
// struct SLAVE_DATA{
//     int Scommand;
//     long Strack;
// };

SLAVE_DATA Screen_Info;

// Callback function to be called when the EStop button is pressed.
void onESTOPPressed() {
  Serial.println("ESTOP has been pressed!");
  ESTOPbuttonpressed = true;
}

// Callback function for Travel Button
void onTRAVELPressed(){
  Serial.println("Travel Button Pressed");
  TravelButtonPressed = true;
}

// Callback function for limit buttons
void onLIMITFORWARDPressed() {
  Serial.print("Limit Forward reached");
  LIMITFORWARDbuttonpressed = true;
}

void onLIMITBACKWARDPressed() {
Serial.print("Limit Backward reached");
  LIMITBACKWARDbuttonpressed = true; 
}

void CheckRotation() {
  
  // Check the Rotary Encoder
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      
      if (counter > 1) {
      counter --;
      currentDir ="CCW";
       selectedupline = counter;
      }
  
    } else { 
      // Encoder is rotating CW so increment
      
      if (counter < 9) {
      counter ++;
      currentDir ="CW";
      selectedupline = counter;
      }
    }

    if (selectedupline != previousupline) { 
    Serial.print("Direction: ");
    Serial.print(currentDir);
    Serial.print(" | Counter: ");
    Serial.print(counter);
    Serial.print(" | Up Line: ");
    Serial.println(selectedupline);
    selecteddownline = selectedupline + 1; //
    DisplaySelection();
    previousupline = selectedupline;
     // Tell the slave controller that we have selected a track
    // Serial.println("Sending Radio Command");
    // Screen_Info.Scommand = 1;
    // Screen_Info.Strack = selectedupline;
     
    //RadioRoutine();  // Remove these comm
    }
  }

  // Remember last CLK state

  lastStateCLK = currentStateCLK;

}

void ProcessTravelButton() {

  Serial.println("Travel Button pressed!");
  Serial.println("Issuing Start Command");

  RunTraverser(selectedupline);
    
  //RadioRoutine();   // run the radio routine

  TraverserRunning= true;

}

void ProcessEStop(){
  if (TraverserStopped){
        Serial.println("Restarting the Traverser");
        RestartTheTraverser();
        ESTOPbuttonpressed = false;
        TraverserStopped = false;
      }
      else {
        Serial.println("Stopping the Traverser");
        StopTheTraverser();
        ESTOPbuttonpressed = false;
        TraverserStopped = true;
      }
  
}

void setup() {
  
  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Setup Serial Monitor
   Serial.begin(115200);

  Serial.println("Running Setup");
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT); // Initialize Serial for debuging purposes.
  
  // Initialize the buttons
  ESTOP.begin();
  ESTOP.onPressed(onESTOPPressed);
  TRAVEL.begin();
  TRAVEL.onPressed(onTRAVELPressed);
  LIMITFORWARD.begin();
  LIMITFORWARD.onPressed(onLIMITFORWARDPressed);
  LIMITBACKWARD.begin();
  LIMITBACKWARD.onPressed(onLIMITBACKWARDPressed);

  // We are at track 1 so set the variables
  selectedupline = 1;
   
  selectedupline = 1;
  selecteddownline = 2; //start position
  UpLine = 1;
  DownLine = 2;

  // Draw the screen
  SetUpDisplay();
  DrawInitialScreen();
  // 

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);


  // Now start the traverser driver
  showProgressMessage("STARTUP");
  SetupTraverser();
  ClearProgressMessage();
   
  
  //ShowCurrentTracks();

  //DisplaySelection();

  //RadioSetup();   // Temporarily disable this.
  //radio.printDetails();

  // Now get the traverser running
  //TraverserSetup();
  //CalibrateTraverser();

  Serial.println("End of Setup");
}

void loop() {

  // Read the state of the buttons
  //Serial.println("Starting Loop");

  ESTOP.read();

  //Serial.println("ESTOP Button Read");

  //Serial.print("ESTOP Button - ");
  //Serial.println(ESTOPbuttonpressed);

  if (ESTOPbuttonpressed == true){
    if (TraverserStopped){
      Serial.println("Restarting the Traverser");
      RestartTheTraverser();
      ESTOPbuttonpressed = false;
      TraverserStopped = false;
    }
    else {
      Serial.println("Stopping the Traverser");
      StopTheTraverser();
      ESTOPbuttonpressed = false;
      TraverserStopped = true;
    }

  }

  TRAVEL.read();

  if (TravelButtonPressed == true){
    ProcessTravelButton();
    TravelButtonPressed = false;
    showProgressMessage("RUNNING");
  }

  LIMITBACKWARD.read();

  if (LIMITBACKWARDbuttonpressed == true){
      Serial.println("Stopping the Traverser - too far in");
      StopTheTraverser();
      LIMITBACKWARDbuttonpressed = false;
      TraverserStopped = true;
  }

  LIMITFORWARD.read();

  if (LIMITFORWARDbuttonpressed == true){
      Serial.println("Stopping the Traverser - too far out");
      StopTheTraverser();
      LIMITFORWARDbuttonpressed = false;
      TraverserStopped = true;
  }
  
  if (TraverserRunning == true) {
    //FlashMessage();
    //loopcount = 1;
    GetProgress();

    // Serial.println("STeps Remaining - ");
    // Serial.println(StepsRemaining);
    if (StepsRemaining == 0) {
      TraverserRunning = false;
      ClearProgressMessage();
      ResetTrack();
    }

  }


  CheckRotation();
  
  // See if we have anything from the Slave processor
  //CheckRadioData();   Temprarily disable this.


}

