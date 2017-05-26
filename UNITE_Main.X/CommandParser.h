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

typedef enum {
    SystemFlag,
    ModeFlag,
    PropertyFlag,
    UnitFlag,
    OverrideFlag,
    ResetFlag,
    Value

} CommandByteIndex;

typedef enum {
    Break,
    Transmission,
    LP,
    TMP,
    MAG,
    CDH,
    EPS,
    GPS,
    Housekeeping,
    Beacon

} System;

typedef enum {
    SamplingRate = 1,
    SweepRate,
    SweepDuration,
    Simplex,
    Duplex,
    Simplex_Duplex
} Property;

typedef enum {
    None,
    MilSec,
    Sec,
    Min,
    Hour,
    Day,
    Sample

} Unit;

typedef enum {
    Interim = 1,
    Science,
    ReEntry,
    Global
} Mode;

void PerformCommands();
void EndMessage();
uint8_t GetNextByte();
void ParseByte(uint8_t, CommandByteIndex);

void addValueByte(uint8_t);
uint16_t convertHexToDecimal(uint8_t *);
unsigned long convertTime(Unit,Unit);
void RunCommand(System, Mode, Property, Unit, unsigned long);

