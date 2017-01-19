/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

/* Device header file */
#if defined(__XC16__)
    #include <xc.h>
#elif defined(__C30__)
    #if defined(__PIC24E__)
    	#include <p24Exxxx.h>
    #elif defined (__PIC24F__)||defined (__PIC24FK__)
	#include <p24Fxxxx.h>
    #elif defined(__PIC24H__)
	#include <p24Hxxxx.h>
    #endif
#endif

#include <stdint.h>        /* Includes uint16_t definition */
#include <stdbool.h>       /* Includes true/false definition */

/******************************************************************************/
/* Interrupt Vector Options                                                   */
/******************************************************************************/
/*                                                                            */
/* Refer to the C30 (MPLAB C Compiler for PIC24F MCUs and dsPIC33F DSCs) User */
/* Guide for an up to date list of the available interrupt options.           */
/* Alternately these names can be pulled from the device linker scripts.      */
/*                                                                            */
/* PIC24F Primary Interrupt Vector Names:                                     */
/*                                                                            */
/* _INT0Interrupt      _IC4Interrupt                                          */
/* _IC1Interrupt       _IC5Interrupt                                          */
/* _OC1Interrupt       _IC6Interrupt                                          */
/* _T1Interrupt        _OC5Interrupt                                          */
/* _Interrupt4         _OC6Interrupt                                          */
/* _IC2Interrupt       _OC7Interrupt                                          */
/* _OC2Interrupt       _OC8Interrupt                                          */
/* _T2Interrupt        _PMPInterrupt                                          */
/* _T3Interrupt        _SI2C2Interrupt                                        */
/* _SPI1ErrInterrupt   _MI2C2Interrupt                                        */
/* _SPI1Interrupt      _INT3Interrupt                                         */
/* _U1RXInterrupt      _INT4Interrupt                                         */
/* _U1TXInterrupt      _RTCCInterrupt                                         */
/* _ADC1Interrupt      _U1ErrInterrupt                                        */
/* _SI2C1Interrupt     _U2ErrInterrupt                                        */
/* _MI2C1Interrupt     _CRCInterrupt                                          */
/* _CompInterrupt      _LVDInterrupt                                          */
/* _CNInterrupt        _CTMUInterrupt                                         */
/* _INT1Interrupt      _U3ErrInterrupt                                        */
/* _IC7Interrupt       _U3RXInterrupt                                         */
/* _IC8Interrupt       _U3TXInterrupt                                         */
/* _OC3Interrupt       _SI2C3Interrupt                                        */
/* _OC4Interrupt       _MI2C3Interrupt                                        */
/* _T4Interrupt        _U4ErrInterrupt                                        */
/* _T5Interrupt        _U4RXInterrupt                                         */
/* _INT2Interrupt      _U4TXInterrupt                                         */
/* _U2RXInterrupt      _SPI3ErrInterrupt                                      */
/* _U2TXInterrupt      _SPI3Interrupt                                         */
/* _SPI2ErrInterrupt   _OC9Interrupt                                          */
/* _SPI2Interrupt      _IC9Interrupt                                          */
/* _IC3Interrupt                                                              */
/*                                                                            */
/* PIC24H Primary Interrupt Vector Names:                                     */
/*                                                                            */
/* _INT0Interrupt      _SPI2Interrupt                                         */
/* _IC1Interrupt       _C1RxRdyInterrupt                                      */
/* _OC1Interrupt       _C1Interrupt                                           */
/* _T1Interrupt        _DMA3Interrupt                                         */
/* _DMA0Interrupt      _IC3Interrupt                                          */
/* _IC2Interrupt       _IC4Interrupt                                          */
/* _OC2Interrupt       _IC5Interrupt                                          */
/* _T2Interrupt        _IC6Interrupt                                          */
/* _T3Interrupt        _OC5Interrupt                                          */
/* _SPI1ErrInterrupt   _OC6Interrupt                                          */
/* _SPI1Interrupt      _OC7Interrupt                                          */
/* _U1RXInterrupt      _OC8Interrupt                                          */
/* _U1TXInterrupt      _DMA4Interrupt                                         */
/* _ADC1Interrupt      _T6Interrupt                                           */
/* _DMA1Interrupt      _T7Interrupt                                           */
/* _SI2C1Interrupt     _SI2C2Interrupt                                        */
/* _MI2C1Interrupt     _MI2C2Interrupt                                        */
/* _CNInterrupt        _T8Interrupt                                           */
/* _INT1Interrupt      _T9Interrupt                                           */
/* _ADC2Interrupt      _INT3Interrupt                                         */
/* _IC7Interrupt       _INT4Interrupt                                         */
/* _IC8Interrupt       _C2RxRdyInterrupt                                      */
/* _DMA2Interrupt      _C2Interrupt                                           */
/* _OC3Interrupt       _DCIErrInterrupt                                       */
/* _OC4Interrupt       _DCIInterrupt                                          */
/* _T4Interrupt        _U1ErrInterrupt                                        */
/* _T5Interrupt        _U2ErrInterrupt                                        */
/* _INT2Interrupt      _DMA6Interrupt                                         */
/* _U2RXInterrupt      _DMA7Interrupt                                         */
/* _U2TXInterrupt      _C1TxReqInterrupt                                      */
/* _SPI2ErrInterrupt   _C2TxReqInterrupt                                      */
/*                                                                            */
/* PIC24E Primary Interrupt Vector Names:                                     */
/*                                                                            */
/* __INT0Interrupt     __C1RxRdyInterrupt      __U3TXInterrupt                */
/* __IC1Interrupt      __C1Interrupt           __USB1Interrupt                */
/* __OC1Interrupt      __DMA3Interrupt         __U4ErrInterrupt               */
/* __T1Interrupt       __IC3Interrupt          __U4RXInterrupt                */
/* __DMA0Interrupt     __IC4Interrupt          __U4TXInterrupt                */
/* __IC2Interrupt      __IC5Interrupt          __SPI3ErrInterrupt             */
/* __OC2Interrupt      __IC6Interrupt          __SPI3Interrupt                */
/* __T2Interrupt       __OC5Interrupt          __OC9Interrupt                 */
/* __T3Interrupt       __OC6Interrupt          __IC9Interrupt                 */
/* __SPI1ErrInterrupt  __OC7Interrupt          __DMA8Interrupt                */
/* __SPI1Interrupt     __OC8Interrupt          __DMA9Interrupt                */
/* __U1RXInterrupt     __PMPInterrupt          __DMA10Interrupt               */
/* __U1TXInterrupt     __DMA4Interrupt         __DMA11Interrupt               */
/* __AD1Interrupt      __T6Interrupt           __SPI4ErrInterrupt             */
/* __DMA1Interrupt     __T7Interrupt           __SPI4Interrupt                */
/* __NVMInterrupt      __SI2C2Interrupt        __OC10Interrupt                */
/* __SI2C1Interrupt    __MI2C2Interrupt        __IC10Interrupt                */
/* __MI2C1Interrupt    __T8Interrupt           __OC11Interrupt                */
/* __CM1Interrupt      __T9Interrupt           __IC11Interrupt                */
/* __CNInterrupt       __INT3Interrupt         __OC12Interrupt                */
/* __INT1Interrupt     __INT4Interrupt         __IC12Interrupt                */
/* __AD2Interrupt      __C2RxRdyInterrupt      __DMA12Interrupt               */
/* __IC7Interrupt      __C2Interrupt           __DMA13Interrupt               */
/* __IC8Interrupt      __DMA5Interrupt         __DMA14Interrupt               */
/* __DMA2Interrupt     __RTCCInterrupt         __OC13Interrupt                */
/* __OC3Interrupt      __U1ErrInterrupt        __IC13Interrupt                */
/* __OC4Interrupt      __U2ErrInterrupt        __OC14Interrupt                */
/* __T4Interrupt       __CRCInterrupt          __IC14Interrupt                */
/* __T5Interrupt       __DMA6Interrupt         __OC15Interrupt                */
/* __INT2Interrupt     __DMA7Interrupt         __IC15Interrupt                */
/* __U2RXInterrupt     __C1TxReqInterrupt      __OC16Interrupt                */
/* __U2TXInterrupt     __C2TxReqInterrupt      __IC16Interrupt                */
/* __SPI2ErrInterrupt  __U3ErrInterrupt        __ICDInterrupt                 */
/* __SPI2Interrupt     __U3RXInterrupt                                        */
/*                                                                            */
/*                                                                            */
/* For alternate interrupt vector naming, simply add 'Alt' between the prim.  */
/* interrupt vector name '_' and the first character of the primary interrupt */
/* vector name.  There are no Alternate or 'Alt' vectors for the 24E family.  */
/*                                                                            */
/* For example, the vector name _ADC2Interrupt becomes _AltADC2Interrupt in   */
/* the alternate vector table.                                                */
/*                                                                            */
/* Example Syntax:                                                            */
/*                                                                            */
/* void __attribute__((interrupt,auto_psv)) <Vector Name>(void)               */
/* {                                                                          */
/*     <Clear Interrupt Flag>                                                 */
/* }                                                                          */
/*                                                                            */
/* For more comprehensive interrupt examples refer to the C30 (MPLAB C        */
/* Compiler for PIC24 MCUs and dsPIC DSCs) User Guide in the                  */
/* <compiler installation directory>/doc directory for the latest compiler    */
/* release.                                                                   */
/*                                                                            */
/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/* TODO Add interrupt routine code here. */
