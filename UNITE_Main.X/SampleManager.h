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

extern bool shouldSample;
extern int results[16];

extern int currentLangmuirProbeWait;
extern int currentMagnetometerWait;
extern int currentTemperatureWait;
extern int currentGPSWait;

void BeginTemperatureSampling();
void BeginGPSSampling();
void BeginMagnetometerSampling();
void BeginLangmuirProbeSampling();

void TakeTemperatureSample();
void TakeGPSSample();
void TakeMagnetometerSample();
void TakeProbeSample();