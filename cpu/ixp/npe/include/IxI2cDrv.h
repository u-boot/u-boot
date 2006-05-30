/**
 * @file IxI2cDrv.h
 *
 * @brief  Header file for the IXP400 I2C Driver (IxI2cDrv)
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
 * @defgroup IxI2cDrv IXP400 I2C Driver(IxI2cDrv) API 
 *
 * @brief IXP400 I2C Driver Public API
 *
 * @{
 */
#ifndef IXI2CDRV_H
#define IXI2CDRV_H

#ifdef __ixp46X
#include "IxOsal.h"

/*
 * Section for #define
 */

/**
 * @ingroup IxI2cDrv
 * @brief The interval of micro/mili seconds the IXP will wait before it polls for
 * 			status from the ixI2cIntrXferStatus; Every 20us is 1 byte @
 * 			400Kbps and 4 bytes	@ 100Kbps. This is dependent on delay type selected
 *          through the API ixI2cDrvDelayTypeSelect.
 */
#define IX_I2C_US_POLL_FOR_XFER_STATUS	20

/**
 * @ingroup IxI2cDrv
 * @brief The number of tries that will be attempted to call a callback
 *          function if the callback does not or is unable to resolve the
 *          issue it is called to resolve
 */
#define IX_I2C_NUM_OF_TRIES_TO_CALL_CALLBACK_FUNC	10


/**
 * @ingroup IxI2cDrv
 * @brief Number of tries slave will poll the IDBR Rx full bit before it
 *			gives up
 */
#define IX_I2C_NUM_TO_POLL_IDBR_RX_FULL 0x100

/**
 * @ingroup IxI2cDrv
 * @brief Number of tries slave will poll the IDBR Tx empty bit before it
 *			gives up
 */
#define IX_I2C_NUM_TO_POLL_IDBR_TX_EMPTY 0x100

/*
 * Section for enum
 */

/**
 * @ingroup IxI2cDrv
 * 
 * @enum IxI2cMasterStatus
 *
 * @brief The master status - transfer complete, bus error or arbitration loss
 */
typedef enum
{
    IX_I2C_MASTER_XFER_COMPLETE = IX_SUCCESS,
	IX_I2C_MASTER_XFER_BUS_ERROR,
	IX_I2C_MASTER_XFER_ARB_LOSS
} IxI2cMasterStatus;


/**
 * @ingroup IxI2cDrv
 * 
 * @enum IX_I2C_STATUS
 *
 * @brief The status that can be returned in a I2C driver initialization
 */
typedef enum
{
	IX_I2C_SUCCESS = IX_SUCCESS, /**< Success status */
	IX_I2C_FAIL, /**< Fail status */
	IX_I2C_NOT_SUPPORTED, /**< hardware does not have dedicated I2C hardware */
	IX_I2C_NULL_POINTER, /**< parameter passed in is NULL */
	IX_I2C_INVALID_SPEED_MODE_ENUM_VALUE, /**< speed mode selected is invalid */
	IX_I2C_INVALID_FLOW_MODE_ENUM_VALUE, /**< flow mode selected is invalid */
	IX_I2C_SLAVE_ADDR_CB_MISSING, /**< slave callback is NULL */
	IX_I2C_GEN_CALL_CB_MISSING, /**< general callback is NULL */
	IX_I2C_INVALID_SLAVE_ADDR, /**< invalid slave address specified */
	IX_I2C_INT_BIND_FAIL, /**< interrupt bind fail */
	IX_I2C_INT_UNBIND_FAIL, /**< interrupt unbind fail */
	IX_I2C_NOT_INIT, /**< I2C is not initialized yet */
	IX_I2C_MASTER_BUS_BUSY, /**< master detected a I2C bus busy */
	IX_I2C_MASTER_ARB_LOSS, /**< master experienced arbitration loss */
	IX_I2C_MASTER_XFER_ERROR, /**< master experienced a transfer error */
	IX_I2C_MASTER_BUS_ERROR, /**< master detected a I2C bus error */
	IX_I2C_MASTER_NO_BUFFER, /**< no buffer provided for master transfer */
	IX_I2C_MASTER_INVALID_XFER_MODE, /**< xfer mode selected is invalid */
	IX_I2C_SLAVE_ADDR_NOT_DETECTED, /**< polled slave addr not detected */
	IX_I2C_GEN_CALL_ADDR_DETECTED, /**< polling detected general call */
	IX_I2C_SLAVE_READ_DETECTED, /**< polling detected slave read request */
	IX_I2C_SLAVE_WRITE_DETECTED, /**< polling detected slave write request */
	IX_I2C_SLAVE_NO_BUFFER, /**< no buffer provided for slave transfers */
	IX_I2C_DATA_SIZE_ZERO, /**< data size transfer is zero - invalid */
	IX_I2C_SLAVE_WRITE_BUFFER_EMPTY, /**< slave buffer is used till empty */
	IX_I2C_SLAVE_WRITE_ERROR, /**< slave write experienced an error */
	IX_I2C_SLAVE_OR_GEN_READ_BUFFER_FULL, /**< slave buffer is filled up */
	IX_I2C_SLAVE_OR_GEN_READ_ERROR /**< slave read experienced an error */
} IX_I2C_STATUS;

