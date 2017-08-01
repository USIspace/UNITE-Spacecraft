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

#define SIMPLEX_RES_TIMEOUT     1000
#define DUPLEX_RES_TIMEOUT      100000


extern bool isSending;
extern bool simplexTimeoutFlag;
extern bool duplexTimeoutFlag;

extern uint8_t temperaturePowerSwitch;
/**********************
  Transmission Methods
 **********************/

void TransmitQueue();
uint8_t Read(TransmissionUnit unit);
void Send(uint8_t byte, TransmissionUnit unit);
uint16_t PackageData(System, uint16_t, uint8_t *, uint16_t);
bool IsLineBusy();

void HandleCommand();

/****************
  Power Methods
 ****************/

void TogglePowerSwitches();
void ReadPowerSwitches();

void SetLangmuirProbePower(bool);
void SetMagnetometerPower(bool);
void SetTemperaturePower(bool);
void SetGPSPower(bool);

bool isLangmuirProbeOn(void);
bool isMagnetometerOn(void);
bool isTemperatureOn(void);
bool isGPSOn(void);
bool isDuplexOn(void);