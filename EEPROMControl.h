#ifndef EEPROMCONTROL_H
#define EEPROMCONTROL_H

int ReadEEProm();
void UpdateEEProm(int newtrack);
bool CheckIfEEPROMSaveDue(int newtrack);
void TrackSelectionHasChanged(int newtrack);

#endif