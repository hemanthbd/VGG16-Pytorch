/** ###################################################################
**     THIS COMPONENT MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : serial.H
**     Project   : rtimage
**     Processor : 56858
**     Component : AsynchroSerial
**     Version   : Component 02.472, Driver 02.06, CPU db: 2.87.105
**     Compiler  : Metrowerks DSP C Compiler
**     Date/Time : 3/22/2018, 2:02 AM
**     Abstract  :
**         This bean "AsynchroSerial" implements an asynchronous serial
**         communication. The bean supports different settings of
**         parity, word width, stop-bit and communication speed,
**         user can select interrupt or polling handler.
**         Communication speed can be changed also in runtime.
**         The bean requires one on-chip asynchronous serial channel.
**     Settings  :
**         Serial channel              : SCI0
**
**         Protocol
**             Init baud rate          : 57600baud
**             Width                   : 8 bits
**             Stop bits               : 1
**             Parity                  : none
**             Breaks                  : Disabled
**             Input buffer size       : 128
**             Output buffer size      : 1024
**
**         Registers
**             Input buffer            : SCI_0_DR  [2097124]
**             Output buffer           : SCI_0_DR  [2097124]
**             Control register        : SCI_0_CR  [2097121]
**             Mode register           : SCI_0_CR  [2097121]
**             Baud setting reg.       : SCI_0_BR  [2097120]
**
**         Input interrupt
**             Vector name             : INT_SCI0_RxFull
**             Priority                : 1
**
**         Output interrupt
**             Vector name             : INT_SCI0_TxEmpty
**             Priority                : 1
**
**         Used pins:
**         ----------------------------------------------------------
**           Function | On package           |    Name
**         ----------------------------------------------------------
**            Input   |     73               |  GPIOE0_RXD0
**            Output  |     74               |  GPIOE1_TXD0
**         ----------------------------------------------------------
**
**
**
**     Contents  :
**         RecvChar        - byte serial_RecvChar(serial_TComData *Chr);
**         SendChar        - byte serial_SendChar(serial_TComData Chr);
**         RecvBlock       - byte serial_RecvBlock(serial_TComData *Ptr, word Size, word *Rcv);
**         SendBlock       - byte serial_SendBlock(serial_TComData *Ptr, word Size, word *Snd);
**         ClearRxBuf      - byte serial_ClearRxBuf(void);
**         ClearTxBuf      - byte serial_ClearTxBuf(void);
**         GetCharsInRxBuf - word serial_GetCharsInRxBuf(void);
**         GetCharsInTxBuf - word serial_GetCharsInTxBuf(void);
**
**     Copyright : 1997 - 2009 Freescale Semiconductor, Inc. All Rights Reserved.
**     
**     http      : www.freescale.com
**     mail      : support@freescale.com
** ###################################################################*/

#ifndef __serial
#define __serial

/* MODULE serial. */

#include "Cpu.h"



#ifndef __BWUserType_serial_TError
#define __BWUserType_serial_TError
  typedef union {
    byte err;
    struct {
      bool OverRun  : 1;               /* Overrun error flag */
      bool Framing  : 1;               /* Framing error flag */
      bool Parity   : 1;               /* Parity error flag */
      bool RxBufOvf : 1;               /* Rx buffer full error flag */
      bool Noise    : 1;               /* Noise error flag */
      bool Break    : 1;               /* Break detect */
      bool LINSync  : 1;               /* LIN synchronization error */
      bool BitError  : 1;              /* Bit error flag - mismatch to the expected value happened. */
    } errName;
  } serial_TError;                     /* Error flags. For languages which don't support bit access is byte access only to error flags possible. */
#endif

#ifndef __BWUserType_serial_TComData
#define __BWUserType_serial_TComData
  typedef byte serial_TComData;        /* User type for communication. Size of this type depends on the communication data witdh */
#endif

#define serial_INP_BUF_SIZE  128       /* Length of the RX buffer */

#define serial_OUT_BUF_SIZE  1024      /* Length of the TX buffer */

