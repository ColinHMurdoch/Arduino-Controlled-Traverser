/*
 * This sketch runs the TFT screen and send results to an arduino Nano which runs
 * the traverser motor routines.
 */

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#include <SPI.h>
#include <Wire.h>
#include <i2cSimpleTransfer.h>

#define MINPRESSURE 200
#define MAXPRESSURE 1000

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
//These settings are for the McuFriend 2.8" shield

const int XP = 7, XM = A1, YP = A2, YM = 6;
//const int TS_LEFT = 918, TS_RT = 122, TS_TOP = 99, TS_BOT = 895; // for Portrait Orientation
//const int TS_LEFT = 99, TS_RT = 895, TS_TOP = 122, TS_BOT = 918;  // For Landscape Orientation
const int TS_LEFT = 123, TS_RT = 923, TS_TOP = 895, TS_BOT = 98; // for Portrait orientation 2


TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int UpLine = 1;
int DownLine = UpLine + 1;
bool TraverserRunning = false; //Indicates if the traverser is running
bool TraverserStopped = false; //Indicated if emergency stop issued

// Now set up the I2C data
#define i2c_traverser_slave 17

// structure of the data
struct SLAVE_DATA{
    int Scommand;
    long Strack;
};

SLAVE_DATA traverser_request;


//Adafruit_GFX_Button in_btn, out_btn, run_btn, estop_btn;
Adafruit_GFX_Button btn1, btn2, btn3, btn4, btn5, btn6, btn7, btn8, btn9, run_btn, estop_btn;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars

bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    return pressed;
}

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void setup(void)
{
    Serial.begin(9600);
    Wire.begin(); // join i2c bus (address optional for master)
    
    uint16_t ID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(ID, HEX);
    Serial.println("Calibrate for your Touch Panel");
    if (ID == 0xD3D3) ID = 0x9486; // write-only shield
    tft.begin(ID);
    //tft.setRotation(0);            //PORTRAIT
    tft.setRotation(2);           //LANDSCAPE
    tft.fillScreen(BLACK);
    showmsgXY(1, 1, 2, YELLOW, "Nantyderry");
    showmsgXY(130, 1, 2, YELLOW, "Traverser");
    tft.drawFastHLine(0, 18, tft.width(), WHITE);
    showmsgXY(1, 25, 2, WHITE, "    UP-Track-DOWN");
    
    Serial.print("Drawing Screen");
    ShowSelections();
   
    Serial.println("Defining Buttons");
    btn1.initButton(&tft,  40, 130, 60, 40, WHITE, CYAN, BLACK, "1", 2);
    btn2.initButton(&tft,  120, 130, 60, 40, WHITE, CYAN, BLACK, "2", 2);
    btn3.initButton(&tft,  200, 130, 60, 40, WHITE, CYAN, BLACK, "3", 2);
    btn4.initButton(&tft,  40, 180, 60, 40, WHITE, CYAN, BLACK, "4", 2);
    btn5.initButton(&tft,  120, 180, 60, 40, WHITE, CYAN, BLACK, "5", 2);
    btn6.initButton(&tft,  200, 180, 60, 40, WHITE, CYAN, BLACK, "6", 2);
    btn7.initButton(&tft,  40, 230, 60, 40, WHITE, CYAN, BLACK, "7", 2);
    btn8.initButton(&tft,  120, 230, 60, 40, WHITE, CYAN, BLACK, "8", 2);
    btn9.initButton(&tft,  200, 230, 60, 40, WHITE, CYAN, BLACK, "9", 2);
 
    run_btn.initButton(&tft, 60, 285, 100, 40, WHITE, GREEN, BLACK, "Run", 2);
    estop_btn.initButton(&tft, 180, 285, 100, 40, WHITE, RED, WHITE, "STOP", 2);

    btn1.drawButton(false);
    btn2.drawButton(false);
    btn3.drawButton(false);
    btn4.drawButton(false);
    btn5.drawButton(false);
    btn6.drawButton(false);
    btn7.drawButton(false);
    btn8.drawButton(false);  
    btn9.drawButton(false);
    run_btn.drawButton(false);
    estop_btn.drawButton(false);
    
}


