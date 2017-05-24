/* 
 * File:   SystemConfiguration.h
 * Author: zacksnyder
 *
 * Created on May 22, 2017, 11:32 AM
 */

#ifndef SYSTEMCONFIGURATION_H
#define	SYSTEMCONFIGURATION_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* SYSTEMCONFIGURATION_H */

typedef struct {
    unsigned long sampleRate;
    unsigned long sweepRate;
    uint16_t sweepDuration;
} Properties;

typedef struct {
    Properties Interim;
    Properties Science;
    Properties ReEntry;
} Instrument;

extern Instrument LangmuirProbe;
extern Instrument TemperatureSensors;
extern Instrument Magnetometer;
extern Instrument GPSUnit;

uint16_t GetSampleRate(Instrument instrument, UNITEMode currentMode);
uint16_t GetSweepRate(Instrument instrument, UNITEMode currentMode);
uint16_t GetSweepDuration(Instrument instrument, UNITEMode currentMode);

// Transmission Methods
uint8_t GetSystemHeaderID(System);