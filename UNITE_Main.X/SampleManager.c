#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>
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
#include "mcc_generated_files/spi1.h"
#include "mcc_generated_files/uart1.h"


/*****************************
  Data Storage Initializations
 *****************************/
const int RESULTS_SIZE = 16;
int langmuirProbeResults[16];
int magnetometerResults[16];
int temperatureResults[16];

uint16_t langmuirProbeDiagData[5];
uint16_t magnetometerDiagData[3];
double convertedMagDiagData[3];
uint16_t temperatureDiagData[8];
double convertedTempDiagData[8];
double gpsPosition[3];
float gpsVelocity[3];
int gpsError;
uint8_t gpsDatum;
double gpsTime;
double gpsAltitude;


uint8_t langmuirProbeBuffer[300] = {NULL};
uint8_t magnetometerBuffer[30] = {NULL};
uint8_t temperatureBuffer[32] = {NULL};
uint8_t gpsBuffer[48] = {NULL};
uint8_t housekeepingBuffer[28] = {NULL};

char myFunString[29] = "Hey, World! UNITE Rules Now!";

// GPS Properties
uint8_t gpsLockAttempts = 0;
GPSDataIndex gpsIndex = 0;
char unparsedSBFGPSBuffer[200] = {NULL};
char unparsedGGAGPSBuffer[100] = {NULL};
int gpsSBFMessageLength = 0;
int latDecPrecision = 4;
int longDecPrecision = 4;

int currentLangmuirProbeBufferIndex = 0;
int currentLangmuirProbeCalIndex = 0;
int currentMagnetometerBufferIndex = 0;
int currentTemperatureBufferIndex = 0;
int currentGPSBufferIndex = 0;
int currentHousekeepingBufferIndex = 0;

uint16_t LP_BUFFER_SIZE = 300;
uint16_t MAG_BUFFER_SIZE = 30;
uint16_t TMP_BUFFER_SIZE = 32;
uint16_t GPS_BUFFER_SIZE = 48;
uint16_t HOUSE_BUFFER_SIZE = 28;

/******************************
  Instrument Timer Properties
 ******************************/

unsigned long currentLangmuirProbeWait = 0;
unsigned long currentLangmuirProbeCalWait = 0;
unsigned long currentMagnetometerWait = 0;
unsigned long currentTemperatureWait = 0;
unsigned long currentGPSWait = 0;
unsigned long currentHousekeepingWait = 0;

int langmuirProbeCallbackCount = 0;
int magnetometerCallbackCount = 0;

unsigned long currentLangmuirProbeSweepProgress = 0;
unsigned long currentMagnetometerSweepProgress = 0;

int currentMagnetometerOrbitProgress = 0;

bool isLangmuirProbeSampling = false;
bool isLangmuirProbeSweeping = false;

bool isMagnetometerSampling = false;
bool shouldMagnetometerSample = false;

bool isLangmuirCalibrating = false;


bool isProbeVoltagePositive = true;
bool isProbeSweepPositive = true; // Initialize to true to correctly sweep probe halfway through
const uint16_t electronVoltage = 0xE666; // 58982 = 4V
const uint16_t ionVoltage = 0x1999; // 6553 = -4V
const uint16_t maxSweepVoltage = 0xF333; // 62259 = 4.5V
const uint16_t minSweepVoltage = 0x0CCE; // 3277 = -4.5V
bool isGPSSentenceSBF = false;
volatile bool isGPSReadReady = false;
uint16_t currentLangmuirProbeVoltage = 0;
double preciseSweepVoltage = 0.0;

/**************************
  Sampling Configurations
 **************************/

ADCSampleConfig lpADCConfig = {
    0x0038, // AN3, 4, 5                   // adc channel select   
    3 // number of adc channels to sample
};

ADCSampleConfig magADCConfig = {
    0x00C4, // AN2, 6, 7
    3
};

ADCSampleConfig tmpADCConfig = {
    0xFF00, // AN8, 9, A, B, C, D, E, F
    8
};


/*****************************
 Begin/End Instrument Sampling
 *****************************/

// MARK: Begin Methods

void TrySampleLangmuirProbe() {

    if (!isLangmuirProbeSampling) {
        
        time_t sampleRate = GetSampleRate(&LangmuirProbe);
        
        if (sampleRate != 0) { // If sample rate is zero, don't sample
            if (++currentLangmuirProbeWait >= sampleRate) {

                if (isLangmuirProbeOn()) {

                    _LATG9 = 1;

                    // Sample Plasma Probe Data and store in buffer
                    isLangmuirProbeSampling = true;

                    currentLangmuirProbeWait = 0;
                } else {
                    SetLangmuirProbePower(1);
                }
            } // Wait to check MAG before turning switch off
        }
    }
}

