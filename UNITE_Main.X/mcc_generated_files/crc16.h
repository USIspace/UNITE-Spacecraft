/* 
 * File:   crc16.h
 * Author: zacksnyder
 *
 * Created on July 12, 2017, 10:01 AM
 */

#ifndef CRC16_H
#define	CRC16_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* CRC16_H */

// Includes
//#include "../system.h"
//#include <p24FJ256GA110.h>
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

// Initializer
// Poly: 1021, Degree: 16
void CRC16_Initialize();

// Returns the CRC-16 word for data string
uint16_t CRC16SendBytes(uint8_t *data, int length, uint16_t InitCrcWord);