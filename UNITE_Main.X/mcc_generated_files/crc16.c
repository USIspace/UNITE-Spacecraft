#include "crc16.h"
#include "../system.h"

void CRC16_Initialize()
{
    CRCCONbits.PLEN = 0xF;
    CRCXOR = 0x1021;
}

uint16_t CRC16SendBytes(uint8_t *data, int length, uint16_t InitCrcWord)
{
    CRCWDAT = InitCrcWord;
    CRCCONbits.CRCGO = 1;
    while (!CRCCONbits.CRCMPT)
        ;
    int i;
    // Takes about 4.6 us for eight bytes on a 24HJ running at 40 MIPS
    uint16_t dataword;
    for (i = 0; i < length; i += 2) {
        dataword = ((uint16_t)(*data)<< 8) + *(data+1);
        data += 2;
        while (CRCCONbits.CRCFUL);
        CRCDAT = dataword;
    }

    // Make sure it can take the last bytes
    while (CRCCONbits.CRCFUL);
 
    // 16-bit writes two zero bytes to flush out the system
    CRCDAT = 0;
 
    // Wait until everything has been taken from the FIFO
    // IT's still shifting, so give it a little extra after that
    while (!CRCCONbits.CRCMPT);
    wait_for(5);
//    Nop();
//    Nop();
//    Nop();
//    Nop();
//    Nop();
    
    CRCCONbits.CRCGO = 0;
    return CRCWDAT;
}