void TryCalibrateLangmuirProbe() {
    
    
    if (++currentLangmuirProbeCalWait >= lpSamplesPerCalibration) {
        isLangmuirCalibrating = true; 
        _LATE5 = 1; //EN to 1 when calibrating probe
        
        CalLangmuirProbe();
    }
}

void TrySampleMagnetometer() {

    time_t sampleRate = GetSampleRate(&Magnetometer);
    
    if (sampleRate != 0) {
        if ((++currentMagnetometerWait >= sampleRate) || shouldMagnetometerSample) {

            if (isMagnetometerOn()) {

                // Sample Magnetometer Data and store in buffer

                /*
                 * Magnetometer Orbit Sampling
                 */
                shouldMagnetometerSample = true;
                isMagnetometerSampling = true;
//                TakeMagnetometerSample();

                currentMagnetometerWait = 0;
            } else {
                SetMagnetometerPower(1);
            }
        }
    }
    
    
    // Only turn off Power Switch if both LP and Mag aren't sampling
    time_t lpSampleRate = GetSampleRate(&LangmuirProbe);
    bool shouldLangmuirProbePowerOff = ((currentLangmuirProbeWait < lpSampleRate && !isLangmuirProbeSampling) || lpSampleRate == 0);
    bool shouldMagnetometerPowerOff = ((currentMagnetometerWait < sampleRate && !shouldMagnetometerSample) || sampleRate == 0);
    
    if (shouldLangmuirProbePowerOff && shouldMagnetometerPowerOff)
        SetMagnetometerPower(0);
    
}

void TrySampleTemperature() {

    time_t sampleRate = GetSampleRate(&TemperatureSensors);
    
    if (sampleRate != 0) {
        if (++currentTemperatureWait >= sampleRate) {

            if (isTemperatureOn()) {

                // Sample Temperature Sensor Data and store in buffer
                TakeTemperatureSample();

                currentTemperatureWait = 0;
            } else {
                SetTemperaturePower(1);
            }
        } else SetTemperaturePower(0);
    } else SetTemperaturePower(0);

}

void TrySampleGPS() {

    time_t sampleRate = GetSampleRate(&GPS);
    
    if (sampleRate != 0) {
        if (++currentGPSWait >= sampleRate) {

            if (isGPSOn()) {
                // Sample GPS Data and store in buffer

                // Clear out GPS Buffer
                memset(gpsBuffer, 0, sizeof(gpsBuffer));

                
                if (IS_GPS_INTERRUPT_ENABLED)
                    // Turn on GPS Interrupt
                    IEC0bits.U1RXIE = 1;
                
                else
                    // Read GPS Sentence
                    TakeGPSPollSample();

            } else {
                SetGPSPower(1);
            }

        } else SetGPSPower(0);
    } else SetGPSPower(0);

}

void TrySampleHousekeeping() {

    time_t sampleRate = GetSampleRate(&Housekeeping);
    
    if (sampleRate != 0) {
        if (++currentHousekeepingWait >= sampleRate) {

            TakeHousekeepingSample();
            currentHousekeepingWait = 0;
        }
    }
}

// MARK: End Methods

void EndLangmuirProbeSampling() {
    
    // Set Probe Voltage to 4V 
    currentLangmuirProbeVoltage = electronVoltage;
    uint16_t voltageHigh = (currentLangmuirProbeVoltage & 0xFF00) >> 8;
    uint16_t voltageLow = (currentLangmuirProbeVoltage & 0x00FF) << 8;
    
    _LATG7 = 0;
    SPI1_Exchange16bit(voltageHigh);
    SPI1_Exchange16bit(voltageLow);
    _LATG7 = 1;
    
    // Try Calibrate Probe Tip
    TryCalibrateLangmuirProbe();
    
    currentLangmuirProbeSweepProgress = 0;
    isLangmuirProbeSampling = false;
    isProbeVoltagePositive = true;
    isProbeSweepPositive = true;

    PackageData(LPSubSys, (int)timeInMin, langmuirProbeBuffer, currentLangmuirProbeBufferIndex);

    // Reset Progress Values
    memset(langmuirProbeBuffer, 0, sizeof(langmuirProbeBuffer));
    currentLangmuirProbeBufferIndex = 0;
    currentLangmuirProbeWait = 0;

    // Resize LP Buffer based on sampling duration: Temperature sweep point # + Density sample point #
    LP_BUFFER_SIZE = (convertTime(Sec, MilSec)) / (GetSweepRate(&LangmuirProbe) * 5) + GetSweepDuration(&LangmuirProbe);

    
}

void EndLangmuirProbeCal() {
   isLangmuirCalibrating = false;
   _LATE5 = 0; //EN equal to zero when not calibrating
   _LATG9 = 1; //DEMUX high for sweep
   currentLangmuirProbeCalWait = 0;
}

