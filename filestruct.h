#ifndef FILESTRUCT_H  
#define FILESTRUCT_H  

#define LIMITFORWARD_PIN 18
#define LIMITBACKWARD_PIN 16

struct SLAVE_DATA{
    int Scommand;  // 2 bytes
    long Strack;   // 4 bytes
};                 // total 6 bytes

extern SLAVE_DATA Screen_Info;

extern int selectedupline;  // defined in main.cpp
extern int selecteddownline; 
extern int previousupline;
extern int counter;
extern int UpLine;
extern int DownLine;
extern long StepsToMove;
extern long StepsRemaining;
extern bool TraverserRunning;  
extern bool TraverserStopped;
extern long IncompletedSteps;
extern bool TravelButtonPressed;
extern bool LIMITFORWARDbuttonpressed;
extern bool LIMITBACKWARDbuttonpressed;

//extern unsigned long NextActionTime;


#endif