#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "SatelliteMode.h"
#include "adc1.h"
#include "mcc_generated_files/uart1.h"

/*************************************
 Satellite Mode Definitions and Values
 *************************************/



// Stores the current mode the satellite should be in
// Will be used as a switch for the satellite to change modes
static UNITEMode currentMode = interim; 
static bool shouldChangeMode = false;

SatelliteMode InterimMode = {
    60,  // Time in seconds between each sample of sensors
    400, // Altitude to begin sampling in this mode 
    300, // Altitude to end sampling and switch to new mode
};

SatelliteMode ScienceMode = {
    30,
    300,
    200,
};

SatelliteMode ReEntryMode = {
    15,
    200,
    0,
};

SatelliteMode SafeMode = {
    0,
    0,
    0,
};


/**************
 Getter Methods
 **************/

void GetTempData(int *buffer, int bufferSize) {
    // Sample Temp Data and store in buffer (an array of size bufferSize)
    int sensorToStartAt = 8;
    
    int count;
    int channel;
    for (count = 0; count < bufferSize; count++) {
        channel = count + sensorToStartAt; // Increment ADC channel
        
        // Fill buffer and divide by 4 to send to Arduino
        buffer[count] = (ADC1_ResultGetFromChannel(channel) / 4);
        wait_ms(5);
    }
    
}

void GetGPSData(int *buffer, int bufferSize) {
    // Sample GPS Data and store in buffer
}

void GetMagnetometerData(int *buffer, int bufferSize) {
    // Sample Magnetometer Data and store in buffer
}

void GetProbeData(int *buffer, int bufferSize) {
    // Sample Plasma Probe Data and store in buffer
}

/********************
 Data Manager Methods
 ********************/

// Description: Saves data to SD card for packaging and sending later
// *package => ready-to-send package
// packageSize => size of package to send
void SaveData(int *package, int packageSize) {
    
}

// Description: Packaging algorithm for collected data
// *package => pointer to a string which will be filled by this method
// stringLength => length of package string in characters
// *temps => pointer to an array of Temperature data
// *gps => pointer to an array of GPS data
// *mags => pointer to an array of Magnetometer data
// *densities => pointer to an array of Plasma Probe data
void PackageData(int *package, int stringLength, int *temps, int *gps, int *mags, int *densities) {
    
    
    int i;
    for (i = 0; i < stringLength; i++) {
        // Fill package char by char from buffers here
    }
    
}

// Description: Handler for sending data via UART
// *dataString => pointer to a string of packaged data
// stringLength => length of packaged data string
void SendData(int *dataString, int stringLength) {
    // Send data via UART here
    int i;
    for (i = 0; i < stringLength; i++) {
        UART1_Write(dataString[i]);
    }
}

/********************
 Mode Switch Handling
 ********************/

// Description: Handler for switching satellite modes
UNITEMode UpdateMode() {
    // Check current altitude
    // Determine which mode should be active
    // Return value of mode
    
    if (shouldChangeMode) {
        
        shouldChangeMode = false;
        
        switch (currentMode) {
            case interim:
                return science;
            case science:
                return reentry;
            case reentry:
                return safe;
            default:
                return safe;
        }
    } else {
        return currentMode;
    }
}

// Description: Wait to start next mode
<<<<<<< HEAD
unsigned long DelayForMode() {
=======
unsigned int DelayForMode() {
>>>>>>> origin/Dev-Branch
    
    switch (currentMode) {
        case interim:
            return InterimMode.sampleRateInSec;
        case science:
            return ScienceMode.sampleRateInSec;
        case reentry:
            return ReEntryMode.sampleRateInSec;
        case safe:
            return SafeMode.sampleRateInSec;
        default:
            return 1;
    }
} 
<<<<<<< HEAD
=======

>>>>>>> origin/Dev-Branch
void Clear(int *buffer, int size) {
    int i;
    for (i = 0; i < size; i++) {
        buffer[i] = 0;
    }
}

<<<<<<< HEAD
void CheckForModeUpdate(unsigned long time) {

=======
void CheckForModeUpdate(int time) {
    
>>>>>>> origin/Dev-Branch
    /*
     Debugging only
    UART1_Write(0);
    UART1_Write(time);
    UART1_Write(0);
<<<<<<< HEAD
     */

=======
    */
    
>>>>>>> origin/Dev-Branch
    // Time begins with an offset of 15 min for balloon test
    // After 30 min switch from interim to science mode
    if ((time == 1800) || ((currentMode == interim) && (time > 1800))) {
        shouldChangeMode = true;
<<<<<<< HEAD

        // After 105 min switch from science to reentry mode
    } else if ((time == 6300) || ((currentMode == science) && (time > 6300))) {
        shouldChangeMode = true;

        // After 165 min switch from reentry to safe mode and end loop
=======
    
    // After 105 min switch from science to reentry mode
    } else if ((time == 6300) || ((currentMode == science) && (time > 6300))) {
        shouldChangeMode = true;
        
    // After 165 min switch from reentry to safe mode and end loop
>>>>>>> origin/Dev-Branch
    } else if ((time == 9900) || ((currentMode == reentry) && (time > 9900))) {
        shouldChangeMode = true;
    }
}
<<<<<<< HEAD
void BeginSample() {
    int SamplePackage[8]; //This is the building of the package from the ADC data

    const int ARRAY_SIZE = 8;

    unsigned long timeCount = 0;
    /******************************************************************************/
    /* MAIN LOOP HERE                                              */
    /******************************************************************************/
    int wait;
    // Wait for 900 cycles of 1 second (15 min)
    for (wait = 0; wait < 900; wait++) {
        wait_ms(1000);
    }

    while (currentMode != safe) {

        CheckForModeUpdate(timeCount);

=======

void BeginSample() {
    int SamplePackage[8]; //This is the building of the package from the ADC data
    const int ARRAY_SIZE = 8;

    int timeCount = 0;
    
    while (currentMode != safe) {

        CheckForModeUpdate(timeCount);
        
>>>>>>> origin/Dev-Branch
        Clear(SamplePackage, ARRAY_SIZE);

        GetTempData(SamplePackage, ARRAY_SIZE);
        SendData(SamplePackage, ARRAY_SIZE);

        currentMode = UpdateMode();
<<<<<<< HEAD
        wait_sec(DelayForMode());

=======
        //wait_ms(DelayForMode());

        //UART1_Write(111);
        //check if more data - send while more
        //When done send a finished data package 
>>>>>>> origin/Dev-Branch


        timeCount = timeCount + (DelayForMode());
    }
<<<<<<< HEAD
}

=======
}
>>>>>>> origin/Dev-Branch