byte serial_RecvChar(serial_TComData *Chr);
/*
** ===================================================================
**     Method      :  serial_RecvChar (component AsynchroSerial)
**
**     Description :
**         If any data is received, this method returns one
**         character, otherwise it returns an error code (it does
**         not wait for data). This method is enabled only if the
**         receiver property is enabled.
**         [Note:] Because the preferred method to handle error and
**         break exception in the interrupt mode is to use events
**         <OnError> and <OnBreak> the return value ERR_RXEMPTY has
**         higher priority than other error codes. As a consequence
**         the information about an exception in interrupt mode is
**         returned only if there is a valid character ready to be
**         read.
**         Version specific information for Freescale 56800
**         derivatives ] 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this method only sets the selected DMA channel. Then the
**         status of the DMA transfer can be checked using
**         GetCharsInRxBuf method. See an example of a typical usage
**         for details about the communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Chr             - Pointer to a received character
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_RXEMPTY - No data in receiver
**                           ERR_BREAK - Break character is detected
**                           (only when the <Interrupt service>
**                           property is disabled and the <Break
**                           signal> property is enabled)
**                           ERR_COMMON - common error occurred (the
**                           <GetError> method can be used for error
**                           specification)
**                           Version specific information for
**                           Freescale 56800 derivatives ] 
**                           DMA mode:
**                           If DMA controller is available on the
**                           selected CPU and the receiver is
**                           configured to use DMA controller then
**                           only ERR_OK, ERR_RXEMPTY, and ERR_SPEED
**                           error code can be returned from this
**                           method.
** ===================================================================
*/

byte serial_SendChar(serial_TComData Chr);
/*
** ===================================================================
**     Method      :  serial_SendChar (component AsynchroSerial)
**
**     Description :
**         Sends one character to the channel. If the bean is
**         temporarily disabled (Disable method) SendChar method
**         only stores data into an output buffer. In case of a zero
**         output buffer size, only one character can be stored.
**         Enabling the bean (Enable method) starts the transmission
**         of the stored data. This method is available only if the
**         transmitter property is enabled.
**         Version specific information for Freescale 56800
**         derivatives ] 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the transmitter is configured to use DMA controller then
**         this method only sets selected DMA channel. Then the
**         status of the DMA transfer can be checked using
**         GetCharsInTxBuf method. See an example of a typical usage
**         for details about communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Chr             - Character to send
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_TXFULL - Transmitter is full
** ===================================================================
*/

byte serial_RecvBlock(serial_TComData *Ptr,word Size,word *Rcv);
/*
** ===================================================================
**     Method      :  serial_RecvBlock (component AsynchroSerial)
**
**     Description :
**         If any data is received, this method returns the block of
**         the data and its length (and incidental error), otherwise
**         it returns an error code (it does not wait for data).
**         This method is available only if non-zero length of the
**         input buffer is defined and the receiver property is
**         enabled.
**         If less than requested number of characters is received
**         only the available data is copied from the receive buffer
**         to the user specified destination. The value ERR_EXEMPTY
**         is returned and the value of variable pointed by the Rcv
**         parameter is set to the number of received characters.
**         Version specific information for Freescale 56800
**         derivatives ] 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this method only sets the selected DMA channel. Then the
**         status of the DMA transfer can be checked using
**         GetCharsInRxBuf method. See an example of a typical usage
**         for details about communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Ptr             - Pointer to the block of received data
**         Size            - Size of the block
**       * Rcv             - Pointer to real number of the received
**                           data
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_RXEMPTY - The receive buffer didn't
**                           contain the requested number of data.
**                           Only available data has been returned.
**                           ERR_COMMON - common error occurred (the
**                           GetError method can be used for error
**                           specification)
**                           Version specific information for
**                           Freescale 56800 derivatives ] 
**                           DMA mode: If DMA controller is available
**                           on the selected CPU and the receiver is
**                           configured to use DMA controller then
**                           only ERR_OK, ERR_RXEMPTY, and ERR_SPEED
**                           error codes can be returned from this
**                           method.
** ===================================================================
*/

