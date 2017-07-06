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
    unsigned long stopTime; 
} SatelliteMode;



extern UNITEMode currentMode;
extern unsigned long totalTime;
extern uint16_t timeInMin;
extern bool isDuplexConnected;
extern unsigned long lastAltitude;

void Satellite_Initialize(void);
UNITEMode UpdateMode(void);

void TakeSample(void);

void SetTime(void *, int);
void SetAltitude(void *, int);
