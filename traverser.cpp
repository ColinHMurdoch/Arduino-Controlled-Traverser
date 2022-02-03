#include <Wire.h>
#include <i2cSimpleTransfer.h>
#include "filestruct.h"
#include "display.h"

SLAVE_DATA traverser_request; // define a structure for passing data to the stepper driver.

// Now set up the I2C data
#define i2c_traverser_slave 17

void RunTraverser() {

// Traverser Commands commands expected
//  Command   Track
//  1       1 to 9    The range of track options
//  2       0         The traverser request was issued
//  3       1 to 9    The traverser has arrived at the track
//  4       1 to 9    Acknowledgement
//  7       1 to 9    No. of STeps Remaining
//  8       1 to 9    Restart Traverser
//  9       0         Stop the Traverser


  Serial.println("Sending Run Data");
  Serial.print("Selected Up Line - ");
  Serial.println(UpLine);
 
  traverser_request.Scommand = 1;
  traverser_request.Strack = UpLine;
  
  Serial.print(traverser_request.Scommand);
  Serial.print(" - ");
  Serial.println(traverser_request.Strack);
  
  Wire.beginTransmission(i2c_traverser_slave);
  i2cSimpleWrite(traverser_request);
  Wire.endTransmission();
 
  delay(100);     // Just for example use.  Use some sort of timed action for implementation
  
  TraverserRunning = true;   
  StepsRemaining = traverser_request.Strack * 1100; // approximate calculation
  
  // display running message
  tft.setTextColor(TFT_RED);
  tft.setTextSize(4);   
  tft.fillRect(48, 45, 190, 50, TFT_BLACK);
  tft.setCursor(40, 50);
  tft.print("RUNNING");  
    
}

void RestartTraverser(){

  Serial.println("Sending Restart Command");
 
  traverser_request.Scommand = 8;
  traverser_request.Strack = UpLine;
  
  Serial.print(traverser_request.Scommand);
  Serial.print(" - ");
  Serial.println(traverser_request.Strack);
  
  Wire.beginTransmission(i2c_traverser_slave);
  i2cSimpleWrite(traverser_request);
  Wire.endTransmission();
 
  delay(200);     // Just for example use.  Use some sort of timed action for implementation
  
  TraverserRunning = true;   
  
  tft.setTextColor(TFT_RED);
  tft.setTextSize(4);   
  tft.fillRect(48, 45, 190, 50, TFT_BLACK);
  tft.setCursor(40, 50);
  tft.print("RUNNING");  
  delay(2000);
  tft.fillRect(40, 45, 190, 50, TFT_BLACK);
  
}  

void StopTraverser(){

  traverser_request.Scommand = 9;
  traverser_request.Strack = UpLine;
  
  Serial.print(traverser_request.Scommand);
  Serial.print(" - ");
  Serial.println(traverser_request.Strack);
  
  Wire.beginTransmission(i2c_traverser_slave);
  i2cSimpleWrite(traverser_request);
  Wire.endTransmission();
 
  delay(100);     // Just for example use.  Use some sort of timed action for implementation
  
  Wire.requestFrom( i2c_traverser_slave, sizeof(traverser_request) );    // request data from the Slave device the size of our struct

  if ( Wire.available() == sizeof(traverser_request) ) {
      i2cSimpleRead(traverser_request);
  }

  StepsRemaining = traverser_request.Strack;

  TraverserRunning = false;   
  TraverserStopped = true;   
  tft.fillRect(48, 45, 190, 50, BLACK);
  tft.setCursor(40, 50);
  tft.print("STOPPED");

}

void ResetTrack(int selection) {
  UpLine = selection;
  DownLine = UpLine + 1;
  ShowSelections();
  delay(200); // To avoid double or triple press
} 


void GetProgress() {


    // get the number of steps from the slave to see when traverser stopped
    Wire.requestFrom( i2c_traverser_slave, sizeof(traverser_request) );    // request data from the Slave device the size of our struct

    delay(10); // wait for a reply

    if ( Wire.available() == sizeof(traverser_request) ) {
       i2cSimpleRead(traverser_request);
    }

  StepsRemaining = traverser_request.Strack;

}

