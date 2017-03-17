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
    int sampleRateInSec; // Time in minutes between each sample of sensors
    int startAltitudeInKm; // Altitude to begin sampling in this mode 
    int endAltitudeInKm; // Altitude to end sampling and switch to new mode
} SatelliteMode;

/*
 TODO: Add methods from SatelliteMode source file
 */

static UNITEMode currentMode;
static bool shouldChangeMode;

void GetTempData(int *buffer, int bufferSize);
void GetGPSData(int *buffer, int bufferSize);
void GetMagnetometerData(int *buffer, int bufferSize);
void GetProbeData(int *buffer, int bufferSize);
void SaveData(int *package, int packageSize);
void PackageData(int *package, int stringLength, int *temps, int *gps, int *mags, int *densities);
void SendData(int *dataString, int stringLength);

UNITEMode UpdateMode();
unsigned int DelayForMode();

void BeginSample();