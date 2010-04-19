/** 
 * @file IxUART.h
 *
 * @date 12-OCT-01
 *
 * @brief Public header for the Intel IXP400 internal UART, generic driver.
 * 
 * Design Notes:
 * This driver allows you to perform the following functions: 
 *	Device Initialization,
 *	send/receive characters.
 *
 * Perform Uart IOCTL for the following:
 *	Set/Get the current baud rate,
 *	set parity,
 *	set the number of Stop bits,
 *	set the character Length (5,6,7,8),
 *	enable/disable Hardware flow control.
 *
 * Only Polled mode is supported for now.
 *    
 * 
 * @par
 * IXP400 SW Release version 2.0
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * @par
 * -- End of Copyright Notice --
*/

/**
 * @defgroup IxUARTAccAPI IXP400 UART Access (IxUARTAcc) API
 *
 * @brief IXP400 UARTAcc Driver Public API
 * 
 * @{
 */


/* Defaults */

/**
 * @defgroup DefaultDefines Defines for Default Values
 *
 * @brief Default values which can be used for UART configuration
 *
 * @sa ixUARTDev
 */

/**
 * @def IX_UART_DEF_OPTS
 *
 * @brief The default hardware options to set the UART to -
 * no flow control, 8 bit word, 1 stop bit, no parity 
 *
 * @ingroup DefaultDefines
 */
#define IX_UART_DEF_OPTS (CLOCAL | CS8)

/**
 * @def IX_UART_DEF_XMIT
 *
 * @brief The default UART FIFO size - must be no bigger than 64
 *
 * @ingroup DefaultDefines
 */
#define IX_UART_DEF_XMIT 64

/**
 * @def IX_UART_DEF_BAUD
 *
 * @brief The default UART baud rate - 9600
 *
 * @ingroup DefaultDefines
 */
#define IX_UART_DEF_BAUD 9600

/**
 * @def IX_UART_MIN_BAUD
 *
 * @brief The minimum UART baud rate - 9600
 *
 * @ingroup DefaultDefines
 */
#define IX_UART_MIN_BAUD 9600

/**
 * @def IX_UART_MAX_BAUD
 *
 * @brief The maximum UART baud rate - 926100
 *
 * @ingroup DefaultDefines
 */
#define IX_UART_MAX_BAUD 926100

/**
 * @def IX_UART_XTAL
 *
 * @brief The UART clock speed
 *
 * @ingroup DefaultDefines
 */
#define IX_UART_XTAL 14745600



/* IOCTL commands (Request codes) */

/**
 * @defgroup IoctlCommandDefines Defines for IOCTL Commands
 *
 * @brief IOCTL Commands (Request codes) which can be used
 *        with @ref ixUARTIoctl
 */


/**
 * @ingroup IoctlCommandDefines
 *
 * @def IX_BAUD_SET
 *
 * @brief Set the baud rate
 */
#define IX_BAUD_SET	0

/**
 * @ingroup IoctlCommandDefines
 *
 * @def IX_BAUD_GET
 *
 * @brief Get the baud rate
 */
#define IX_BAUD_GET	1

/**
 * @ingroup IoctlCommandDefines
 * @def IX_MODE_SET
 * @brief Set the UART mode of operation
 */
#define IX_MODE_SET	2

/**
 * @ingroup IoctlCommandDefines
 *
 * @def IX_MODE_GET
 *
 * @brief Get the current UART mode of operation
 */
#define IX_MODE_GET	3

/**
 * @ingroup IoctlCommandDefines
 *
 * @def IX_OPTS_SET
 *
 * @brief Set the UART device options
 */
#define IX_OPTS_SET	4

/**
 * @ingroup IoctlCommandDefines
 *
 * @def IX_OPTS_GET
 *
 * @brief Get the UART device options
 */
#define IX_OPTS_GET	5

/**
 * @ingroup IoctlCommandDefines
 *
 * @def IX_STATS_GET
 *
 * @brief Get the UART statistics
 */
#define IX_STATS_GET	6


/* POSIX style ioctl arguments */

/**
 * @defgroup IoctlArgDefines Defines for IOCTL Arguments
 *
 * @brief POSIX style IOCTL arguments which can be used
 *        with @ref ixUARTIoctl
 *
 * @sa ixUARTMode
 */


/**
 * @ingroup IoctlArgDefines
 *
 * @def CLOCAL
 *
 * @brief Software flow control
 */
#ifdef CLOCAL
#undef CLOCAL
#endif
#define CLOCAL		0x1

/**
 * @ingroup IoctlArgDefines
 *
 * @def CREAD
 *
 * @brief Enable interrupt receiver
 */
#ifdef CREAD
#undef CREAD
#endif
#define CREAD		0x2

/**
 * @ingroup IoctlArgDefines
 *
 * @def CSIZE
 *
 * @brief Characters size
 */
#ifdef CSIZE
#undef CSIZE
#endif
#define CSIZE		0xc

/**
 * @ingroup IoctlArgDefines
 *
 * @def CS5
 *
 * @brief 5 bits
 */
#ifdef CS5
#undef CS5
#endif
#define CS5		0x0

/**
 * @ingroup IoctlArgDefines
 *
 * @def CS6
 *
 * @brief 6 bits
 */
