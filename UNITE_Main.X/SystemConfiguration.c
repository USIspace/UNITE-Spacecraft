#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "CommandParser.h"
#include "SystemConfiguration.h"
#include "SampleManager.h"
#include "SatelliteMode.h"
#include "TransmitManager.h"


//
TransmissionMode TransmissionUnitForMode = {
    SimplexUnit,        // Startup mode
    SimplexOrDuplex,    // Interim mode
    DuplexUnit,         // Science mode
    SimplexUnit         // Reentry mode
};

// VALUES BELOW ARE ARBITRARY AND SUBJECT TO CHANGE
Instrument LangmuirProbe = {
    { 1, 1, 16 },   // Startup mode: Sample every 1 minute, sample a sweep every 5 ms, sample for 16 sec
    { 60, 1, 16 },  // Interim mode: Sample every 60 minutes, sample a sweep every 5 ms, sample for 16 sec
    { 30, 1, 16 },  // Science mode: Sample every 30 minutes, sample a sweep every 5 ms, sample for 16 sec
    { 15, 1, 16 }   // Reentry mode: Sample every 15 minutes, sample a sweep every 5 ms, sample for 16 sec
};

Instrument TemperatureSensors = {
    { 1, 0, 0 },    // Startup mode: Sample every 1 minute
    { 4, 0, 0 },    // Interim mode: Sample every 4 minutes
    { 2, 0, 0 },    // Science mode: Sample every 2 minutes  
    { 1, 0, 0 }     // Reentry mode: Sample every 1 minute
};

Instrument Magnetometer = {
    { 1, 1, 20 },   // Startup Mode: Sample every 1 minute
    { 60, 1, 20 },  // Interim Mode: Sample every 60 minutes
    { 30, 1, 20 },  // Science Mode: Sample every 30 minutes
    { 15, 1, 20 }   // Reentry Mode: Sample every 15 minutes
};

Instrument GPS = {
    { 1, 0, 0 },    // Startup Mode: Sample every 1 minute
    { 4, 0, 0 },    // Interim Mode: Sample every 4 minutes
    { 6, 0, 0 },    // Science Mode: Sample every 6 minutes
    { 3, 0, 0 }     // Reentry Mode: Sample every 3 minutes
};

// Description: Getter for an instrument sample rate
// *instrument -> pointer to an Instrument
// Return: Sample rate in minutes
unsigned long GetSampleRate(Instrument *instrument) {
    
    switch (currentMode) {
        case startup: return instrument->Startup.sampleRate;
        case interim: return instrument->Interim.sampleRate;
        case science: return instrument->Science.sampleRate;
        case reentry: return instrument->ReEntry.sampleRate;
        default: return 0;
    }
}
// Description: Getter for an instrument sweep rate
// *instrument -> pointer to an Instrument
// Return: Sweep rate in Timer ticks for the instrument
unsigned long GetSweepRate(Instrument *instrument) {
    
    switch (currentMode) {
        case startup: return instrument->Startup.sweepRate;
        case interim: return instrument->Interim.sweepRate;
        case science: return instrument->Science.sweepRate;
        case reentry: return instrument->ReEntry.sweepRate;
        default: return 0;
    }
}
// Description: Getter for an instrument sweep duration
// *instrument -> pointer to an Instrument
// Return: Sweep duration in seconds
unsigned long GetSweepDuration(Instrument *instrument) {
    
    switch (currentMode) {
        case startup: return instrument->Startup.sweepDuration;
        case interim: return instrument->Interim.sweepDuration;
        case science: return instrument->Science.sweepDuration;
        case reentry: return instrument->ReEntry.sweepDuration;
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
        default: return 0;
    }
}

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
        case startup: return TransmissionUnitForMode.startup;
        case interim: return TransmissionUnitForMode.interim;
        case science: return TransmissionUnitForMode.science;
        case reentry: return TransmissionUnitForMode.reentry;
        default: return DiagUnit;
    }
}