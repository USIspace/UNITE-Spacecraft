/* 
 * File:   CommandParser.h
 * Author: zacksnyder
 *
 * Created on May 22, 2017, 10:32 AM
 */

#ifndef COMMANDPARSER_H
#define	COMMANDPARSER_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* COMMANDPARSER_H */

/*
 * COMMAND MESSAGE FORMAT
 * 
 * UNITEXXXYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 * 
 * UNITE: Command call-sign
 * XXX: 3 Digit Sequence Number
 * Y...Y: 34 Digit Command
 * 
 * * SMPUORVVVV
 * * 
 * * S: System to Alter => Transmission, LP, TMP, MAG, C&DH, EPS, GPS, Housekeeping
 * * M: Satellite Mode to Alter => FirstWeek, Interim, Stabilize, Science, Reentry, Fallback, All, Current
 * * P: Property to Alter => Sampling Rate, Sweeping Rate, Sweeping Duration, Transmission Unit
 * * U: Time Unit for Value => Milliseconds, Seconds, Minute, Hour, Day, Sample
 * * O: Is Value the new default => No, Yes  (Bool)
 * * R: Does a System reset need to occur => No, Yes  (Bool) DEPRECATED
 * * VVVV: 4 Digit Value 
 */


// Defines the order of the bytes in a command
typedef enum {
    SystemFlag = 0,
    ModeFlag,
    PropertyFlag,
    UnitFlag,
    OverrideFlag,
    ResetFlag,
    Value

} CommandByteIndex;

// Defines the System for the first byte of a command
typedef enum {
    Break = 0,
    Transmission,
    LPSubSys,
    TMPSubSys,
    MAGSubSys,
    CDHSubSys,
    EPSSubSys,
    GPSSubSys,
    LPCal

} System;

// Defines the Mode for the second byte of a command
typedef enum {
    FirstWeek = 1,
    Interim,
    Stabilize,
    Science,
    ReEntry,
    Fallback,
    Global,
    Current
} Mode;

// Defines the Property for the third byte of a command
typedef enum {
    SamplingRate = 1,
    SweepRate,
    SweepDuration,
    Simplex,
    Duplex,
    Simplex_Duplex,
} Property;

// Defines the time Unit for the fourth byte of a command
typedef enum {
    None = 0,
    MilSec,
    Sec,
    Min,
    Hour,
    Day,
    Sample,
} Unit;


// Parses and performs a command byte string
void PerformCommands(uint8_t *, uint16_t);

unsigned long convertTime(Unit current, Unit final);