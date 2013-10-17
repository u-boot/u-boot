/**
 * @file IxParityENAcc.h
 *
 * @author Intel Corporation
 * @date 24 Mar 2004
 *
 * @brief This file contains the public API for the IXP400 Parity Error 
 * Notifier access component.
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
 * @defgroup IxParityENAcc IXP400 Parity Error Notifier (IxParityENAcc) API
 *
 * @brief The public API for the Parity Error Notifier 
 * 
 * @{
 */

#ifndef IXPARITYENACC_H
#define IXPARITYENACC_H

#ifdef __ixp46X

#include "IxOsal.h"

/*
 * #defines for function return types, etc.
 */

/**
 * @ingroup IxParityENAcc
 *
 * @enum IxParityENAccStatus
 *
 * @brief The status as returend from the API
 */
typedef enum /**< IxParityENAccStatus */
{
  IX_PARITYENACC_SUCCESS = IX_SUCCESS, /**< The request is successful */
  IX_PARITYENACC_INVALID_PARAMETERS,   /**< Invalid or NULL parameters passed */
  IX_PARITYENACC_NOT_INITIALISED,      /**< Access layer has not been initialised before accessing the APIs */
  IX_PARITYENACC_ALREADY_INITIALISED,  /**< Access layer has already been initialised */ 
  IX_PARITYENACC_OPERATION_FAILED,     /**< Operation did not succeed due to hardware failure */
  IX_PARITYENACC_NO_PARITY             /**< No parity condition exits or has already been cleared */
} IxParityENAccStatus;

/**
 * @ingroup IxParityENAcc
 *
 * @enum IxParityENAccParityType
 * 
 * @brief Odd or Even Parity Type
 */
typedef enum  /**< IxParityENAccParityType */
{
  IX_PARITYENACC_EVEN_PARITY,    /**< Even Parity */
  IX_PARITYENACC_ODD_PARITY      /**< Odd Parity */
} IxParityENAccParityType;

/**
 * @ingroup IxParityENAcc
 *
 * @enum IxParityENAccConfigOption
 *
 * @brief The parity error enable/disable configuration option
 */
typedef enum /**< IxParityENAccConfigOption */
{
  IX_PARITYENACC_DISABLE,       /**< Disable parity error detection */
  IX_PARITYENACC_ENABLE         /**< Enable parity error detection */
} IxParityENAccConfigOption;

/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccNpeConfig
 *
 * @brief NPE parity detection is to be enabled/disabled
 */
typedef struct /**< IxParityENAccNpeConfig */
{
  IxParityENAccConfigOption ideEnabled; /**< NPE IMem, DMem and External */
  IxParityENAccParityType parityOddEven; /**< Parity - Odd or Even */
} IxParityENAccNpeConfig ;


/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccMcuConfig
 *
 * @brief MCU pairty detection is to be enabled/disabled
 */
typedef struct /**< IxParityENAccMcuConfig */ 
{
  IxParityENAccConfigOption singlebitDetectEnabled;      /**< Single-bit parity error detection */
  IxParityENAccConfigOption singlebitCorrectionEnabled;  /**< Single-bit parity error correction */
  IxParityENAccConfigOption multibitDetectionEnabled;    /**< Multi-bit  parity error detection */
} IxParityENAccMcuConfig ;


/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccEbcConfig
 *
 * @brief Expansion Bus Controller parity detection is to be enabled or disabled 
 *
 * Note: All the Chip Select(s) and External Masters will have the same parity
 */
typedef struct /**< IxParityENAccEbcConfig */
{
  IxParityENAccConfigOption ebcCs0Enabled;    /**< Expansion Bus Controller - Chip Select 0 */
  IxParityENAccConfigOption ebcCs1Enabled;    /**< Expansion Bus Controller - Chip Select 1 */
  IxParityENAccConfigOption ebcCs2Enabled;    /**< Expansion Bus Controller - Chip Select 2 */
  IxParityENAccConfigOption ebcCs3Enabled;    /**< Expansion Bus Controller - Chip Select 3 */
  IxParityENAccConfigOption ebcCs4Enabled;    /**< Expansion Bus Controller - Chip Select 4 */
  IxParityENAccConfigOption ebcCs5Enabled;    /**< Expansion Bus Controller - Chip Select 5 */
  IxParityENAccConfigOption ebcCs6Enabled;    /**< Expansion Bus Controller - Chip Select 6 */
  IxParityENAccConfigOption ebcCs7Enabled;    /**< Expansion Bus Controller - Chip Select 7 */
  IxParityENAccConfigOption ebcExtMstEnabled; /**< External Master on Expansion bus */
  IxParityENAccParityType parityOddEven;      /**< Parity - Odd or Even */
} IxParityENAccEbcConfig ;