void EndMagnetometerSampling() {
//    TMR3_Stop();

    PackageData(MAGSubSys, (int) timeInMin, magnetometerBuffer, MAG_BUFFER_SIZE);
    memset(magnetometerBuffer, 0, sizeof (magnetometerBuffer));
    currentMagnetometerBufferIndex = 0;
    currentMagnetometerWait = 0;
//    MAG_BUFFER_SIZE = (GetSweepDuration(&Magnetometer) * convertTime(Sec, MilSec))/(GetSweepRate(&Magnetometer) * 100) * 3;
}

void EndTemperatureSensorSampling() {
    PackageData(TMPSubSys, (int) timeInMin, temperatureBuffer, TMP_BUFFER_SIZE);
    memset(temperatureBuffer, 0, sizeof (temperatureBuffer));
    currentTemperatureBufferIndex = 0;
    currentTemperatureWait = 0;
}

void EndGPSSampling() {

    // Parse GPS sentence
    ParseSBFGPSSample((uint8_t *)unparsedSBFGPSBuffer);
    if (unparsedGGAGPSBuffer[1] != 0) ParseGPSSample((uint8_t *)unparsedGGAGPSBuffer);

    //Only package and send if GPS is locked
    if (currentGPSBufferIndex > 0 /* && !(gpsBuffer[currentGPSBufferIndex - 2] == 0xFF && gpsBuffer[currentGPSBufferIndex - 2] == 0xFF)/*10*/) {
        PackageData(GPSSubSys, (int)timeInMin, gpsBuffer, GPS_BUFFER_SIZE);
        gpsLockAttempts = 0;
    } else gpsLockAttempts++;

    // Reset buffer variables
    memset(unparsedSBFGPSBuffer, 0, sizeof(unparsedSBFGPSBuffer));
    memset(unparsedGGAGPSBuffer, 0, sizeof(unparsedGGAGPSBuffer));
    memset(gpsBuffer, 0, sizeof(gpsBuffer));
    if (gpsLockAttempts == 0) currentGPSWait = 0;
    currentGPSBufferIndex = 0;
    gpsIndex = 0;
    U1STAbits.OERR = 0; // Clear any errors
}

void EndHousekeepingSampling() {
    PackageData(EPSSubSys, (int)timeInMin, housekeepingBuffer, HOUSE_BUFFER_SIZE);
    memset(housekeepingBuffer, 0, sizeof(housekeepingBuffer));
    currentHousekeepingBufferIndex = 0;
    currentHousekeepingWait = 0;
}

/***************************
  Sweeping Progress Manager
 **************************/

void ManageSweepingProgress() {

    // Magnetometer Sampling
    if (isMagnetometerSampling) {
        if (++currentMagnetometerSweepProgress > GetSweepRate(&Magnetometer)) {
            
            // Take sample
            TakeMagnetometerSample();
            
            // Reset sweep rate
            currentMagnetometerSweepProgress = 0;
            
            // Should end Magnetometer Orbit Sampling
            if (currentMagnetometerOrbitProgress > GetSweepDuration(&Magnetometer)) {
                EndMagnetometerSampling();
                shouldMagnetometerSample = false;
                isMagnetometerSampling = false;
                currentMagnetometerOrbitProgress = 0;
            } else {
                // Increment Orbit progress
                currentMagnetometerOrbitProgress += GetSweepRate(&Magnetometer);
            }
        }
    }
    
    // Langmuir Probe Sampling
    if (isLangmuirProbeSampling) {

        // Reset sweeping
        isLangmuirProbeSweeping = false;
        TMR4_Stop();

        // Set correct voltage for sample
        currentLangmuirProbeVoltage = isProbeVoltagePositive ? electronVoltage : ionVoltage;
        isProbeVoltagePositive = !isProbeVoltagePositive;

        // Take one sample
        TakeProbeSample(false);

        // Take a sweep when halfway through the sample
        unsigned long sampleDuration = (GetSweepDuration(&LangmuirProbe) / 2);
        if (currentLangmuirProbeSweepProgress++ == sampleDuration) {
            isLangmuirProbeSweeping = true;
            currentLangmuirProbeVoltage = minSweepVoltage;
            preciseSweepVoltage = (double)minSweepVoltage;
            TMR4_Start();
        }

        if (currentLangmuirProbeSweepProgress >= GetSweepDuration(&LangmuirProbe)) {
            TakeProbeSample(true);
            EndLangmuirProbeSampling();
        }
    }

    
}

/***********************
 Langmuir Probe Calibration 
 ***********************/

