#ifndef DISPLAY_H
#define DISPLAY_H

void SetUpDisplay();
void TraverserLanded();
void StartCommand();
void ShowSelections();
void showmsgXY(int x, int y, int sz, char colour, String msg);
void ClearSelections();
void ShowCurrentTracks();
void FlashTrack();
void ResetTrack(int selection);
void DrawInitialScreen();
void DisplaySelection();
void FlashMessage();
//void ProcessData();

#endif
