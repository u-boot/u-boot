/* SPDX-License-Identifier: BSD-3-Clause */
/**********************************************************************
 * Copyright (C) 2012-2019 Cadence Design Systems, Inc.
 **********************************************************************
 * WARNING: This file is auto-generated using api-generator utility.
 *          api-generator: 12.02.13bb8d5
 *          Do not edit it manually.
 **********************************************************************
 * Cadence Core Driver for LPDDR4.
 **********************************************************************
 */

#ifndef LPDDR4_IF_H
#define LPDDR4_IF_H

#include <linux/types.h>

/** @defgroup ConfigInfo  Configuration and Hardware Operation Information
 *  The following definitions specify the driver operation environment that
 *  is defined by hardware configuration or client code. These defines are
 *  located in the header file of the core driver.
 *  @{
 */

/**********************************************************************
* Defines
**********************************************************************/
/** Number of chip-selects */
#define LPDDR4_MAX_CS (2U)

/** Number of accessible registers for controller. */
#define LPDDR4_CTL_REG_COUNT (459U)

/** Number of accessible registers for PHY Independent Module. */
#define LPDDR4_PHY_INDEP_REG_COUNT (300U)

/** Number of accessible registers for PHY. */
#define LPDDR4_PHY_REG_COUNT (1423U)

/**
 *  @}
 */

/** @defgroup DataStructure Dynamic Data Structures
 *  This section defines the data structures used by the driver to provide
 *  hardware information, modification and dynamic operation of the driver.
 *  These data structures are defined in the header file of the core driver
 *  and utilized by the API.
 *  @{
 */

/**********************************************************************
* Forward declarations
**********************************************************************/
typedef struct lpddr4_config_s lpddr4_config;
typedef struct lpddr4_privatedata_s lpddr4_privatedata;
typedef struct lpddr4_debuginfo_s lpddr4_debuginfo;
typedef struct lpddr4_fspmoderegs_s lpddr4_fspmoderegs;
typedef struct lpddr4_reginitdata_s lpddr4_reginitdata;

/**********************************************************************
* Enumerations
**********************************************************************/
/** This is used to indicate whether the Controller, PHY, or PHY Independent module is addressed. */
typedef enum
{
	LPDDR4_CTL_REGS = 0U,
	LPDDR4_PHY_REGS = 1U,
	LPDDR4_PHY_INDEP_REGS = 2U
} lpddr4_regblock;

/** Controller status or error interrupts. */
typedef enum
{
	LPDDR4_RESET_DONE = 0U,
	LPDDR4_BUS_ACCESS_ERROR = 1U,
	LPDDR4_MULTIPLE_BUS_ACCESS_ERROR = 2U,
	LPDDR4_ECC_MULTIPLE_CORR_ERROR = 3U,
	LPDDR4_ECC_MULTIPLE_UNCORR_ERROR = 4U,
	LPDDR4_ECC_WRITEBACK_EXEC_ERROR = 5U,
	LPDDR4_ECC_SCRUB_DONE = 6U,
	LPDDR4_ECC_SCRUB_ERROR = 7U,
	LPDDR4_PORT_COMMAND_ERROR = 8U,
	LPDDR4_MC_INIT_DONE = 9U,
	LPDDR4_LP_DONE = 10U,
	LPDDR4_BIST_DONE = 11U,
	LPDDR4_WRAP_ERROR = 12U,
	LPDDR4_INVALID_BURST_ERROR = 13U,
	LPDDR4_RDLVL_ERROR = 14U,
	LPDDR4_RDLVL_GATE_ERROR = 15U,
	LPDDR4_WRLVL_ERROR = 16U,
	LPDDR4_CA_TRAINING_ERROR = 17U,
	LPDDR4_DFI_UPDATE_ERROR = 18U,
	LPDDR4_MRR_ERROR = 19U,
	LPDDR4_PHY_MASTER_ERROR = 20U,
	LPDDR4_WRLVL_REQ = 21U,
	LPDDR4_RDLVL_REQ = 22U,
	LPDDR4_RDLVL_GATE_REQ = 23U,
	LPDDR4_CA_TRAINING_REQ = 24U,
	LPDDR4_LEVELING_DONE = 25U,
	LPDDR4_PHY_ERROR = 26U,
	LPDDR4_MR_READ_DONE = 27U,
	LPDDR4_TEMP_CHANGE = 28U,
	LPDDR4_TEMP_ALERT = 29U,
	LPDDR4_SW_DQS_COMPLETE = 30U,
	LPDDR4_DQS_OSC_BV_UPDATED = 31U,
	LPDDR4_DQS_OSC_OVERFLOW = 32U,
	LPDDR4_DQS_OSC_VAR_OUT = 33U,
	LPDDR4_MR_WRITE_DONE = 34U,
	LPDDR4_INHIBIT_DRAM_DONE = 35U,
	LPDDR4_DFI_INIT_STATE = 36U,
	LPDDR4_DLL_RESYNC_DONE = 37U,
	LPDDR4_TDFI_TO = 38U,
	LPDDR4_DFS_DONE = 39U,
	LPDDR4_DFS_STATUS = 40U,
	LPDDR4_REFRESH_STATUS = 41U,
	LPDDR4_ZQ_STATUS = 42U,
	LPDDR4_SW_REQ_MODE = 43U,
	LPDDR4_LOR_BITS = 44U
} lpddr4_ctlinterrupt;

