/**
 * @file IxSspAcc.h
 *
 * @brief  Header file for the IXP400 SSP Serial Port Access (IxSspAcc)
 *
 * @version $Revision: 0.1 $
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
 * SPDX-License-Identifier:	BSD-3-Clause
 * @par
 * -- End of Copyright Notice --
 */

/**
 * @defgroup IxSspAcc IXP400 SSP Serial Port Access (IxSspAcc) API 
 *
 * @brief IXP400 SSP Serial Port Access Public API
 *
 * @{
 */
#ifndef IXSSPACC_H
#define IXSSPACC_H

#ifdef __ixp46X

#include "IxOsal.h"

/*
 * Section for enum
 */
/**
 * @ingroup IxSspAcc
 * 
 * @enum IxSspAccDataSize
 *
 * @brief The data sizes in bits that are supported by the protocol
 */
typedef enum
{
	DATA_SIZE_TOO_SMALL = 0x2,
	DATA_SIZE_4 = 0x3,
	DATA_SIZE_5,
	DATA_SIZE_6,
	DATA_SIZE_7,
	DATA_SIZE_8,
	DATA_SIZE_9,
	DATA_SIZE_10,
	DATA_SIZE_11,
	DATA_SIZE_12,
	DATA_SIZE_13,
	DATA_SIZE_14,
	DATA_SIZE_15,
	DATA_SIZE_16,
	DATA_SIZE_TOO_BIG
} IxSspAccDataSize;

/**
 * @ingroup IxSspAcc
 * 
 * @enum IxSspAccPortStatus
 *
 * @brief The status of the SSP port to be set to enable/disable
 */
typedef enum
{
	SSP_PORT_DISABLE = 0x0,
	SSP_PORT_ENABLE,
	INVALID_SSP_PORT_STATUS
} IxSspAccPortStatus;

/**
 * @ingroup IxSspAcc
 * 
 * @enum IxSspAccFrameFormat
 *
 * @brief The frame format that is to be used - SPI, SSP, or Microwire
 */
typedef enum
{
	SPI_FORMAT = 0x0,
	SSP_FORMAT,
	MICROWIRE_FORMAT,
	INVALID_FORMAT
} IxSspAccFrameFormat;

/**
 * @ingroup IxSspAcc
 * 
 * @enum IxSspAccClkSource
 *
 * @brief The source to produce the SSP serial clock
 */
typedef enum
{
	ON_CHIP_CLK = 0x0,
	EXTERNAL_CLK,
	INVALID_CLK_SOURCE
} IxSspAccClkSource;

/**
 * @ingroup IxSspAcc
 * 
 * @enum IxSspAccSpiSclkPhase
 *
 * @brief The SPI SCLK Phase: 
 * 0 - SCLK is inactive one cycle at the start of a frame and 1/2 cycle at the
 *		end of a frame.
 * 1 - SCLK is inactive 1/2 cycle at the start of a frame and one cycle at the
 *		end of a frame.
 */
typedef enum
{
	START_ONE_END_HALF = 0x0,
	START_HALF_END_ONE,
	INVALID_SPI_PHASE
} IxSspAccSpiSclkPhase;

/**
 * @ingroup IxSspAcc
 * 
 * @enum IxSspAccSpiSclkPolarity
 *
 * @brief The SPI SCLK Polarity can be set to either low or high.
 */
typedef enum
{
	SPI_POLARITY_LOW = 0x0,
	SPI_POLARITY_HIGH,
	INVALID_SPI_POLARITY
} IxSspAccSpiSclkPolarity;

/**
 * @ingroup IxSspAcc
 * 
 * @enum IxSspAccMicrowireCtlWord
 *
 * @brief The Microwire control word can be either 8 or 16 bit.
 */
typedef enum
{
	MICROWIRE_8_BIT = 0x0,
	MICROWIRE_16_BIT,
	INVALID_MICROWIRE_CTL_WORD
} IxSspAccMicrowireCtlWord;

/**
 * @ingroup IxSspAcc
 * 
 * @enum IxSspAccFifoThreshold
 *
 * @brief The threshold in frames (each frame is defined by IxSspAccDataSize)
 *			that can be set for the FIFO to trigger a threshold exceed when
 *			checking with the ExceedThresholdCheck functions or an interrupt
 *			when it is enabled.
 */
