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

#define IS_DEBUG false

#define ORBIT_DUR_MIN 93

typedef struct {
    int sampleRate; // Seconds between each sample
    int sweepRate;  // Rate in tmr ticks between sweep samples
    int sweepDuration;   // Length of sweep in seconds
} Properties;

typedef struct {
    Properties Interim;
    Properties Science;
    Properties ReEntry;
} Instrument;

typedef enum {
    SimplexUnit,
    DuplexUnit,
    SimplexOrDuplex,
    GPSUnit
} TransmissionUnit;


typedef struct {
    TransmissionUnit interim;
    TransmissionUnit science;
    TransmissionUnit reentry;
    
} TransmissionMode;

extern Instrument LangmuirProbe;
extern Instrument TemperatureSensors;
extern Instrument Magnetometer;
extern Instrument GPS;

extern TransmissionMode TransmissionUnitForMode;

int GetSampleRate(Instrument *);
int GetSweepRate(Instrument *);
int GetSweepDuration(Instrument *);

// Transmission Methods
uint8_t GetSystemHeaderID(System);
uint16_t GetDayTimeInMin(unsigned long);
uint8_t GetTransmissionPackageLength(TransmissionUnit);

TransmissionUnit GetTransmissionUnitForMode();