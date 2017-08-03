/* 
 * File:   SatelliteMode.h
 * Author: zacksnyder
 *
 * Created on February 28, 2017, 1:43 PM
 */

#ifndef SATELLITEMODE_H
#define	SATELLITEMODE_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* SATELLITEMODE_H */

// Defines the operational modes
typedef enum {
    startup,
    interim,
    science,
    reentry,
    safe
} UNITEMode;

// Struct to store operational mode properties
typedef struct {
    int startAltitudeInKm;  // Altitude to begin sampling in this mode 
    int endAltitudeInKm;    // Altitude to end sampling and switch to new mode
    unsigned long stopTime; // In case GPS fails, uses this value to switch modes
} SatelliteMode;


// Global variable for current operational mode
extern UNITEMode currentMode;
// Global variable for total mission time in seconds
extern unsigned long totalTime;
// Global variable for time in minutes since 00:00 UTC
extern double timeInMin;
// Global variable to track when Duplex is connected
extern bool isDuplexConnected;
// Global variable to track the last GPS altitude reading
extern unsigned long lastAltitude;

// Intializes configuration values and main loop timer
void Satellite_Initialize(void);
// Returns the proper operational mode based on lastAltitude and totalTime
UNITEMode UpdateMode(void);

// Runs all the major functions of UNITE
// Called by Timer 5
void MainLoop(void);

// Updates timeInMin from GPS time of day
void SetTime(uint8_t *, int);
// Updates lastAltitude from GPS altitude
void SetAltitude(uint8_t *, int);