/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccHWParityConfig
 *
 * @brief Parity error configuration of the Hardware Blocks
 */
typedef struct /**< IxParityENAccHWParityConfig */
{ 
  IxParityENAccNpeConfig npeAConfig;     /**< NPE A parity detection is to be enabled/disabled */
  IxParityENAccNpeConfig npeBConfig;     /**< NPE B parity detection is to be enabled/disabled */
  IxParityENAccNpeConfig npeCConfig;     /**< NPE C parity detection is to be enabled/disabled */
  IxParityENAccMcuConfig mcuConfig;      /**< MCU pairty detection is to be enabled/disabled */
  IxParityENAccConfigOption swcpEnabled; /**< SWCP parity detection is to be enabled */
  IxParityENAccConfigOption aqmEnabled;  /**< AQM parity detection is to be enabled */
  IxParityENAccEbcConfig ebcConfig;      /**< Expansion Bus Controller parity detection is to be enabled/disabled */
} IxParityENAccHWParityConfig;


/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccNpeParityErrorStats
 *
 * @brief NPE parity error statistics
 */
typedef struct  /* IxParityENAccNpeParityErrorStats */
{
  UINT32 parityErrorsIMem;         /**< Parity errors in Instruction Memory */
  UINT32 parityErrorsDMem;         /**< Parity errors in Data Memory */
  UINT32 parityErrorsExternal;     /**< Parity errors in NPE External Entities */
} IxParityENAccNpeParityErrorStats;

/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccMcuParityErrorStats
 *
 * @brief DDR Memory Control Unit parity error statistics
 * 
 * Note: There could be two outstanding parity errors at any given time whose address
 * details captured. If there is no room for the new interrupt then it would be treated
 * as overflow parity condition.
 */
typedef struct  /* IxParityENAccMcuParityErrorStats */
{
  UINT32 parityErrorsSingleBit;    /**< Parity errors of the type Single-Bit */
  UINT32 parityErrorsMultiBit;     /**< Parity errors of the type Multi-Bit */
  UINT32 parityErrorsOverflow;     /**< Parity errors when more than two parity errors occured */
} IxParityENAccMcuParityErrorStats;

/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccEbcParityErrorStats
 *
 * @brief Expansion Bus Controller parity error statistics
 */
typedef struct  /* IxParityENAccEbcParityErrorStats */
{
  UINT32 parityErrorsInbound;      /**< Odd bit parity errors on inbound transfers */
  UINT32 parityErrorsOutbound;     /**< Odd bit parity errors on outbound transfers */
} IxParityENAccEbcParityErrorStats;


/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccParityErrorStats
 *
 * @brief Parity Error Statistics for the all the hardware blocks
 */
typedef struct  /**< IxParityENAccParityErrorStats */
{
  IxParityENAccNpeParityErrorStats  npeStats;  /**< NPE parity error statistics */
  IxParityENAccMcuParityErrorStats  mcuStats;  /**< MCU parity error statistics */
  IxParityENAccEbcParityErrorStats  ebcStats;  /**< EBC parity error statistics */
  UINT32                            swcpStats; /**< SWCP parity error statistics */
  UINT32                            aqmStats;  /**< AQM parity error statistics */
} IxParityENAccParityErrorStats;


/**
 * @ingroup IxParityENAcc
 *
 * @enum IxParityENAccParityErrorSource
 *
 * @brief The source of the parity error notification 
 */
