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

/**
 * This file contains sanity API functions. The purpose of sanity functions
 * is to check input parameters validity. They take the same parameters as
 * original API functions and return 0 on success or EINVAL on wrong parameter
 * value(s).
 */

#ifndef LPDDR4_SANITY_H
#define LPDDR4_SANITY_H

#include <errno.h>
#include <linux/types.h>
#include "lpddr4_if.h"

#define CDN_EOK             0U      /* no error */

static inline uint32_t lpddr4_configsf(const lpddr4_config *obj);
static inline uint32_t lpddr4_privatedatasf(const lpddr4_privatedata *obj);
static inline uint32_t lpddr4_reginitdatasf(const lpddr4_reginitdata *obj);

static inline uint32_t lpddr4_sanityfunction1(const lpddr4_config* config, const uint16_t* configsize);
static inline uint32_t lpddr4_sanityfunction2(const lpddr4_privatedata* pd, const lpddr4_config* cfg);
static inline uint32_t lpddr4_sanityfunction3(const lpddr4_privatedata* pd);
static inline uint32_t lpddr4_sanityfunction4(const lpddr4_privatedata* pd, const lpddr4_regblock cpp, const uint32_t* regvalue);
static inline uint32_t lpddr4_sanityfunction5(const lpddr4_privatedata* pd, const lpddr4_regblock cpp);
static inline uint32_t lpddr4_sanityfunction6(const lpddr4_privatedata* pd, const uint64_t* mmrvalue, const uint8_t* mmrstatus);
static inline uint32_t lpddr4_sanityfunction7(const lpddr4_privatedata* pd, const uint8_t* mrwstatus);
static inline uint32_t lpddr4_sanityfunction8(const lpddr4_privatedata* pd, const lpddr4_reginitdata* regvalues);
static inline uint32_t lpddr4_sanityfunction11(const lpddr4_privatedata* pd, const lpddr4_reginitdata* regvalues);
static inline uint32_t lpddr4_sanityfunction14(const lpddr4_privatedata* pd, const uint64_t* mask);
static inline uint32_t lpddr4_sanityfunction15(const lpddr4_privatedata* pd, const uint64_t* mask);
static inline uint32_t lpddr4_sanityfunction16(const lpddr4_privatedata* pd, const lpddr4_ctlinterrupt intr, const bool* irqstatus);
static inline uint32_t lpddr4_sanityfunction17(const lpddr4_privatedata* pd, const lpddr4_ctlinterrupt intr);
static inline uint32_t lpddr4_sanityfunction18(const lpddr4_privatedata* pd, const uint32_t* mask);
static inline uint32_t lpddr4_sanityfunction20(const lpddr4_privatedata* pd, const lpddr4_phyindepinterrupt intr, const bool* irqstatus);
static inline uint32_t lpddr4_sanityfunction21(const lpddr4_privatedata* pd, const lpddr4_phyindepinterrupt intr);
static inline uint32_t lpddr4_sanityfunction22(const lpddr4_privatedata* pd, const lpddr4_debuginfo* debuginfo);
static inline uint32_t lpddr4_sanityfunction23(const lpddr4_privatedata* pd, const lpddr4_lpiwakeupparam* lpiwakeupparam, const lpddr4_ctlfspnum* fspnum, const uint32_t* cycles);
static inline uint32_t lpddr4_sanityfunction25(const lpddr4_privatedata* pd, const lpddr4_eccenable* eccparam);
static inline uint32_t lpddr4_sanityfunction26(const lpddr4_privatedata* pd, const lpddr4_eccenable* eccparam);
static inline uint32_t lpddr4_sanityfunction27(const lpddr4_privatedata* pd, const lpddr4_reducmode* mode);
static inline uint32_t lpddr4_sanityfunction28(const lpddr4_privatedata* pd, const lpddr4_reducmode* mode);
static inline uint32_t lpddr4_sanityfunction29(const lpddr4_privatedata* pd, const bool* on_off);
static inline uint32_t lpddr4_sanityfunction31(const lpddr4_privatedata* pd, const lpddr4_dbimode* mode);
static inline uint32_t lpddr4_sanityfunction32(const lpddr4_privatedata* pd, const lpddr4_ctlfspnum* fspnum, const uint32_t* cycles);

