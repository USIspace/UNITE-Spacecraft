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

uint8_t langmuirProbeBuffer[600] = { NULL };
uint8_t magnetometerBuffer[33] = { NULL };
uint8_t temperatureBuffer[32] = { NULL };
uint8_t gpsBuffer[33] = { NULL };

char myFunString[29] = "Hey, World! UNITE Rules Now!";

// GPS Properties
bool isGPSReading = false;
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

uint16_t LP_BUFFER_SIZE = 600;
uint16_t MAG_BUFFER_SIZE = 33;
uint16_t TMP_BUFFER_SIZE = 32;
uint16_t GPS_BUFFER_SIZE = 33;

/******************************
  Instrument Timer Properties
 ******************************/

int currentLangmuirProbeWait = 0;
int currentMagnetometerWait = 0;
int currentTemperatureWait = 0;
int currentGPSWait = 0;

int langmuirProbeCallbackCount = 0;
int magnetometerCallbackCount = 0;

int currentLangmuirProbeSweepProgress = 0;
int currentMagnetometerSweepProgress = 0;

int currentMagnetometerOrbitProgress = 0;

bool isLangmuirProbeSweeping = false;
bool isMagnetometerSweeping = false;
bool shouldMagnetometerSample = false;

bool isLangmuirProbeSweepPositive = true;
int currentLangmuirProbeSweepPosition = 0; //0x1000 : -32767

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

void BeginLangmuirProbeSampling() {
    // Sample Plasma Probe Data and store in buffer
    //    isLangmuirProbeSweeping = true;
    //    TMR2_Start();
}

void BeginMagnetometerSampling() {
    // Sample Magnetometer Data and store in buffer

    /* 
     * Magnetometer Sweeping Algorithm
     */
    //    isMagnetometerSweeping = true;
    //    TMR3_Start();                           // TMR3 samples every 100 ms and stores in results

    /*
     * Magnetometer Orbit Sampling
     */
    shouldMagnetometerSample = true;
    TakeMagnetometerSample();
}

void BeginTemperatureSampling() {
    TakeTemperatureSample();
}

void BeginGPSSampling() {
    // Sample GPS Data and store in buffer

    _LATE3 = LED_ON;

    // Clear out GPS Buffer
    memset(gpsBuffer, 0, sizeof (gpsBuffer));

    // Turn on GPS Interrupt
    IEC0bits.U1RXIE = 1;

}

// MARK: End Methods

void EndLangmuirProbeSampling() {
    TMR2_Stop();
    currentLangmuirProbeSweepProgress = 0;
    isLangmuirProbeSweeping = false;

    //    PackageData(LPSubSys, (int)timeInMin, langmuirProbeBuffer, LP_BUFFER_SIZE);
    memset(langmuirProbeBuffer, 0, sizeof(langmuirProbeBuffer));
    currentLangmuirProbeBufferIndex = 0;
    currentLangmuirProbeWait = 0;
    LP_BUFFER_SIZE = (GetSweepDuration(&LangmuirProbe) * convertTime(Sec, MilSec)) / (GetSweepRate(&LangmuirProbe) * 5) * 3;

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
    if (currentGPSBufferIndex > 20)
        PackageData(GPSSubSys, (int) timeInMin, gpsBuffer, GPS_BUFFER_SIZE);
    memset(gpsBuffer, 0, sizeof(gpsBuffer));
    currentGPSBufferIndex = 0;
    currentGPSWait = 0;
    gpsIndex = 0;

    _LATE3 = LED_OFF;
}

void TransmitTestString() {

    //    PackageData(LPSubSys, GetDayTimeInMin(totalTime), myFunString, 28);
}

/***************************
  Sweeping Progress Manager
 **************************/

void ManageSweepingProgress() {

    if (isLangmuirProbeSweeping) {
        if (++currentLangmuirProbeSweepProgress > GetSweepDuration(&LangmuirProbe)) {
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

    // Sweeping Algorithm
    int voltageAdjustment = isLangmuirProbeSweepPositive ? 655 : -655;
    currentLangmuirProbeSweepPosition = max(0x8000, min(0x7FFF, currentLangmuirProbeSweepPosition + voltageAdjustment));

    if (currentLangmuirProbeSweepPosition == 0x7FFF) isLangmuirProbeSweepPositive = false;
    else if (currentLangmuirProbeSweepPosition == 0x8000) isLangmuirProbeSweepPositive = true;

    _RG9 = 0;
    SPI1_Exchange16bit(currentLangmuirProbeSweepPosition & 0xFFFF);
    _RG9 = 1;

    // Sample Probe from ADC
    ADC1_GetResultFromChannels(langmuirProbeResults, lpADCConfig.channelSelect, lpADCConfig.channelCount);

    // Manipulate Data Here


    int probeResultSize = 3;
    Copy(langmuirProbeResults, langmuirProbeBuffer, 0, currentLangmuirProbeBufferIndex, probeResultSize);
    currentLangmuirProbeBufferIndex += probeResultSize;

    if (currentLangmuirProbeBufferIndex >= LP_BUFFER_SIZE) {
        EndLangmuirProbeSampling();
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
    Copy(magnetometerResults, magnetometerBuffer, 0, currentMagnetometerBufferIndex, magnetometerResultSize);
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
    Copy(temperatureResults, temperatureBuffer, 0, currentTemperatureBufferIndex, temperatureResultSize);
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

// Reads GPS sentence one byte at a time starting from '$'
// int samplePos -> index variable to store in unparsed GPS buffer

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

    AppendToGPSBuffer((uint8_t *) semiparsedSentence, ++copiedIndex);

    switch (dataType) {
        case Time: SetTime((uint8_t *) semiparsedSentence, copiedIndex);
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

                    AppendToGPSBuffer((uint8_t *) unit, 1);
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

    _RG9 = 0;

    //    SPI1_Exchange8bit(spiTesting++);
    SPI1_Exchange16bit(spi16Testing);
    //    SPI1_Exchange16bit(currentLangmuirProbeSweepPosition);

    currentLangmuirProbeSweepPosition = currentLangmuirProbeSweepPosition + 655;

    _RG9 = 1;
}