/**
 * @ingroup IxI2cDrv
 * 
 * @enum IxI2cSpeedMode
 *
 * @brief Type of speed modes supported by the I2C hardware.
 */
typedef enum
{
    IX_I2C_NORMAL_MODE = 0x0,
    IX_I2C_FAST_MODE
} IxI2cSpeedMode;

/**
 * @ingroup IxI2cDrv
 * 
 * @enum IxI2cXferMode
 *
 * @brief Used for indicating it is a repeated start or normal transfer
 */
typedef enum
{
    IX_I2C_NORMAL = 0x0,
    IX_I2C_REPEATED_START
} IxI2cXferMode;

/**
 * @ingroup IxI2cDrv
 * 
 * @enum IxI2cFlowMode
 *
 * @brief Used for indicating it is a poll or interrupt mode
 */
typedef enum
{
    IX_I2C_POLL_MODE = 0x0,
    IX_I2C_INTERRUPT_MODE
} IxI2cFlowMode;

/**
 * @ingroup IxI2cDrv
 * 
 * @enum IxI2cDelayMode
 *
 * @brief Used for selecting looping delay or OS scheduler delay
 */
typedef enum
{
    IX_I2C_LOOP_DELAY = 1,  /**< delay in microseconds */
    IX_I2C_SCHED_DELAY      /**< delay in miliseconds */
} IxI2cDelayMode;

/**
 * @ingroup IxI2cDrv
 * 
 * @brief The pointer to the function that will be called when the master
 *			has completed its receive. The parameter that is passed will
 *			provide the status of the read (success, arb loss, or bus
 *			error), the transfer mode (normal or repeated start, the
 *			buffer pointer and number of bytes transferred.
 */
typedef void (*IxI2cMasterReadCallbackP)(IxI2cMasterStatus, IxI2cXferMode, char*, UINT32);

/**
 * @ingroup IxI2cDrv
 * 
 * @brief The pointer to the function that will	be called when the master
 *			has completed its transmit. The parameter that is passed will
 *			provide the status of the write (success, arb loss, or buss
 *			error), the transfer mode (normal or repeated start), the
 *			buffer pointer and number of bytes transferred.
 */
typedef void (*IxI2cMasterWriteCallbackP)(IxI2cMasterStatus, IxI2cXferMode, char*, UINT32);

/**
 * @ingroup IxI2cDrv
 * 
 * @brief The pointer to the function that will be called when a slave
 *			address detected in interrupt mode for a read. The parameters
 *			that is passed will provide the read status, buffer pointer,
 *			buffer size, and the bytes received. When a start of a read
 *			is initiated there will be no buffer allocated and this callback
 *			will be called to request for a buffer. While receiving, if the
 *			buffer gets filled, this callback will be called to request for
 *			a new buffer while sending the filled buffer's pointer and size,
 *			and data size received. When the receive is complete, this
 *			callback will be called to process the data and free the memory
 *			by passing the buffer's pointer and size, and data size received.
 */
typedef void (*IxI2cSlaveReadCallbackP)(IX_I2C_STATUS, char*, UINT32, UINT32);

