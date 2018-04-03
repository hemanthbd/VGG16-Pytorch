/** ###################################################################
**     THIS COMPONENT MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : serial.C
**     Project   : rtimage
**     Processor : 56858
**     Component : AsynchroSerial
**     Version   : Component 02.472, Driver 02.06, CPU db: 2.87.105
**     Compiler  : Metrowerks DSP C Compiler
**     Date/Time : 3/22/2018, 3:26 AM
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

/* MODULE serial. */

#include "serial.h"
#include "Events.h"


#define OVERRUN_ERR      1             /* Overrun error flag bit    */
#define FRAMING_ERR      2             /* Framing error flag bit    */
#define PARITY_ERR       4             /* Parity error flag bit     */
#define CHAR_IN_RX       8             /* Char is in RX buffer      */
#define FULL_TX          16            /* Full transmit buffer      */
#define RUNINT_FROM_TX   32            /* Interrupt is in progress  */
#define FULL_RX          64            /* Full receive buffer       */
#define NOISE_ERR        128           /* Noise erorr flag bit      */
#define IDLE_ERR         256           /* Idle character flag bit   */
#define BREAK_ERR        512           /* Break detect              */
#define COMMON_ERR       2048          /* Common error of RX       */

static word SerFlag;                   /* Flags for serial communication */
                                       /* Bits: 0 - OverRun error */
                                       /*       1 - Framing error */
                                       /*       2 - Parity error */
                                       /*       3 - Char in RX buffer */
                                       /*       4 - Full TX buffer */
                                       /*       5 - Running int from TX */
                                       /*       6 - Full RX buffer */
                                       /*       7 - Noise error */
                                       /*       8 - Idle character  */
                                       /*       9 - Break detected  */
                                       /*      10 - Unused */
                                       /*      11 - Unused */
static word InpLen;                    /* Length of input buffer's content */
static serial_TComData *InpPtrR;       /* Pointer for reading from input buffer */
static serial_TComData *InpPtrW;       /* Pointer for writing to input buffer */
static serial_TComData InpBuffer[serial_INP_BUF_SIZE]; /* Input buffer for SCI commmunication */
static word OutLen;                    /* Length of output bufer's content */
static serial_TComData *OutPtrR;       /* Pointer for reading from output buffer */
static serial_TComData *OutPtrW;       /* Pointer for writing to output buffer */
static serial_TComData OutBuffer[serial_OUT_BUF_SIZE]; /* Output buffer for SCI commmunication */