typedef enum  /**< IxParityENAccParityErrorSource  */
{
  IX_PARITYENACC_NPE_A_IMEM,     /**< NPE A - Instruction memory */
  IX_PARITYENACC_NPE_A_DMEM,     /**< NPE A - Data memory */
  IX_PARITYENACC_NPE_A_EXT,      /**< NPE A - External Entity*/
  IX_PARITYENACC_NPE_B_IMEM,     /**< NPE B - Instruction memory */
  IX_PARITYENACC_NPE_B_DMEM,     /**< NPE B - Data memory */
  IX_PARITYENACC_NPE_B_EXT,      /**< NPE B - External Entity*/
  IX_PARITYENACC_NPE_C_IMEM,     /**< NPE C - Instruction memory */
  IX_PARITYENACC_NPE_C_DMEM,     /**< NPE C - Data memory */
  IX_PARITYENACC_NPE_C_EXT,      /**< NPE C - External Entity*/
  IX_PARITYENACC_SWCP,           /**< SWCP */
  IX_PARITYENACC_AQM,            /**< AQM */
  IX_PARITYENACC_MCU_SBIT,       /**< DDR Memory Controller Unit - Single bit parity */
  IX_PARITYENACC_MCU_MBIT,       /**< DDR Memory Controller Unit - Multi bit parity */
  IX_PARITYENACC_MCU_OVERFLOW,   /**< DDR Memory Controller Unit - Parity errors in excess of two */
  IX_PARITYENACC_EBC_CS,         /**< Expansion Bus Controller - Chip Select */
  IX_PARITYENACC_EBC_EXTMST      /**< Expansion Bus Controller - External Master */
} IxParityENAccParityErrorSource;

/**
 * @ingroup IxParityENAcc
 *
 * @enum IxParityENAccParityErrorAccess
 *
 * @brief The type of access resulting in parity error
 */
typedef enum  /**< IxParityENAccParityErrorAccess  */
{
  IX_PARITYENACC_READ,   /**< Read Access  */
  IX_PARITYENACC_WRITE   /**< Write Access */
} IxParityENAccParityErrorAccess;

/**
 * @ingroup IxParityENAcc
 *
 * @typedef IxParityENAccParityErrorAddress
 *
 * @brief The memory location which has parity error
 */
typedef UINT32 IxParityENAccParityErrorAddress;

/**
 * @ingroup IxParityENAcc
 *
 * @typedef IxParityENAccParityErrorData
 *
 * @brief The data read from the memory location which has parity error
 */
typedef UINT32 IxParityENAccParityErrorData;

/**
 * @ingroup IxParityENAcc
 *
 * @enum IxParityENAccParityErrorRequester
 *
 * @brief The requester interface through which the SDRAM memory access 
 * resulted in the parity error.
 */
typedef enum  /**< IxParityENAccParityErrorRequester  */
{
  IX_PARITYENACC_MPI,     /**< Direct Memory Port Interface  */
  IX_PARITYENACC_AHB_BUS  /**< South or North AHB Bus */
} IxParityENAccParityErrorRequester;

/**
 * @ingroup IxParityENAcc
 *
 * @enum IxParityENAccAHBErrorMaster
 *
 * @brief The Master on the AHB bus interface whose transaction might have 
 * resulted in the parity error notification to XScale.
 */
typedef enum  /**< IxParityENAccAHBErrorMaster */
{
  IX_PARITYENACC_AHBN_MST_NPE_A,       /**< NPE - A */
  IX_PARITYENACC_AHBN_MST_NPE_B,       /**< NPE - B */
  IX_PARITYENACC_AHBN_MST_NPE_C,       /**< NPE - C */
  IX_PARITYENACC_AHBS_MST_XSCALE,      /**< XScale Bus Interface Unit */
  IX_PARITYENACC_AHBS_MST_PBC,         /**< PCI Bus Controller */
  IX_PARITYENACC_AHBS_MST_EBC,         /**< Expansion Bus Controller */
  IX_PARITYENACC_AHBS_MST_AHB_BRIDGE,  /**< AHB Bridge */
  IX_PARITYENACC_AHBS_MST_USBH         /**< USB Host Controller */
} IxParityENAccAHBErrorMaster;

/**
 * @ingroup IxParityENAcc
 *
 * @enum IxParityENAccAHBErrorSlave
 *
 * @brief The Slave on the AHB bus interface whose transaction might have 
 * resulted in the parity error notification to XScale.
 */
typedef enum  /**< IxParityENAccAHBErrorSlave */
{
  IX_PARITYENACC_AHBN_SLV_MCU,         /**< Memory Control Unit */
  IX_PARITYENACC_AHBN_SLV_AHB_BRIDGE,  /**< AHB Bridge */
  IX_PARITYENACC_AHBS_SLV_MCU,         /**< XScale Bus Interface Unit */
  IX_PARITYENACC_AHBS_SLV_APB_BRIDGE,  /**< APB Bridge */
  IX_PARITYENACC_AHBS_SLV_AQM,         /**< AQM */
  IX_PARITYENACC_AHBS_SLV_RSA,         /**< RSA (Crypto Bus) */
  IX_PARITYENACC_AHBS_SLV_PBC,         /**< PCI Bus Controller */
  IX_PARITYENACC_AHBS_SLV_EBC,         /**< Expansion Bus Controller */
  IX_PARITYENACC_AHBS_SLV_USBH         /**< USB Host Controller */
} IxParityENAccAHBErrorSlave;