void CalLangmuirProbe() {
    Clear(langmuirProbeResults, RESULTS_SIZE,0);
    
    int i = 0;

    for(i = 0; i < 4; i++) {
        switch(i) {
            case 0: //1M Ohm
                _LATE6 = 0;
                _LATE7 = 0;
                ADC1_GetResultFromChannels(langmuirProbeResults, lpADCConfig.channelSelect, lpADCConfig.channelCount);
                break;
                
            case 1: //10M Ohm
                _LATE6 = 0;
                _LATE7 = 1;
                ADC1_GetResultFromChannels(langmuirProbeResults, lpADCConfig.channelSelect, lpADCConfig.channelCount);
                break;
                
            case 2: //100M Ohm
                _LATE6 = 1;
                _LATE7 = 0;
                ADC1_GetResultFromChannels(langmuirProbeResults, lpADCConfig.channelSelect, lpADCConfig.channelCount);
                break;
                
            case 3: //1G Ohm
                _LATE6 = 1;
                _LATE7 = 1;
                ADC1_GetResultFromChannels(langmuirProbeResults, lpADCConfig.channelSelect, lpADCConfig.channelCount);  
                break;
                
            default: break;
        }
        
        int probeResultSize = 1;
        
        // Diagnostic Data -> Calibration Value
        CopyIntToDoubleByte(langmuirProbeResults, langmuirProbeDiagData, LP_VOLTAGE_CHL, i + 1, probeResultSize);
        
//        langmuirProbeResults[LP_VOLTAGE_CHL] /= 4; // Trim to size of a byte
        
//        CopyIntToByte(langmuirProbeResults, langmuirProbeBuffer, LP_VOLTAGE_CHL, currentLangmuirProbeBufferIndex, probeResultSize);
//        currentLangmuirProbeBufferIndex += probeResultSize;
        
        currentLangmuirProbeBufferIndex += CopyIntToByteArray(langmuirProbeResults, langmuirProbeBuffer, LP_VOLTAGE_CHL, currentLangmuirProbeBufferIndex, probeResultSize);

        
    }
    
    EndLangmuirProbeCal();
}

/***********************
  Single Sample Methods
 ***********************/

void TakeProbeSample(bool isTemp) {
    Clear(langmuirProbeResults, RESULTS_SIZE, 0);

    uint16_t voltageHigh = (currentLangmuirProbeVoltage & 0xFF00) >> 8;
    uint16_t voltageLow = (currentLangmuirProbeVoltage & 0x00FF) << 8;
    
    _LATG7 = 0;
    SPI1_Exchange16bit(voltageHigh);
    SPI1_Exchange16bit(voltageLow);
    _LATG7 = 1;

    // Sample Probe from ADC
    ADC1_GetResultFromChannels(langmuirProbeResults, lpADCConfig.channelSelect, lpADCConfig.channelCount);

    // Diagnostic Data -> Temperature Value
    if (isTemp) {
        memset(langmuirProbeDiagData, 0, sizeof(langmuirProbeDiagData));
        CopyIntToDoubleByte(langmuirProbeResults, langmuirProbeDiagData, LP_TEMP_CHL, 0, 1);
    }
    
    // Uncomment to scale data down to a byte
    int i;
    for (i = 0; i < RESULTS_SIZE; i++) {
        langmuirProbeResults[i] /= 4;
    }

    int probeResultSize = 1;
    CopyIntToByte(langmuirProbeResults, langmuirProbeBuffer, isTemp ? LP_TEMP_CHL : LP_VOLTAGE_CHL, currentLangmuirProbeBufferIndex, probeResultSize);
    currentLangmuirProbeBufferIndex += probeResultSize;

//    int probeResultSize = 1;
//    CopyIntToByte(langmuirProbeResults, langmuirProbeBuffer, isTemp ? LP_TEMP_CHL : LP_VOLTAGE_CHL, currentLangmuirProbeBufferIndex, probeResultSize);

    
    // Sweeping Algorithm
    if (isLangmuirProbeSweeping) {

        // Calculation for the voltage adjustment needed to sweep from min voltage to max voltage
        // ~655 for a 1s sweep from -4.5 -> 4.5 -> -4.5
        double voltageAdjustment = 655.36; // maxSweepVoltage * 4 / (convertTime(Sec, MilSec) / GetSweepRate(&LangmuirProbe) / 5) + 1;
        preciseSweepVoltage += isProbeSweepPositive ? voltageAdjustment : -voltageAdjustment;
        
        // Applies the voltage adjustment within range(minSweepVoltage, maxSweepVoltage)
        currentLangmuirProbeVoltage = (uint16_t)preciseSweepVoltage;

        // Checks if the sweep should change from positive to negative
        if (currentLangmuirProbeVoltage >= maxSweepVoltage - (voltageAdjustment / 4)) isProbeSweepPositive = false;
        else if (currentLangmuirProbeVoltage <= minSweepVoltage + (voltageAdjustment / 4)) isProbeSweepPositive = true;
    }
}

