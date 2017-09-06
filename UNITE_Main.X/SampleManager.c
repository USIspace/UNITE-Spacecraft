#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


/*****************************
  Data Storage Initializations
 *****************************/
const int RESULTS_SIZE = 16;
int langmuirProbeResults[16];
int magnetometerResults[16];
int temperatureResults[16];

uint8_t langmuirProbeBuffer[300] = { NULL };
uint8_t magnetometerBuffer[33] = { NULL };
uint8_t temperatureBuffer[32] = { NULL };
uint8_t gpsBuffer[33] = { NULL };

char myFunString[29] = "Hey, World! UNITE Rules Now!";

// GPS Properties
bool isGPSReading = false;
bool isGPSLocked = true;
GPSDataIndex gpsIndex = 0;
char unparsedGPSBuffer[74] = {NULL};
char gpsTimeBuffer[10];
char gpsLatitudeBuffer[20];
char gpsLatDirection[1];
char gpsLongitudeBuffer[20];
char gpsLongDirection[1];
char gpsAltitudeBuffer[10];
char gpsAltUnit[1];

int currentLangmuirProbeBufferIndex = 0;
int currentMagnetometerBufferIndex = 0;
int currentTemperatureBufferIndex = 0;
int currentGPSBufferIndex = 0;

uint16_t LP_BUFFER_SIZE = 300;
uint16_t MAG_BUFFER_SIZE = 33;
uint16_t TMP_BUFFER_SIZE = 32;
uint16_t GPS_BUFFER_SIZE = 33;

/******************************
  Instrument Timer Properties
 ******************************/

unsigned long currentLangmuirProbeWait = 0;
unsigned long currentMagnetometerWait = 0;
unsigned long currentTemperatureWait = 0;
unsigned long currentGPSWait = 0;

int langmuirProbeCallbackCount = 0;
int magnetometerCallbackCount = 0;

unsigned long currentLangmuirProbeSweepProgress = 0;
unsigned long currentMagnetometerSweepProgress = 0;

int currentMagnetometerOrbitProgress = 0;

bool isLangmuirProbeSampling = false;
bool isLangmuirProbeSweeping = false;

bool isMagnetometerSweeping = false;
bool shouldMagnetometerSample = false;

bool isProbeVoltagePositive = true;
bool isProbeSweepPositive = true;       // Initialize to true to correctly sweep probe halfway through
const uint16_t electronVoltage = 0xE666;     // 58982 = 4V
const uint16_t ionVoltage = 0x1999;          // 6553 = -4V
const uint16_t maxSweepVoltage = 0xF333;     // 62259 = 4.5V
const uint16_t minSweepVoltage = 0x0CCC;     // 3276 = -4.5V
int currentLangmuirProbeVoltage = 0;

/**************************
  Sampling Configurations
 **************************/