/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccAHBErrorTransaction
 *
 * @brief The Master and Slave on the AHB bus interface whose transaction might
 * have resulted in the parity error notification to XScale.
 *
 * NOTE: This information may be used in the data abort exception handler
 * to differentiate between the XScale and non-XScale access to the SDRAM
 * memory.
 */
typedef struct  /**< IxParityENAccAHBErrorTransaction  */
{
  IxParityENAccAHBErrorMaster  ahbErrorMaster; /**< Master on AHB bus */
  IxParityENAccAHBErrorSlave   ahbErrorSlave;  /**< Slave  on AHB bus */
} IxParityENAccAHBErrorTransaction;

/**
 * @ingroup IxParityENAcc
 *
 * @struct IxParityENAccParityErrorContextMessage
 *
 * @brief Parity Error Context Message
 */
typedef struct /**< IxParityENAccParityErrorContextMessage */
{
  IxParityENAccParityErrorSource     pecParitySource; /**< Source info of parity error */
  IxParityENAccParityErrorAccess     pecAccessType;   /**< Read or Write Access
                                                           Read  - NPE, SWCP, AQM, DDR MCU,
                                                                   Exp Bus Ctrlr (Outbound)
                                                           Write - DDR MCU,
                                                                   Exp Bus Ctrlr (Inbound 
                                                                   i.e., External Master) */
  IxParityENAccParityErrorAddress    pecAddress;      /**< Address faulty location
                                                           Valid only for AQM, DDR MCU, 
                                                           Exp Bus Ctrlr */
  IxParityENAccParityErrorData       pecData;         /**< Data read from the faulty location
                                                           Valid only for AQM and DDR MCU
                                                           For DDR MCU it is the bit location
                                                           of the Single-bit parity */
  IxParityENAccParityErrorRequester  pecRequester;    /**< Requester of SDRAM memory access
                                                           Valid only for the DDR MCU */
  IxParityENAccAHBErrorTransaction   ahbErrorTran;    /**< Master and Slave information on the
                                                           last AHB Error Transaction */
} IxParityENAccParityErrorContextMessage;

/**
 * @ingroup IxParityENAcc
 *
 * @typedef IxParityENAccCallback
 *
 * @brief This prototype shows the format of a callback function.
 *
 * The callback will be used to notify the parity error to the client application.
 * The callback will be registered  by @ref ixParityENAccCallbackRegister.
 *
 * It will be called from an ISR when a parity error is detected and thus
 * needs to follow the interrupt callable function conventions.
 *
 */
typedef void (*IxParityENAccCallback) (void);


/*
 * Prototypes for interface functions.
 */

/**
 * @ingroup IxParityENAcc
 *
 * @fn IxParityENAccStatus ixParityENAccInit(void)
 *
 * @brief This function will initialise the IxParityENAcc component.
 *
 * This function will initialise the IxParityENAcc component. It should only be
 * called once, prior to using the IxParityENAcc component.
 *
 * <OL><LI>It initialises the internal data structures, registers the ISR that 
 * will be triggered when a parity error occurs in IXP4xx silicon.</LI></OL>
 *
 * @li Re-entrant   : No
 * @li ISR Callable : No
 * 
 * @return @li IX_PARITYENACC_SUCCESS - Initialization is successful
 *         @li IX_PARITYENACC_ALREADY_INITIALISED - The access layer has already 
 *             been initialized
 *         @li IX_PARITYENACC_OPERATION_FAILED - The request failed because the 
 *             operation didn't succeed on the hardware. Refer to error trace/log
 *             for details.
 */

PUBLIC IxParityENAccStatus ixParityENAccInit(void);