/** PHY Independent Module status or error interrupts. */
typedef enum
{
	LPDDR4_PHY_INDEP_INIT_DONE_BIT = 0U,
	LPDDR4_PHY_INDEP_CONTROL_ERROR_BIT = 1U,
	LPDDR4_PHY_INDEP_CA_PARITY_ERR_BIT = 2U,
	LPDDR4_PHY_INDEP_RDLVL_ERROR_BIT = 3U,
	LPDDR4_PHY_INDEP_RDLVL_GATE_ERROR_BIT = 4U,
	LPDDR4_PHY_INDEP_WRLVL_ERROR_BIT = 5U,
	LPDDR4_PHY_INDEP_CALVL_ERROR_BIT = 6U,
	LPDDR4_PHY_INDEP_WDQLVL_ERROR_BIT = 7U,
	LPDDR4_PHY_INDEP_UPDATE_ERROR_BIT = 8U,
	LPDDR4_PHY_INDEP_RDLVL_REQ_BIT = 9U,
	LPDDR4_PHY_INDEP_RDLVL_GATE_REQ_BIT = 10U,
	LPDDR4_PHY_INDEP_WRLVL_REQ_BIT = 11U,
	LPDDR4_PHY_INDEP_CALVL_REQ_BIT = 12U,
	LPDDR4_PHY_INDEP_WDQLVL_REQ_BIT = 13U,
	LPDDR4_PHY_INDEP_LVL_DONE_BIT = 14U,
	LPDDR4_PHY_INDEP_BIST_DONE_BIT = 15U,
	LPDDR4_PHY_INDEP_TDFI_INIT_TIME_OUT_BIT = 16U,
	LPDDR4_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT = 17U
} lpddr4_phyindepinterrupt;

/** List of informations and warnings from driver. */
typedef enum
{
	LPDDR4_DRV_NONE = 0U,
	LPDDR4_DRV_SOC_PLL_UPDATE = 1U
} lpddr4_infotype;

/** Low power interface wake up timing parameters */
typedef enum
{
	LPDDR4_LPI_PD_WAKEUP_FN = 0U,
	LPDDR4_LPI_SR_SHORT_WAKEUP_FN = 1U,
	LPDDR4_LPI_SR_LONG_WAKEUP_FN = 2U,
	LPDDR4_LPI_SR_LONG_MCCLK_GATE_WAKEUP_FN = 3U,
	LPDDR4_LPI_SRPD_SHORT_WAKEUP_FN = 4U,
	LPDDR4_LPI_SRPD_LONG_WAKEUP_FN = 5U,
	LPDDR4_LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_FN = 6U
} lpddr4_lpiwakeupparam;