void TakeMagnetometerSample() {
    Clear(magnetometerResults, RESULTS_SIZE, 0);
    ADC1_GetResultFromChannels(magnetometerResults, magADCConfig.channelSelect, magADCConfig.channelCount);

    // Diagnostic Data -> X, Y, Z
    memset(magnetometerDiagData, 0, sizeof(magnetometerDiagData));
    CopyIntToDoubleByte(magnetometerResults, magnetometerDiagData, 0, 0, sizeof(magnetometerDiagData));
    
    // Uncomment to scale results down to a byte
    int i;
    for (i = 0; i < RESULTS_SIZE; i++) {
        magnetometerResults[i] = (magnetometerResults[i] - 520) / 2;
    }

    int magnetometerResultSize = 3;
    CopyIntToByte(magnetometerResults, magnetometerBuffer, 0, currentMagnetometerBufferIndex, magnetometerResultSize);
    currentMagnetometerBufferIndex += magnetometerResultSize;

//    int magnetometerResultSize = 3;
//    currentMagnetometerBufferIndex += CopyIntToByteArray(magnetometerResults, magnetometerBuffer, 0, currentMagnetometerBufferIndex, magnetometerResultSize);
    
    if (currentMagnetometerBufferIndex >= MAG_BUFFER_SIZE) {
        EndMagnetometerSampling();
    }


}

void TakeTemperatureSample() {
    Clear(temperatureResults, RESULTS_SIZE, 0);
    ADC1_GetResultFromChannels(temperatureResults, tmpADCConfig.channelSelect, tmpADCConfig.channelCount);

    // Diagnostic Data -> Temp 1-8
    memset(temperatureDiagData, 0, sizeof(temperatureDiagData));
    CopyIntToDoubleByte(temperatureResults, temperatureDiagData, 0, 0, sizeof(temperatureDiagData));
    
    // Uncomment to Scale Results down to a byte
    int i;
    for (i = 0; i < RESULTS_SIZE; i++) {
        temperatureResults[i] = temperatureResults[i] / 4;
    }
    
    int temperatureResultSize = 8;
    CopyIntToByte(temperatureResults, temperatureBuffer, 0, currentTemperatureBufferIndex, temperatureResultSize);
    currentTemperatureBufferIndex += temperatureResultSize;
    
//    int temperatureResultSize = 8;
//    currentTemperatureBufferIndex += CopyIntToByteArray(temperatureResults, temperatureBuffer, 0, currentTemperatureBufferIndex, temperatureResultSize);
    

    if (currentTemperatureBufferIndex >= TMP_BUFFER_SIZE) {
        EndTemperatureSensorSampling();
    }
}

void TakeHousekeepingSample() {
   
    housekeepingBuffer[currentHousekeepingBufferIndex++] = b1Charge >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = b2Charge >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = b1Voltage >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = b2Voltage >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = b1Current >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = b2Current >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = bussPlusVoltage >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = solar1Voltage >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = solar2Voltage >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = solar3Voltage >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = solar4Voltage >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = simplexTemp >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = duplexTemp >> 2;
    housekeepingBuffer[currentHousekeepingBufferIndex++] = epsTemp >> 2;
    
    if (currentHousekeepingBufferIndex >= HOUSE_BUFFER_SIZE) {
        EndHousekeepingSampling();
    }
}

/**********************
  GPS Sampling Methods
 **********************/

// Adds a parsed GPS sentence element to sending buffer
// uint8_t *buffer -> parsed GPS buffer
// int bufferSize -> length of buffer

void AppendToGPSBuffer(uint8_t *buffer, int bufferSize) {

    int i;
    for (i = 0; i < bufferSize; i++) {
        // Makes sure that the data doesn't overrun the buffer
        if ((currentGPSBufferIndex + i) < GPS_BUFFER_SIZE)
            gpsBuffer[(currentGPSBufferIndex + i)] = buffer[i];
    }

    currentGPSBufferIndex += i;
}

// Description: Reads GPS sentence one byte at a time starting from '$'
// int samplePos -> index variable to store in unparsed GPS buffer
// Return: Next index