/**
 * @ingroup IxParityENAcc
 *
 * @fn IxParityENAccStatus ixParityENAccCallbackRegister (
           IxParityENAccCallback parityErrNfyCallBack)
 *
 * @brief This function will register a new callback with IxParityENAcc component. 
 * It can also reregister a new callback replacing the old callback.
 *
 * @param parityErrNfyCallBack [in] - This parameter will specify the call-back
 * function supplied by the client application.
 *
 * This interface registers the user application supplied call-back handler with
 * the parity error handling access component after the init.
 *
 * The callback function will be called from an ISR that will be triggered by the 
 * parity error in the IXP400 silicon.
 *
 * The following actions will be performed by this function:
 * <OL><LI>Check for the prior initialisation of the module before registering or
 * re-registering of the callback.
 * Check for parity error detection disabled before re-registration of the callback.
 * </LI></OL>
 *
 * @li   Re-entrant   : No
 * @li   ISR Callable : No
 *
 * @return @li IX_PARITYENACC_SUCCESS - The parameters check passed and the 
 *             registration is successful.
 *         @li IX_PARITYENACC_INVALID_PARAMETERS - Request failed due to NULL 
 *             parameter passed.
 *         @li IX_PARITYENACC_OPERATION_FAILED - The request failed because the 
 *             parity error detection not yet disabled.
 *         @li IX_PARITYENACC_NOT_INITIALISED - The operation requested prior to
 *             the initialisation of the access layer.
 */

PUBLIC IxParityENAccStatus ixParityENAccCallbackRegister (
           IxParityENAccCallback parityErrNfyCallBack);

/**
 * @ingroup IxParityENAcc
 *
 * @fn IxParityENAccStatus ixParityENAccParityDetectionConfigure (
           const IxParityENAccHWParityConfig *hwParityConfig)
 *
 * @brief This interface allows the client application to enable the parity 
 * error detection on the underlying hardware block.
 *
 * @param hwParityConfig [in] - Hardware blocks for which the parity error 
 * detection is to be enabled or disabled.
 *
 * The client application allocates and provides the reference to the buffer.
 *
 * It will also verify whether the specific hardware block is functional or not.
 *
 * NOTE: Failure in enabling or disabling of one or more components result in
 * trace message but still returns IX_PARITYENACC_SUCCESS. Refer to the function 
 * @ref ixParityENAccParityDetectionQuery on how to verify the failures while 
 * enabling/disabling paritys error detection.
 *
 * It shall be invoked after the Init and CallbackRegister functions but before
 * any other function of the IxParityENAcc layer.
 *
 * @li   Re-entrant   : No
 * @li   ISR Callable : No
 *
 * @return @li IX_PARITYENACC_SUCCESS - The parameters check passed and the 
 *             request to enable/disable is successful.
 *         @li IX_PARITYENACC_INVALID_PARAMETERS-The request failed due to  
 *             NULL parameter supplied.
 *         @li IX_PARITYENACC_OPERATION_FAILED - The request failed because the 
 *             operation didn't succeed on the hardware.
 *         @li IX_PARITYENACC_NOT_INITIALISED - The operation requested prior to
 *             the initialisation of the access layer.
 */

PUBLIC IxParityENAccStatus ixParityENAccParityDetectionConfigure (
           const IxParityENAccHWParityConfig *hwParityConfig);

/**
 * @ingroup IxParityENAcc
 *
 * @fn IxParityENAccStatus ixParityENAccParityDetectionQuery (
           IxParityENAccHWParityConfig * const hwParityConfig)
 *
 * @brief This interface allows the client application to determine the 
 * status of the parity error detection on the specified hardware blocks
 *
 * @param hwParityConfig [out] - Hardware blocks whose parity error detection 
 * has been enabled or disabled.
 *
 * The client application allocates and provides the reference to the buffer.
 *
 * This interface can be used immediately after the interface @ref
 * ixParityENAccParityDetectionConfigure to see on which of the hardware blocks
 * the parity error detection has either been enabled or disabled based on the
 * client application request.
 *
 * @li   Re-entrant   : No
 * @li   ISR Callable : No
 *
 * @return @li IX_PARITYENACC_SUCCESS - The parameters check passed and the 
 *             request to query on whether the hardware parity error detection
 *             is enabled or disabled is successful.
 *         @li IX_PARITYENACC_INVALID_PARAMETERS-The request failed due to  
 *             NULL parameter or invalid values supplied.
 *         @li IX_PARITYENACC_NOT_INITIALISED - The operation requested prior 
 *             to the initialisation of the access layer.
 */

PUBLIC IxParityENAccStatus ixParityENAccParityDetectionQuery(
           IxParityENAccHWParityConfig * const hwParityConfig);