/** Half Datapath mode setting */
typedef enum
{
	LPDDR4_REDUC_ON = 0U,
	LPDDR4_REDUC_OFF = 1U
} lpddr4_reducmode;

/** ECC Control parameter setting */
typedef enum
{
	LPDDR4_ECC_DISABLED = 0U,
	LPDDR4_ECC_ENABLED = 1U,
	LPDDR4_ECC_ERR_DETECT = 2U,
	LPDDR4_ECC_ERR_DETECT_CORRECT = 3U
} lpddr4_eccenable;

/** Data Byte Inversion mode setting */
typedef enum
{
	LPDDR4_DBI_RD_ON = 0U,
	LPDDR4_DBI_RD_OFF = 1U,
	LPDDR4_DBI_WR_ON = 2U,
	LPDDR4_DBI_WR_OFF = 3U
} lpddr4_dbimode;

/** Controller Frequency Set Point number  */
typedef enum
{
	LPDDR4_FSP_0 = 0U,
	LPDDR4_FSP_1 = 1U,
	LPDDR4_FSP_2 = 2U
} lpddr4_ctlfspnum;

/**********************************************************************
* Callbacks
**********************************************************************/
/**
 * Reports informations and warnings that need to be communicated.
 * Params:
 * pD - driver state info specific to this instance.
 * infoType - Type of information.
 */
typedef void (*lpddr4_infocallback)(const lpddr4_privatedata* pd, lpddr4_infotype infotype);

/**
 * Reports interrupts received by the controller.
 * Params:
 * pD - driver state info specific to this instance.
 * ctlInterrupt - Interrupt raised
 * chipSelect - Chip for which interrupt raised
 */
typedef void (*lpddr4_ctlcallback)(const lpddr4_privatedata* pd, lpddr4_ctlinterrupt ctlinterrupt, uint8_t chipselect);

/**
 * Reports interrupts received by the PHY Independent Module.
 * Params:
 * privateData - driver state info specific to this instance.
 * phyIndepInterrupt - Interrupt raised
 * chipSelect - Chip for which interrupt raised
 */
typedef void (*lpddr4_phyindepcallback)(const lpddr4_privatedata* pd, lpddr4_phyindepinterrupt phyindepinterrupt, uint8_t chipselect);

/**
 *  @}
 */

/** @defgroup DriverFunctionAPI Driver Function API
 *  Prototypes for the driver API functions. The user application can link statically to the
 *  necessary API functions and call them directly.
 *  @{
 */

/**********************************************************************
* API methods
**********************************************************************/

/**
 * Checks configuration object.
 * @param[in] config Driver/hardware configuration required.
 * @param[out] configSize Size of memory allocations required.
 * @return CDN_EOK on success (requirements structure filled).
 * @return ENOTSUP if configuration cannot be supported due to driver/hardware constraints.
 */
uint32_t lpddr4_probe(const lpddr4_config* config, uint16_t* configsize);

/**
 * Init function to be called after LPDDR4_probe() to set up the
 * driver configuration.  Memory should be allocated for drv_data
 * (using the size determined using LPDDR4_probe)  before calling this
 * API.  init_settings should be initialised with base addresses for
 * PHY Indepenent Module, Controller and PHY before calling this
 * function.  If callbacks are required for interrupt handling, these
 * should also be configured in init_settings.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] cfg Specifies driver/hardware configuration.
 * @return CDN_EOK on success
 * @return EINVAL if illegal/inconsistent values in cfg.
 * @return ENOTSUP if hardware has an inconsistent configuration or doesn't support feature(s) required by 'config' parameters.
 */
uint32_t lpddr4_init(lpddr4_privatedata* pd, const lpddr4_config* cfg);

/**
 * Start the driver.
 * @param[in] pD Driver state info specific to this instance.
 */
