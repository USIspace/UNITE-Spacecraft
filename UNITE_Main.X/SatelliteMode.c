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
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/tmr2.h"
#include "mcc_generated_files/tmr3.h"
#include "mcc_generated_files/tmr4.h"
#include "mcc_generated_files/tmr5.h"
#include "mcc_generated_files/spi2.h"
#include "time.h"

/******************************
 Satellite Mode Initializations
 ******************************/

/*******************
  Global Variables
 *******************/

// Stores the current mode the satellite should be in
// Will be used as a switch for the satellite to change modes
static UNITEMode currentMode = safe;
static bool shouldSample = false;
static bool shouldChangeMode = false;
int results[16];

unsigned long totalTime = 0; // Keeps track of overall mission clock


// CONSTANTS
unsigned long INTERIM_STOP_TIME = 1800; // 30 mins in
unsigned long SCIENCE_STOP_TIME = 3600; // 60 mins in
unsigned long REENTRY_STOP_TIME = 10800; // 3 hrs in

/*******************************
  Satellite Mode Configurations
 *******************************/

SatelliteMode InterimMode = {
    10, // Time in seconds between each sample of sensors
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

/**************************
  Sampling Configurations
 **************************/

ADCSampleConfig lpADCConfig {
    0,   // adc channel to start at   
    0    // number of adc channels to sample
};

ADCSampleConfig magADCConfig {
    0,
    0
};

ADCSampleConfig tmpADCConfig {
    8,
    8
};
 
void Satellite_Initialize() {

    InitializeADC1();
    TMR5_Initialize();
    
    currentMode = interim;
    
    TMR1_Stop();
    TMR5_Start();
    
    TMR5_INTERRUPT_TICKER_FACTOR = InterimMode.sampleRateInSec;
    
    _LATE2 = LED_ON;
}

/**************
 Getter Methods
 **************/

void GetTempData(uint8_t *buffer, int bufferSize) {
    
    shouldSample = true;                    // Allows ADC to sample data
    TMR1_INTERRUPT_TICKER_FACTOR = 1;       // Sample for a duration of 1 sec
    
    TMR4_Start();                           // TMR4 samples every 1 s and stores in results
    TMR1_Start();
    while (shouldSample);                   // shouldSample is set to false upon TMR1 interrupt
    TMR1_Stop();
    TMR4_Stop();
    
    int i;
    for (i = 0; i < bufferSize; i++) {
        buffer[i] = results[i] / 4;
    }

}

void GetGPSData(int *buffer, int bufferSize) {
    // Sample GPS Data and store in buffer
}

void GetMagnetometerData(int *buffer, int bufferSize) {
    // Sample Magnetometer Data and store in buffer
    
    shouldSample = true;                    // Allows ADC to sample data
    TMR1_INTERRUPT_TICKER_FACTOR = 5;       // Sample for a duration of 5 sec
    
    TMR3_Start();                           // TMR3 samples every 5 ms and stores in results
    TMR1_Start();   
    while (shouldSample);                   // shouldSample is set to false upon TMR1 interrupt
    TMR1_Stop();
    TMR3_Stop();
    
    int i;
    for (i = 0; i < bufferSize; i++) {
        buffer[i] = results[i] / 4;
    }
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

void SendData(uint8_t *dataString, int stringLength) {
    
    
    /*==============
     Save to Arduino
     ==============*/
    
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

        int i;
        for (i = 0; i < 8; i++) {
            UART3_Write(255); //Will tell us it is in interim mode 
            UART3_Write(0);
        }
        switch (currentMode) {
            case interim:
                
                // Switch Timers
                _LATE2 = LED_OFF;
                _LATE3 = LED_ON;

                TMR5_INTERRUPT_TICKER_FACTOR = ScienceMode.sampleRateInSec;
                
                return science;
            case science:

                _LATE3 = LED_OFF;
                _LATE4 = LED_ON;
                
                TMR5_INTERRUPT_TICKER_FACTOR = ReEntryMode.sampleRateInSec;

                
                return reentry;
            case reentry:
           
                _LATE4 = LED_OFF;
                
                TMR5_Stop();
                
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

void Clear(uint8_t *buffer, int size) {
    int i;
    for (i = 0; i < size; i++) {
        buffer[i] = 0;
    }
}

void CheckForModeUpdate(unsigned long time) {

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

void TakeSample() {
    //int SamplePackage[8]; //This is the building of the package from the ADC data
    uint8_t SDPackage[8]; //Use this package structure to save to SD Card
    const int ARRAY_SIZE = 8;


    Clear(SDPackage, ARRAY_SIZE);

    GetTempData(SDPackage, ARRAY_SIZE);
    SendData(SDPackage, ARRAY_SIZE);
 
    // Mode Update Test
    totalTime = totalTime + DelayForMode();
    CheckForModeUpdate(totalTime);
    currentMode = UpdateMode();

}