int TakeGPSSample(int samplePos) {

    char nextChar = Read(GPSUnit);

    if (samplePos == 0) {

        if ('$' == nextChar) {
            unparsedSBFGPSBuffer[samplePos] = nextChar;
            unparsedGGAGPSBuffer[samplePos++] = nextChar;
        }

    } else if (samplePos == 1) {
        
        if ('@' == nextChar) {
            isGPSSentenceSBF = true;
            unparsedSBFGPSBuffer[samplePos++] = nextChar;
        } else {
            isGPSSentenceSBF = false;
            unparsedGGAGPSBuffer[samplePos++] = nextChar;
        }
    } else if (samplePos == 6 && isGPSSentenceSBF) {
        
        gpsSBFMessageLength = nextChar;
        unparsedSBFGPSBuffer[samplePos++] = nextChar;
        
    } else {

        
        if ('\n' == nextChar || ((samplePos == gpsSBFMessageLength) && isGPSSentenceSBF)) {
            samplePos = 0;
            
            if (isGPSSentenceSBF && IS_GPS_INTERRUPT_ENABLED) { 
                                
                // Turn off GPS interrupt
                IEC0bits.U1RXIE = 0;
    
                EndGPSSampling();
            }
            
            return samplePos;
            
        } else if ('$' == nextChar) {
            samplePos = 0;
        } else {

            if (isGPSSentenceSBF) {
                unparsedSBFGPSBuffer[min(samplePos, sizeof (unparsedSBFGPSBuffer))] = nextChar;
                samplePos++;
            } else {
                unparsedGGAGPSBuffer[min(samplePos, sizeof (unparsedGGAGPSBuffer))] = nextChar;
                samplePos++;
            }
        }

    }

    return samplePos % (int)(isGPSSentenceSBF ? sizeof(unparsedSBFGPSBuffer) : sizeof(unparsedGGAGPSBuffer));
}

// GPS Sentence Reader

void TakeGPSPollSample() {
    
    uint8_t unsuccessfulAttempts = 0;
    char nextChar;
    bool isFinished = false;
    
    while (!isFinished) {
        // Get Start Character
        do {
            nextChar = Read(GPSUnit);
        } while (nextChar != '$' && unsuccessfulAttempts++ < 255);

        if (nextChar != '$') return;

        unparsedGGAGPSBuffer[0] = nextChar;
        unparsedSBFGPSBuffer[0] = nextChar;

        // Get Determiner
        nextChar = Read(GPSUnit);

        // Read SBF Sentence
        if (nextChar == '@') {
            unparsedSBFGPSBuffer[1] = nextChar;
            isGPSSentenceSBF = true;

            int i;
            uint16_t messageLength = sizeof (unparsedSBFGPSBuffer);

            // Read Sentence into SBF Buffer
            for (i = 2; i < messageLength; i++) {
            
                unparsedSBFGPSBuffer[i] = Read(GPSUnit);

                // Update message length after reading it
                if (i == 7) {
                    messageLength = (uint16_t) unparsedSBFGPSBuffer[i - 1] + ((uint16_t) unparsedSBFGPSBuffer[i] << 8);
                }
            }

            // Stop Reading GPS
            isFinished = true;
            EndGPSSampling();

           // Read GGA Sentence
        } else if (nextChar == 'G') {
            unparsedGGAGPSBuffer[1] = nextChar;

            int i;
            // Read Sentence into GGA Buffer
            for (i = 2; i < sizeof (unparsedGGAGPSBuffer); i++) {
            
                char cGGA = Read(GPSUnit);
                unparsedGGAGPSBuffer[i] = cGGA;

                if (cGGA == '\n') i = sizeof (unparsedGGAGPSBuffer);
            }
        }
    }
}

// GPS ASCII compression algorithm
// char *string -> ASCII decimal string (without decimal point)
// int startIndex -> index to start at in string
// int size -> length of string
// bool addTrailing -> true: append a trailing zero, false: append a leading zero

int CompressAscii(char *string, int startIndex, int size, bool addTrailing) {

    int k = 0;
    int parsedIndex = 0;

    // Even size string
    if ((size % 2) == 0) {
        while (k < size) {

            if ((k % 2) == 0) {
                string[startIndex + parsedIndex] = string[startIndex + k] << 4;
            } else {
                string[startIndex + parsedIndex++] += (string[startIndex + k] & 0x0F);
            }

            k++;
        }

        return parsedIndex;

    } else {

        while (k < size) {

            if (addTrailing) {
                if ((k % 2) == 0) {
                    string[startIndex + parsedIndex] = string[startIndex + k] << 4;
                } else {
                    string[startIndex + parsedIndex++] += (string[startIndex + k] & 0x0F);
                }
            } else {
                if (k == 0) {
                    string[startIndex + parsedIndex++] &= 0x0F;
                } else if ((k % 2) != 0) {
                    string[startIndex + parsedIndex] = string[startIndex + k] << 4;
                } else {
                    string[startIndex + parsedIndex++] += (string[startIndex + k] & 0x0F);
                }
            }

            k++;
        }

        return parsedIndex;
    }
}
    
// Appends a double value to the GPS buffer 
// double value -> value to append
void AppendDoubleToGPSBuffer(long double value) {
    
    union {
        long double d;
        char bytes[sizeof(long double)];
    } u;
    
    u.d = value;
    
//    uint8_t bytes[8];
//    
//    memcpy(bytes, &value, sizeof(double));
    
    AppendToGPSBuffer(u.bytes, sizeof(long double));
}