/**
 * @ingroup IxParityENAcc
 *
 * @fn IxParityENAccStatus ixParityENAccParityErrorContextGet( 
           IxParityENAccParityErrorContextMessage * const pecMessage)
 *
 * @brief This interface allows the client application to determine the 
 * status of the parity error context on hardware block for which the
 * current parity error interrupt triggered.
 *
 * @param pecMessage [out] - The parity error context information of the
 * parity interrupt currently being process.
 *
 * The client application allocates and provides the reference to the buffer.
 *
 * Refer to the data structure @ref IxParityENAccParityErrorContextMessage
 * for details.
 *
 * The routine will will fetch the parity error context in the following
 * priority, if multiple parity errors observed.
 * 
 * <pre>
 * 0 - MCU (Multi-bit and single-bit in that order)
 * 1 - NPE-A
 * 2 - NPE-B
 * 3 - NPE-C
 * 4 - SWCP
 * 5 - QM
 * 6 - EXP
 *
 * NOTE: The information provided in the @ref IxParityENAccAHBErrorTransaction
 * may be of help for the client application to decide on the course of action 
 * to take. This info is taken from the Performance Monitoring Unit register
 * which records most recent error observed on the AHB bus. This information
 * might have been overwritten by some other error by the time it is retrieved.
 * </pre>
 *
 * @li   Re-entrant   : No
 * @li   ISR Callable : Yes
 *
 * @return @li IX_PARITYENACC_SUCCESS-The parameters check passed and the 
 *             request to get the parity error context information is successful.
 *         @li IX_PARITYENACC_INVALID_PARAMETERS-The request failed due to  
 *             NULL parameter is passed
 *         @li IX_PARITYENACC_OPERATION_FAILED - The request failed because 
 *             the operation didn't succeed on the hardware.
 *         @li IX_PARITYENACC_NOT_INITIALISED - The operation requested prior
 *             to the initialisation of the access layer.
 *         @li IX_PARITYENACC_NO_PARITY - No parity condition exits or has 
 *             already been cleared
 */

PUBLIC IxParityENAccStatus ixParityENAccParityErrorContextGet( 
           IxParityENAccParityErrorContextMessage * const pecMessage);

/**
 * @ingroup IxParityENAcc
 *
 * @fn IxParityENAccStatus ixParityENAccParityErrorInterruptClear (
          const IxParityENAccParityErrorContextMessage *pecMessage)
 *
 * @brief This interface helps the client application to clear off the
 * interrupt condition on the hardware block identified in the parity
 * error context message.  Please refer to the table below as the operation
 * varies depending on the interrupt source.
 * 
 * @param pecMessage [in] - The parity error context information of the
 * hardware block whose parity error interrupt condition is to disabled.
 *
 * The client application allocates and provides the reference to the buffer.
 *
 * <pre>
 * ****************************************************************************
 * Following actions will be taken during the interrupt clear for respective
 * hardware blocks.
 * 
 *  Parity Source     Actions taken during Interrupt clear 
 *  -------------     -------------------------------------------------------
 *  NPE-A             Interrupt will be masked off at the interrupt controller 
 *                    so that it will not trigger continuously.
 *                      Client application has to take appropriate action and 
 *                      re-configure the parity error detection subsequently. 
 *                      The client application will not be notified of further 
 *                      interrupts, until the re-configuration is done using
 *                      @ref ixParityENAccParityDetectionConfigure.
 *
 *  NPE-B             Interrupt will be masked off at the interrupt controller 
 *                    so that it will not trigger continuously.
 *                      Client application has to take appropriate action and 
 *                      re-configure the parity error detection subsequently. 
 *                      The client application will not be notified of further
 *                      interrupts, until the re-configuration is done using
 *                      @ref ixParityENAccParityDetectionConfigure.
 *
 *  NPE-C             Interrupt will be masked off at the interrupt controller 
 *                      Client application has to take appropriate action and 
 *                      re-configure the parity error detection subsequently. 
 *                      The client application will not be notified of further
 *                      interrupts, until the re-configuration is done using
 *                      @ref ixParityENAccParityDetectionConfigure.
 *
 *  SWCP              Interrupt will be masked off at the interrupt controller.
 *                      Client application has to take appropriate action and 
 *                      re-configure the parity error detection subsequently. 
 *                      The client application will not be notified of further
 *                      interrupts, until the re-configuration is done using
 *                      @ref ixParityENAccParityDetectionConfigure.
 *
 *  AQM               Interrupt will be masked off at the interrupt controller.
 *                         Client application has to take appropriate action and 
 *                         re-configure the parity error detection subsequently. 
 *                         The client application will not be notified of further
 *                         interrupts, until the re-configuration is done using
 *                         @ref ixParityENAccParityDetectionConfigure.
 *
 *  MCU               Parity interrupt condition is cleared at the SDRAM MCU for 
 *                    the following:
 *                    1. Single-bit
 *                    2. Multi-bit
 *                    3. Overflow condition i.e., more than two parity conditions 
 *                       occurred
 *                    Note that single-parity errors do not result in data abort
 *                    and not all data aborts caused by multi-bit parity error.
 *
 *  EXP               Parity interrupt condition is cleared at the expansion bus 
 *                    controller for the following:
 *                    1. External master initiated Inbound write
 *                    2. Internal master (IXP400) initiated Outbound read
 * ****************************************************************************
 * </pre>
 * @li   Re-entrant   : No
 * @li   ISR Callable : No
 *
 * @return @li IX_PARITYENACC_SUCCESS-The parameters check passed and the request
 *             to clear the parity error interrupt condition is successful.
 *         @li IX_PARITYENACC_INVALID_PARAMETERS-The request failed due to 
 *             NULL parameters have been passed or contents have been
 *             supplied with invalid values.
 *         @li IX_PARITYENACC_OPERATION_FAILED - The request failed because 
 *             the operation didn't succeed on the hardware.
 *         @li IX_PARITYENACC_NOT_INITIALISED - The operation requested prior
 *             to the initialisation of the access layer.
 */