/*
** ===================================================================
**     Method      :  HWEnDi (component AsynchroSerial)
**
**     Description :
**         Enables or disables the peripheral(s) associated with the bean.
**         The method is called automatically as a part of the Enable and 
**         Disable methods and several internal methods.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
static void HWEnDi(void)
{
  getReg(SCI_0_SR);                    /* Reset interrupt request flags */
  setRegBits(SCI_0_CR, (SCI_0_CR_TE_MASK | SCI_0_CR_RE_MASK | SCI_0_CR_RFIE_MASK | SCI_0_CR_REIE_MASK)); /* Enable device */
}

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
byte serial_RecvChar(serial_TComData *Chr)
{
  register byte Result = ERR_OK;       /* Return error code */

  if (InpLen > 0) {                    /* Is number of received chars greater than 0? */
    EnterCritical();                   /* Disable global interrupts */
    InpLen--;                          /* Decrease number of received chars */
    *Chr = *(InpPtrR++);               /* Received char */
    /*lint -save -e946 Disable MISRA rule (103) checking. */
    if (InpPtrR >= (InpBuffer + serial_INP_BUF_SIZE)) { /* Is the pointer out of the receive buffer? */
      InpPtrR = InpBuffer;             /* Set pointer to the first item into the receive buffer */
    }
    /*lint -restore */
    Result = (byte)((SerFlag & (OVERRUN_ERR|COMMON_ERR|FULL_RX))? ERR_COMMON : ERR_OK);
    SerFlag &= ~(OVERRUN_ERR|COMMON_ERR|FULL_RX|CHAR_IN_RX); /* Clear all errors in the status variable */
    ExitCritical();                    /* Enable global interrupts */
  } else {
    return ERR_RXEMPTY;                /* Receiver is empty */
  }
  return Result;                       /* Return error code */
}

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
byte serial_SendChar(serial_TComData Chr)
{
  if (OutLen == serial_OUT_BUF_SIZE) { /* Is number of chars in buffer is the same as a size of the transmit buffer */
    return ERR_TXFULL;                 /* If yes then error */
  }
  EnterCritical();                     /* Disable global interrupts */
  OutLen++;                            /* Increase number of bytes in the transmit buffer */
  *(OutPtrW++) = Chr;                  /* Store char to buffer */
    /*lint -save -e946 Disable MISRA rule (103) checking. */
  if (OutPtrW >= (OutBuffer + serial_OUT_BUF_SIZE)) { /* Is the pointer out of the transmit buffer */
    OutPtrW = OutBuffer;               /* Set pointer to first item in the transmit buffer */
  }
    /*lint -restore */
  setRegBit(SCI_0_CR, TEIE);           /* Enable transmit interrupt */
  ExitCritical();                      /* Enable global interrupts */
  return ERR_OK;                       /* OK */
}

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
byte serial_RecvBlock(serial_TComData *Ptr,word Size,word *Rcv)
{
  register word count;                 /* Number of received chars */
  register byte result = ERR_OK;       /* Last error */

  for (count = 0; count < Size; count++) {
    result = serial_RecvChar(Ptr++);
    if (result != ERR_OK) {            /* Receiving given number of chars */
      *Rcv = count;                    /* Return number of received chars */
      return result;                   /* Return last error */
    }
  }
  *Rcv = count;                        /* Return number of received chars */
  return result;                       /* OK */
}

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
byte serial_SendBlock(serial_TComData *Ptr,word Size,word *Snd)
{
  register word count;                 /* Number of sent chars */
  register byte result = ERR_OK;       /* Last error */

  for (count = 0; count < Size; count++) {
    result = serial_SendChar(*Ptr++);
    if (result != ERR_OK) {            /* Sending given number of chars */
      *Snd = count;                    /* Return number of sent chars */
      return result;                   /* Return last error */
    }
  }
  *Snd = count;                        /* Return number of sended chars */
  return result;                       /* Return error code */
}

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
byte serial_ClearRxBuf(void)
{
  EnterCritical();                     /* Disable global interrupts */
  InpLen = 0;                          /* Set number of chars in the transmit buffer to 0 */
  InpPtrR = InpPtrW = InpBuffer;       /* Set pointers on the first item in the transmit buffer */
  ExitCritical();                      /* Enable global interrupts */
  return ERR_OK;                       /* OK */
}

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
byte serial_ClearTxBuf(void)
{
  EnterCritical();                     /* Disable global interrupts */
  OutLen = 0;                          /* Set number of chars in the receive buffer to 0 */
  OutPtrR = OutPtrW = OutBuffer;       /* Set pointers on the first item in the receive buffer */
  ExitCritical();                      /* Enable global interrupts */
  return ERR_OK;                       /* OK */
}

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
word serial_GetCharsInRxBuf(void)
{
  return InpLen;                       /* Return number of chars in receive buffer */
}

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
word serial_GetCharsInTxBuf(void)
{
  return OutLen;                       /* Return number of chars in the transmitter buffer */
}

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
#define ON_ERROR    1
#define ON_FULL_RX  2
#define ON_RX_CHAR  4
#pragma interrupt alignsp saveall
void serial_InterruptRx(void)
{
  register serial_TComData Data;       /* Temporary variable for data */
  register word OnFlags = 0;           /* Temporary variable for flags */
  register word StatReg = getReg(SCI_0_SR); /* Read status register */

  Data = (serial_TComData)getReg(SCI_0_DR); /* Read data from the receiver */
  if(StatReg & (SCI_0_SR_OR_MASK)) {   /* Is HW overrun error detected? */
    setReg(SCI_0_SR, 0);               /* Reset error request flags */
    SerFlag |= OVERRUN_ERR;            /* Set flag OVERRUN_ERR */
    OnFlags |= ON_ERROR;               /* Set flag "OnError" */
  }
  if (InpLen < serial_INP_BUF_SIZE) {  /* Is number of bytes in the receive buffer lower than size of buffer? */
    InpLen++;                          /* Increse number of chars in the receive buffer */
    *(InpPtrW++) = Data;               /* Save received char to the receive buffer */
    /*lint -save -e946 Disable MISRA rule (103) checking. */
    if (InpPtrW >= (InpBuffer + serial_INP_BUF_SIZE)) { /* Is the pointer out of the receive buffer? */
      InpPtrW = InpBuffer;             /* Set pointer on the first item into the receive buffer */
    }
    /*lint -restore */
    OnFlags |= ON_RX_CHAR;             /* Set flag "OnRXChar" */
    if (InpLen == serial_INP_BUF_SIZE) { /* Is number of bytes in the receive buffer equal as a size of buffer? */
      OnFlags |= ON_FULL_RX;           /* If yes then set flag "OnFullRxBuff" */
    }
  } else {
    SerFlag |= FULL_RX;                /* If yes then set flag buffer overflow */
    OnFlags |= ON_ERROR;               /* Set flag "OnError" */
  }
  if (OnFlags & ON_ERROR) {            /* Was error flag detect? */
    serial_OnError();                  /* If yes then invoke user event */
  }
  if (OnFlags & ON_RX_CHAR) {          /* Is OnRxChar flag set? */
    serial_OnRxChar();                 /* If yes then invoke user event */
  }
  if (OnFlags & ON_FULL_RX) {          /* Is OnFullRxBuf flag set? */
    serial_OnFullRxBuf();              /* If yes then invoke user event */
  }
}

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
#define ON_FREE_TX  1
#define ON_TX_CHAR  2
#pragma interrupt alignsp saveall
void serial_InterruptTx(void)
{
  register word OnFlags = 0;           /* Temporary variable for flags */

  if (SerFlag & RUNINT_FROM_TX) {      /* Is flag "running int from TX" set? */
    OnFlags |= ON_TX_CHAR;             /* Set flag "OnTxChar" */
  }
  SerFlag &= ~RUNINT_FROM_TX;          /* Reset flag "running int from TX" */
  if (OutLen) {                        /* Is number of bytes in the transmit buffer greater then 0? */
    OutLen--;                          /* Decrease number of chars in the transmit buffer */
    SerFlag |= RUNINT_FROM_TX;         /* Set flag "running int from TX"? */
    getReg(SCI_0_SR);                  /* Reset interrupt request flags */
    SCI_0_DR = (serial_TComData)*(OutPtrR++); /* Store char to transmitter register */
    /*lint -save -e946 Disable MISRA rule (103) checking. */
    if (OutPtrR >= (OutBuffer + serial_OUT_BUF_SIZE)) { /* Is the pointer out of the transmit buffer? */
      OutPtrR = OutBuffer;             /* Set pointer on the first item into the transmit buffer */
    }
    /*lint -restore */
  } else {
    OnFlags |= ON_FREE_TX;             /* Set flag "OnFreeTxBuf" */
    clrRegBit(SCI_0_CR, TEIE);         /* Disable transmit interrupt */
  }
  if (OnFlags & ON_TX_CHAR) {          /* Is flag "OnTxChar" set? */
    serial_OnTxChar();                 /* If yes then invoke user event */
  }
  if (OnFlags & ON_FREE_TX) {          /* Is flag "OnFreeTxBuf" set? */
    serial_OnFreeTxBuf();              /* If yes then invoke user event */
  }
}

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
#pragma interrupt alignsp saveall
void serial_InterruptError(void)
{
  register word StatReg = getReg(SCI_0_SR); /* Read status register */

  setReg(SCI_0_SR, 0);                 /* Reset error request flags */
  if(StatReg & (SCI_0_SR_OR_MASK|SCI_0_SR_NF_MASK|SCI_0_SR_FE_MASK|SCI_0_SR_PF_MASK)) { /* Is an error detected? */
    SerFlag |= COMMON_ERR;             /* If yes then set an internal flag */
  }
  if (SerFlag & COMMON_ERR) {          /* Was any error set? */
    serial_OnError();                  /* If yes then invoke user event */
  }
}

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
void serial_Init(void)
{
  SerFlag = 0;                         /* Reset flags */
  /* SCI_0_CR: LOOP=0,SWAI=0,RSRC=0,M=0,WAKE=0,POL=0,PE=0,PT=0,TEIE=0,TIIE=0,RFIE=0,REIE=0,TE=0,RE=0,RWU=0,SBK=0 */
  setReg(SCI_0_CR, 0);                 /* Set the SCI configuration */
  InpLen = 0;                          /* No char in the receive buffer */
  InpPtrW = InpPtrR = InpBuffer;       /* Set pointer on the first item in the receive buffer */
  OutLen = 0;                          /* No char in the transmit buffer */
  OutPtrW = OutPtrR = OutBuffer;       /* Set pointer on the first item in the transmit buffer */
  /* SCI_0_BR: ??=0,??=0,??=0,SBR=65 */
  setReg(SCI_0_BR, 65);                /* Set prescaler bits */
  HWEnDi();                            /* Enable/disable device according to status flags */
}


/* END serial. */


/*
** ###################################################################
**
**     This file was created by Processor Expert 3.00 [04.35]
**     for the Freescale 56800 series of microcontrollers.
**
** ###################################################################
*/