// Appends a float value to the GPS buffer
// float value -> value to append
void AppendFloatToGPSBuffer(float value) {
    
    union {
        float f;
        char bytes[sizeof(float)];
    } u;
    
    u.f = value;
//    uint8_t bytes[4] = { NULL };
//    
//    memcpy(bytes, &value, sizeof(bytes));
    
    AppendToGPSBuffer(u.bytes, sizeof(float));
}

// Gets length of next GPS sentence subsection
// char *src -> original ascii GPS sentence
// int start -> index to start reading from
// Returns length of substring between commas

int GetSubStringLength(char *src, int start) {

    int i = 0;
    while (src[i + start] != ',') i++;

    return i;
}

// Copies a substring of src to dest
// char *src -> original ascii GPS sentence
// char *dest -> destination string
// int start -> index to start copying from
// int length -> length of substring to copy
// Returns number of characters copied

int CopySubstring(char *src, char *dest, int start, int length) {

    int i;
    for (i = 0; i < length; i++) {
        dest[i] = src[start + i];
    }

    return i;
}

double ParseDouble(char *string, int length) {

    int progress;
    int numArray[length - 1];
    int decIndex;
    double numberValue;

    for (progress = 0; progress < length; progress++) {
        char next = string[progress];

        if (next != '.' && next != ',') numArray[progress] = next & 0x0F;
        else if (next == '.') decIndex = progress;
        else if (next == ',') progress = length;
    }

    // Calculate tens offset of decimal place
    for (progress = 0; progress < sizeof (numArray); progress++) {
        int exponent = decIndex - 1 - progress; // Linear relation of decIndex to ten's place

        if (progress < decIndex) numberValue += (numArray[progress] * pow(10.0, (double)exponent));
        if (progress >= decIndex) numberValue += (double) (numArray[progress]) / (double)(pow(10.0, (exponent > 0) ? (double)(exponent) : (double)(-exponent)));
    }

    return numberValue;
}


int AsciiFromInt(int integer, char *dest, int length) {

    int mod = integer;
    double exponent = length - 1;
    int i = 0;

    while (mod > 0 && exponent >= 0) {
        dest[i++] = (char)((int)(mod / pow(10.0, exponent)));
        mod %= (int)(pow(10.0, exponent--));
    }

    return i;
}

// Returns a double value from a substring of a GPS sentence
// char *src -> source string
// int start -> start index for substring
// int length -> length of substring

double GetDoubleFromSubString(char *src, int start, int length) {

    char subString[length];
    CopySubstring(src, subString, start, length);
    double value = ParseDouble(subString, length);
    return value;
}

// Returns a double value from a byte array
// uint8_t *src -> source array
// int offset -> double starting index in source
double GetDoubleFromByteArray(uint8_t *src, int offset) {
    
    int i;
    uint64_t v = 0;
    long double d;
    
    for (i = 7; i >= 0; i--) {
        v = (v << 8) + src[i + offset];
    }
    
    memcpy(&d, &v, sizeof(d));
    
    return d;
}

// Returns a float value from a byte array
// uint8_t *src -> source array
// int start -> float starting index in array
float GetFloatFromByteArray(uint8_t *src, int start) {
    
    int i;
    uint32_t v = 0;
    float f;
    
    for (i = 3; i >= 0; i--) {
        v = (v << 8) + src[i + start];
    }
    
    memcpy(&f, &v, sizeof(f));
    
    return f;
}

// Compresses and appends an integer value to GPS buffer
// int value -> integer to append
// int length -> number of digits to append (e.g. 12345 has 5 digits)

void AppendIntToGPSBuffer(char *ascii, int value, int length) {

    // Convert to an ASCII value
    AsciiFromInt(value, ascii, length);
    // Compress ASCII string
    int compSize = CompressAscii(ascii, 0, length, false);

    AppendToGPSBuffer((uint8_t *) ascii, compSize);
}

// Main GPS parsing method
// char *unparsedSentence -> ASCII GPS sentence to parse (GPGGA)

 void ParseGPSSample(char *unparsedSentence) {

    int i;
    char header[] = "$GPGGA";
    char *sentenceHeader;

    for (i = 0; i < 100; i++) {
        if (unparsedSentence[i] == ',') {
            gpsIndex++;
        } else {

            if (gpsIndex == Header) {

                // Check if correct header
                sentenceHeader = strstr(unparsedSentence, header);
                if (sentenceHeader == NULL) i = 74;
                else i += (strlen(header) - 1);

            } else if (gpsIndex == Time) {

                int length = GetSubStringLength(unparsedSentence, i);

                if (length > 0) {

                    gpsTime = GetDoubleFromSubString(unparsedSentence, i, length);
                    
                    // Set satellite's current time
                    SetTime(gpsTime);
                }

                i += length - 1;

            } else if (gpsIndex == Altitude) {

                int length = GetSubStringLength(unparsedSentence, i);

                if (length > 0) {

                    //Get altitude value from GPS substring
                    gpsAltitude = GetDoubleFromSubString(unparsedSentence, i, length);
                    gpsAltitude /= 1000.0; // m -> km

                    
                    // Set Satellite altitude
                    SetAltitude(gpsAltitude);

                }

                i += length - 1;

                
            }
        }
    }
}
 
