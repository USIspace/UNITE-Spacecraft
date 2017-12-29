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


#define LP_VOLTAGE_CHL 1
#define LP_TEMP_CHL 2

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
extern unsigned long currentLangmuirProbeCalWait;
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

/* Diagnostic Data */
extern uint16_t langmuirProbeDiagData[5];
extern uint16_t magnetometerDiagData[3];
extern uint16_t temperatureDiagData[8];
extern double gpsPosition[3];
extern float gpsVelocity[3];
extern int gpsError;
extern uint8_t gpsDatum;
extern double gpsTime;
extern double gpsAltitude;

/* Sampling Functions */

// Start sampling instruments
void TrySampleLangmuirProbe();
void TrySampleMagnetometer();
void TrySampleTemperature();
void TrySampleGPS();
void TrySampleHousekeeping();

// Stop sampling instruments
void EndLangmuirProbeSampling();
void EndMagnetometerSampling();
void EndTemperatureSensorSampling();
void EndGPSSampling();
void EndHousekeepingSampling();

// Manages the Sweeping progress of an instrument (called by Timer 1)
void ManageSweepingProgress();

// Takes an individual sample of each instrument
void TakeProbeSample(bool isTemp);
void CalLangmuirProbe();
void TakeMagnetometerSample();
void TakeTemperatureSample();
void TakeHousekeepingSample();
int TakeGPSSample(int);

// Parses a GPS sentence
void ParseGPSSample();
void ParseSBFGPSSample(uint8_t *);

// Testing/Debugging Functions
void TestADC();
void TestDACSPI();