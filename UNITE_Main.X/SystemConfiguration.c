#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "CommandParser.h"
#include "SystemConfiguration.h"
#include "SampleManager.h"
#include "SatelliteMode.h"
#include "TransmitManager.h"



TransmissionMode TransmissionUnitForMode = {
    SimplexOrDuplex,    // Startup mode
    SimplexOrDuplex,    // Interim mode
    SimplexOrDuplex,    // Stabilize mode
    SimplexOrDuplex,    // Science mode
    SimplexUnit,        // Reentry mode
    SimplexUnit         // Fallback mode  
};

// VALUES BELOW ARE ARBITRARY AND SUBJECT TO CHANGE

unsigned long lpSamplesPerCalibration = 1;

Instrument LangmuirProbe = { //7 //53
    { 144, 2, 12 },  // First Week Mode: Sample every 2 minutes, sample a sweep every 2 * 5 ms, sample for 12 sec 
    { 36000, 2, 12 },  // Interim Mode: Sample every 60 minutes, sample a sweep every 2 * 5 ms, sample for 12 sec
    { 10080, 2, 12 },  // Stabilize Mode: Sample every 30 minutes, sample a sweep every 2 * 5 ms, sample for 12 sec
    { 30, 2, 12 },  // Science Mode: Sample every 30 minutes, sample a sweep every 2 * 5 ms, sample for 12 sec
    { 15, 2, 12 },  // Reentry Mode: Sample every 15 minutes, sample a sweep every 2 * 5 ms, sample for 12 sec
    { 360, 2, 12 }   // Fallback Mode: Sample every __ minutes, sample a sweep every 2 * 5 ms, sample for 12 sec
};

Instrument TemperatureSensors = { //2 //29
    { 30, 0, 0 },    // First Week Mode: Sample every 1 minute //30
    { 480, 0, 0 },    // Interim Mode: Sample every 4 minutes
    { 480, 0, 0 },    // Stabilize Mode: Sample every 2 minutes
    { 480, 0, 0 },    // Science Mode: Sample every 2 minutes  
    { 30, 0, 0 },    // Reentry Mode: Sample every 1 minute
    { 60, 0, 0 }     // Fallback Mode: Sample every __ minutes
};

Instrument Magnetometer = { //5 /59
    { 288, 10, 60 * ORBIT_DUR_MIN },    // First Week Mode: Sample every 1 minute, sample every 10 s, sample for 93 min
    { 36000, 10, 60 * ORBIT_DUR_MIN },    // Interim Mode: Sample every 60 minutes, sample every 10 s, sample for 93 min
    { 5040, 10, 60 * ORBIT_DUR_MIN },    // Stabilize Mode: Sample every 30 minutes, sample every 10 s, sample for 93 min
    { 2880, 10, 60 * ORBIT_DUR_MIN },    // Science Mode: Sample every 30 minutes, sample every 10 s, sample for 93 min
    { 1440, 10, 60 * ORBIT_DUR_MIN },    // Reentry Mode: Sample every 15 minutes, sample every 10 s, sample for 93 min
    { 5760, 10, 60 * ORBIT_DUR_MIN }     // Fallback Mode: Sample every __ minutes, sample every 10 s, sample for 93 min
};

Instrument GPS = { //5 //51
    { 93, 0, 0 },        // First Week Mode: Sample every 93 minute
    { 720, 0, 0 },        // Interim Mode: Sample every 720 minutes
    { 720, 0, 0 },        // Stabilize Mode: Sample every 720 minutes
    { 12, 0, 0 },        // Science Mode: Sample every 12 minutes
    { 12, 0, 0 },        // Reentry Mode: Sample every 12 minutes
    { 720, 0, 0 }         // Fallback Mode: Sample every 720 minutes
};

