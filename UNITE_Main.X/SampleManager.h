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

typedef struct {
    uint16_t channelSelect;
    int channelCount;
} ADCSampleConfig;

//extern ADCSampleConfig lpADCConfig;
//extern ADCSampleConfig magADCConfig;
//extern ADCSampleConfig tmpADCConfig;

// State Variables
extern int currentLangmuirProbeWait;
extern int currentMagnetometerWait;
extern int currentTemperatureWait;
extern int currentGPSWait;

extern int currentLangmuirProbeSweepProgress;
extern int currentMagnetometerSweepProgress;

extern bool isLangmuirProbeSweeping;
extern bool isMagnetometerSweeping;

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

void TakeTemperatureSample();
void TakeGPSSample();
void TakeMagnetometerSample();
void TakeProbeSample();