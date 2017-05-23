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

void Satellite_Initialize(void);

void SaveData(int *package, int packageSize);
void PackageData(int *package, int stringLength, int *temps, int *gps, int *mags, int *densities);
void SendData(uint8_t *dataString, int stringLength);

UNITEMode UpdateMode(void);

void TakeSample(void);
