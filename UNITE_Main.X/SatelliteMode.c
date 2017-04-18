#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "SatelliteMode.h"
#include "adc1.h"
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/uart2.h"
#include "mcc_generated_files/uart3.h"
#include "mcc_generated_files/uart4.h"
#include "mcc_generated_files/tmr2.h"
#include "mcc_generated_files/tmr3.h"
#include "mcc_generated_files/tmr4.h"
#include "mcc_generated_files/tmr5.h"
#include "time.h"

/******************************
 Satellite Mode Initializations
 ******************************/



// Stores the current mode the satellite should be in
// Will be used as a switch for the satellite to change modes
static UNITEMode currentMode = interim;
static bool shouldChangeMode = false;
unsigned long totalTime = 0; // Keeps track of overall mission clock


// CONSTANTS
unsigned long INTERIM_STOP_TIME = 1800; // 30 mins in
unsigned long SCIENCE_STOP_TIME = 3600; // 60 mins in
unsigned long REENTRY_STOP_TIME = 10800; // 3 hrs in


SatelliteMode InterimMode = {
    TMR3_INTERRUPT_TICKER_FACTOR, // Time in seconds between each sample of sensors
    400, // Altitude to begin sampling in this mode 
    300, // Altitude to end sampling and switch to new mode
};

SatelliteMode ScienceMode = {
    TMR4_INTERRUPT_TICKER_FACTOR,
    300,
    200,
};

SatelliteMode ReEntryMode = {
    TMR5_INTERRUPT_TICKER_FACTOR,
    200,
    0,
};

SatelliteMode SafeMode = {
    0,
    0,
    0,
};
 
void Satellite_Initialize() {

    TMR3_Initialize();
    TMR4_Initialize();
    TMR5_Initialize();
    
    TMR2_Stop();
    TMR3_Start();
    TMR4_Stop();
    TMR5_Stop();
}

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
        UART3_Write(dataString[i]);
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
                // Switch Timers

                TMR3_Stop();
                TMR4_Start();
                UART3_Write(321); //Will tell us it is in interim mode
                return science;
            case science:

                TMR4_Stop();
                TMR5_Start();
                UART3_Write(322); //Will tell us it is in science mode
                return reentry;
            case reentry:

                TMR5_Stop();
                 UART3_Write(323); //Will tell us it is in reentry mode
                return safe;
            default:
                return safe;
        }
    } else {
        return currentMode;
    }
}

// Description: Wait to start next mode

unsigned int DelayForMode() {

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

void Clear(int *buffer, int size) {
    int i;
    for (i = 0; i < size; i++) {
        buffer[i] = 0;
    }
}

void CheckForModeUpdate(unsigned long time) {

    /*
     Debugging only
    UART1_Write(0);
    UART1_Write(time);
    UART1_Write(0);
     */


    // Time begins with an offset of 15 min for balloon test
    // After 30 min switch from interim to science mode
    if ((time == INTERIM_STOP_TIME) || ((currentMode == interim) && (time > INTERIM_STOP_TIME))) {
        shouldChangeMode = true;

        // After 105 min switch from science to reentry mode
    } else if ((time == SCIENCE_STOP_TIME) || ((currentMode == science) && (time > SCIENCE_STOP_TIME))) {
        shouldChangeMode = true;

        // After 165 min switch from reentry to safe mode and end loop
    } else if ((time == REENTRY_STOP_TIME) || ((currentMode == reentry) && (time > REENTRY_STOP_TIME))) {
        shouldChangeMode = true;
    }
}

void BeginSample() {
    int SamplePackage[8]; //This is the building of the package from the ADC data
    const int ARRAY_SIZE = 8;


    Clear(SamplePackage, ARRAY_SIZE);

    GetTempData(SamplePackage, ARRAY_SIZE);
    SendData(SamplePackage, ARRAY_SIZE);
 

    // UNCOMMENT IF NOT TESTING
    //if (!shouldChangeMode) {
        totalTime = totalTime + DelayForMode();
    //}
    CheckForModeUpdate(totalTime);


    currentMode = UpdateMode();



}