typedef enum
{
	FIFO_TSHLD_1 =	0x0,
	FIFO_TSHLD_2,
	FIFO_TSHLD_3,
	FIFO_TSHLD_4,
	FIFO_TSHLD_5,
	FIFO_TSHLD_6,
	FIFO_TSHLD_7,
	FIFO_TSHLD_8,
	FIFO_TSHLD_9,
	FIFO_TSHLD_10,
	FIFO_TSHLD_11,
	FIFO_TSHLD_12,
	FIFO_TSHLD_13,
	FIFO_TSHLD_14,
	FIFO_TSHLD_15,
	FIFO_TSHLD_16,
	INVALID_FIFO_TSHLD
} IxSspAccFifoThreshold;

/**
 * @ingroup IxSspAcc
 * 
 * @enum IX_SSP_STATUS
 *
 * @brief The statuses that can be returned in a SSP Serial Port Access
 */
typedef enum
{
	IX_SSP_SUCCESS = IX_SUCCESS, /**< Success status */
	IX_SSP_FAIL, /**< Fail status */
	IX_SSP_RX_FIFO_OVERRUN_HANDLER_MISSING, /**<
						Rx FIFO Overrun handler is NULL. */
	IX_SSP_RX_FIFO_HANDLER_MISSING, /**<
					Rx FIFO threshold hit or above handler is NULL
					*/
	IX_SSP_TX_FIFO_HANDLER_MISSING, /**<
					Tx FIFO threshold hit or below handler is NULL
					*/
	IX_SSP_FIFO_NOT_EMPTY_FOR_SETTING_CTL_CMD, /**<
					Tx FIFO not empty and therefore microwire
					control command size setting is not allowed.	*/
	IX_SSP_INVALID_FRAME_FORMAT_ENUM_VALUE, /**<
					frame format selected is invalid. */
	IX_SSP_INVALID_DATA_SIZE_ENUM_VALUE, /**<
					data size selected is invalid. */
	IX_SSP_INVALID_CLOCK_SOURCE_ENUM_VALUE, /**<
					source clock selected is invalid. */
	IX_SSP_INVALID_TX_FIFO_THRESHOLD_ENUM_VALUE, /**<
					Tx FIFO threshold selected is invalid. */
	IX_SSP_INVALID_RX_FIFO_THRESHOLD_ENUM_VALUE, /**<
					Rx FIFO threshold selected is invalid. */
	IX_SSP_INVALID_SPI_PHASE_ENUM_VALUE, /**<
					SPI phase selected is invalid. */
	IX_SSP_INVALID_SPI_POLARITY_ENUM_VALUE, /**<
					SPI polarity selected is invalid. */
	IX_SSP_INVALID_MICROWIRE_CTL_CMD_ENUM_VALUE, /**<
					Microwire control command selected is invalid
					*/
	IX_SSP_INT_UNBIND_FAIL, /**< Interrupt unbind fail to unbind SSP
	                interrupt */
	IX_SSP_INT_BIND_FAIL, /**< Interrupt bind fail during init */
	IX_SSP_RX_FIFO_NOT_EMPTY, /**<
					Rx FIFO not empty while trying to change data
					size. */
	IX_SSP_TX_FIFO_NOT_EMPTY, /**<
					Rx FIFO not empty while trying to change data
					size or microwire control command size.	*/
	IX_SSP_POLL_MODE_BLOCKING, /**<
					poll mode selected blocks interrupt mode from
					being selected.	*/
	IX_SSP_TX_FIFO_HIT_BELOW_THRESHOLD, /**<
					Tx FIFO level hit or below threshold. */
	IX_SSP_TX_FIFO_EXCEED_THRESHOLD, /**<
					Tx FIFO level exceeded threshold. */
	IX_SSP_RX_FIFO_HIT_ABOVE_THRESHOLD, /**<
					Rx FIFO level hit or exceeded threshold. */
	IX_SSP_RX_FIFO_BELOW_THRESHOLD, /**<
					Rx FIFO level below threshold. */
	IX_SSP_BUSY, /**< SSP is busy. */
	IX_SSP_IDLE, /**< SSP is idle. */
	IX_SSP_OVERRUN_OCCURRED, /**<
					SSP has experienced an overrun. */
	IX_SSP_NO_OVERRUN, /**<
					SSP did not experience an overrun. */
	IX_SSP_NOT_SUPORTED, /**< hardware does not support SSP */
	IX_SSP_NOT_INIT, /**< SSP Access not intialized */
	IX_SSP_NULL_POINTER /**< parameter passed in is NULL */
} IX_SSP_STATUS;

