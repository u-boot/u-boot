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
#ifndef LPDDR4_STRUCTS_IF_H
#define LPDDR4_STRUCTS_IF_H

#include <linux/types.h>
#include "lpddr4_if.h"

/** @defgroup DataStructure Dynamic Data Structures
 *  This section defines the data structures used by the driver to provide
 *  hardware information, modification and dynamic operation of the driver.
 *  These data structures are defined in the header file of the core driver
 *  and utilized by the API.
 *  @{
 */

/**********************************************************************
* Structures and unions
**********************************************************************/
/**
 * Configuration of device.
 * Object of this type is used for probe and init functions.
 */
struct lpddr4_config_s
{
	/** Base address of controller registers */
	struct lpddr4_ctlregs_s* ctlbase;
	/** Information/warning handler */
	lpddr4_infocallback infohandler;
	/** Controller interrupt handler */
	lpddr4_ctlcallback ctlinterrupthandler;
	/** PHY Independent Module interrupt handler */
	lpddr4_phyindepcallback phyindepinterrupthandler;
};

/**
 * Structure contains private data for Core Driver that should not be used by
 * upper layers. This is not a part of API and manipulating of those data may cause
 * unpredictable behavior of Core Driver.
 */
struct lpddr4_privatedata_s
{
	/** Base address of controller registers */
	struct lpddr4_ctlregs_s* ctlbase;
	/** Information/warning handler */
	lpddr4_infocallback infohandler;
	/** Controller interrupt handler */
	lpddr4_ctlcallback ctlinterrupthandler;
	/** PHY Independent Module interrupt handler */
	lpddr4_phyindepcallback phyindepinterrupthandler;
};

/** Structure to contain debug information reported by the driver. */
struct lpddr4_debuginfo_s
{
	/** PLL Lock error. */
	bool pllerror;
	/** I/O calibration error. */
	bool iocaliberror;
	/** RX offset error. */
	bool rxoffseterror;
	/** CA training error. */
	bool catraingerror;
	/** Write levelling error. */
	bool wrlvlerror;
	/** Gate Level error. */
	bool gatelvlerror;
	/** Read Level error. */
	bool readlvlerror;
	/** Write DQ training error. */
	bool dqtrainingerror;
};

/** Frequency Set Point mode register values */
struct lpddr4_fspmoderegs_s
{
	/** MR1 register data for the FSP. */
	uint8_t mr1data_fn[LPDDR4_MAX_CS];
	/** MR2 register data for the FSP. */
	uint8_t mr2data_fn[LPDDR4_MAX_CS];
	/** MR3 register data for the FSP. */
	uint8_t mr3data_fn[LPDDR4_MAX_CS];
	/** MR11 register data for the FSP. */
	uint8_t mr11data_fn[LPDDR4_MAX_CS];
	/** MR12 register data for the FSP. */
	uint8_t mr12data_fn[LPDDR4_MAX_CS];
	/** MR13 register data for the FSP. */
	uint8_t mr13data_fn[LPDDR4_MAX_CS];
	/** MR14 register data for the FSP. */
	uint8_t mr14data_fn[LPDDR4_MAX_CS];
	/** MR22 register data for the selected frequency. */
	uint8_t mr22data_fn[LPDDR4_MAX_CS];
};

/** Structure to hold data set to initalise registers. */
struct lpddr4_reginitdata_s
{
	/** Register initialisation data for the Controller. */
	uint32_t denalictlreg[LPDDR4_CTL_REG_COUNT];
	/** Should be set to true, if the corresponding denaliCtlReg element has been updated. */
	bool updatectlreg[LPDDR4_CTL_REG_COUNT];
	/** Register initialisation data for PHY independent module. */
	uint32_t denaliphyindepreg[LPDDR4_PHY_INDEP_REG_COUNT];
	/** Should be set to true, if the corresponding denaliPhyIndepReg element has been updated. */
	bool updatephyindepreg[LPDDR4_PHY_INDEP_REG_COUNT];
	/** Register initialisation data for the PHY. */
	uint32_t denaliphyreg[LPDDR4_PHY_REG_COUNT];
	/** Should be set to true, if the corresponding denaliPhyReg element has been updated. */
	bool updatephyreg[LPDDR4_PHY_REG_COUNT];
};

#endif  /* LPDDR4_STRUCTS_IF_H */