uint32_t lpddr4_start(const lpddr4_privatedata* pd);

/**
 * Read a register from the controller, PHY or PHY Independent Module
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] cpp Indicates whether controller, PHY or PHY Independent Module register
 * @param[in] regOffset Register offset
 * @param[out] regValue Register value read
 * @return CDN_EOK on success.
 * @return EINVAL if regOffset if out of range or regValue is NULL
 */
uint32_t lpddr4_readreg(const lpddr4_privatedata* pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t* regvalue);

/**
 * Write a register in the controller, PHY or PHY Independent Module
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] cpp Indicates whether controller, PHY or PHY Independent Module register
 * @param[in] regOffset Register offset
 * @param[in] regValue Register value to be written
 * @return CDN_EOK on success.
 * @return EINVAL if regOffset is out of range or regValue is NULL
 */
uint32_t lpddr4_writereg(const lpddr4_privatedata* pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t regvalue);

/**
 * Read a memory mode register from DRAM
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] readModeRegVal Value to set in 'read_modereg' parameter.
 * @param[out] mmrValue Value which is read from memory mode register(mmr) for all devices.
 * @param[out] mmrStatus Status of mode register read(mrr) instruction.
 * @return CDN_EOK on success.
 * @return EINVAL if regNumber is out of range or regValue is NULL
 */
uint32_t lpddr4_getmmrregister(const lpddr4_privatedata* pd, uint32_t readmoderegval, uint64_t* mmrvalue, uint8_t* mmrstatus);

/**
 * Write a memory mode register in DRAM
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] writeModeRegVal Value to set in 'write_modereg' parameter.
 * @param[out] mrwStatus Status of mode register write(mrw) instruction.
 * @return CDN_EOK on success.
 * @return EINVAL if regNumber is out of range or regValue is NULL
 */
uint32_t lpddr4_setmmrregister(const lpddr4_privatedata* pd, uint32_t writemoderegval, uint8_t* mrwstatus);

/**
 * Write a set of initialisation values to the controller registers
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] regValues Register values to be written
 * @return CDN_EOK on success.
 * @return EINVAL if regValues is NULL
 */
uint32_t lpddr4_writectlconfig(const lpddr4_privatedata* pd, const lpddr4_reginitdata* regvalues);

/**
 * Write a set of initialisation values to the PHY registers
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] regValues Register values to be written
 * @return CDN_EOK on success.
 * @return EINVAL if regValues is NULL
 */
uint32_t lpddr4_writephyconfig(const lpddr4_privatedata* pd, const lpddr4_reginitdata* regvalues);

/**
 * Write a set of initialisation values to the PHY Independent Module
 * registers
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] regValues Register values to be written
 * @return CDN_EOK on success.
 * @return EINVAL if regValues is NULL
 */
uint32_t lpddr4_writephyindepconfig(const lpddr4_privatedata* pd, const lpddr4_reginitdata* regvalues);

/**
 * Read values of the controller registers in bulk (Set 'updateCtlReg'
 * to read) and store in memory.
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] regValues Register values which are read
 * @return CDN_EOK on success.
 * @return EINVAL if regValues is NULL
 */
uint32_t lpddr4_readctlconfig(const lpddr4_privatedata* pd, lpddr4_reginitdata* regvalues);

/**
 * Read the values of the PHY module registers in bulk (Set
 * 'updatePhyReg' to read) and store in memory.
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] regValues Register values which are read
 * @return CDN_EOK on success.
 * @return EINVAL if regValues is NULL
 */
uint32_t lpddr4_readphyconfig(const lpddr4_privatedata* pd, lpddr4_reginitdata* regvalues);

/**
 * Read the values of the PHY Independent module registers in bulk(Set
 * 'updatePhyIndepReg' to read) and store in memory.
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] regValues Register values which are read
 * @return CDN_EOK on success.
 * @return EINVAL if regValues is NULL
 */