Instrument Housekeeping = {
    { 0, 0, 0},     // First Week Mode: Sample every 0 minutes
    { 0, 0, 0},     // Interim Mode: Sample every 0 minutes
    { 0, 0, 0},     // Stabilize Mode: Sample every 0 minutes
    { 0, 0, 0},     // Science Mode: Sample every 0 minutes
    { 0, 0, 0},     // Reentry Mode: Sample every 0 minutes
    { 0, 0, 0}      // Fallback Mode: Sample every 0 minutes
};

// Description: Getter for an instrument sample rate
// *instrument -> pointer to an Instrument
// Return: Sample rate in minutes
unsigned long GetSampleRate(Instrument *instrument) {
    
    switch (currentMode) {
        case firstWeek: return instrument->FirstWeek.sampleRate;
        case interim: return instrument->Interim.sampleRate;
        case stabilize: return instrument->Stabilize.sampleRate;
        case science: return instrument->Science.sampleRate;
        case reentry: return instrument->ReEntry.sampleRate;
        case fallback: return instrument->Fallback.sampleRate;
        default: return 0;
    }
}
// Description: Getter for an instrument sweep rate
// *instrument -> pointer to an Instrument
// Return: Sweep rate in Timer ticks for the instrument
unsigned long GetSweepRate(Instrument *instrument) {
    
    switch (currentMode) {
        case firstWeek: return instrument->FirstWeek.sweepRate;
        case interim: return instrument->Interim.sweepRate;
        case stabilize: return instrument->Stabilize.sweepRate;
        case science: return instrument->Science.sweepRate;
        case reentry: return instrument->ReEntry.sweepRate;
        case fallback: return instrument->Fallback.sweepRate;
        default: return 0;
    }
}
// Description: Getter for an instrument sweep duration
// *instrument -> pointer to an Instrument
// Return: Sweep duration in seconds
unsigned long GetSweepDuration(Instrument *instrument) {
    
    switch (currentMode) {
        case firstWeek: return instrument->FirstWeek.sweepDuration;
        case interim: return instrument->Interim.sweepDuration;
        case stabilize: return instrument->Stabilize.sweepDuration;
        case science: return instrument->Science.sweepDuration;
        case reentry: return instrument->ReEntry.sweepDuration;
        case fallback: return instrument->Fallback.sweepDuration;
        default: return 0;
    }
}

// Description: Gives each instrument an identifier for its data package
// system -> the subsystem being packaged
// Return: A byte-length ID for a subsystem
uint8_t GetSystemHeaderID(System system) {
    switch (system) {
        case LPSubSys: return 0x10;
        case MAGSubSys: return 0x20;
        case TMPSubSys: return 0x30;
        case GPSSubSys: return 0x40;
        case EPSSubSys: return 0x50;
        case LPCal: return 0x60;
        default: return 0;
    }
}

// Description: Returns 

// Description: Calculates the number of minutes since 00:00 UTC
// totalMissionTimeInSec -> mission duration in seconds
// Return: Number of minutes since 00:00 UTC in two bytes
uint16_t GetDayTimeInMin(unsigned long totalMissionTimeInSec) {
    return (totalMissionTimeInSec / 60) % 1440;
}

// Description: Getter the length of a package for a transmission unit
// unit -> a type of TransmissionUnit
// Return: A byte for the max size of a data package
uint8_t GetTransmissionPackageLength(TransmissionUnit unit) {
    switch(unit) {
        case SimplexUnit: return 35;
        default: return 0;
    }
}

// Description: Getter for a TransmissionUnit for the current operational mode
// Return: TransmissionUnit for the current mode
TransmissionUnit GetTransmissionUnitForMode() {
    switch (currentMode) {
        case firstWeek: return TransmissionUnitForMode.firstWeek;
        case interim: return TransmissionUnitForMode.interim;
        case stabilize: return TransmissionUnitForMode.stabilize;
        case science: return TransmissionUnitForMode.science;
        case reentry: return TransmissionUnitForMode.reentry;
        case fallback: return TransmissionUnitForMode.fallback;
        default: return DiagUnit;
    }
}