ADCSampleConfig lpADCConfig = {
    0x0038, // AN3, 4, 5                   // adc channel select   
    3       // number of adc channels to sample
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

void BeginLangmuirProbeSampling() {
    // Sample Plasma Probe Data and store in buffer
        isLangmuirProbeSampling = true;
}

void BeginMagnetometerSampling() {
    // Sample Magnetometer Data and store in buffer

    /* 
     * Magnetometer Sweeping Algorithm
     */
    //    isMagnetometerSweeping = true;
    //    TMR3_Start();                  // TMR3 samples every 100 ms and stores in results

    /*
     * Magnetometer Orbit Sampling
     */
    shouldMagnetometerSample = true;
    TakeMagnetometerSample();
}

void BeginTemperatureSampling() {
    // Sample Temperature Sensor Data and store in buffer
    TakeTemperatureSample();
}

void BeginGPSSampling() {
    // Sample GPS Data and store in buffer

    // Clear out GPS Buffer
    memset(gpsBuffer, 0, sizeof (gpsBuffer));

    // Turn on GPS Interrupt
    IEC0bits.U1RXIE = 1;

}

// MARK: End Methods

void EndLangmuirProbeSampling() {
    currentLangmuirProbeSweepProgress = 0;
    isLangmuirProbeSampling = false;
    isProbeVoltagePositive = false;
    
    //    PackageData(LPSubSys, (int)timeInMin, langmuirProbeBuffer, LP_BUFFER_SIZE);
    memset(langmuirProbeBuffer, 0, sizeof(langmuirProbeBuffer));
    currentLangmuirProbeBufferIndex = 0;
    currentLangmuirProbeWait = 0;
    
    // Resize LP Buffer based on sampling duration: Temperature sweep point # + Density sample point #
    LP_BUFFER_SIZE = (convertTime(Sec, MilSec)) / (GetSweepRate(&LangmuirProbe) * 5) + GetSweepDuration(&LangmuirProbe);

}

void EndMagnetometerSampling() {
    TMR3_Stop();
    currentMagnetometerSweepProgress = 0;
    isMagnetometerSweeping = false;

    PackageData(MAGSubSys, (int) timeInMin, magnetometerBuffer, MAG_BUFFER_SIZE);
    memset(magnetometerBuffer, 0, sizeof(magnetometerBuffer));
    currentMagnetometerBufferIndex = 0;
    currentMagnetometerWait = 0;
    //    MAG_BUFFER_SIZE = (GetSweepDuration(&Magnetometer) * convertTime(Sec, MilSec))/(GetSweepRate(&Magnetometer) * 100) * 3;

}

void EndTemperatureSensorSampling() {
    PackageData(TMPSubSys, (int) timeInMin, temperatureBuffer, TMP_BUFFER_SIZE);
    memset(temperatureBuffer, 0, sizeof(temperatureBuffer));
    currentTemperatureBufferIndex = 0;
    currentTemperatureWait = 0;
}

void EndGPSSampling() {

    // Turn off GPS interrupt
    IEC0bits.U1RXIE = 0;

    // Parse GPS sentence
    ParseGPSSample(unparsedGPSBuffer);

    //Only package and send if GPS is locked
    if (currentGPSBufferIndex > 20) {
        PackageData(GPSSubSys, (int) timeInMin, gpsBuffer, GPS_BUFFER_SIZE);
        isGPSLocked = true;
    }
    else isGPSLocked = false;
    
    memset(gpsBuffer, 0, sizeof(gpsBuffer));
    currentGPSBufferIndex = 0;
    currentGPSWait = 0;
    gpsIndex = 0;
}

/***************************
  Sweeping Progress Manager
 **************************/

void ManageSweepingProgress() {

    if (isLangmuirProbeSampling) {
        
        // Reset sweeping
        isLangmuirProbeSweeping = false;
        TMR2_Stop();
        
        // Set correct voltage for sample
        currentLangmuirProbeVoltage = isProbeVoltagePositive ? electronVoltage : ionVoltage;
        isProbeVoltagePositive = !isProbeVoltagePositive;
        
        // Take one sample
        TakeProbeSample();
        
        // Take a sweep when halfway through the sample
        if (++currentLangmuirProbeSweepProgress == GetSweepDuration(&LangmuirProbe) / 2) {
            isLangmuirProbeSweeping = true;
            currentLangmuirProbeVoltage = minSweepVoltage;
            TMR2_Start();
        }
        
        if (currentLangmuirProbeSweepProgress >= GetSweepDuration(&LangmuirProbe)) {
            EndLangmuirProbeSampling();
        }
    }

    /*
    if (isMagnetometerSweeping) {
        if (++currentMagnetometerSweepProgress > GetSweepDuration(&Magnetometer)) {
            EndMagnetometerSampling();
        }
    }*/
}

/***********************
  Single Sample Methods
 ***********************/

void TakeProbeSample() {
    Clear(langmuirProbeResults, RESULTS_SIZE, 0);
    
    _RG9 = 0;
    SPI1_Exchange16bit(currentLangmuirProbeVoltage & 0xFFFF);
    _RG9 = 1;

    // Sample Probe from ADC
    ADC1_GetResultFromChannels(langmuirProbeResults, lpADCConfig.channelSelect, lpADCConfig.channelCount);

    // Manipulate Data Here


    int probeResultSize = 3;
    CopyIntToByte(langmuirProbeResults, langmuirProbeBuffer, 0, currentLangmuirProbeBufferIndex, probeResultSize);
    currentLangmuirProbeBufferIndex += probeResultSize;

    // Sweeping Algorithm
    if (isLangmuirProbeSweeping) {
        
        // Calculation for the voltage adjustment needed to sweep from min voltage to max voltage
        // ~590 for a 1s sweep from -4.5 -> 4.5 -> -4.5
        int voltageAdjustment = maxSweepVoltage * 4 / 
                                    (convertTime(Sec,MilSec) / 
                                        GetSweepRate(&LangmuirProbe) / 5) + 1; 
        
        // Applies the voltage adjustment within range(minSweepVoltage, maxSweepVoltage)
        currentLangmuirProbeVoltage = max(minSweepVoltage, 
                                          min(maxSweepVoltage, 
                                                currentLangmuirProbeVoltage + 
                                                isProbeSweepPositive ? voltageAdjustment : -voltageAdjustment));

        // Checks if the sweep should change from positive to negative
        if (currentLangmuirProbeVoltage == maxSweepVoltage) isProbeSweepPositive = false;
        else if (currentLangmuirProbeVoltage == minSweepVoltage) isProbeSweepPositive = true;
    }
}

void TakeMagnetometerSample() {
    Clear(magnetometerResults, RESULTS_SIZE, 0);
    ADC1_GetResultFromChannels(magnetometerResults, magADCConfig.channelSelect, magADCConfig.channelCount);

    // Scale Results down to a byte
    int i;
    for (i = 0; i < RESULTS_SIZE; i++) {
        magnetometerResults[i] = (magnetometerResults[i] - 520) / 2;
    }

    int magnetometerResultSize = 3;
    CopyIntToByte(magnetometerResults, magnetometerBuffer, 0, currentMagnetometerBufferIndex, magnetometerResultSize);
    currentMagnetometerBufferIndex += magnetometerResultSize;

    if (currentMagnetometerBufferIndex >= MAG_BUFFER_SIZE) {
        EndMagnetometerSampling();

        if (currentMagnetometerOrbitProgress++ > 9) {
            shouldMagnetometerSample = false;
            currentMagnetometerOrbitProgress = 0;
        }
    }


}

void TakeTemperatureSample() {
    Clear(temperatureResults, RESULTS_SIZE, 0);
    ADC1_GetResultFromChannels(temperatureResults, tmpADCConfig.channelSelect, tmpADCConfig.channelCount);

    // Scale Results down to a byte
    int i;
    for (i = 0; i < RESULTS_SIZE; i++) {
        temperatureResults[i] = temperatureResults[i] / 4;
    }

    int temperatureResultSize = 8;
    CopyIntToByte(temperatureResults, temperatureBuffer, 0, currentTemperatureBufferIndex, temperatureResultSize);
    currentTemperatureBufferIndex += temperatureResultSize;

    if (currentTemperatureBufferIndex >= TMP_BUFFER_SIZE) {
        EndTemperatureSensorSampling();
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

    if (samplePos == -1) {

        if ('$' == nextChar) {
            samplePos++;
            unparsedGPSBuffer[samplePos++] = nextChar;
        }

    } else {

        if ('$' == nextChar) samplePos = 0;
        else
            if (('\n' == nextChar) || (samplePos == 74)) {
            samplePos = -2;
            isGPSReading = false;
            EndGPSSampling();
        }

        unparsedGPSBuffer[samplePos++] = nextChar;

    }

    return samplePos;
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

// Custom GPS decimal parser
// char *originalString -> original ascii decimal string to parse (must be comma separated)
// int startIndex -> index to start parsing from
// GPSDataIndex dataType -> checks what kind of decimal 

uint8_t ParseDecimal(char *originalString, int startIndex, GPSDataIndex dataType) {
    char semiparsedSentence[20] = {NULL};
    bool isDecimal = false;
    int parseProgress = 0;
    int copiedIndex = 0;
    int decPos = 0;

    while (originalString[startIndex + parseProgress] != ',') {
        if (originalString[startIndex + parseProgress] == '.') {
            decPos = CompressAscii(semiparsedSentence, decPos, parseProgress, isDecimal);
            copiedIndex = decPos;
            isDecimal = true;
        }

        semiparsedSentence[isDecimal ? copiedIndex++ : parseProgress] = originalString[startIndex + parseProgress];
        parseProgress++;
    }

    copiedIndex = CompressAscii(semiparsedSentence, (decPos + 1), (copiedIndex) - decPos, isDecimal) + decPos;

    if (dataType != Time) AppendToGPSBuffer((uint8_t *) semiparsedSentence, ++copiedIndex);

    switch (dataType) {
        case Time: SetTime((uint8_t *) semiparsedSentence, ++copiedIndex);
            break;
        case Altitude: SetAltitude((uint8_t *) semiparsedSentence, copiedIndex);
            break;
        default: break;
    }
    return (parseProgress - 1);
}

// Main GPS parsing method
// char *unparsedSentence -> ASCII GPS sentence to parse (GPGGA)

void ParseGPSSample(char *unparsedSentence) {

    int i;
    char unit[1] = {','};
    char header[] = "$GPGGA";
    char *sentenceHeader;

    for (i = 0; i < 74; i++) {
        if (unparsedSentence[i] == ',') {
            gpsIndex++;
        } else {

            switch (gpsIndex) {

                case Header:

                    // Check if correct header
                    sentenceHeader = strstr(unparsedSentence, header);
                    if (sentenceHeader == NULL) i = 74;
                    else i += (strlen(header) - 1);

                    break;

                case Time:

                    i += ParseDecimal(unparsedSentence, i, gpsIndex);

                    break;

                case Latitude:

                    i += ParseDecimal(unparsedSentence, i, gpsIndex);

                    AppendToGPSBuffer((uint8_t *) unit, 1);
                    break;

                case LatDirection:

                    unit[0] = unparsedSentence[i];
                    AppendToGPSBuffer((uint8_t *) unit, 1);

                    unit[0] = ',';
                    AppendToGPSBuffer((uint8_t *) unit, 1);
                    break;

                case Longitude:

                    i += ParseDecimal(unparsedSentence, i, gpsIndex);

                    unit[0] = ',';
                    AppendToGPSBuffer((uint8_t *) unit, 1);
                    break;

                case LongDirection:

                    unit[0] = unparsedSentence[i];
                    AppendToGPSBuffer((uint8_t *) unit, 1);

                    unit[0] = ',';
                    AppendToGPSBuffer((uint8_t *) unit, 1);
                    break;

                case Altitude:

                    i += ParseDecimal(unparsedSentence, i, gpsIndex);

                    unit[0] = ',';
                    AppendToGPSBuffer((uint8_t *) unit, 1);
                    break;

                case AltUnit:

                    unit[0] = unparsedSentence[i];
                    AppendToGPSBuffer((uint8_t *) unit, 1);
                    break;

                default: break;
            }
        }
    }
}


uint8_t spiTesting = 0x00;
uint16_t spi16Testing = 0x8000;

void TestDACSPI() {
    
    _LATG7 = 0;

    //    SPI1_Exchange8bit(spiTesting++);
    SPI1_Exchange16bit(0x00FF);
    SPI1_Exchange16bit(0xFF00);
    
//    SPI1_Exchange16bit(spi16Testing);
    //    SPI1_Exchange16bit(currentLangmuirProbeSweepPosition);

    _LATG7 = 1;
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
