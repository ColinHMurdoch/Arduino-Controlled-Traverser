#ifndef STEPPER_H
#define STEPPER_H

void MoveThStepper(long howfar);
void StopTheTraverser();
void RestartTheTraverser();
void RunTraverser(int x);
void ResetTrack();
void GetProgress();
void SetupTraverser();
void HomeTheDeck();

#endif