/**
 * @ingroup IxI2cDrv
 * 
 * @brief The pointer to the function that will be called when a slave
 *			address detected in interrupt mode for a write. The parameters
 *			that is passed will provide the write status, buffer pointer,
 *			buffer size, and the bytes received. When a start of a write is
 *			initiated there will be no buffer allocated and this callback
 *			will be called to request for a buffer and to fill it with data.
 *			While transmitting, if the data in the buffer empties, this
 *			callback will be called to request for more data to be filled in
 *			the same or new buffer. When the transmit is complete, this
 *			callback will be called to free the memory or other actions to
 *			be taken.
 */
typedef void (*IxI2cSlaveWriteCallbackP)(IX_I2C_STATUS, char*, UINT32, UINT32);

/**
 * @ingroup IxI2cDrv
 * 
 * @brief The pointer to the function that will be called when a general
 *			call detected in interrupt mode for a read. The parameters that
 *			is passed will provide the read status, buffer pointer, buffer
 *			size, and the bytes received. When a start of a read is
 *			initiated there will be no buffer allocated and this callback
 *			will be called to request for a buffer. While receiving, if the
 *			buffer gets filled, this callback will be called to request for
 *			a new buffer while sending the filled buffer's pointer and size,
 *			and data size received. When the receive is complete, this
 *			callback will be called to process the data and free the memory
 *			by passing the buffer's pointer and size, and data size received.
 */
typedef void (*IxI2cGenCallCallbackP)(IX_I2C_STATUS, char*, UINT32, UINT32);

/*
 * Section for struct
 */

/**
 * @brief contains all the variables required to initialize the I2C unit
 * 
 * Structure to be filled and used for calling initialization
 */
typedef struct
{
	IxI2cSpeedMode I2cSpeedSelect;	/**<Select either normal (100kbps)
									or fast mode (400kbps)*/
	IxI2cFlowMode I2cFlowSelect;	/**<Select interrupt or poll mode*/	
	IxI2cMasterReadCallbackP MasterReadCBP;
									/**<The master read callback pointer */
	IxI2cMasterWriteCallbackP MasterWriteCBP;
									/**<The master write callback pointer */
	IxI2cSlaveReadCallbackP SlaveReadCBP;
									/**<The slave read callback pointer */
	IxI2cSlaveWriteCallbackP SlaveWriteCBP;
									/**<The slave write callback pointer */
	IxI2cGenCallCallbackP GenCallCBP;
									/**<The general call callback pointer */
	BOOL I2cGenCallResponseEnable;	/**<Enable/disable the unit to
									respond to generall calls.*/
	BOOL I2cSlaveAddrResponseEnable;/**<Enable/disable the unit to
									respond to the slave address set in
									ISAR*/
	BOOL SCLEnable;					/**<Enable/disable the unit from
									driving the SCL line during master
									mode operation*/
	UINT8 I2cHWAddr;				/**<The address the unit will
									response to as a slave device*/
} IxI2cInitVars;

/**
 * @brief contains results of counters and their overflow
 * 
 * Structure contains all values of counters and associated overflows.
 */
typedef struct
{
	UINT32 ixI2cMasterXmitCounter;			/**<Total bytes transmitted as
											master.*/
	UINT32 ixI2cMasterFailedXmitCounter;	/**<Total bytes failed for
											transmission as master.*/
	UINT32 ixI2cMasterRcvCounter;			/**<Total bytes received as
											master.*/
	UINT32 ixI2cMasterFailedRcvCounter;		/**<Total bytes failed for
											receival as master.*/
	UINT32 ixI2cSlaveXmitCounter;			/**<Total bytes transmitted as
											slave.*/
	UINT32 ixI2cSlaveFailedXmitCounter;		/**<Total bytes failed for
											transmission as slave.*/
	UINT32 ixI2cSlaveRcvCounter;			/**<Total bytes received as
											slave.*/
	UINT32 ixI2cSlaveFailedRcvCounter;		/**<Total bytes failed for
											receival as slave.*/
	UINT32 ixI2cGenAddrCallSucceedCounter;	/**<Total bytes successfully
											transmitted for general address.*/
	UINT32 ixI2cGenAddrCallFailedCounter;	/**<Total bytes failed transmission
											for general address.*/
	UINT32 ixI2cArbLossCounter;				/**<Total instances of arbitration
											loss has occured.*/
} IxI2cStatsCounters;


