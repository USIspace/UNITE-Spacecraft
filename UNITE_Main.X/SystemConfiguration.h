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


// Set to true if debugging software without Simplex or Duplex connected
#define IS_DEBUG false

// Set to true if using diagnostic mode
#define IS_DIAG true

// Set to true if Duplex should timeout when waiting for input
#define IS_DUP_TIMEOUT_ENABLED true

// Set to true is Simplex should timeout when waiting for input
#define IS_SIM_TIMEOUT_ENABLED true

// Set to true if GPS should timeout when waiting for input
#define IS_GPS_TIMEOUT_ENABLED true

// Constant for the duration of an orbit in minutes
#define ORBIT_DUR_MIN 1 //93

// Set Main Loop Timer Interval
#define MAIN_LOOP_TIMER_INTERVAL 60

// Set to true if GPS should be interrupt driven
#define IS_GPS_INTERRUPT_ENABLED false

// Set GPS Lock Failure
#define GPS_LOCK_FAILURE 30

// Set Maximum file buffer for Duplex
#define MAX_DUP_FILES_WAITING 5

// Struct for instrument sampling properties
typedef struct {
    unsigned long sampleRate;       // Minutes between each sample
    unsigned long sweepRate;        // Rate in Timer ticks between sweep samples
    unsigned long sweepDuration;    // Length of sweep in seconds
} Properties;

// Struct to store instrument properties for each mode
typedef struct {
    Properties FirstWeek;
    Properties Interim;
    Properties Stabilize;
    Properties Science;
    Properties ReEntry;
    Properties Fallback;
} Instrument;

// Defines types for transmission and UART communication lines 
typedef enum {
    SimplexUnit,
    DuplexUnit,
    SimplexOrDuplex,
    GPSUnit,
    DiagUnit
} TransmissionUnit;

// Struct to store transmission units for each mode
typedef struct {
    TransmissionUnit firstWeek;
    TransmissionUnit interim;
    TransmissionUnit stabilize;
    TransmissionUnit science;
    TransmissionUnit reentry;
    TransmissionUnit fallback;

} TransmissionMode;

// Declares how many samples Langmuir Probe should take per calibration
extern unsigned long lpSamplesPerCalibration;

// Declares global structs for each instrument
extern Instrument LangmuirProbe;
extern Instrument TemperatureSensors;
extern Instrument Magnetometer;
extern Instrument GPS;
extern Instrument Housekeeping;

// Declares a global struct for transmission modes for each operational mode 
extern TransmissionMode TransmissionUnitForMode;

/* Sampling Methods */

// Returns the sample rate for an instrument based on the current mode
unsigned long GetSampleRate(Instrument *);
// Returns the sweep rate for an instrument based on the current mode
unsigned long GetSweepRate(Instrument *);
// Returns the sweep duration for an instrument based on the current mode
unsigned long GetSweepDuration(Instrument *);

/* Transmission Methods */

// Returns the Package Header ID for an instrument
uint8_t GetSystemHeaderID(System);
// Returns the time of day in minutes since 00:00 UTC
uint16_t GetDayTimeInMin(unsigned long);
// Returns the max package length for a transmission unit
uint8_t GetTransmissionPackageLength(TransmissionUnit);
// Returns the transmission unit for current operational mode
TransmissionUnit GetTransmissionUnitForMode();