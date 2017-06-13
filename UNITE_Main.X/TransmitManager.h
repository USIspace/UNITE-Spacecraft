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


extern bool isSending;

/**********************
  Transmission Methods
 **********************/

void TransmitQueue();

uint16_t PackageData(System, uint16_t, uint8_t *, uint16_t);

/****************
  Power Methods
 ****************/

bool TogglePowerSwitches();
bool PowerOnDuplex();
bool PowerOffDuplex();