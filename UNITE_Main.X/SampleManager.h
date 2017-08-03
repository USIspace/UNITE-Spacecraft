/* 
 * File:   SampleManager.h
 * Author: zacksnyder
 *
 * Created on May 11, 2017, 10:00 AM
 */

#ifndef SAMPLEMANAGER_H
#define	SAMPLEMANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* SAMPLEMANAGER_H */

// Defines types of GPS sentence data
typedef enum {
    Header,
    Time,
    Latitude,
    LatDirection,
    Longitude,
    LongDirection,
    Fix,
    Satellites,
    Elevation,
    Altitude,
    AltUnit,
    Done
} GPSDataIndex;

// Struct to store ADC sampling properties
typedef struct {
    uint16_t channelSelect;
    int channelCount;
} ADCSampleConfig;

// Holds the raw data array sizes based on the instrument's sampling configs
extern uint16_t LP_BUFFER_SIZE;
extern uint16_t MAG_BUFFER_SIZE;

/* State Variables */

// Sampling wait timers for each instrument
extern unsigned long currentLangmuirProbeWait;
extern unsigned long currentMagnetometerWait;
extern unsigned long currentTemperatureWait;
extern unsigned long currentGPSWait;

// Used by sweeping timer
extern int langmuirProbeCallbackCount;
extern int magnetometerCallbackCount;

// Sampling progress
extern unsigned long currentLangmuirProbeSweepProgress;
extern unsigned long currentMagnetometerSweepProgress;

// Sampling state
extern bool isLangmuirProbeSampling;
extern bool isMagnetometerSweeping;
extern bool shouldMagnetometerSample;

// Did GPS obtain lock for last sample
extern bool isGPSLocked;


/* Sampling Functions */

// Start sampling instruments
void BeginTemperatureSampling();
void BeginGPSSampling();
void BeginMagnetometerSampling();
void BeginLangmuirProbeSampling();

// Stop sampling instruments
void EndLangmuirProbeSampling();
void EndMagnetometerSampling();
void EndTemperatureSensorSampling();
void EndGPSSampling();

// Manages the Sweeping progress of an instrument (called by Timer 1)
void ManageSweepingProgress();

// Takes an individual sample of each instrument
void TakeProbeSample();
void TakeMagnetometerSample();
void TakeTemperatureSample();
int TakeGPSSample(int);

// Parses a GPS sentence
void ParseGPSSample();

// Testing/Debugging Functions
void TransmitTestString();
void TestDACSPI();