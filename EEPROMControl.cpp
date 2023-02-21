//This file contains the routines for reading, witing and updating the #
// data stored inthe EEPROM

#include <EEPROM.h>
#include <Arduino.h>

struct EEPROMDATA {
    int currenttrack; // 2 bytes
    long numofwrites; // 4 bytes
};

EEPROMDATA TraverserData; 

long LastTrackChangeTime = millis();
int UpdateInterval = 600000; // 1000 * 60 * 10 for 10 minutes

int eeAddress = 0;      // this is the base address.  Once the number of writes
                        // exceeds 50000 to 100000 increase this address

int ReadEEProm(){

    EEPROM.get(eeAddress, TraverserData);
    if (TraverserData.currenttrack > 0 || TraverserData.currenttrack < 10){
        return TraverserData.currenttrack;
    }
    else {
        return 0;
    }
} // end ReadEEProm

void UpdateEEProm(int newtrack){

    TraverserData.currenttrack = newtrack;
    TraverserData.numofwrites = TraverserData.numofwrites + 1;

    EEPROM.put(eeAddress, TraverserData);

}

bool CheckIfEEPROMSaveDue(int newtrack){

    if ((millis() - LastTrackChangeTime) > UpdateInterval){
        UpdateEEProm(newtrack);
        return true;
    }
    else {
        return false;
    }

}

// This function should be called when a new track is selected
// to restart the timer.
void TrackSelectionHasChanged(int newtrack){

    LastTrackChangeTime = millis()

}