/**
 * @ingroup IxSspAcc
 *
 * @brief SSP Rx FIFO Overrun handler
 *
 * This function is called for the client to handle Rx FIFO Overrun that occurs
 * in the SSP hardware
 */
typedef void (*RxFIFOOverrunHandler)(void);

/**
 * @ingroup IxSspAcc
 * 
 * @brief SSP Rx FIFO Threshold hit or above handler
 *
 * This function is called for the client to handle Rx FIFO threshold hit or
 * or above that occurs in the SSP hardware
 */
typedef void (*RxFIFOThresholdHandler)(void);

/**
 * @ingroup IxSspAcc
 * 
 * @brief SSP Tx FIFO Threshold hit or below handler
 *
 * This function is called for the client to handle Tx FIFO threshold hit or
 * or below that occurs in the SSP hardware
 */
typedef void (*TxFIFOThresholdHandler)(void);


/*
 * Section for struct
 */
/**
 * @ingroup IxSspAcc
 *
 * @brief contains all the variables required to initialize the SSP serial port
 *		hardware.
 * 
 * Structure to be filled and used for calling initialization
 */
typedef struct
{
	IxSspAccFrameFormat FrameFormatSelected;/**<Select between SPI, SSP and
												Microwire. */
	IxSspAccDataSize DataSizeSelected;		/**<Select between 4 and 16. */
	IxSspAccClkSource ClkSourceSelected;	/**<Select clock source to be
												on-chip or external. */
	IxSspAccFifoThreshold TxFIFOThresholdSelected;
											/**<Select Tx FIFO threshold
												between 1 to 16. */
	IxSspAccFifoThreshold RxFIFOThresholdSelected;
											/**<Select Rx FIFO threshold
												between 1 to 16. */
	BOOL RxFIFOIntrEnable;					/**<Enable/disable Rx FIFO
												threshold interrupt. Disabling
												this interrupt will require
												the use of the polling function
												RxFIFOExceedThresholdCheck. */
	BOOL TxFIFOIntrEnable;					/**<Enable/disable Tx FIFO
												threshold interrupt. Disabling
												this interrupt will require
												the use of the polling function
												TxFIFOExceedThresholdCheck. */
	RxFIFOThresholdHandler RxFIFOThsldHdlr;	/**<Pointer to function to handle
												a Rx FIFO interrupt. */
	TxFIFOThresholdHandler TxFIFOThsldHdlr;	/**<Pointer to function to handle
												a Tx FIFO interrupt. */
	RxFIFOOverrunHandler RxFIFOOverrunHdlr;	/**<Pointer to function to handle
												a Rx FIFO overrun interrupt. */
	BOOL LoopbackEnable;					/**<Select operation mode to be
												normal or loopback mode. */
	IxSspAccSpiSclkPhase SpiSclkPhaseSelected;
											/**<Select SPI SCLK phase to start
												with one inactive cycle and end
												with 1/2 inactive cycle or
												start with 1/2 inactive cycle
												and end with one inactive
												cycle. (Only used in
												SPI format). */
	IxSspAccSpiSclkPolarity SpiSclkPolaritySelected;
											/**<Select SPI SCLK idle state
												to be low or high. (Only used in
												SPI format). */
	IxSspAccMicrowireCtlWord MicrowireCtlWordSelected;
											/**<Select Microwire control
												format to be 8 or 16-bit. (Only
												used in Microwire format). */
	UINT8 SerialClkRateSelected;			/**<Select between 0 (1.8432Mbps)
												and 255 (7.2Kbps). The
												formula used is	Bit rate = 
												3.6864x10^6 / 
												(2 x (SerialClkRateSelect + 1))
												*/
} IxSspInitVars;

