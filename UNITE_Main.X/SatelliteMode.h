/* 
 * File:   InterimMode.h
 * Author: zacksnyder
 *
 * Created on February 28, 2017, 1:43 PM
 */

#ifndef INTERIMMODE_H
#define	INTERIMMODE_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* INTERIMMODE_H */

typedef enum {
    interim,
    science,
    reentry,
    safe
} UNITEMode;


typedef struct {
    int startAltitudeInKm; // Altitude to begin sampling in this mode 
    int endAltitudeInKm; // Altitude to end sampling and switch to new mode
} SatelliteMode;



extern UNITEMode currentMode;
extern uint8_t transmitQueue[2000];

void Satellite_Initialize(void);

void SaveData(uint8_t *, uint16_t);
uint16_t PackageData(System, uint16_t, uint8_t *, uint16_t);
void SendData(uint8_t *, uint16_t, uint16_t);

UNITEMode UpdateMode(void);

void TakeSample(void);
