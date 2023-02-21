#ifndef DISPLAY_H
#define DISPLAY_H

void SetUpDisplay();
void showmsgXY(int x, int y, int sz, char colour, String msg);
void showProgressMessage(char *msg);
void ClearProgressMessage();
void ShowSelections();
void ClearSelections();
void ShowCurrentTracks();
void FlashTrack();
void FlashMessage();
void ResetTrack(int selection);
void DrawInitialScreen();
void DisplaySelection();
void TraverserLanded();
void StartCommand();

//void ProcessData();

#endif