/*
 * Section for prototypes interface functions
 */

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvInit(
	IxI2cInitVars *InitVarsSelected)
 *
 * @brief Initializes the I2C Driver.
 *
 * @param "IxI2cInitVars [in] *InitVarsSelected" - struct containing required
 *			variables for initialization 
 *
 * Global Data	:
 *		- None.
 *
 * This API will check if the hardware supports this I2C driver and the validity
 * of the parameters passed in. It will continue to process the parameters
 * passed in by setting the speed of the I2C unit (100kbps or 400kbps), setting
 * the flow to either interrupt or poll mode, setting the address of the I2C unit,
 * enabling/disabling the respond to General Calls, enabling/disabling the respond
 * to Slave Address and SCL line driving. If it is interrupt mode, then it will
 * register the callback routines for master, slavetransfer and general call receive.
 *
 * @return 
 *      - IX_I2C_SUCCESS - Successfully initialize and enable the I2C
 *							hardware.
 *		- IX_I2C_NOT_SUPPORTED - The hardware does not support or have a
 *                              dedicated I2C unit to support this driver
 *		- IX_I2C_NULL_POINTER - The parameter passed in is a NULL pointed
 *		- IX_I2C_INVALID_SPEED_MODE_ENUM_VALUE - The speed mode selected in the
 *												InitVarsSelected is invalid
 *		- IX_I2C_INVALID_FLOW_MODE_ENUM_VALUE - The flow mode selected in the
 *												InitVarsSelected is invalid
 *		- IX_I2C_INVALID_SLAVE_ADDR - The address 0x0 is reserved for
 *										general call.
 *		- IX_I2C_SLAVE_ADDR_CB_MISSING - interrupt mode is selected but
 *										slave address callback pointer is NULL
 *		- IX_I2C_GEN_CALL_CB_MISSING - interrupt mode is selected but
 *										general call callback pointer is NULL
 *		- IX_I2C_INT_BIND_FAIL - The ISR for the I2C failed to bind
 *		- IX_I2C_INT_UNBIND_FAIL - The ISR for the I2C failed to unbind
 *
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvInit(IxI2cInitVars *InitVarsSelected);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvUninit(
	void)
 *
 * @brief Disables the I2C hardware
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will disable the I2C hardware, unbind interrupt, and unmap memory.
 *
 * @return 
 *      - IX_I2C_SUCCESS - successfully un-initialized I2C
 *		- IX_I2C_INT_UNBIND_FAIL - failed to unbind the I2C interrupt
 *		- IX_I2C_NOT_INIT - I2C not init yet.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvUninit(void);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvSlaveAddrSet(
	UINT8 SlaveAddrSet)
 *
 * @brief Sets the I2C Slave Address
 *
 * @param "UINT8 [in] SlaveAddrSet" - Slave Address to be inserted into ISAR
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will insert the SlaveAddrSet into the ISAR.
 *
 * @return 
 *      - IX_I2C_SUCCESS - successfuly set the slave addr
 *		- IX_I2C_INVALID_SLAVE_ADDR - invalid slave address (zero) specified
 *		- IX_I2C_NOT_INIT - I2C not init yet.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvSlaveAddrSet(UINT8 SlaveAddrSet);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvBusScan(
	void)
 *
 * @brief scans the I2C bus for slave devices
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will prompt all slave addresses for a reply except its own
 *
 * @return 
 *      - IX_I2C_SUCCESS - found at least one slave device
 *		- IX_I2C_FAIL - Fail to find even one slave device
 *		- IX_I2C_BUS_BUSY - The I2C bus is busy (held by another I2C master)
 *		- IX_I2C_ARB_LOSS - The I2C bus was loss to another I2C master
 *		- IX_I2C_NOT_INIT - I2C not init yet.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : yes
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvBusScan(void);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvWriteTransfer(
	UINT8 SlaveAddr,
	char *bufP,
	UINT32 dataSize,
	IxI2cXferMode XferModeSelect)
 *
 * @param "UINT8 [in] SlaveAddr" - The slave address to request data from.
 * @param "char [in] *bufP" - The pointer to the data to be transmitted.
 * @param "UINT32 [in] dataSize" - The number of bytes requested.
 * @param "IxI2cXferMode [in] XferModeSelect" - the transfer mode selected,
 *			either repeated start (ends w/o stop) or normal (start and stop)
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will try to obtain master control of the I2C bus and transmit the
 * number of bytes, specified by dataSize, to the user specified slave
 * address from the buffer pointer. It will use either interrupt or poll mode
 * depending on the method selected. 
 *
 * If in interrupt mode and IxI2cMasterWriteCallbackP is not NULL, the driver
 * will initiate the transfer and return immediately. The function pointed to
 * by IxI2cMasterWriteCallbackP will be called in the interrupt service
 * handlers when the operation is complete.
 *
 * If in interrupt mode and IxI2cMasterWriteCallbackP is NULL, then the driver
 * will wait for the operation to complete, and then return.
 *
 * And if the repeated start transfer mode is selected, then it will not send a
 * stop signal at the end of all the transfers.
 * *NOTE*: If repeated start transfer mode is selected, it has to end with a
 *			normal mode transfer mode else the bus will continue to be held
 *			by the IXP.
 *
 * @return 
 *      - IX_I2C_SUCCESS - Successfuuly wrote data to slave.
 *		- IX_I2C_MASTER_BUS_BUSY - The I2C bus is busy (held by another I2C master)
 *		- IX_I2C_MASTER_ARB_LOSS - The I2C bus was loss to another I2C master
 *		- IX_I2C_MASTER_XFER_ERROR - There was a transfer error
 *      - IX_I2C_MASTER_BUS_ERROR - There was a bus error during transfer
 *		- IX_I2C_MASTER_NO_BUFFER - buffer pointer is NULL
 *      - IX_I2C_MASTER_INVALID_XFER_MODE - Xfer mode selected is invalid
 *      - IX_I2C_DATA_SIZE_ZERO - dataSize passed in is zero, which is invalid
 *		- IX_I2C_NOT_INIT - I2C not init yet.
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvWriteTransfer(
	UINT8 SlaveAddr,
	char *bufP,
	UINT32 dataSize,
	IxI2cXferMode XferModeSelect);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvReadTransfer(
	UINT8 SlaveAddr,
	char *bufP,
	UINT32 dataSize,
	IxI2cXferMode XferModeSelect)
 *
 * @brief Initiates a transfer to receive bytes of data from a slave
 *			device through the I2C bus.
 *
 * @param "UINT8 [in] SlaveAddr" - The slave address to request data from.
 * @param "char [out] *bufP" - The pointer to the buffer to store the
 *			requested data.
 * @param "UINT32 [in] dataSize" - The number of bytes requested.
 * @param "IxI2cXferMode [in] XferModeSelect" - the transfer mode selected,
 *			either repeated start (ends w/o stop) or normal (start and stop)
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will try to obtain master control of the I2C bus and receive the
 * number of bytes, specified by dataSize, from the user specified address
 * into the receive buffer. It will use either interrupt or poll mode depending
 * on the mode selected.
 *
 * If in interrupt mode and IxI2cMasterReadCallbackP is not NULL, the driver
 * will initiate the transfer and return immediately. The function pointed to
 * by IxI2cMasterReadCallbackP will be called in the interrupt service
 * handlers when the operation is complete.
 *
 * If in interrupt mode and IxI2cMasterReadCallbackP is NULL, then the driver will
 * wait for the operation to complete, and then return.
 *
 * And if the repeated start transfer mode is selected, then it will not send a
 * stop signal at the end of all the transfers.
 * *NOTE*: If repeated start transfer mode is selected, it has to end with a
 *			normal mode transfer mode else the bus will continue to be held
 *			by the IXP.
 *
 * @return 
 *      - IX_I2C_SUCCESS - Successfuuly read slave data
 *		- IX_I2C_MASTER_BUS_BUSY - The I2C bus is busy (held by another I2C master)
 *		- IX_I2C_MASTER_ARB_LOSS - The I2C bus was loss to another I2C master
 *		- IX_I2C_MASTER_XFER_ERROR - There was a bus error during transfer
 *      - IX_I2C_MASTER_BUS_ERROR - There was a bus error during transfer
 *		- IX_I2C_MASTER_NO_BUFFER - buffer pointer is NULL
 *      - IX_I2C_MASTER_INVALID_XFER_MODE - Xfer mode selected is invalid
 *      - IX_I2C_INVALID_SLAVE_ADDR - invalid slave address (zero) specified
 *      - IX_I2C_DATA_SIZE_ZERO - dataSize passed in is zero, which is invalid
 *		- IX_I2C_NOT_INIT - I2C not init yet.
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvReadTransfer(
	UINT8 SlaveAddr,
	char *bufP,
	UINT32 dataSize,
	IxI2cXferMode XferModeSelect);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvSlaveAddrAndGenCallDetectedCheck(
	void)
 *
 * @brief Checks the I2C Status Register to determine if a slave address is
 *			detected
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API is used in polled mode to determine if the I2C unit is requested
 * for a slave or general call transfer. If it is requested for a slave
 * transfer then it will determine if it is a general call (read only), read,
 * or write transfer requested.
 *
 * @return 
 *      - IX_I2C_SLAVE_ADDR_NOT_DETECTED - The I2C unit is not requested for slave
 *										transfer
 *		- IX_I2C_GEN_CALL_ADDR_DETECTED - The I2C unit is not requested for slave
 *									transfer but for general call
 *      - IX_I2C_SLAVE_READ_DETECTED - The I2C unit is requested for a read
 *      - IX_I2C_SLAVE_WRITE_DETECTED - The I2C unit is requested for a write
 *		- IX_I2C_NOT_INIT - I2C not init yet.
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvSlaveAddrAndGenCallDetectedCheck(void);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvSlaveOrGenDataReceive(
	char *bufP,
	UINT32 bufSize,
	UINT32 *dataSizeRcvd)
 *
 * @brief Performs the slave receive or general call receive data transfer
 *
 * @param	"char [in] *bufP" - the pointer to the buffer to store data
 *			"UINT32 [in] bufSize" - the buffer size allocated
 *			"UINT32 [in] *dataSizeRcvd" - the length of data received in bytes
 *
 * Global Data	:
 *		- None.
 *                        
 * This API is only used in polled mode to perform the slave read or general call
 * receive data. It will continuously store the data received into bufP until
 * complete or until bufP is full in which it will return
 * IX_I2C_SLAVE_OR_GEN_READ_BUFFER_FULL. If in interrupt mode, the callback will be
 * used.
 *
 * @return 
 *      - IX_I2C_SUCCESS - The I2C driver transferred the data successfully.
 *		- IX_I2C_SLAVE_OR_GEN_READ_BUFFER_FULL - The I2C driver has ran out of
 *			space to store the received data.
 *		- IX_I2C_SLAVE_OR_GEN_READ_ERROR - The I2C driver didn't manage to
 *			detect the IDBR Rx Full bit
 *      - IX_I2C_DATA_SIZE_ZERO - bufSize passed in is zero, which is invalid
 *		- IX_I2C_SLAVE_NO_BUFFER - buffer pointer is NULL
 *      - IX_I2C_NULL_POINTER - dataSizeRcvd is NULL
 *		- IX_I2C_NOT_INIT - I2C not init yet.
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvSlaveOrGenDataReceive(
	char *bufP,
	UINT32 bufSize,
	UINT32 *dataSizeRcvd);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvSlaveDataTransmit(
	char *bufP,
	UINT32 dataSize,
	UINT32 *dataSizeXmtd)
 *
 * @brief Performs the slave write data transfer
 *
 * @param	"char [in] *bufP" - the pointer to the buffer for data to be
 *				transmitted
 *			"UINT32 [in] bufSize" - the buffer size allocated
 *			"UINT32 [in] *dataSizeRcvd" - the length of data trasnmitted in
 *				bytes
 *
 * Global Data	:
 *		- None.
 *                        
 * This API is only used in polled mode to perform the slave transmit data. It
 * will continuously transmit the data from bufP until complete or until bufP
 * is empty in which it will return IX_I2C_SLAVE_WRITE_BUFFER_EMPTY. If in
 * interrupt mode, the callback will be used.
 *
 * @return 
 *      - IX_I2C_SUCCESS - The I2C driver transferred the data successfully.
 *      - IX_I2C_SLAVE_WRITE_BUFFER_EMPTY - The I2C driver needs more data to
 *			transmit.
 *      - IX_I2C_SLAVE_WRITE_ERROR -The I2C driver didn't manage to detect the
 *          IDBR Tx empty bit or the slave stop bit.
 *      - IX_I2C_DATA_SIZE_ZERO - dataSize passed in is zero, which is invalid
 *		- IX_I2C_SLAVE_NO_BUFFER - buffer pointer is NULL
 *      - IX_I2C_NULL_POINTER - dataSizeXmtd is NULL
 *		- IX_I2C_NOT_INIT - I2C not init yet.
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvSlaveDataTransmit(
	char *bufP,
	UINT32 dataSize,
	UINT32 *dataSizeXmtd);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvSlaveOrGenCallBufReplenish(
	char *bufP,
	UINT32 bufSize)
 *
 * @brief Replenishes the buffer which stores buffer info for both slave and
 *			general call
 *
 * @param	"char [in] *bufP" - pointer to the buffer allocated
 *			"UINT32 [in] bufSize" - size of the buffer
 *
 * Global Data	:
 *		- None.
 *                        
 * This API is only used in interrupt mode for replenishing the same buffer
 * that is used by both slave and generall call by updating the buffer info
 * with new info and clearing previous offsets set by previous transfers.
 *
 * @return 
 *      - None
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC void
ixI2cDrvSlaveOrGenCallBufReplenish(
	char *bufP,
	UINT32 bufSize);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvStatsGet(IxI2cStatsCounters *I2cStats)
 *
 * @brief Returns the I2C Statistics through the pointer passed in
 *
 * @param - "IxI2cStatsCounters [out] *I2cStats" - I2C statistics counter will
 *			be read and written to the location pointed by this pointer.
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will return the statistics counters of the I2C driver.
 *
 * @return 
 *      - IX_I2C_NULL_POINTER - pointer passed in is NULL
 *		- IX_I2C_SUCCESS - successfully obtained I2C statistics
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : no
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvStatsGet(IxI2cStatsCounters *I2cStats);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvStatsReset(void)
 *
 * @brief Reset I2C statistics counters.
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will reset the statistics counters of the I2C driver.
 *
 * @return 
 *      - None
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : no
 *
 */