/**
 * @ingroup IxSspAcc
 *
 * @brief contains counters of the SSP statistics
 * 
 * Structure contains all values of counters and associated overflows.
 */
typedef struct
{
	UINT32 ixSspRcvCounter;		/**<Total frames received. */
	UINT32 ixSspXmitCounter;	/**<Total frames transmitted. */
	UINT32 ixSspOverflowCounter;/**<Total occurrences of overflow. */
} IxSspAccStatsCounters;


/*
 * Section for prototypes interface functions
 */

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccInit (
	IxSspInitVars *initVarsSelected);
 *
 * @brief Initializes the SSP Access module.
 *
 * @param "IxSspAccInitVars [in] *initVarsSelected" - struct containing required
 *			variables for initialization 
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will initialize the SSP Serial Port hardware to the user specified
 * configuration. Then it will enable the SSP Serial Port.
 * *NOTE*: Once interrupt or polling mode is selected, the mode cannot be
 * changed via the interrupt enable/disable function but the init needs to be
 * called again to change it.
 *
 * @return 
 *      - IX_SSP_SUCCESS - Successfully initialize and enable the SSP
 *									serial port.
 *		- IX_SSP_RX_FIFO_HANDLER_MISSING - interrupt mode is selected but RX FIFO
 *									handler pointer is NULL
 *		- IX_SSP_TX_FIFO_HANDLER_MISSING - interrupt mode is selected but TX FIFO
 *									handler pointer is NULL
 *		- IX_SSP_RX_FIFO_OVERRUN_HANDLER_MISSING - interrupt mode is selected but
 *									RX FIFO Overrun handler pointer is NULL
 *		- IX_SSP_RX_FIFO_NOT_EMPTY - Rx FIFO not empty, data size change is not
 *			                        allowed.
 *		- IX_SSP_TX_FIFO_NOT_EMPTY - Tx FIFO not empty, data size change is not
 *			                        allowed.
 *		- IX_SSP_INVALID_FRAME_FORMAT_ENUM_VALUE - frame format selected is invalid
 *		- IX_SSP_INVALID_DATA_SIZE_ENUM_VALUE - data size selected is invalid
 *		- IX_SSP_INVALID_CLOCK_SOURCE_ENUM_VALUE - clock source selected is invalid
 *		- IX_SSP_INVALID_TX_FIFO_THRESHOLD_ENUM_VALUE - Tx FIFO threshold level
 *									selected is invalid
 *		- IX_SSP_INVALID_RX_FIFO_THRESHOLD_ENUM_VALUE - Rx FIFO threshold level
 *									selected is invalid
 *		- IX_SSP_INVALID_SPI_PHASE_ENUM_VALUE - SPI phase selected is invalid
 *		- IX_SSP_INVALID_SPI_POLARITY_ENUM_VALUE - SPI polarity selected is invalid
 *		- IX_SSP_INVALID_MICROWIRE_CTL_CMD_ENUM_VALUE - microwire control command
 *									size is invalid
 *      - IX_SSP_INT_UNBIND_FAIL - interrupt handler failed to unbind SSP interrupt
 *		- IX_SSP_INT_BIND_FAIL - interrupt handler failed to bind to SSP interrupt
 *									hardware trigger
 *      - IX_SSP_NOT_SUPORTED - hardware does not support SSP
 *      - IX_SSP_NULL_POINTER - parameter passed in is NULL
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccInit (IxSspInitVars *initVarsSelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccUninit (
	void)
 *
 * @brief Un-initializes the SSP Serial Port Access component
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will disable the SSP Serial Port hardware. The client can call the
 * init function again if they wish to enable the SSP.
 *
 * @return 
 *      - IX_SSP_SUCCESS - successfully uninit SSP component
 *      - IX_SSP_INT_UNBIND_FAIL - interrupt handler failed to unbind SSP interrupt
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccUninit (void);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccFIFODataSubmit (
	UINT16 *data,
	UINT32 amtOfData)
 *
 * @brief Inserts data into the SSP Serial Port's FIFO
 *
 * @param	"UINT16 [in] *data" - pointer to the location to transmit the data
 *				from
 *			"UINT32 [in] amtOfData" - number of data to be transmitted.
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will insert the amount of data specified by "amtOfData" from buffer
 * pointed to by "data" into the FIFO to be transmitted by the hardware.
 *
 * @return 
 *      - IX_SSP_SUCCESS - Data inserted successfully into FIFO
 *		- IX_SSP_FAIL - FIFO insufficient space
 *		- IX_SSP_NULL_POINTER - data pointer passed by client is NULL
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccFIFODataSubmit (
	UINT16* data,
	UINT32 amtOfData);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccFIFODataReceive (
	UINT16 *data,
	UINT32 amtOfData)
 *
 * @brief Extract data from the SSP Serial Port's FIFO
 *
 * @param	"UINT16 [in] *data" - pointer to the location to receive the data into
 *			"UINT32 [in] amtOfData" - number of data to be received.
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will extract the amount of data specified by "amtOfData" from the
 * FIFO already received by the hardware into the buffer pointed to by "data".
 *
 * @return 
 *      - IX_SSP_SUCCESS - Data extracted successfully from FIFO
 *		- IX_SSP_FAIL - FIFO has no data
 *		- IX_SSP_NULL_POINTER - data pointer passed by client is NULL
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccFIFODataReceive (
	UINT16* data,
	UINT32 amtOfData);


/**
 * Polling Functions
 */

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccTxFIFOHitOrBelowThresholdCheck (
		void)
 *
 * @brief Check if the Tx FIFO threshold has been hit or fallen below.
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will return whether the Tx FIFO threshold has been exceeded or not
 *
 * @return 
 *      - IX_SSP_TX_FIFO_HIT_BELOW_THRESHOLD - Tx FIFO level hit or below threshold .
 *		- IX_SSP_TX_FIFO_EXCEED_THRESHOLD - Tx FIFO level exceeded threshold.
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccTxFIFOHitOrBelowThresholdCheck (
	void);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccRxFIFOHitOrAboveThresholdCheck (
		void)
 *
 * @brief Check if the Rx FIFO threshold has been hit or exceeded.
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will return whether the Rx FIFO level is below threshold or not
 *
 * @return 
 *      - IX_SSP_RX_FIFO_HIT_ABOVE_THRESHOLD - Rx FIFO level hit or exceeded threshold
 *		- IX_SSP_RX_FIFO_BELOW_THRESHOLD - Rx FIFO level below threshold
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOHitOrAboveThresholdCheck (
	void);


/**
 * Configuration functions
 *
 * NOTE: These configurations are not required to be called once init is called
 * unless configurations need to be changed on the fly.
 */

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccSSPPortStatusSet (
	IxSspAccPortStatus portStatusSelected)
 *
 * @brief Enables/disables the SSP Serial Port hardware.
 *
 * @param "IxSspAccPortStatus [in] portStatusSelected" - Set the SSP port to
 *			enable or disable
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will enable/disable the SSP Serial Port hardware.
 * NOTE: This function is called by init to enable the SSP after setting up the
 * configurations and by uninit to disable the SSP.
 *
 * @return 
 *      - IX_SSP_SUCCESS - Port status set with valid enum value
 *		- IX_SSP_FAIL - invalid enum value
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccSSPPortStatusSet (
	IxSspAccPortStatus portStatusSelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccFrameFormatSelect (
	IxSspAccFrameFormat frameFormatSelected)
 *
 * @brief Sets the frame format for the SSP Serial Port hardware
 *
 * @param "IxSspAccFrameFormat [in] frameFormatSelected" - The frame format of
 *			SPI, SSP or Microwire can be selected as the format
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will set the format for the transfers via user input.
 * *NOTE*: The SSP hardware will be disabled to clear the FIFOs. Then its
 * previous state (enabled/disabled) restored after changing the format.
 *
 * @return 
 *      - IX_SSP_SUCCESS - frame format set with valid enum value
 *		- IX_SSP_INVALID_FRAME_FORMAT_ENUM_VALUE - invalid frame format value
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccFrameFormatSelect (
	IxSspAccFrameFormat frameFormatSelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccDataSizeSelect (
	IxSspAccDataSize dataSizeSelected)
 *
 * @brief Sets the data size for transfers
 *
 * @param "IxSspAccDataSize [in] dataSizeSelected" - The data size between 4
 *			and 16 that can be selected for data transfers
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will set the data size for the transfers via user input. It will
 * disallow the change of the data size if either of the Rx/Tx FIFO is not
 * empty to prevent data loss.
 * *NOTE*: The SSP port will be disabled if the FIFOs are found to be empty and
 *			if between the check and disabling of the SSP (which clears the
 *			FIFOs) data is received into the FIFO, it might be lost.
 * *NOTE*: The FIFOs can be cleared by disabling the SSP Port if necessary to
 *			force the data size change.
 *
 * @return 
 *      - IX_SSP_SUCCESS - data size set with valid enum value
 *		- IX_SSP_RX_FIFO_NOT_EMPTY - Rx FIFO not empty, data size change is not
 *							allowed.
 *		- IX_SSP_TX_FIFO_NOT_EMPTY - Tx FIFO not empty, data size change is not
 *							allowed.
 *		- IX_SSP_INVALID_DATA_SIZE_ENUM_VALUE - invalid enum value
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccDataSizeSelect (
	IxSspAccDataSize dataSizeSelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccClockSourceSelect(
	IxSspAccClkSource clkSourceSelected)
 *
 * @brief Sets the clock source of the SSP Serial Port hardware
 *
 * @param "IxSspAccClkSource [in] clkSourceSelected" - The clock source from
 *			either external source on on-chip can be selected as the source
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will set the clock source for the transfers via user input.
 *
 * @return 
 *      - IX_SSP_SUCCESS - clock source set with valid enum value
 *		- IX_SSP_INVALID_CLOCK_SOURCE_ENUM_VALUE - invalid enum value
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccClockSourceSelect (
	IxSspAccClkSource clkSourceSelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccSerialClockRateConfigure (
	UINT8 serialClockRateSelected)
 *
 * @brief Sets the on-chip Serial Clock Rate of the SSP Serial Port hardware.
 *
 * @param "UINT8 [in] serialClockRateSelected" - The serial clock rate that can
 *			be set is between 7.2Kbps and 1.8432Mbps. The formula used is
 *			Bit rate = 3.6864x10^6 / (2 x (SerialClockRateSelected + 1))
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will set the serial clock rate for the transfers via user input.
 *
 * @return 
 *      - IX_SSP_SUCCESS - Serial clock rate configured successfully
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccSerialClockRateConfigure (
	UINT8 serialClockRateSelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccRxFIFOIntEnable (
	RxFIFOThresholdHandler rxFIFOIntrHandler)
 *
 * @brief Enables service request interrupt whenever the Rx FIFO hits its
 *		threshold
 *
 * @param "void [in] *rxFIFOIntrHandler(UINT32)" - function pointer to the
 *				interrupt handler for the Rx FIFO exceeded.
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will enable the service request interrupt for the Rx FIFO
 *
 * @return 
 *      - IX_SSP_SUCCESS - Rx FIFO level interrupt enabled successfully
 *		- IX_SSP_RX_FIFO_HANDLER_MISSING - missing handler for Rx FIFO level interrupt
 *		- IX_SSP_POLL_MODE_BLOCKING - poll mode is selected at init, interrupt not
 *				allowed to be enabled. Use init to enable interrupt mode.
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOIntEnable (
	RxFIFOThresholdHandler rxFIFOIntrHandler);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccRxFIFOIntDisable (
	void)
 *
 * @brief Disables service request interrupt of the Rx FIFO.
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will disable the service request interrupt of the Rx FIFO.
 *
 * @return 
 *		- IX_SSP_SUCCESS - Rx FIFO Interrupt disabled successfully
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOIntDisable (
	void);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccTxFIFOIntEnable (
	TxFIFOThresholdHandler txFIFOIntrHandler)
 *
 * @brief Enables service request interrupt of the Tx FIFO.
 *
 * @param "void [in] *txFIFOIntrHandler(UINT32)" - function pointer to the
 *				interrupt handler for the Tx FIFO exceeded.
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will enable the service request interrupt of the Tx FIFO.
 *
 * @return 
 *      - IX_SSP_SUCCESS - Tx FIFO level interrupt enabled successfully
 *		- IX_SSP_TX_FIFO_HANDLER_MISSING - missing handler for Tx FIFO level interrupt
 *		- IX_SSP_POLL_MODE_BLOCKING - poll mode is selected at init, interrupt not
 *				allowed to be enabled. Use init to enable interrupt mode.
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccTxFIFOIntEnable (
	TxFIFOThresholdHandler txFIFOIntrHandler);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccTxFIFOIntDisable (
	void)
 *
 * @brief Disables service request interrupt of the Tx FIFO
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will disable the service request interrupt of the Tx FIFO
 *
 * @return 
 *		- IX_SSP_SUCCESS - Tx FIFO Interrupt disabled successfuly.
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccTxFIFOIntDisable (
	void);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccLoopbackEnable (
	BOOL loopbackEnable)
 *
 * @brief Enables/disables the loopback mode
 *
 * @param "BOOL [in] loopbackEnable" - true to enable and false to disable.
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will set the mode of operation to either loopback or normal mode
 * according to the user input.
 *
 * @return 
 *		- IX_SSP_SUCCESS - Loopback enabled successfully
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccLoopbackEnable (
	BOOL loopbackEnable);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccSpiSclkPolaritySet (
	IxSspAccSpiSclkPolarity spiSclkPolaritySelected)
 *
 * @brief Sets the SPI SCLK Polarity to Low or High
 *
 * @param - "IxSspAccSpiSclkPolarity [in] spiSclkPolaritySelected" - SPI SCLK
 *				polarity that can be selected to either high or low
 *
 * Global Data	:
 *		- None.
 *                        
 * This API is only used for the SPI frame format and will set the SPI SCLK polarity
 * to either low or high
 *
 * @return 
 *      - IX_SSP_SUCCESS - SPI Sclk polarity set with valid enum value
 *		- IX_SSP_INVALID_SPI_POLARITY_ENUM_VALUE - invalid SPI polarity value
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccSpiSclkPolaritySet (
	IxSspAccSpiSclkPolarity spiSclkPolaritySelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccSpiSclkPhaseSet (
	IxSspAccSpiSclkPhase spiSclkPhaseSelected)
 *
 * @brief Sets the SPI SCLK Phase
 *
 * @param "IxSspAccSpiSclkPhase [in] spiSclkPhaseSelected" - Phase of either
 *			the SCLK is inactive one cycle at the start of a frame and 1/2
 *			cycle at the end of a frame, OR
 *			the SCLK is inactive 1/2 cycle at the start of a frame and one
 *			cycle at the end of a frame.
 *
 * Global Data	:
 *		- IX_SSP_SUCCESS - SPI Sclk phase set with valid enum value
 *		- IX_SSP_INVALID_SPI_PHASE_ENUM_VALUE - invalid SPI phase value
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *                        
 * This API is only used for the SPI frame format and will set the SPI SCLK
 * phase according to user input.
 *
 * @return 
 *      - None
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccSpiSclkPhaseSet (
	IxSspAccSpiSclkPhase spiSclkPhaseSelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccMicrowireControlWordSet (
	IxSspAccMicrowireCtlWord microwireCtlWordSelected)
 *
 * @brief Sets the Microwire control word to 8 or 16 bit format
 *
 * @param "IxSspAccMicrowireCtlWord [in] microwireCtlWordSelected" - Microwire
 *			control word format can be either 8 or 16 bit format
 *
 * Global Data	:
 *		- None.
 *                        
 * This API is only used for the Microwire frame format and will set the
 * control word to 8 or 16 bit format
 *
 * @return 
 *      - IX_SSP_SUCCESS - Microwire Control Word set with valid enum value
 *		- IX_SSP_TX_FIFO_NOT_EMPTY - Tx FIFO not empty, data size change is not
 *							allowed.
 *		- IX_SSP_INVALID_MICROWIRE_CTL_CMD_ENUM_VALUE - invalid enum value
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccMicrowireControlWordSet (
	IxSspAccMicrowireCtlWord microwireCtlWordSelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccTxFIFOThresholdSet (
	IxSspAccFifoThreshold txFIFOThresholdSelected)
 *
 * @brief Sets the Tx FIFO Threshold.
 *
 * @param "IxSspAccFifoThreshold [in] txFIFOThresholdSelected" - Threshold that
 *		is set for a Tx FIFO service request to be triggered
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will set the threshold for a Tx FIFO threshold to be triggered
 *
 * @return 
 *      - IX_SSP_SUCCESS - Tx FIFO Threshold set with valid enum value
 *		- IX_SSP_INVALID_TX_FIFO_THRESHOLD_ENUM_VALUE - invalid enum value
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccTxFIFOThresholdSet (
	IxSspAccFifoThreshold txFIFOThresholdSelected);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccRxFIFOThresholdSet (
	IxSspAccFifoThreshold rxFIFOThresholdSelected)
 *
 * @brief Sets the Rx FIFO Threshold.
 *
 * @param "IxSspAccFifoThreshold [in] rxFIFOThresholdSelected" - Threshold that
 *		is set for a Tx FIFO service request to be triggered
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will will set the threshold for a Rx FIFO threshold to be triggered
 *
 * @return 
 *      - IX_SSP_SUCCESS - Rx FIFO Threshold set with valid enum value
 *		- IX_SSP_INVALID_RX_FIFO_THRESHOLD_ENUM_VALUE - invalid enum value
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOThresholdSet (
	IxSspAccFifoThreshold rxFIFOThresholdSelected);


/**
 * Debug functions
 */

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccStatsGet (
	IxSspAccStatsCounters *sspStats)
 *
 * @brief Returns the SSP Statistics through the pointer passed in
 *
 * @param "IxSspAccStatsCounters [in] *sspStats" - SSP statistics counter will
 *			be read and written to the location pointed by this pointer.
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will return the statistics counters of the SSP transfers.
 *
 * @return 
 *      - IX_SSP_SUCCESS - Stats obtained into the pointer provided successfully
 *		- IX_SSP_FAIL - client provided pointer is NULL
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccStatsGet (
	IxSspAccStatsCounters *sspStats);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccStatsReset (
	void)
 *
 * @brief Resets the SSP Statistics
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will reset the SSP statistics counters.
 *
 * @return 
 *      - None
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC void
ixSspAccStatsReset (
	void);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccShow (
		void)
 *
 * @brief Display SSP status registers and statistics counters.
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will display the status registers of the SSP and the statistics
 * counters.
 *
 * @return 
 *		- IX_SSP_SUCCESS - SSP show called successfully.
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccShow (
	void);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccSSPBusyCheck (
		void)
 *
 * @brief Determine the state of the SSP serial port hardware.
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will return the state of the SSP serial port hardware - busy or
 * idle
 *
 * @return 
 *      - IX_SSP_BUSY - SSP is busy
 *		- IX_SSP_IDLE - SSP is idle.
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccSSPBusyCheck (
	void);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccTxFIFOLevelGet (
		void)
 *
 * @brief Obtain the Tx FIFO's level
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will return the level of the Tx FIFO
 *
 * @return 
 *      - 0..16; 0 can also mean SSP not initialized and will need to be init.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC UINT8
ixSspAccTxFIFOLevelGet (
	void);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccRxFIFOLevelGet (
		void)
 *
 * @brief Obtain the Rx FIFO's level
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will return the level of the Rx FIFO
 *
 * @return 
 *      - 0..16; 0 can also mean SSP not initialized and will need to be init.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC UINT8
ixSspAccRxFIFOLevelGet (
	void);

/**
 * @ingroup IxSspAcc
 * 
 * @fn ixSspAccRxFIFOOverrunCheck (
		void)
 *
 * @brief Check if the Rx FIFO has overrun its FIFOs
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will return whether the Rx FIFO has overrun its 16 FIFOs
 *
 * @return 
 *      - IX_SSP_OVERRUN_OCCURRED - Rx FIFO overrun occurred
 *		- IX_SSP_NO_OVERRUN - Rx FIFO did not overrun
 *		- IX_SSP_NOT_INIT - SSP not initialized. SSP init needs to be called.
 *
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_SSP_STATUS
ixSspAccRxFIFOOverrunCheck (
	void);

#endif /* __ixp46X */
#endif /* IXSSPACC_H */