// SVT GPS parsing method
// char *unparsedSentence -> PVTCartesian GPS sentence to parse (SBF)
 void ParseSBFGPSSample(uint8_t *unparsedSentence) {
     
     // Local variables
     int i;
     char syncBytes[] = "$@";
     uint16_t messageLength = unparsedSentence[6] + ((uint16_t)unparsedSentence[7] << 8);
    
     int error;
     uint8_t time[2] = {NULL};
     uint8_t datum;
    
     long double xPos;
     long double yPos;
     long double zPos;
     
     float xVel;
     float yVel;
     float zVel;
     
     // Check if sentence header matches expected output
     char sentenceHeader[3] = {(char)unparsedSentence[0], (char)unparsedSentence[1]};
     if (strcmp(sentenceHeader, syncBytes) != 0) return;
     
     // Parse Message Body
     for (i = 8; i < messageLength; i++) {
         switch (i) {
             case 9: // Time
                 time[1] = unparsedSentence[i];
                 time[0] = unparsedSentence[i+1];
             case 15: // error code
                 error = unparsedSentence[i];
                 break;
             case 16: // x position
                 xPos = GetDoubleFromByteArray(unparsedSentence, i);
                 break; 
             case 24: // y position
                 yPos = GetDoubleFromByteArray(unparsedSentence, i);
                 break;
             case 32: // z position
                 zPos = GetDoubleFromByteArray(unparsedSentence, i);
                 break;
             
             case 44: // x velocity
                 xVel = GetFloatFromByteArray(unparsedSentence, i);
                 break;
             case 48: // y velocity
                 yVel = GetFloatFromByteArray(unparsedSentence, i);
                 break;
             case 52: // z velocity
                 zVel = GetFloatFromByteArray(unparsedSentence, i);
             case 73: // Datum
                 datum = unparsedSentence[i];
            // ADD NUMBER OF CONNECTED SATELLITES
             default: break;
         }
     }
     
    uint8_t gpsComponents[4] = { datum, (uint8_t)error, sizeof(xPos), sizeof(xVel) }; 
    uint8_t gpsFiller[5] = { NULL };
    
     // Diagnostic Data
     gpsPosition[0] = xPos;
     gpsPosition[1] = yPos;
     gpsPosition[2] = zPos;
     
     gpsVelocity[0] = xVel;
     gpsVelocity[1] = yVel;
     gpsVelocity[2] = zVel;
     
     gpsError = error;
     gpsDatum = datum;
     
     // Append to GPS Buffer 
     
     // GPS Packet 1
     AppendDoubleToGPSBuffer(xPos);
     AppendDoubleToGPSBuffer(yPos);
     AppendDoubleToGPSBuffer(zPos);
     
     AppendToGPSBuffer(time, 2);
     AppendToGPSBuffer(gpsComponents, 4);
     AppendToGPSBuffer(gpsFiller, 5);
     
     // GPS Packet 2
     AppendFloatToGPSBuffer(xVel);
     AppendFloatToGPSBuffer(yVel);
     AppendFloatToGPSBuffer(zVel);
     
     AppendToGPSBuffer( time, 2);
     AppendToGPSBuffer( gpsComponents, 2);
 }


uint8_t spiTesting = 0x00;
int spi16Testing = 0x9C40;

void TestDACSPI() {

    uint16_t high = (spi16Testing & 0xFF00) >> 8;
    uint16_t low = (spi16Testing & 0x00FF) << 8;
    
    _LATG9 = 1;
    _LATG7 = 0;

    //    SPI1_Exchange8bit(spiTesting++);
    
    SPI1_Exchange16bit(high);
    SPI1_Exchange16bit(low);

    //    SPI1_Exchange16bit(spi16Testing);
    //    SPI1_Exchange16bit(currentLangmuirProbeSweepPosition);

    _LATG7 = 1;
    
//    spi16Testing += 655;
}

uint16_t channel = 0xFF00; //FF00, 00C4
int count = 8;
int res[16];

void TestADC() {

    ADC1_GetResultFromChannels(res, channel, count);

    int i;
    for (i = 0; i < count; i++) {
        Send(res[i] / 4, DiagUnit);
    }
}