uint32_t lpddr4_readphyindepconfig(const lpddr4_privatedata* pd, lpddr4_reginitdata* regvalues);

/**
 * Read the current interrupt mask for the controller
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] mask Value of interrupt mask
 * @return CDN_EOK on success.
 * @return EINVAL if mask pointer is NULL
 */
uint32_t lpddr4_getctlinterruptmask(const lpddr4_privatedata* pd, uint64_t* mask);

/**
 * Sets the interrupt mask for the controller
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] mask Value of interrupt mask to be written
 * @return CDN_EOK on success.
 * @return EINVAL if mask pointer is NULL
 */
uint32_t lpddr4_setctlinterruptmask(const lpddr4_privatedata* pd, const uint64_t* mask);

/**
 * Check whether a specific controller interrupt is active
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] intr Interrupt to be checked
 * @param[out] irqStatus Status of the interrupt, TRUE if active
 * @return CDN_EOK on success.
 * @return EINVAL if intr is not valid
 */
uint32_t lpddr4_checkctlinterrupt(const lpddr4_privatedata* pd, lpddr4_ctlinterrupt intr, bool* irqstatus);

/**
 * Acknowledge  a specific controller interrupt
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] intr Interrupt to be acknowledged
 * @return CDN_EOK on success.
 * @return EINVAL if intr is not valid
 */
uint32_t lpddr4_ackctlinterrupt(const lpddr4_privatedata* pd, lpddr4_ctlinterrupt intr);

/**
 * Read the current interrupt mask for the PHY Independent Module
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] mask Value of interrupt mask
 * @return CDN_EOK on success.
 * @return EINVAL if mask pointer is NULL
 */
uint32_t lpddr4_getphyindepinterruptmask(const lpddr4_privatedata* pd, uint32_t* mask);

/**
 * Sets the interrupt mask for the PHY Independent Module
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] mask Value of interrupt mask to be written
 * @return CDN_EOK on success.
 * @return EINVAL if mask pointer is NULL
 */
uint32_t lpddr4_setphyindepinterruptmask(const lpddr4_privatedata* pd, const uint32_t* mask);

/**
 * Check whether a specific PHY Independent Module interrupt is active
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] intr Interrupt to be checked
 * @param[out] irqStatus Status of the interrupt, TRUE if active
 * @return CDN_EOK on success.
 * @return EINVAL if intr is not valid
 */
uint32_t lpddr4_checkphyindepinterrupt(const lpddr4_privatedata* pd, lpddr4_phyindepinterrupt intr, bool* irqstatus);

/**
 * Acknowledge  a specific PHY Independent Module interrupt
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] intr Interrupt to be acknowledged
 * @return CDN_EOK on success.
 * @return EINVAL if intr is not valid
 */
uint32_t lpddr4_ackphyindepinterrupt(const lpddr4_privatedata* pd, lpddr4_phyindepinterrupt intr);

/**
 * Retrieve status information after a failed init.  The
 * DebugStructInfo will be filled  in with error codes which can be
 * referenced against the driver documentation for further details.
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] debugInfo status
 * @return CDN_EOK on success.
 * @return EINVAL if debugInfo is NULL
 */
uint32_t lpddr4_getdebuginitinfo(const lpddr4_privatedata* pd, lpddr4_debuginfo* debuginfo);

/**
 * Get the current value of Low power Interface wake up time.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] lpiWakeUpParam LPI timing parameter
 * @param[in] fspNum Frequency copy
 * @param[out] cycles Timing value(in cycles)
 * @return CDN_EOK on success.
 * @return EINVAL if powerMode is NULL
 */
uint32_t lpddr4_getlpiwakeuptime(const lpddr4_privatedata* pd, const lpddr4_lpiwakeupparam* lpiwakeupparam, const lpddr4_ctlfspnum* fspnum, uint32_t* cycles);

