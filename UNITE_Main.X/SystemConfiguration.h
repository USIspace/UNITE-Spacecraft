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

typedef enum {
    SimplexOnly,
    DuplexOnly,
    Simplex_DuplexMix
} TransmissionUnit;

extern Instrument LangmuirProbe;
extern Instrument TemperatureSensors;
extern Instrument Magnetometer;
extern Instrument GPSUnit;

extern TransmissionUnit currentTransmissionUnit;

uint16_t GetSampleRate(Instrument *);
uint16_t GetSweepRate(System);
uint16_t GetSweepDuration(System);

// Transmission Methods
uint8_t GetSystemHeaderID(System);
uint16_t GetDayTimeInMin(unsigned long);
uint8_t GetTransmissionPackageLength(TransmissionUnit);