void loop(void)
{
    bool down = Touch_getXY();
    btn1.press(down && btn1.contains(pixel_x, pixel_y));
    btn2.press(down && btn2.contains(pixel_x, pixel_y));
    btn3.press(down && btn3.contains(pixel_x, pixel_y));
    btn4.press(down && btn4.contains(pixel_x, pixel_y));
    btn5.press(down && btn5.contains(pixel_x, pixel_y));
    btn6.press(down && btn6.contains(pixel_x, pixel_y));
    btn7.press(down && btn7.contains(pixel_x, pixel_y));
    btn8.press(down && btn8.contains(pixel_x, pixel_y));
    btn9.press(down && btn9.contains(pixel_x, pixel_y));
    run_btn.press(down && run_btn.contains(pixel_x, pixel_y));
    estop_btn.press(down && estop_btn.contains(pixel_x, pixel_y));
    
    if (btn1.justReleased()) 
        btn1.drawButton();
    if (btn2.justReleased()) 
        btn2.drawButton();
    if (btn3.justReleased()) 
        btn3.drawButton();  
    if (btn4.justReleased()) 
        btn4.drawButton();  
    if (btn5.justReleased()) 
        btn5.drawButton();  
    if (btn6.justReleased()) 
        btn6.drawButton();  
    if (btn7.justReleased()) 
        btn7.drawButton();  
    if (btn8.justReleased()) 
        btn8.drawButton();               
    if (btn9.justReleased()) 
        btn9.drawButton();         
    if (run_btn.justReleased())
        run_btn.drawButton();
    if (estop_btn.justReleased())
        estop_btn.drawButton();  
           
    if (btn1.justPressed()) {
      btn1.drawButton(true);
      ResetTrack(1); 
    }
    if (btn2.justPressed()) {
      btn2.drawButton(true);
      ResetTrack(2); 
    }
    if (btn3.justPressed()) {
      btn3.drawButton(true);
      ResetTrack(3); 
    }
   if (btn4.justPressed()) {
      btn4.drawButton(true);
      ResetTrack(4); 
    }
   if (btn5.justPressed()) {
      btn5.drawButton(true);
      ResetTrack(5); 
    }
   if (btn6.justPressed()) {
      btn6.drawButton(true);
      ResetTrack(6); 
    }
   if (btn7.justPressed()) {
      btn7.drawButton(true);
      ResetTrack(7); 
    }
   if (btn8.justPressed()) {
      btn8.drawButton(true);
      ResetTrack(8); 
    }
   if (btn9.justPressed()) {
      btn9.drawButton(true);
      ResetTrack(9); 
    }         
    if (run_btn.justPressed()) {
        run_btn.drawButton(true);
        if (TraverserStopped) {
          RestartTraverser();
        }
        else {
          RunTraverser();
        }
    }
    if (estop_btn.justPressed()) {
        estop_btn.drawButton(true);
        StopTraverser();
    }
}

void showmsgXY(int x, int y, int sz, char colour, const char *msg)
{
    int16_t x1, y1;
    uint16_t wid, ht;
    //tft.drawFastHLine(0, y, tft.width(), WHITE);
    tft.setCursor(x, y);
    tft.setTextColor(colour);
    tft.setTextSize(sz);
    tft.print(msg);
    delay(10);
}


void ShowSelections() {
    tft.setTextColor(MAGENTA);
    tft.fillRect(48, 45, 60, 50, BLACK);
    tft.setTextSize(5);
    tft.setCursor(48, 50);
    tft.print(UpLine);
    tft.fillRect(170, 50, 60, 50, BLACK);
    tft.setCursor(170, 50);
    tft.print(DownLine);
}

void RunTraverser() {
  

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
 
  delay(200);     // Just for example use.  Use some sort of timed action for implementation
  
  TraverserRunning = true;   
  
  tft.setTextColor(RED);
  tft.setTextSize(4);   
  tft.fillRect(48, 45, 190, 50, BLACK);
  tft.setCursor(40, 50);
  tft.print("RUNNING");  
  delay(2000);
  tft.fillRect(40, 45, 190, 50, BLACK);
  
  ShowSelections();
  TraverserRunning = false;
    
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
  
  tft.setTextColor(RED);
  tft.setTextSize(4);   
  tft.fillRect(48, 45, 190, 50, BLACK);
  tft.setCursor(40, 50);
  tft.print("RUNNING");  
  delay(2000);
  tft.fillRect(40, 45, 190, 50, BLACK);
  
  ShowSelections();
  TraverserRunning = false;
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
 
  delay(200);     // Just for example use.  Use some sort of timed action for implementation
  
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