byte serial_SendBlock(serial_TComData *Ptr,word Size,word *Snd);
/*
** ===================================================================
**     Method      :  serial_SendBlock (component AsynchroSerial)
**
**     Description :
**         Sends a block of characters to the channel.
**         This method is available only if non-zero length of the
**         output buffer is defined and the transmitter property is
**         enabled.
**         Version specific information for Freescale 56800
**         derivatives ] 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the transmitter is configured to use DMA controller then
**         this method only sets the selected DMA channel. Then the
**         status of the DMA transfer can be checked using
**         GetCharsInTxBuf method. See typical usage for details
**         about communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Ptr             - Pointer to the block of data to send
**         Size            - Size of the block
**       * Snd             - Pointer to number of data that are sent
**                           (moved to buffer)
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_TXFULL - It was not possible to send
**                           requested number of bytes
** ===================================================================
*/

byte serial_ClearRxBuf(void);
/*
** ===================================================================
**     Method      :  serial_ClearRxBuf (component AsynchroSerial)
**
**     Description :
**         Clears the receive buffer.
**         This method is available only if non-zero length of the
**         input buffer is defined and the receiver property is
**         enabled.
**         Version specific information for Freescale 56800
**         derivatives ] 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this method only stops selected DMA channel.
**     Parameters  : None
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
** ===================================================================
*/

byte serial_ClearTxBuf(void);
/*
** ===================================================================
**     Method      :  serial_ClearTxBuf (component AsynchroSerial)
**
**     Description :
**         Clears the transmit buffer.
**         This method is available only if non-zero length of the
**         output buffer is defined and the receiver property is
**         enabled.
**         Version specific information for Freescale 56800
**         derivatives ] 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the transmitter is configured to use DMA controller then
**         this method only stops selected DMA channel.
**     Parameters  : None
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
** ===================================================================
*/

word serial_GetCharsInRxBuf(void);
/*
** ===================================================================
**     Method      :  serial_GetCharsInRxBuf (component AsynchroSerial)
**
**     Description :
**         Returns the number of characters in the input buffer.
**         This method is available only if the receiver property is
**         enabled.
**         Version specific information for Freescale 56800
**         derivatives ] 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this method returns the number of characters in the
**         receive buffer.
**     Parameters  : None
**     Returns     :
**         ---             - The number of characters in the input
**                           buffer.
** ===================================================================
*/

word serial_GetCharsInTxBuf(void);
/*
** ===================================================================
**     Method      :  serial_GetCharsInTxBuf (component AsynchroSerial)
**
**     Description :
**         Returns the number of characters in the output buffer.
**         This method is available only if the transmitter property
**         is enabled.
**         Version specific information for Freescale 56800
**         derivatives ] 
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the transmitter is configured to use DMA controller then
**         this method returns the number of characters in the
**         transmit buffer.
**     Parameters  : None
**     Returns     :
**         ---             - The number of characters in the output
**                           buffer.
** ===================================================================
*/

void serial_InterruptRx(void);
/*
** ===================================================================
**     Method      :  serial_InterruptRx (component AsynchroSerial)
**
**     Description :
**         The method services the receive interrupt of the selected 
**         peripheral(s) and eventually invokes the bean's event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

void serial_InterruptTx(void);
/*
** ===================================================================
**     Method      :  serial_InterruptTx (component AsynchroSerial)
**
**     Description :
**         The method services the receive interrupt of the selected 
**         peripheral(s) and eventually invokes the bean's event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

void serial_InterruptError(void);
/*
** ===================================================================
**     Method      :  serial_InterruptError (component AsynchroSerial)
**
**     Description :
**         The method services the receive interrupt of the selected 
**         peripheral(s) and eventually invokes the bean's event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

void serial_Init(void);
/*
** ===================================================================
**     Method      :  serial_Init (component AsynchroSerial)
**
**     Description :
**         Initializes the associated peripheral(s) and the bean internal 
**         variables. The method is called automatically as a part of the 
**         application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/



/* END serial. */

/*
** ###################################################################
**
**     This file was created by Processor Expert 3.00 [04.35]
**     for the Freescale 56800 series of microcontrollers.
**
** ###################################################################
*/

#endif /* ifndef __serial */
