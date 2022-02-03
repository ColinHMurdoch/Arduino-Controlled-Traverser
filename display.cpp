#include <Arduino.h>

#include "filestruct.h"
#include "display.h"
#include <TFT_ILI9341.h> // Hardware-specific library
#include <SPI.h>

TFT_ILI9341 tft = TFT_ILI9341();       // Invoke custom library


long LastFlash = 0;
int FlashState = LOW;

const int interval = 2000;

//extern SLAVE_DATA Screen_Info;

void SetUpDisplay() {
  
  // Clear the buffer.
  Serial.println("Clearing Display");
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  
}

void showmsgXY(int x, int y, int sz, char colour, char *msg)
{
    //int16_t x1, y1;
    //uint16_t wid, ht;
    //tft.drawFastHLine(0, y, tft.width(), TFT_WHITE);
    tft.setCursor(x, y);
    tft.setTextColor(colour);
    tft.setTextSize(sz);
    tft.print(msg);
    delay(10);
  
}


void ShowSelections() {
  
    tft.setTextColor(TFT_MAGENTA);
    tft.fillRect(48, 200, 60, 50, TFT_BLACK);
    tft.setTextSize(5);
    tft.setCursor(48, 200);
    tft.print(selectedupline);
    tft.fillRect(170, 200, 60, 50, TFT_BLACK);
    tft.setCursor(170, 200);
    tft.print(selecteddownline);
}

void ClearSelections() {
  
    tft.fillRect(48, 200, 60, 50, TFT_BLACK);
    tft.fillRect(170, 200, 60, 50, TFT_BLACK);
    
}

void ShowCurrentTracks() {
    tft.setTextColor(TFT_MAGENTA);
    tft.fillRect(48, 115, 60, 50, TFT_BLACK);
    tft.setTextSize(5);
    tft.setCursor(48, 115);
    tft.print(UpLine);
    tft.fillRect(170, 115, 60, 50, TFT_BLACK);
    tft.setCursor(170, 115);
    tft.print(DownLine);
}

void FlashTrack() {

   unsigned long currentMillis = millis();
   
    if (currentMillis - LastFlash >= interval) {
        tft.fillRect(48, 115, 60, 50, TFT_BLACK);
        tft.fillRect(170, 115, 60, 50, TFT_BLACK);
        LastFlash = currentMillis;
    }
    else  {
        ShowCurrentTracks();
    }
   
}

void FlashMessage() {

  unsigned long thisMillis = millis();
  
  if(thisMillis - LastFlash >= interval) {
    LastFlash = thisMillis;
    if (FlashState == LOW){
    showmsgXY(40, 200, 4, TFT_MAGENTA, "RUNNING");
    FlashState = HIGH;
    }
    else {
      tft.fillRect(40, 200, 190, 50, TFT_BLACK);
      FlashState = LOW;
    }
  }
}

void ResetTrack(int selection) {
  UpLine = selection;
  DownLine = UpLine + 1;
  ShowSelections();
  //delay(200); // To avoid double or triple press
  
}

void DrawInitialScreen() {

    showmsgXY(50, 1, 2, TFT_YELLOW, "Nantyderry");
    showmsgXY(50, 30, 2, TFT_YELLOW, "Traverser");
    tft.drawFastHLine(0, 55, tft.width(), TFT_WHITE);
    showmsgXY(1, 65, 2, TFT_WHITE, "   Current Tracks");
    showmsgXY(1, 90, 2, TFT_WHITE, "    UP      DOWN");
  
    Serial.print("Drawing Screen");
    ShowCurrentTracks();

    tft.drawFastHLine(0, 165, tft.width(), TFT_WHITE);
    showmsgXY(1, 175, 2, TFT_WHITE, "  Selected Tracks");
  
    tft.drawFastHLine(0, 250, tft.width(), TFT_WHITE);

    showmsgXY(1, 260, 2, TFT_WHITE, " Select Track Below");
    showmsgXY(1, 285, 2, TFT_GREEN, "    PRESS to Run");

 }

void DisplaySelection() {

  Serial.print("Selected Up Line - ");
  Serial.println(selectedupline);
 
  Screen_Info.Scommand = 1;
  Screen_Info.Strack = selectedupline;
  
  Serial.print(Screen_Info.Scommand);
  Serial.print(" - ");
  Serial.println(Screen_Info.Strack);

  ShowSelections();

}



void TraverserLanded()
  {
  // Tell the screen that the traverser has arrived and can be updated
   
  Screen_Info.Scommand = 3;
  Screen_Info.Strack = selectedupline;
  
  Serial.print(Screen_Info.Scommand);
  Serial.print(" - ");
  Serial.println(Screen_Info.Strack);
  
  //ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   

  // Wire.beginTransmission(i2c_traverser_slave);
  // i2cSimpleWrite(Screen_Info);
  // Wire.endTransmission();
  
   
}

// void ProcessData() {

// // Radio commands expected
// //  Command   Track
// //  1       1 to 9    The range of track options
// //  2       0         The traverser request was issued
// //  3       1 to 9    The traverser has arrived at the track
// //  4       1 to 9    Acknowledgement

//  int RunCommand = 0;
  
//     //i2cSimpleRead(track_selection); 

//     RunCommand = Screen_Info.Scommand;
//     selectedupline = Screen_Info.Strack;

//     Serial.print("Command Received - ");
//     Serial.print(RunCommand);
//     Serial.print(" to track ");
//     Serial.println(Screen_Info.Strack);

//     if (RunCommand == 1){  // a selection has been made
//       selecteddownline = selectedupline + 1;     


//       ShowSelections();
//       previousupline = selectedupline;
//       counter=selectedupline;
//     }

//     if (RunCommand == 2){
//       Serial.println("Running Traverser");
//       TraverserRunning = true;
//       ClearSelections();

//       showmsgXY(40, 200, 4, TFT_YELLOW, "RUNNING");
//       TravelButtonPressed = true;
//       }

//       if (RunCommand == 3){
//         Serial.println("Traverser Arrived");
//         TraverserRunning = false;
//         //ClearSelections();
//         tft.fillRect(40, 200, 190, 50, TFT_BLACK);
//         //tft.fillRect(170, 200, 60, 50, BLACK);
//         UpLine = selectedupline;
//         DownLine = UpLine + 1;
        
//         ShowCurrentTracks();
//         }

//       if (RunCommand == 4) {
//         Serial.println("Command Acknowledged");
//       }
      
// }

void StartCommand() {

  Serial.println("Issue Start Command");
 
  Screen_Info.Scommand = 2;
  Screen_Info.Strack = selectedupline;
  
  Serial.print(Screen_Info.Scommand);
  Serial.print(" - ");
  Serial.println(Screen_Info.Strack);
  
  // Change the following code to directly drive the A4988 chip.
  //ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   ADDITION   

  //Wire.beginTransmission(i2c_traverser_slave);
  //i2cSimpleWrite(Screen_Info);
  //Wire.endTransmission();
 
  //delay(200);     // Just for example use.  Use some sort of timed action for implementation
  
  //TraverserRunning = true;   

  //delay(5000);
  
}


