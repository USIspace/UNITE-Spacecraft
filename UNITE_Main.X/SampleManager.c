#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "SatelliteMode.h"
#include "SampleManager.h"
#include "adc1.h"
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/uart2.h"
#include "mcc_generated_files/uart3.h"
#include "mcc_generated_files/uart4.h"
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/tmr2.h"
#include "mcc_generated_files/tmr3.h"
#include "mcc_generated_files/tmr4.h"


/***********************
  Manager Declarations
 **********************/
bool shouldSample = false;
int results[16];


/**************************
  Sampling Configurations
 **************************/

ADCSampleConfig lpADCConfig = {
    0,   // adc channel to start at   
    0    // number of adc channels to sample
};

ADCSampleConfig magADCConfig = {
    0x00C4,      // AN2, 6, 7
    0
};

ADCSampleConfig tmpADCConfig = {
    0xFF00,      // AN8, 9, A, B, C, D, E, F
    8
};


/********************
 Data Manager Methods
 ********************/

void GetTempData(uint8_t *buffer, int bufferSize) {
    
    TMR1_INTERRUPT_TICKER_FACTOR = 1;       // Sample for a duration of 1 sec
    
    TMR4_Start();                           // TMR4 samples every 1 s and stores in results
    TMR1_Start();
    
    int i;
    for (i = 0; i < bufferSize; i++) {
        buffer[i] = results[i] / 4;
    }

}

void GetGPSData(int *buffer, int bufferSize) {
    // Sample GPS Data and store in buffer
}

void GetMagnetometerData(uint8_t *buffer, int bufferSize) {
    // Sample Magnetometer Data and store in buffer
    
    TMR1_INTERRUPT_TICKER_FACTOR = 5;       // Sample for a duration of 5 sec
    
    TMR3_Start();                           // TMR3 samples every 100 ms and stores in results
    TMR1_Start();   
    
    /*
    int i;
    for (i = 0; i < bufferSize; i++) {
        buffer[i] = results[i] / 4;
    }*/
}

void GetProbeData(int *buffer, int bufferSize) {
    // Sample Plasma Probe Data and store in buffer
}

/***********************
  Single Sample Methods
 ***********************/

void TakeTemperatureSample() {
    ADC1_GetResultFromChannels(results, tmpADCConfig.channelSelect, tmpADCConfig.channelCount);
}

void TakeGPSSample() {

}

void TakeMagnetometerSample() {
    ADC1_GetResultFromChannels(results, magADCConfig.channelSelect, magADCConfig.channelCount);
    
    uint8_t buffer[3];
    int i;
    for (i = 0; i < 3; i++) {
        buffer[i] = results[i] / 4;
    }
    SendData(buffer, 3);
}

void TakeProbeSample() {
    ADC1_GetResultFromChannels(results, lpADCConfig.channelSelect, lpADCConfig.channelCount);

}