#define lpddr4_probesf lpddr4_sanityfunction1
#define lpddr4_initsf lpddr4_sanityfunction2
#define lpddr4_startsf lpddr4_sanityfunction3
#define lpddr4_readregsf lpddr4_sanityfunction4
#define lpddr4_writeregsf lpddr4_sanityfunction5
#define lpddr4_getmmrregistersf lpddr4_sanityfunction6
#define lpddr4_setmmrregistersf lpddr4_sanityfunction7
#define lpddr4_writectlconfigsf lpddr4_sanityfunction8
#define lpddr4_writephyconfigsf lpddr4_sanityfunction8
#define lpddr4_writephyindepconfigsf lpddr4_sanityfunction8
#define lpddr4_readctlconfigsf lpddr4_sanityfunction11
#define lpddr4_readphyconfigsf lpddr4_sanityfunction11
#define lpddr4_readphyindepconfigsf lpddr4_sanityfunction11
#define lpddr4_getctlinterruptmasksf lpddr4_sanityfunction14
#define lpddr4_setctlinterruptmasksf lpddr4_sanityfunction15
#define lpddr4_checkctlinterruptsf lpddr4_sanityfunction16
#define lpddr4_ackctlinterruptsf lpddr4_sanityfunction17
#define lpddr4_getphyindepinterruptmsf lpddr4_sanityfunction18
#define lpddr4_setphyindepinterruptmsf lpddr4_sanityfunction18
#define lpddr4_checkphyindepinterrupsf lpddr4_sanityfunction20
#define lpddr4_ackphyindepinterruptsf lpddr4_sanityfunction21
#define lpddr4_getdebuginitinfosf lpddr4_sanityfunction22
#define lpddr4_getlpiwakeuptimesf lpddr4_sanityfunction23
#define lpddr4_setlpiwakeuptimesf lpddr4_sanityfunction23
#define lpddr4_geteccenablesf lpddr4_sanityfunction25
#define lpddr4_seteccenablesf lpddr4_sanityfunction26
#define lpddr4_getreducmodesf lpddr4_sanityfunction27
#define lpddr4_setreducmodesf lpddr4_sanityfunction28
#define lpddr4_getdbireadmodesf lpddr4_sanityfunction29
#define lpddr4_getdbiwritemodesf lpddr4_sanityfunction29
#define lpddr4_setdbimodesf lpddr4_sanityfunction31
#define lpddr4_getrefreshratesf lpddr4_sanityfunction32
#define lpddr4_setrefreshratesf lpddr4_sanityfunction32
#define lpddr4_refreshperchipselectsf lpddr4_sanityfunction3

/**
 * Function to validate struct Config
 *
 * @param[in] obj pointer to struct to be verified
 * @returns 0 for valid
 * @returns EINVAL for invalid
 */
static inline uint32_t lpddr4_configsf(const lpddr4_config *obj)
{
	uint32_t ret = 0;

	if (obj == NULL)
	{
		ret = EINVAL;
	}

	return ret;
}

/**
 * Function to validate struct PrivateData
 *
 * @param[in] obj pointer to struct to be verified
 * @returns 0 for valid
 * @returns EINVAL for invalid
 */
static inline uint32_t lpddr4_privatedatasf(const lpddr4_privatedata *obj)
{
	uint32_t ret = 0;

	if (obj == NULL)
	{
		ret = EINVAL;
	}

	return ret;
}

/**
 * Function to validate struct RegInitData
 *
 * @param[in] obj pointer to struct to be verified
 * @returns 0 for valid
 * @returns EINVAL for invalid
 */