#ifdef CS6
#undef CS6
#endif
#define CS6		0x4

/**
 * @ingroup IoctlArgDefines
 *
 * @def CS7
 *
 * @brief 7 bits
 */
#ifdef CS7
#undef CS7
#endif
#define CS7		0x8

/**
 * @ingroup IoctlArgDefines
 *
 * @def CS8
 *
 * @brief 8 bits
 */
#ifdef CS8
#undef CS8
#endif
#define CS8		0xc

/**
 * @ingroup IoctlArgDefines
 *
 * @def STOPB
 *
 * @brief Send two stop bits (else one)
 */
#define STOPB		0x20

/**
 * @ingroup IoctlArgDefines
 *
 * @def PARENB
 *
 * @brief Parity detection enabled (else disabled)
 */
#ifdef PARENB
#undef PARENB
#endif
#define PARENB		0x40

/**
 * @ingroup IoctlArgDefines
 *
 * @def PARODD
 *
 * @brief Odd parity  (else even)
 */
#ifdef PARODD
#undef PARODD
#endif
#define PARODD		0x80

/**
 * @enum ixUARTMode
 * @brief The mode to set to UART to.
 */
typedef enum
{
    INTERRUPT=0,	/**< Interrupt mode */
    POLLED,		/**< Polled mode */
    LOOPBACK		/**< Loopback mode */
} ixUARTMode;

/**
 * @struct ixUARTStats
 * @brief Statistics for the UART.
 */
typedef struct
{
    UINT32  rxCount;
    UINT32  txCount;	
    UINT32  overrunErr;
    UINT32  parityErr;
    UINT32  framingErr;
    UINT32  breakErr;
} ixUARTStats;

/**
 * @struct ixUARTDev
 * @brief Device descriptor for the UART.
 */
typedef struct
{
    UINT8 *addr;	/**< device base address */
    ixUARTMode mode;	/**< interrupt, polled or loopback */
    int baudRate;	/**< baud rate */
    int freq;		/**< UART clock frequency */     
    int options;	/**< hardware options */
    int fifoSize;	/**< FIFO xmit size */

    ixUARTStats   stats;	/**< device statistics */
} ixUARTDev;

/**
 * @ingroup IxUARTAccAPI
 *
 * @fn IX_STATUS ixUARTInit(ixUARTDev* pUART)
 * 
 * @param pUART	@ref ixUARTDev [in] - pointer to UART structure describing our device.
 * 
 * @brief Initialise the UART. This puts the chip in a quiescent state.
 * 
 * @pre The base address for the UART must contain a valid value.
 *      Also the baud rate and hardware options must contain sensible values
 *      otherwise the defaults will be used as defined in ixUART.h
 *
 * @post UART is initialized and ready to send and receive data.
 *
 * @note This function should only be called once per device.
 *
 * @retval IX_SUCCESS - UART device successfully initialised.
 * @retval IX_FAIL - Critical error, device not initialised.
 ***************************************************************************/
PUBLIC IX_STATUS ixUARTInit(ixUARTDev* pUART);

/**
 * @ingroup IxUARTAccAPI
 *
 * @fn IX_STATUS ixUARTPollOutput(ixUARTDev* pUART, int outChar)
 *
 * @param pUART @ref ixUARTDev [out] - pointer to UART structure describing our device.
 * @param outChar int [out] - character to transmit.
 *
 * @brief Transmit a character in polled mode.
 *
 * @pre UART device must be initialised.
 *
 * @retval IX_SUCCESS - character was successfully transmitted.
 * @retval IX_FAIL - output buffer is full (try again).
 ***************************************************************************/
PUBLIC IX_STATUS ixUARTPollOutput(ixUARTDev* pUART, int outChar);

/**
 * @ingroup IxUARTAccAPI
 *
 * @fn IX_STATUS ixUARTPollInput(ixUARTDev* pUART, char *inChar)
 *
 * @param pUART	@ref ixUARTDev [in] - pointer to UART structure describing our device.
 * @param *inChar char [in] - character read from the device.
 *
 * @brief Receive a character in polled mode.
 *
 * @pre UART device must be initialised.
 *
 * @retval IX_SUCCESS - character was successfully read.
 * @retval IX_FAIL - input buffer empty (try again).
 ***************************************************************************/
PUBLIC IX_STATUS ixUARTPollInput(ixUARTDev* pUART, char *inChar);

/**
 * @ingroup IxUARTAccAPI
 *
 * @fn IX_STATUS ixUARTIoctl(ixUARTDev* pUART, int cmd, void* arg)
 *
 * @param pUART	@ref ixUARTDev [in] - pointer to UART structure describing our device.
 * @param cmd int [in] - an ioctl request code.
 * @param arg void* [in] - optional argument used to set the device mode,
 *	 baud rate, and hardware options.
 *
 * @brief Perform I/O control routines on the device.
 *
 * @retval IX_SUCCESS - requested feature was set/read successfully.
 * @retval IX_FAIL - error setting/reading the requested feature.
 *
 * @sa IoctlCommandDefines
 * @sa IoctlArgDefines
 ***************************************************************************/    
PUBLIC IX_STATUS ixUARTIoctl(ixUARTDev* pUART, int cmd, void* arg);

/**
 * @} defgroup IxUARTAcc
 */