/**
 * Set the current value of Low power Interface wake up time.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] lpiWakeUpParam LPI timing parameter
 * @param[in] fspNum Frequency copy
 * @param[in] cycles Timing value(in cycles)
 * @return CDN_EOK on success.
 * @return EINVAL if powerMode is NULL
 */
uint32_t lpddr4_setlpiwakeuptime(const lpddr4_privatedata* pd, const lpddr4_lpiwakeupparam* lpiwakeupparam, const lpddr4_ctlfspnum* fspnum, const uint32_t* cycles);

/**
 * Get the current value for ECC auto correction
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] eccParam ECC parameter setting
 * @return CDN_EOK on success.
 * @return EINVAL if on_off is NULL
 */
uint32_t lpddr4_geteccenable(const lpddr4_privatedata* pd, lpddr4_eccenable* eccparam);

/**
 * Set the value for ECC auto correction.  This API must be called
 * before startup of memory.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] eccParam ECC control parameter setting
 * @return CDN_EOK on success.
 * @return EINVAL if on_off is NULL
 */
uint32_t lpddr4_seteccenable(const lpddr4_privatedata* pd, const lpddr4_eccenable* eccparam);

/**
 * Get the current value for the Half Datapath option
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] mode Half Datapath setting
 * @return CDN_EOK on success.
 * @return EINVAL if mode is NULL
 */
uint32_t lpddr4_getreducmode(const lpddr4_privatedata* pd, lpddr4_reducmode* mode);

/**
 * Set the value for the Half Datapath option.  This API must be
 * called before startup of memory.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] mode Half Datapath setting
 * @return CDN_EOK on success.
 * @return EINVAL if mode is NULL
 */
uint32_t lpddr4_setreducmode(const lpddr4_privatedata* pd, const lpddr4_reducmode* mode);

/**
 * Get the current value for Data Bus Inversion setting.  This will be
 * compared with the   current DRAM setting using the MR3 register.
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] on_off DBI read value
 * @return CDN_EOK on success.
 * @return EINVAL if on_off is NULL
 */
uint32_t lpddr4_getdbireadmode(const lpddr4_privatedata* pd, bool* on_off);

/**
 * Get the current value for Data Bus Inversion setting.  This will be
 * compared with the   current DRAM setting using the MR3 register.
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] on_off DBI write value
 * @return CDN_EOK on success.
 * @return EINVAL if on_off is NULL
 */
uint32_t lpddr4_getdbiwritemode(const lpddr4_privatedata* pd, bool* on_off);

/**
 * Set the mode for Data Bus Inversion. This will also be set in DRAM
 * using the MR3   controller register. This API must be called before
 * startup of memory.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] mode status
 * @return CDN_EOK on success.
 * @return EINVAL if mode is NULL
 */
uint32_t lpddr4_setdbimode(const lpddr4_privatedata* pd, const lpddr4_dbimode* mode);

/**
 * Get the current value for the refresh rate (reading Refresh per
 * command timing).
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] fspNum Frequency set number
 * @param[out] cycles Refresh rate (in cycles)
 * @return CDN_EOK on success.
 * @return EINVAL if rate is NULL
 */
uint32_t lpddr4_getrefreshrate(const lpddr4_privatedata* pd, const lpddr4_ctlfspnum* fspnum, uint32_t* cycles);

/**
 * Set the refresh rate (writing Refresh per command timing).
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] fspNum Frequency set number
 * @param[in] cycles Refresh rate (in cycles)
 * @return CDN_EOK on success.
 * @return EINVAL if rate is NULL
 */
uint32_t lpddr4_setrefreshrate(const lpddr4_privatedata* pd, const lpddr4_ctlfspnum* fspnum, const uint32_t* cycles);

/**
 * Handle Refreshing per chip select
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] trefInterval status
 * @return CDN_EOK on success.
 * @return EINVAL if chipSelect is invalid
 */
uint32_t lpddr4_refreshperchipselect(const lpddr4_privatedata* pd, const uint32_t trefinterval);

#endif  /* LPDDR4_IF_H */
