#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <stdio.h>
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "CommandParser.h"
#include "SystemConfiguration.h"
#include "SampleManager.h"
#include "SatelliteMode.h"
#include "TransmitManager.h"
#include "adc1.h"
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/tmr2.h"
#include "mcc_generated_files/tmr3.h"
#include "mcc_generated_files/tmr4.h"


/*****************************
  Data Storage Initializations
 *****************************/
const int RESULTS_SIZE = 16;
int langmuirProbeResults[16];
int magnetometerResults[16];
int temperatureResults[16];

uint8_t langmuirProbeBuffer[200];
uint8_t magnetometerBuffer[600];
uint8_t temperatureBuffer[32];
uint8_t gpsBuffer[20];

int currentLangmuirProbeBufferIndex = 0;
int currentMagnetometerBufferIndex = 0;
int currentTemperatureBufferIndex = 0;
int currentGPSBufferIndex = 0;

const uint16_t LP_BUFFER_SIZE = 200;
const uint16_t MAG_BUFFER_SIZE = 600;
const uint16_t TMP_BUFFER_SIZE = 32;
const uint16_t GPS_BUFFER_SIZE = 20;

/******************************
  Instrument Timer Properties
 ******************************/

int currentLangmuirProbeWait = 0;
int currentMagnetometerWait = 0;
int currentTemperatureWait = 0;
int currentGPSWait = 0;

int currentLangmuirProbeSweepProgress = 0;
int currentMagnetometerSweepProgress = 0;

bool isLangmuirProbeSweeping = false;
bool isMagnetometerSweeping = false;

/**************************
  Sampling Configurations
 **************************/

ADCSampleConfig lpADCConfig = {
    0,   // adc channel to start at   
    0    // number of adc channels to sample
};

ADCSampleConfig magADCConfig = {
    0x00C4,      // AN2, 6, 7
    3
};

ADCSampleConfig tmpADCConfig = {
    0xFF00,      // AN8, 9, A, B, C, D, E, F
    8
};


/*****************************
 Begin/End Instrument Sampling
 *****************************/

// MARK: Begin Methods

void BeginLangmuirProbeSampling() {
    // Sample Plasma Probe Data and store in buffer
    isLangmuirProbeSweeping = true;
    TMR2_Start();
}

void BeginMagnetometerSampling() {
    // Sample Magnetometer Data and store in buffer
    _LATE3 = LED_ON;

    isMagnetometerSweeping = true;
    TMR3_Start();                           // TMR3 samples every 100 ms and stores in results
}

void BeginTemperatureSampling() {
    _LATE4 = LED_ON;
    TakeTemperatureSample();
    _LATE4 = LED_OFF;
}

void BeginGPSSampling() {
    // Sample GPS Data and store in buffer
}

// MARK: End Methods

void EndLangmuirProbeSampling() {
    TMR2_Stop();
    currentLangmuirProbeSweepProgress = 0;
    isLangmuirProbeSweeping = false;
    
//    PackageData(LP, GetDayTimeInMin(totalTime), langmuirProbeBuffer, LP_BUFFER_SIZE);
    currentLangmuirProbeBufferIndex = 0;
    currentLangmuirProbeWait = 0;
}

void EndMagnetometerSampling() {
    TMR3_Stop();
    currentMagnetometerSweepProgress = 0;
    isMagnetometerSweeping = false;
    
    PackageData(MAG, GetDayTimeInMin(totalTime), magnetometerBuffer, MAG_BUFFER_SIZE);
    currentMagnetometerBufferIndex = 0;
    currentMagnetometerWait = 0;
    _LATE3 = LED_OFF;
}

void EndTemperatureSensorSampling() {
    PackageData(TMP, GetDayTimeInMin(totalTime), temperatureBuffer, TMP_BUFFER_SIZE);
    currentTemperatureBufferIndex = 0;
    currentTemperatureWait = 0;
    _LATE4 = LED_OFF;
}

void EndGPSSampling() {
    
}

/***************************
  Sweeping Progress Manager
 **************************/

void ManageSweepingProgress() {
    
    if (isLangmuirProbeSweeping) {
        if (++currentLangmuirProbeSweepProgress++ > GetSweepDuration(&LangmuirProbe)) {
            EndLangmuirProbeSampling();
        }
    }
    
    if (isMagnetometerSweeping) {
        if (++currentMagnetometerSweepProgress > GetSweepDuration(&Magnetometer)) {
            EndMagnetometerSampling();
        }
    }
}
/***********************
  Single Sample Methods
 ***********************/

void TakeProbeSample() {
    Clear(langmuirProbeResults, RESULTS_SIZE, 0);
    ADC1_GetResultFromChannels(langmuirProbeResults, lpADCConfig.channelSelect, lpADCConfig.channelCount);
    
    
    
    int probeResultSize = 1;
    if (currentLangmuirProbeBufferIndex < LP_BUFFER_SIZE) {
        Copy(langmuirProbeResults, langmuirProbeBuffer, 0, currentLangmuirProbeBufferIndex, probeResultSize);
        currentLangmuirProbeBufferIndex = currentLangmuirProbeBufferIndex + probeResultSize;
    } else {
        EndLangmuirProbeSampling();
    }
}

void TakeMagnetometerSample() {
    Clear(magnetometerResults, RESULTS_SIZE, 0);
    ADC1_GetResultFromChannels(magnetometerResults, magADCConfig.channelSelect, magADCConfig.channelCount);
    
    // Scale Results down to a byte
    int i;
    for (i=0; i < RESULTS_SIZE; i++) {
        magnetometerResults[i] = (magnetometerResults[i] - 520) / 2;
    }
    
    int magnetometerResultSize = 3;
    if (currentMagnetometerBufferIndex < MAG_BUFFER_SIZE) {
        Copy(magnetometerResults, magnetometerBuffer, 0, currentMagnetometerBufferIndex, magnetometerResultSize);
        currentMagnetometerBufferIndex = currentMagnetometerBufferIndex + magnetometerResultSize;
    } else {
        EndMagnetometerSampling();
    }
}

void TakeTemperatureSample() {
    Clear(temperatureResults, RESULTS_SIZE, 0);
    ADC1_GetResultFromChannels(temperatureResults, tmpADCConfig.channelSelect, tmpADCConfig.channelCount);
        
    // Scale Results down to a byte
    int i;
    for (i=0; i < RESULTS_SIZE; i++) {
        temperatureResults[i] = temperatureResults[i] / 4;
    }
    
    int temperatureResultSize = 8;
    if (currentTemperatureBufferIndex < TMP_BUFFER_SIZE) {
        Copy(temperatureResults, temperatureBuffer, 0, currentTemperatureBufferIndex, temperatureResultSize);
        currentTemperatureBufferIndex = currentTemperatureBufferIndex + temperatureResultSize;
    } else {
        EndTemperatureSensorSampling();
    }
}

void TakeGPSSample() {

}



