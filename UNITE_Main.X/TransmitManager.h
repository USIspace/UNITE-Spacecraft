/* 
 * File:   TransmitManager.h
 * Author: zacksnyder
 *
 * Created on May 30, 2017, 11:09 AM
 */

#ifndef TRANSMITMANAGER_H
#define	TRANSMITMANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* TRANSMITMANAGER_H */

// Timeout values in seconds - Subject to change
#define SIMPLEX_RES_TIMEOUT     100000
#define DUPLEX_RES_TIMEOUT      100000

// Number of times the Duplex has restarted before launch
#define DUPLEX_EPOCH_OFFSET     0

// Boolean for whether UNITE is sending packets to a TransmissionUnit
extern bool isSending;

// Global variable to track when Duplex is connected
//bool isDuplexConnected;

// Timeout flags
extern bool simplexTimeoutFlag;
extern bool duplexTimeoutFlag;

extern uint16_t b1Charge;
extern uint16_t b2Charge;
extern uint16_t b1Voltage;
extern uint16_t b2Voltage;
extern uint16_t b1Current;
extern uint16_t b2Current;

extern uint16_t bussPlusVoltage;

extern uint16_t solar1Voltage;
extern uint16_t solar2Voltage;
extern uint16_t solar3Voltage;
extern uint16_t solar4Voltage;

extern uint16_t simplexTemp;
extern uint16_t duplexTemp;
extern uint16_t epsTemp;

/**********************
  Transmission Methods
 **********************/

// Transmits any data found in the sending queue
void TransmitQueue();
// Reads a byte from a TransmissionUnit
uint8_t Read(TransmissionUnit unit);
// Sends a byte to a TransmissionUnit
void Send(uint8_t byte, TransmissionUnit unit);
// Packages a system's data
uint16_t PackageData(System system, uint16_t time, uint8_t *buffer, uint16_t bufferSize);
// Returns true if EPS/Simplex busy line is high
bool IsLineBusy();

// Reads SMS commands from Duplex
void HandleCommand();

// Sets the total time counter from Duplex Epoch
void SetTotalTime();

/****************
  Power Methods
 ****************/

// Sends power switch values to EPS
void TogglePowerSwitches();
// Reads the echo packet back from EPS
void ReadPowerSwitches();

// Power switch setters
void SetLangmuirProbePower(bool);
void SetMagnetometerPower(bool);
void SetTemperaturePower(bool);
void SetGPSPower(bool);

// Power switch getters
bool isLangmuirProbeOn(void);
bool isMagnetometerOn(void);
bool isTemperatureOn(void);
bool isGPSOn(void);
bool isDuplexOn(void);