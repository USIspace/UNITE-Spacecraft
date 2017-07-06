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

typedef struct {
    uint16_t channelSelect;
    int channelCount;
} ADCSampleConfig;

extern uint16_t LP_BUFFER_SIZE;
extern uint16_t MAG_BUFFER_SIZE;

// State Variables
extern int currentLangmuirProbeWait;
extern int currentMagnetometerWait;
extern int currentTemperatureWait;
extern int currentGPSWait;

extern int langmuirProbeCallbackCount;
extern int magnetometerCallbackCount;

extern int currentLangmuirProbeSweepProgress;
extern int currentMagnetometerSweepProgress;

extern bool isLangmuirProbeSweeping;
extern bool isMagnetometerSweeping;
extern bool shouldMagnetometerSample;

// Sampling Functions
void BeginTemperatureSampling();
void BeginGPSSampling();
void BeginMagnetometerSampling();
void BeginLangmuirProbeSampling();

void EndLangmuirProbeSampling();
void EndMagnetometerSampling();
void EndTemperatureSensorSampling();
void EndGPSSampling();

void ManageSweepingProgress();

void TakeProbeSample();
void TakeMagnetometerSample();
void TakeTemperatureSample();
int TakeGPSSample(int);

void ParseGPSSample();

// Testing Functions
void TransmitTestString();
void TestDACSPI();