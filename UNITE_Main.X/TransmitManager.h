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


void TransmitQueue();

void SaveData(uint8_t *, uint16_t);
uint16_t PackageData(System, uint16_t, uint8_t *, uint16_t);
void SendData(uint8_t *, int);
bool ReadACKForMode(TransmissionUnit unit);
void Send(uint8_t byte, TransmissionUnit unit);
uint8_t Read(TransmissionUnit unit);
bool IsLineBusy(TransmissionUnit unit);