static inline uint32_t lpddr4_reginitdatasf(const lpddr4_reginitdata *obj)
{
	uint32_t ret = 0;

	if (obj == NULL)
	{
		ret = EINVAL;
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] config Driver/hardware configuration required.
 * @param[out] configSize Size of memory allocations required.
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction1(const lpddr4_config* config, const uint16_t* configsize)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (configsize == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_configsf(config) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] cfg Specifies driver/hardware configuration.
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction2(const lpddr4_privatedata* pd, const lpddr4_config* cfg)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_configsf(cfg) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction3(const lpddr4_privatedata* pd)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] cpp Indicates whether controller, PHY or PHY Independent Module register
 * @param[out] regValue Register value read
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction4(const lpddr4_privatedata* pd, const lpddr4_regblock cpp, const uint32_t* regvalue)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (regvalue == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(cpp != LPDDR4_CTL_REGS) &&
		(cpp != LPDDR4_PHY_REGS) &&
		(cpp != LPDDR4_PHY_INDEP_REGS)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] cpp Indicates whether controller, PHY or PHY Independent Module register
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction5(const lpddr4_privatedata* pd, const lpddr4_regblock cpp)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(cpp != LPDDR4_CTL_REGS) &&
		(cpp != LPDDR4_PHY_REGS) &&
		(cpp != LPDDR4_PHY_INDEP_REGS)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] mmrValue Value which is read from memory mode register(mmr) for all devices.
 * @param[out] mmrStatus Status of mode register read(mrr) instruction.
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction6(const lpddr4_privatedata* pd, const uint64_t* mmrvalue, const uint8_t* mmrstatus)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (mmrvalue == NULL)
	{
		ret = EINVAL;
	}
	else if (mmrstatus == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] mrwStatus Status of mode register write(mrw) instruction.
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction7(const lpddr4_privatedata* pd, const uint8_t* mrwstatus)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (mrwstatus == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] regValues Register values to be written
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction8(const lpddr4_privatedata* pd, const lpddr4_reginitdata* regvalues)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_reginitdatasf(regvalues) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] regValues Register values which are read
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction11(const lpddr4_privatedata* pd, const lpddr4_reginitdata* regvalues)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (regvalues == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] mask Value of interrupt mask
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction14(const lpddr4_privatedata* pd, const uint64_t* mask)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (mask == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] mask Value of interrupt mask to be written
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction15(const lpddr4_privatedata* pd, const uint64_t* mask)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (mask == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] intr Interrupt to be checked
 * @param[out] irqStatus Status of the interrupt, TRUE if active
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction16(const lpddr4_privatedata* pd, const lpddr4_ctlinterrupt intr, const bool* irqstatus)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (irqstatus == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(intr != LPDDR4_RESET_DONE) &&
		(intr != LPDDR4_BUS_ACCESS_ERROR) &&
		(intr != LPDDR4_MULTIPLE_BUS_ACCESS_ERROR) &&
		(intr != LPDDR4_ECC_MULTIPLE_CORR_ERROR) &&
		(intr != LPDDR4_ECC_MULTIPLE_UNCORR_ERROR) &&
		(intr != LPDDR4_ECC_WRITEBACK_EXEC_ERROR) &&
		(intr != LPDDR4_ECC_SCRUB_DONE) &&
		(intr != LPDDR4_ECC_SCRUB_ERROR) &&
		(intr != LPDDR4_PORT_COMMAND_ERROR) &&
		(intr != LPDDR4_MC_INIT_DONE) &&
		(intr != LPDDR4_LP_DONE) &&
		(intr != LPDDR4_BIST_DONE) &&
		(intr != LPDDR4_WRAP_ERROR) &&
		(intr != LPDDR4_INVALID_BURST_ERROR) &&
		(intr != LPDDR4_RDLVL_ERROR) &&
		(intr != LPDDR4_RDLVL_GATE_ERROR) &&
		(intr != LPDDR4_WRLVL_ERROR) &&
		(intr != LPDDR4_CA_TRAINING_ERROR) &&
		(intr != LPDDR4_DFI_UPDATE_ERROR) &&
		(intr != LPDDR4_MRR_ERROR) &&
		(intr != LPDDR4_PHY_MASTER_ERROR) &&
		(intr != LPDDR4_WRLVL_REQ) &&
		(intr != LPDDR4_RDLVL_REQ) &&
		(intr != LPDDR4_RDLVL_GATE_REQ) &&
		(intr != LPDDR4_CA_TRAINING_REQ) &&
		(intr != LPDDR4_LEVELING_DONE) &&
		(intr != LPDDR4_PHY_ERROR) &&
		(intr != LPDDR4_MR_READ_DONE) &&
		(intr != LPDDR4_TEMP_CHANGE) &&
		(intr != LPDDR4_TEMP_ALERT) &&
		(intr != LPDDR4_SW_DQS_COMPLETE) &&
		(intr != LPDDR4_DQS_OSC_BV_UPDATED) &&
		(intr != LPDDR4_DQS_OSC_OVERFLOW) &&
		(intr != LPDDR4_DQS_OSC_VAR_OUT) &&
		(intr != LPDDR4_MR_WRITE_DONE) &&
		(intr != LPDDR4_INHIBIT_DRAM_DONE) &&
		(intr != LPDDR4_DFI_INIT_STATE) &&
		(intr != LPDDR4_DLL_RESYNC_DONE) &&
		(intr != LPDDR4_TDFI_TO) &&
		(intr != LPDDR4_DFS_DONE) &&
		(intr != LPDDR4_DFS_STATUS) &&
		(intr != LPDDR4_REFRESH_STATUS) &&
		(intr != LPDDR4_ZQ_STATUS) &&
		(intr != LPDDR4_SW_REQ_MODE) &&
		(intr != LPDDR4_LOR_BITS)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] intr Interrupt to be acknowledged
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction17(const lpddr4_privatedata* pd, const lpddr4_ctlinterrupt intr)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(intr != LPDDR4_RESET_DONE) &&
		(intr != LPDDR4_BUS_ACCESS_ERROR) &&
		(intr != LPDDR4_MULTIPLE_BUS_ACCESS_ERROR) &&
		(intr != LPDDR4_ECC_MULTIPLE_CORR_ERROR) &&
		(intr != LPDDR4_ECC_MULTIPLE_UNCORR_ERROR) &&
		(intr != LPDDR4_ECC_WRITEBACK_EXEC_ERROR) &&
		(intr != LPDDR4_ECC_SCRUB_DONE) &&
		(intr != LPDDR4_ECC_SCRUB_ERROR) &&
		(intr != LPDDR4_PORT_COMMAND_ERROR) &&
		(intr != LPDDR4_MC_INIT_DONE) &&
		(intr != LPDDR4_LP_DONE) &&
		(intr != LPDDR4_BIST_DONE) &&
		(intr != LPDDR4_WRAP_ERROR) &&
		(intr != LPDDR4_INVALID_BURST_ERROR) &&
		(intr != LPDDR4_RDLVL_ERROR) &&
		(intr != LPDDR4_RDLVL_GATE_ERROR) &&
		(intr != LPDDR4_WRLVL_ERROR) &&
		(intr != LPDDR4_CA_TRAINING_ERROR) &&
		(intr != LPDDR4_DFI_UPDATE_ERROR) &&
		(intr != LPDDR4_MRR_ERROR) &&
		(intr != LPDDR4_PHY_MASTER_ERROR) &&
		(intr != LPDDR4_WRLVL_REQ) &&
		(intr != LPDDR4_RDLVL_REQ) &&
		(intr != LPDDR4_RDLVL_GATE_REQ) &&
		(intr != LPDDR4_CA_TRAINING_REQ) &&
		(intr != LPDDR4_LEVELING_DONE) &&
		(intr != LPDDR4_PHY_ERROR) &&
		(intr != LPDDR4_MR_READ_DONE) &&
		(intr != LPDDR4_TEMP_CHANGE) &&
		(intr != LPDDR4_TEMP_ALERT) &&
		(intr != LPDDR4_SW_DQS_COMPLETE) &&
		(intr != LPDDR4_DQS_OSC_BV_UPDATED) &&
		(intr != LPDDR4_DQS_OSC_OVERFLOW) &&
		(intr != LPDDR4_DQS_OSC_VAR_OUT) &&
		(intr != LPDDR4_MR_WRITE_DONE) &&
		(intr != LPDDR4_INHIBIT_DRAM_DONE) &&
		(intr != LPDDR4_DFI_INIT_STATE) &&
		(intr != LPDDR4_DLL_RESYNC_DONE) &&
		(intr != LPDDR4_TDFI_TO) &&
		(intr != LPDDR4_DFS_DONE) &&
		(intr != LPDDR4_DFS_STATUS) &&
		(intr != LPDDR4_REFRESH_STATUS) &&
		(intr != LPDDR4_ZQ_STATUS) &&
		(intr != LPDDR4_SW_REQ_MODE) &&
		(intr != LPDDR4_LOR_BITS)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] mask Value of interrupt mask
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction18(const lpddr4_privatedata* pd, const uint32_t* mask)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (mask == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] intr Interrupt to be checked
 * @param[out] irqStatus Status of the interrupt, TRUE if active
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction20(const lpddr4_privatedata* pd, const lpddr4_phyindepinterrupt intr, const bool* irqstatus)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (irqstatus == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(intr != LPDDR4_PHY_INDEP_INIT_DONE_BIT) &&
		(intr != LPDDR4_PHY_INDEP_CONTROL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_CA_PARITY_ERR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_RDLVL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_RDLVL_GATE_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_WRLVL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_CALVL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_WDQLVL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_UPDATE_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_RDLVL_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_RDLVL_GATE_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_WRLVL_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_CALVL_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_WDQLVL_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_LVL_DONE_BIT) &&
		(intr != LPDDR4_PHY_INDEP_BIST_DONE_BIT) &&
		(intr != LPDDR4_PHY_INDEP_TDFI_INIT_TIME_OUT_BIT) &&
		(intr != LPDDR4_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] intr Interrupt to be acknowledged
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction21(const lpddr4_privatedata* pd, const lpddr4_phyindepinterrupt intr)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(intr != LPDDR4_PHY_INDEP_INIT_DONE_BIT) &&
		(intr != LPDDR4_PHY_INDEP_CONTROL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_CA_PARITY_ERR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_RDLVL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_RDLVL_GATE_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_WRLVL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_CALVL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_WDQLVL_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_UPDATE_ERROR_BIT) &&
		(intr != LPDDR4_PHY_INDEP_RDLVL_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_RDLVL_GATE_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_WRLVL_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_CALVL_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_WDQLVL_REQ_BIT) &&
		(intr != LPDDR4_PHY_INDEP_LVL_DONE_BIT) &&
		(intr != LPDDR4_PHY_INDEP_BIST_DONE_BIT) &&
		(intr != LPDDR4_PHY_INDEP_TDFI_INIT_TIME_OUT_BIT) &&
		(intr != LPDDR4_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] debugInfo status
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction22(const lpddr4_privatedata* pd, const lpddr4_debuginfo* debuginfo)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (debuginfo == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] lpiWakeUpParam LPI timing parameter
 * @param[in] fspNum Frequency copy
 * @param[out] cycles Timing value(in cycles)
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction23(const lpddr4_privatedata* pd, const lpddr4_lpiwakeupparam* lpiwakeupparam, const lpddr4_ctlfspnum* fspnum, const uint32_t* cycles)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (lpiwakeupparam == NULL)
	{
		ret = EINVAL;
	}
	else if (fspnum == NULL)
	{
		ret = EINVAL;
	}
	else if (cycles == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(*lpiwakeupparam != LPDDR4_LPI_PD_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SR_SHORT_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SR_LONG_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SR_LONG_MCCLK_GATE_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SRPD_SHORT_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SRPD_LONG_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_FN)
		)
	{
		ret = EINVAL;
	}
	else if (
		(*fspnum != LPDDR4_FSP_0) &&
		(*fspnum != LPDDR4_FSP_1) &&
		(*fspnum != LPDDR4_FSP_2)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] eccParam ECC parameter setting
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction25(const lpddr4_privatedata* pd, const lpddr4_eccenable* eccparam)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (eccparam == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] eccParam ECC control parameter setting
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction26(const lpddr4_privatedata* pd, const lpddr4_eccenable* eccparam)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (eccparam == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(*eccparam != LPDDR4_ECC_DISABLED) &&
		(*eccparam != LPDDR4_ECC_ENABLED) &&
		(*eccparam != LPDDR4_ECC_ERR_DETECT) &&
		(*eccparam != LPDDR4_ECC_ERR_DETECT_CORRECT)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] mode Half Datapath setting
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction27(const lpddr4_privatedata* pd, const lpddr4_reducmode* mode)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (mode == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] mode Half Datapath setting
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction28(const lpddr4_privatedata* pd, const lpddr4_reducmode* mode)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (mode == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(*mode != LPDDR4_REDUC_ON) &&
		(*mode != LPDDR4_REDUC_OFF)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[out] on_off DBI read value
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction29(const lpddr4_privatedata* pd, const bool* on_off)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (on_off == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] mode status
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction31(const lpddr4_privatedata* pd, const lpddr4_dbimode* mode)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (mode == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(*mode != LPDDR4_DBI_RD_ON) &&
		(*mode != LPDDR4_DBI_RD_OFF) &&
		(*mode != LPDDR4_DBI_WR_ON) &&
		(*mode != LPDDR4_DBI_WR_OFF)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] fspNum Frequency set number
 * @param[out] cycles Refresh rate (in cycles)
 * @return 0 success
 * @return EINVAL invalid parameters
 */
static inline uint32_t lpddr4_sanityfunction32(const lpddr4_privatedata* pd, const lpddr4_ctlfspnum* fspnum, const uint32_t* cycles)
{
	/* Declaring return variable */
	uint32_t ret = 0;

	if (fspnum == NULL)
	{
		ret = EINVAL;
	}
	else if (cycles == NULL)
	{
		ret = EINVAL;
	}
	else if (lpddr4_privatedatasf(pd) == EINVAL)
	{
		ret = EINVAL;
	}
	else if (
		(*fspnum != LPDDR4_FSP_0) &&
		(*fspnum != LPDDR4_FSP_1) &&
		(*fspnum != LPDDR4_FSP_2)
		)
	{
		ret = EINVAL;
	}
	else
	{
		/*
		 * All 'if ... else if' constructs shall be terminated with an 'else' statement
		 * (MISRA2012-RULE-15_7-3)
		 */
	}

	return ret;
}

#endif  /* LPDDR4_SANITY_H */
