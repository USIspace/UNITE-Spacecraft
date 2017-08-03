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

// Boolean for whether UNITE is sending packets to a TransmissionUnit
extern bool isSending;

// Timeout flags
extern bool simplexTimeoutFlag;
extern bool duplexTimeoutFlag;

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
uint16_t PackageData(System, uint16_t, uint8_t *, uint16_t);
// Returns true if EPS/Simplex busy line is high
bool IsLineBusy();

// Reads SMS commands from Duplex
void HandleCommand();

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