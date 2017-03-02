#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "SatelliteMode.h"
#include "adc1.h"

/*************************************
 Satellite Mode Definitions and Values
 *************************************/



// Stores the current mode the satellite should be in
// Will be used as a switch for the satellite to change modes
static UNITEMode currentMode; 

SatelliteMode InterimMode = {
    10, // Time in minutes between each sample of sensors
    400, // Altitude to begin sampling in this mode 
    300, // Altitude to end sampling and switch to new mode
};

SatelliteMode ScienceMode = {
    5,
    300,
    200,
};

SatelliteMode ReEntryMode = {
    1,
    200,
    0,
};


/**************
 Getter Methods
 **************/

void GetTempData(int *buffer, int bufferSize) {
    // Sample Temp Data and store in buffer (an array of size bufferSize)
    int sensorToStartAt = 8;
    int numberOfSensors = 8;
    
    int count;
    int channel = sensorToStartAt;
    for (count = 0; count < bufferSize; count++) {
        channel = count + numberOfSensors; // Increment ADC channel
        
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

// Description: Packaging algorithm for collected data
// *package => pointer to a string which will be filled by this method
// stringLength => length of package string in characters
// *temps => pointer to an array of Temperature data
// *gps => pointer to an array of GPS data
// *mags => pointer to an array of Magnetometer data
// *densities => pointer to an array of Plasma Probe data
void PackageData(char *package, int stringLength, int *temps, int *gps, int *mags, int *densities) {
    
    
    int i;
    for (i = 0; i < stringLength; i++) {
        // Fill package char by char from buffers here
    }
    
}

// Description: Handler for sending data via UART
// *dataString => pointer to a string of packaged data
// stringLength => length of packaged data string
void SendData(char *dataString, int stringLength) {
    // Send data via UART here
}

/********************
 Mode Switch Handling
 ********************/

// Description: Handler for switching satellite modes
UNITEMode UpdateToMode() {
    // Check current altitude
    // Determine which mode should be active
    // Return value of mode
    UNITEMode mode = interim;
    return mode;
}