PUBLIC void
ixI2cDrvStatsReset(void);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvShow(void)
 *
 * @brief Displays the I2C status register and the statistics counter.
 *
 * @param - None
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will display the I2C Status register and is useful if any error
 * occurs. It displays the detection of bus error, slave address, general call,
 * address, IDBR receive full, IDBR transmit empty, arbitration loss, slave
 * STOP signal, I2C bus busy, unit busy, ack/nack, and read/write mode. It will
 * also call the ixI2cDrvGetStats and then display the statistics counter.
 *
 * @return 
 *		- IX_I2C_SUCCESS - successfully displayed statistics and status register
 *		- IX_I2C_NOT_INIT - I2C not init yet.
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : no
 *
 */
PUBLIC IX_I2C_STATUS
ixI2cDrvShow(void);

/**
 * @ingroup IxI2cDrv
 * 
 * @fn ixI2cDrvDelayTypeSelect (IxI2cDelayMode delayMechanismSelect)
 *
 * @brief Sets the delay type of either looping delay or OS scheduler delay
 *          according to the argument provided.
 *
 * @param - "IxI2cDelayMode [in] delayTypeSelect" - the I2C delay type selected
 *
 * Global Data	:
 *		- None.
 *                        
 * This API will set the delay type used by the I2C Driver to either looping
 * delay or OS scheduler delay.
 *
 * @return 
 *		- None
 *              
 * @li   Reentrant    : yes
 * @li   ISR Callable : no
 *
 */
PUBLIC void
ixI2cDrvDelayTypeSelect (IxI2cDelayMode delayTypeSelect);

#endif /* __ixp46X */
#endif /* IXI2CDRV_H */