PUBLIC IxParityENAccStatus ixParityENAccParityErrorInterruptClear (
          const IxParityENAccParityErrorContextMessage *pecMessage);

/**
 * @ingroup IxParityENAcc
 *
 * @fn IxParityENAccStatus ixParityENAccStatsGet (
            IxParityENAccParityErrorStats * const ixParityErrorStats)
 *
 * @brief This interface allows the client application to retrieve parity 
 * error statistics for all the hardware blocks 
 *
 * @param ixParityErrorStats - [out] The statistics for all the hardware blocks.
 *
 * The client application allocates and provides the reference to the buffer.
 *
 * @li   Re-entrant   : No
 * @li   ISR Callable : Yes
 *
 * @return @li IX_PARITYENACC_SUCCESS-The parameters check passed and the 
 *             request to retrieve parity error statistics for the hardware
 *             block is successful.
 *         @li IX_PARITYENACC_INVALID_PARAMETERS-The request failed due to a
 *             NULL parameter passed.
 *         @li IX_PARITYENACC_NOT_INITIALISED - The operation requested prior
 *             to the initialisation of the access layer.
 */

PUBLIC IxParityENAccStatus ixParityENAccStatsGet (
            IxParityENAccParityErrorStats * const ixParityErrorStats);

/**
 * @ingroup IxParityENAcc
 *
 * @fn IxParityENAccStatus ixParityENAccStatsShow (void)
 *
 * @brief This interface allows the client application to print all the
 * parity error statistics.
 *
 * @li   Re-entrant   : No
 * @li   ISR Callable : No
 *
 * @return @li IX_PARITYENACC_SUCCESS - The request to show the pairty 
 *             error statistics is successful.
 *         @li IX_PARITYENACC_NOT_INITIALISED - The operation requested
 *             prior to the initialisation of the access layer.
 */

PUBLIC IxParityENAccStatus ixParityENAccStatsShow (void);

/**
 * @ingroup IxParityENAcc
 *
 * @fn IxParityENAccStatus ixParityENAccStatsReset (void)
 *
 * @brief This interface allows the client application to reset all the 
 * parity error statistics.
 *
 * @li   Re-entrant   : No
 * @li   ISR Callable : No
 *
 * @return @li IX_PARITYENACC_SUCCESS - The request to reset the parity 
 *             error statistics is successful.
 *         @li IX_PARITYENACC_NOT_INITIALISED - The operation requested 
 *             prior to the initialisation of the access layer.
 */

PUBLIC IxParityENAccStatus ixParityENAccStatsReset (void);

#endif /* IXPARITYENACC_H */
#endif /* __ixp46X */

/**
 * @} defgroup IxParityENAcc
 */

