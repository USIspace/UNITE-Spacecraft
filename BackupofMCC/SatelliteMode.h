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
    reentry
} UNITEMode;

typedef struct {
    int sampleRateInMin; // Time in minutes between each sample of sensors
    int startAltitudeInKm; // Altitude to begin sampling in this mode 
    int endAltitudeInKm; // Altitude to end sampling and switch to new mode
} SatelliteMode;

/*
 TODO: Add methods from SatelliteMode source file
 */