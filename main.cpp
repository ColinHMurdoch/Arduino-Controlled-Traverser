#include <Arduino.h>
#include <SPI.h>
#include <EasyButton.h>
//#include <RF24.h>
#include <wire.h>
#include "filestruct.h"
#include "traverser.h"
#include "display.h"
#include "radio.h"


#include "filestruct.h"
#include "display.h"
//#include "radio.h"

// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define SW 4 // used as a button pin

// Define the button pins
#define ESTOP_BUTTON_PIN 6
#define LIMITFORWARD_PIN 15
#define LIMITBACKWARD_PIN 14



// Instance of the Emergency Stop Button 
EasyButton ESTOP(ESTOP_BUTTON_PIN);
bool ESTOPbuttonpressed = false;
bool ledon = false;

// Now define the button to move the traverser
EasyButton TRAVEL(SW);
bool TravelButtonPressed = false;

int loopcount = 0;

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


int counter = 1;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;


// Variables to hold selected tracks
int selectedupline = 1;
int selecteddownline = 2; //start position 
int previousupline = 1;
int UpLine = 1;
int DownLine = 2;
bool TraverserStopped =false;
bool TraverserRunning = false;
long StepsToMove = 0;
long IncompletedSteps = 0;
unsigned long NextActionTime = 0;

// struct SLAVE_DATA{
//     int Scommand;  // 2 bytes
//     long Strack;   // 4 bytes
// };                 // total 6 bytes

SLAVE_DATA Screen_Info;


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
    Serial.println("Sending Radio Command");
    Screen_Info.Scommand = 1;
    Screen_Info.Strack = selectedupline;
     
    //RadioRoutine();  // Remove these comm
    }
  }

  // Remember last CLK state

  lastStateCLK = currentStateCLK;

}

void ProcessTravelButton() {

  Serial.println("Travel Button pressed!");
  Serial.println("Issuing Start Command");

  RunTraverser();
    
  //RadioRoutine();   // run the radio routine

  TraverserRunning= true;

}

void ProcessEStop(){
  if (TraverserStopped){
        Serial.println("Restarting the Traverser");
        RestartTraverser();
        ESTOPbuttonpressed = false;
        TraverserStopped = false;
      }
      else {
      Serial.println("Stopping the Traverser");
      RunTraverser();
      ESTOPbuttonpressed = false;
      TraverserStopped = true;
      }
  }
}

void setup() {
  
  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Setup Serial Monitor

  Serial.println("Running Setup");
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT); // Initialize Serial for debuging purposes.
  
  // Setup Serial Monitor
  Serial.begin(115200);

  Wire.begin(); // join i2c bus (address optional for master)
  
  // Initialize the buttons
  ESTOP.begin();
  ESTOP.onPressed(onESTOPPressed);
  TRAVEL.begin();
  TRAVEL.onPressed(onTRAVELPressed);
  
  // LIMITFORWARD.begin();
  // LIMITFORWARD.onPressed(onLIMITFORWARDPressed);
  // LIMITBACKWARD.begin();
  // LIMITBACKWARD.onPressed(onLIMITBACKWARDPressed);
   
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);

   // We are at track 1 so set the variables
  selectedupline = 1;
  selecteddownline = 2; //start position
  UpLine = 1;
  DownLine = 2;

  SetUpDisplay();
  DrawInitialScreen();
  ShowCurrentTracks();

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
      ESTOPbuttonpressed = false;
      TraverserStopped = false;
    }
    else {
    Serial.println("Stopping the Traverser");
    ESTOPbuttonpressed = false;
    TraverserStopped = true;
    }
  }

  TRAVEL.read();
  //CheckButton();
  //Serial.println("TRAVEL Button Read");

  //Serial.print("TRAVEL Button - ");
  //Serial.println(TravelButtonPressed);

  if (TravelButtonPressed == true) {
     
    ProcessTravelButton();
    TravelButtonPressed = false;
  }


  if (TraverserRunning == true) {
    FlashMessage();
    loopcount = 1;
    GetProgress();
    If (StepsRemaining = 0) {
      TraverserRunning = false;
      ResetTrack(selectedupline);
    }

  }

  if (run_btn.justPressed()) {
        run_btn.drawButton(true);
        if (TraverserStopped) {
          RestartTraverser();
        }
        else {
          RunTraverser();
        }

  CheckRotation();
  
  // See if we have anything from the Slave processor
  //CheckRadioData();   Temprarily disable this.


}

