// SPDX-License-Identifier: BSD-3-Clause
/******************************************************************************
 * Copyright (C) 2012-2018 Cadence Design Systems, Inc.
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com/
 *
 * lpddr4.c
 *
 *****************************************************************************
 */
#include "cps_drv_lpddr4.h"
#include "lpddr4_ctl_regs.h"
#include "lpddr4_if.h"
#include "lpddr4_private.h"
#include "lpddr4_sanity.h"
#include "lpddr4_structs_if.h"

#define LPDDR4_CUSTOM_TIMEOUT_DELAY 100000000U

/**
 * Internal Function:Poll for status of interrupt received by the Controller.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] irqBit Interrupt status bit to be checked.
 * @param[in] delay time delay.
 * @return CDN_EOK on success (Interrupt status high).
 * @return EIO on poll time out.
 * @return EINVAL checking status was not successful.
 */
static uint32_t lpddr4_pollctlirq(const lpddr4_privatedata * pd,
				  lpddr4_ctlinterrupt irqbit, uint32_t delay)
{

	uint32_t result = 0U;
	uint32_t timeout = 0U;
	bool irqstatus = false;

	/* Loop until irqStatus found to be 1 or if value of 'result' !=CDN_EOK */
	do {
		if (++timeout == delay) {
			result = EIO;
			break;
		}
		/* cps_delayns(10000000U); */
		result = lpddr4_checkctlinterrupt(pd, irqbit, &irqstatus);
	} while ((irqstatus == false) && (result == (uint32_t) CDN_EOK));

	return result;
}

/**
 * Internal Function:Poll for status of interrupt received by the PHY Independent Module.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] irqBit Interrupt status bit to be checked.
 * @param[in] delay time delay.
 * @return CDN_EOK on success (Interrupt status high).
 * @return EIO on poll time out.
 * @return EINVAL checking status was not successful.
 */
static uint32_t lpddr4_pollphyindepirq(const lpddr4_privatedata * pd,
				       lpddr4_phyindepinterrupt irqbit,
				       uint32_t delay)
{

	uint32_t result = 0U;
	uint32_t timeout = 0U;
	bool irqstatus = false;

	/* Loop until irqStatus found to be 1 or if value of 'result' !=CDN_EOK */
	do {
		if (++timeout == delay) {
			result = EIO;
			break;
		}
		/* cps_delayns(10000000U); */
		result = lpddr4_checkphyindepinterrupt(pd, irqbit, &irqstatus);
	} while ((irqstatus == false) && (result == (uint32_t) CDN_EOK));

	return result;
}

/**
 * Internal Function:Trigger function to poll and Ack IRQs
 * @param[in] pD Driver state info specific to this instance.
 * @return CDN_EOK on success (Interrupt status high).
 * @return EIO on poll time out.
 * @return EINVAL checking status was not successful.
 */
static uint32_t lpddr4_pollandackirq(const lpddr4_privatedata * pd)
{
	uint32_t result = 0U;

	/* Wait for PhyIndependent module to finish up ctl init sequence */
	result =
	    lpddr4_pollphyindepirq(pd, LPDDR4_PHY_INDEP_INIT_DONE_BIT,
				   LPDDR4_CUSTOM_TIMEOUT_DELAY);

	/* Ack to clear the PhyIndependent interrupt bit */
	if (result == (uint32_t) CDN_EOK) {
		result =
		    lpddr4_ackphyindepinterrupt(pd,
						LPDDR4_PHY_INDEP_INIT_DONE_BIT);
	}
	/* Wait for the CTL end of initialization */
	if (result == (uint32_t) CDN_EOK) {
		result =
		    lpddr4_pollctlirq(pd, LPDDR4_MC_INIT_DONE,
				      LPDDR4_CUSTOM_TIMEOUT_DELAY);
	}
	/* Ack to clear the Ctl interrupt bit */
	if (result == (uint32_t) CDN_EOK) {
		result = lpddr4_ackctlinterrupt(pd, LPDDR4_MC_INIT_DONE);
	}
	return result;
}

/**
 * Internal Function: Controller start sequence.
 * @param[in] pD Driver state info specific to this instance.
 * @return CDN_EOK on success.
 * @return EINVAL starting controller was not successful.
 */
static uint32_t lpddr4_startsequencecontroller(const lpddr4_privatedata * pd)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
	lpddr4_infotype infotype;

	/* Set the PI_start to initiate leveling procedure */
	regval =
	    CPS_FLD_SET(LPDDR4__PI_START__FLD,
			CPS_REG_READ(&(ctlregbase->LPDDR4__PI_START__REG)));
	CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_START__REG)), regval);

	/* Set the Ctl_start  */
	regval =
	    CPS_FLD_SET(LPDDR4__START__FLD,
			CPS_REG_READ(&(ctlregbase->LPDDR4__START__REG)));
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__START__REG), regval);

	if (pd->infohandler != NULL) {
		/* If a handler is registered, call it with the relevant information type */
		infotype = LPDDR4_DRV_SOC_PLL_UPDATE;
		pd->infohandler(pd, infotype);
	}

	result = lpddr4_pollandackirq(pd);

	return result;
}

/**
 * Internal Function: To add the offset to given address.
 * @param[in] addr Address to which the offset has to be added.
 * @param[in] regOffset The offset
 * @return regAddr The address value after the summation.
 */
static volatile uint32_t *lpddr4_addoffset(volatile uint32_t * addr,
					   uint32_t regoffset)
{

	volatile uint32_t *local_addr = addr;
	/* Declaring as array to add the offset value. */
	volatile uint32_t *regaddr = &local_addr[regoffset];
	return regaddr;
}

/**
 * Checks configuration object.
 * @param[in] config Driver/hardware configuration required.
 * @param[out] configSize Size of memory allocations required.
 * @return CDN_EOK on success (requirements structure filled).
 * @return ENOTSUP if configuration cannot be supported due to driver/hardware constraints.
 */
uint32_t lpddr4_probe(const lpddr4_config * config, uint16_t * configsize)
{
	uint32_t result;

	result = (uint32_t) (lpddr4_probesf(config, configsize));
	if (result == (uint32_t) CDN_EOK) {
		*configsize = (uint16_t) (sizeof(lpddr4_privatedata));
	}
	return result;
}

/**
 * Init function to be called after LPDDR4_probe() to set up the driver configuration.
 * Memory should be allocated for drv_data (using the size determined using LPDDR4_probe) before
 * calling  this API, init_settings should be initialized with base addresses for PHY Independent Module,
 * Controller and PHY before calling this function.
 * If callbacks are required for interrupt handling, these should also be configured in init_settings.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] cfg Specifies driver/hardware configuration.
 * @return CDN_EOK on success
 * @return EINVAL if illegal/inconsistent values in cfg.
 * @return ENOTSUP if hardware has an inconsistent configuration or doesn't support feature(s)
 * required by 'config' parameters.
 */
uint32_t lpddr4_init(lpddr4_privatedata * pd, const lpddr4_config * cfg)
{
	uint32_t result = 0U;
	uint16_t productid = 0U;

	result = lpddr4_initsf(pd, cfg);
	if (result == (uint32_t) CDN_EOK) {
		/* Validate Magic number */
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) cfg->ctlbase;
		productid = (uint16_t) (CPS_FLD_READ(LPDDR4__CONTROLLER_ID__FLD,
						     CPS_REG_READ(&
								  (ctlregbase->
								   LPDDR4__CONTROLLER_ID__REG))));
		if (productid == PRODUCT_ID) {
			/* Populating configuration data to pD */
			pd->ctlbase = ctlregbase;
			pd->infohandler =
			    (lpddr4_infocallback) cfg->infohandler;
			pd->ctlinterrupthandler =
			    (lpddr4_ctlcallback) cfg->ctlinterrupthandler;
			pd->phyindepinterrupthandler =
			    (lpddr4_phyindepcallback) cfg->
			    phyindepinterrupthandler;
		} else {
			/* Magic number validation failed - Driver doesn't support given IP version */
			result = (uint32_t) EOPNOTSUPP;
		}
	}
	return result;
}

/**
 * Start the driver.
 * @param[in] pD Driver state info specific to this instance.
 */
uint32_t lpddr4_start(const lpddr4_privatedata * pd)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	result = lpddr4_startsf(pd);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Enable PI as the initiator for DRAM */
		regval =
		    CPS_FLD_SET(LPDDR4__PI_INIT_LVL_EN__FLD,
				CPS_REG_READ(&
					     (ctlregbase->
					      LPDDR4__PI_INIT_LVL_EN__REG)));
		regval = CPS_FLD_SET(LPDDR4__PI_NORMAL_LVL_SEQ__FLD, regval);
		CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_INIT_LVL_EN__REG)),
			      regval);

		/* Start PI init sequence. */
		result = lpddr4_startsequencecontroller(pd);
	}
	return result;
}

/**
 * Read a register from the controller, PHY or PHY Independent Module
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] cpp Indicates whether controller, PHY or PHY Independent Module register
 * @param[in] regOffset Register offset
 * @param[out] regValue Register value read
 * @return CDN_EOK on success.
 * @return EINVAL if regOffset if out of range or regValue is NULL
 */
uint32_t lpddr4_readreg(const lpddr4_privatedata * pd, lpddr4_regblock cpp,
			uint32_t regoffset, uint32_t * regvalue)
{
	uint32_t result = 0U;

	result = lpddr4_readregsf(pd, cpp, regvalue);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		if (cpp == LPDDR4_CTL_REGS) {
			if (regoffset >= LPDDR4_CTL_REG_COUNT) {
				/* Return if user provider invalid register number */
				result = EINVAL;
			} else {
				*regvalue =
				    CPS_REG_READ(lpddr4_addoffset
						 (&(ctlregbase->DENALI_CTL_0),
						  regoffset));
			}
		} else if (cpp == LPDDR4_PHY_REGS) {
			if (regoffset >= LPDDR4_PHY_REG_COUNT) {
				/* Return if user provider invalid register number */
				result = EINVAL;
			} else {
				*regvalue =
				    CPS_REG_READ(lpddr4_addoffset
						 (&(ctlregbase->DENALI_PHY_0),
						  regoffset));
			}

		} else {
			if (regoffset >= LPDDR4_PHY_INDEP_REG_COUNT) {
				/* Return if user provider invalid register number */
				result = EINVAL;
			} else {
				*regvalue =
				    CPS_REG_READ(lpddr4_addoffset
						 (&(ctlregbase->DENALI_PI_0),
						  regoffset));
			}
		}
	}
	return result;
}

uint32_t lpddr4_writereg(const lpddr4_privatedata * pd, lpddr4_regblock cpp,
			 uint32_t regoffset, uint32_t regvalue)
{
	uint32_t result = 0U;

	result = lpddr4_writeregsf(pd, cpp);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		if (cpp == LPDDR4_CTL_REGS) {
			if (regoffset >= LPDDR4_CTL_REG_COUNT) {
				/* Return if user provider invalid register number */
				result = EINVAL;
			} else {
				CPS_REG_WRITE(lpddr4_addoffset
					      (&(ctlregbase->DENALI_CTL_0),
					       regoffset), regvalue);
			}
		} else if (cpp == LPDDR4_PHY_REGS) {
			if (regoffset >= LPDDR4_PHY_REG_COUNT) {
				/* Return if user provider invalid register number */
				result = EINVAL;
			} else {
				CPS_REG_WRITE(lpddr4_addoffset
					      (&(ctlregbase->DENALI_PHY_0),
					       regoffset), regvalue);
			}
		} else {
			if (regoffset >= LPDDR4_PHY_INDEP_REG_COUNT) {
				/* Return if user provider invalid register number */
				result = EINVAL;
			} else {
				CPS_REG_WRITE(lpddr4_addoffset
					      (&(ctlregbase->DENALI_PI_0),
					       regoffset), regvalue);
			}
		}
	}

	return result;
}

static uint32_t lpddr4_checkmmrreaderror(const lpddr4_privatedata * pd,
					 uint64_t * mmrvalue,
					 uint8_t * mrrstatus)
{

	uint64_t lowerdata;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
	uint32_t result = (uint32_t) CDN_EOK;

	/* Check if mode register read error interrupt occurred */
	if (lpddr4_pollctlirq(pd, LPDDR4_MRR_ERROR, 100) == 0U) {
		/* Mode register read error interrupt, read MRR status register and return. */
		*mrrstatus =
		    (uint8_t) CPS_FLD_READ(LPDDR4__MRR_ERROR_STATUS__FLD,
					   CPS_REG_READ(&
							(ctlregbase->
							 LPDDR4__MRR_ERROR_STATUS__REG)));
		*mmrvalue = 0;
		result = EIO;
	} else {
		*mrrstatus = 0;
		/* Mode register read was successful, read DATA */
		lowerdata =
		    CPS_REG_READ(&
				 (ctlregbase->
				  LPDDR4__PERIPHERAL_MRR_DATA_0__REG));
		*mmrvalue =
		    CPS_REG_READ(&
				 (ctlregbase->
				  LPDDR4__PERIPHERAL_MRR_DATA_1__REG));
		*mmrvalue = (uint64_t) ((*mmrvalue << WORD_SHIFT) | lowerdata);
		/* Acknowledge MR_READ_DONE interrupt to clear it */
		result = lpddr4_ackctlinterrupt(pd, LPDDR4_MR_READ_DONE);
	}
	return result;
}

uint32_t lpddr4_getmmrregister(const lpddr4_privatedata * pd,
			       uint32_t readmoderegval, uint64_t * mmrvalue,
			       uint8_t * mmrstatus)
{

	uint32_t result = 0U;
	uint32_t tdelay = 1000U;
	uint32_t regval = 0U;

	result = lpddr4_getmmrregistersf(pd, mmrvalue, mmrstatus);
	if (result == (uint32_t) CDN_EOK) {

		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Populate the calculated value to the register  */
		regval =
		    CPS_FLD_WRITE(LPDDR4__READ_MODEREG__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__READ_MODEREG__REG)),
				  readmoderegval);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__READ_MODEREG__REG), regval);

		/* Wait until the Read is done */
		result = lpddr4_pollctlirq(pd, LPDDR4_MR_READ_DONE, tdelay);
	}
	if (result == (uint32_t) CDN_EOK) {
		result = lpddr4_checkmmrreaderror(pd, mmrvalue, mmrstatus);
	}
	return result;
}

static uint32_t lpddr4_writemmrregister(const lpddr4_privatedata * pd,
					uint32_t writemoderegval)
{

	uint32_t result = (uint32_t) CDN_EOK;
	uint32_t tdelay = 1000U;
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

	/* Populate the calculated value to the register  */
	regval =
	    CPS_FLD_WRITE(LPDDR4__WRITE_MODEREG__FLD,
			  CPS_REG_READ(&
				       (ctlregbase->
					LPDDR4__WRITE_MODEREG__REG)),
			  writemoderegval);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__WRITE_MODEREG__REG), regval);

	result = lpddr4_pollctlirq(pd, LPDDR4_MR_WRITE_DONE, tdelay);

	return result;
}

uint32_t lpddr4_setmmrregister(const lpddr4_privatedata * pd,
			       uint32_t writemoderegval, uint8_t * mrwstatus)
{
	uint32_t result = 0U;

	result = lpddr4_setmmrregistersf(pd, mrwstatus);
	if (result == (uint32_t) CDN_EOK) {

		/* Function call to trigger Mode register write */
		result = lpddr4_writemmrregister(pd, writemoderegval);

		if (result == (uint32_t) CDN_EOK) {
			result =
			    lpddr4_ackctlinterrupt(pd, LPDDR4_MR_WRITE_DONE);
		}
		/* Read the status of mode register write */
		if (result == (uint32_t) CDN_EOK) {
			lpddr4_ctlregs *ctlregbase =
			    (lpddr4_ctlregs *) pd->ctlbase;
			*mrwstatus =
			    (uint8_t) CPS_FLD_READ(LPDDR4__MRW_STATUS__FLD,
						   CPS_REG_READ(&
								(ctlregbase->
								 LPDDR4__MRW_STATUS__REG)));
			if ((*mrwstatus) != 0U) {
				result = EIO;
			}
		}
	}

	return result;
}

uint32_t lpddr4_writectlconfig(const lpddr4_privatedata * pd,
			       const lpddr4_reginitdata * regvalues)
{
	uint32_t result;
	uint32_t regnum;

	result = lpddr4_writectlconfigsf(pd, regvalues);
	if (result == (uint32_t) CDN_EOK) {

		/* Iterate through CTL register numbers. */
		for (regnum = 0; regnum < LPDDR4_CTL_REG_COUNT; regnum++) {
			/* Check if the user has requested update */
			if (regvalues->updatectlreg[regnum]) {
				result =
				    lpddr4_writereg(pd, LPDDR4_CTL_REGS, regnum,
						    (uint32_t) (regvalues->
								denalictlreg
								[regnum]));
			}
		}
	}
	return result;
}

uint32_t lpddr4_writephyindepconfig(const lpddr4_privatedata * pd,
				    const lpddr4_reginitdata * regvalues)
{
	uint32_t result;
	uint32_t regnum;

	result = lpddr4_writephyindepconfigsf(pd, regvalues);
	if (result == (uint32_t) CDN_EOK) {

		/* Iterate through PHY Independent module register numbers. */
		for (regnum = 0; regnum < LPDDR4_PHY_INDEP_REG_COUNT; regnum++) {
			/* Check if the user has requested update */
			if (regvalues->updatephyindepreg[regnum]) {
				result =
				    lpddr4_writereg(pd, LPDDR4_PHY_INDEP_REGS,
						    regnum,
						    (uint32_t) (regvalues->
								denaliphyindepreg
								[regnum]));
			}
		}
	}
	return result;
}

uint32_t lpddr4_writephyconfig(const lpddr4_privatedata * pd,
			       const lpddr4_reginitdata * regvalues)
{
	uint32_t result;
	uint32_t regnum;

	result = lpddr4_writephyconfigsf(pd, regvalues);
	if (result == (uint32_t) CDN_EOK) {

		/* Iterate through PHY register numbers. */
		for (regnum = 0; regnum < LPDDR4_PHY_REG_COUNT; regnum++) {
			/* Check if the user has requested update */
			if (regvalues->updatephyreg[regnum]) {
				result =
				    lpddr4_writereg(pd, LPDDR4_PHY_REGS, regnum,
						    (uint32_t) (regvalues->
								denaliphyreg
								[regnum]));
			}
		}
	}
	return result;
}

uint32_t lpddr4_readctlconfig(const lpddr4_privatedata * pd,
			      lpddr4_reginitdata * regvalues)
{
	uint32_t result;
	uint32_t regnum;
	result = lpddr4_readctlconfigsf(pd, regvalues);
	if (result == (uint32_t) CDN_EOK) {
		/* Iterate through CTL register numbers. */
		for (regnum = 0; regnum < LPDDR4_CTL_REG_COUNT; regnum++) {
			/* Check if the user has requested read (updateCtlReg=1) */
			if (regvalues->updatectlreg[regnum]) {
				result =
				    lpddr4_readreg(pd, LPDDR4_CTL_REGS, regnum,
						   (uint32_t *) (&regvalues->
								 denalictlreg
								 [regnum]));
			}
		}
	}
	return result;
}

uint32_t lpddr4_readphyindepconfig(const lpddr4_privatedata * pd,
				   lpddr4_reginitdata * regvalues)
{
	uint32_t result;
	uint32_t regnum;

	result = lpddr4_readphyindepconfigsf(pd, regvalues);
	if (result == (uint32_t) CDN_EOK) {
		/* Iterate through PHY Independent module register numbers. */
		for (regnum = 0; regnum < LPDDR4_PHY_INDEP_REG_COUNT; regnum++) {
			/* Check if the user has requested read (updateCtlReg=1) */
			if (regvalues->updatephyindepreg[regnum]) {
				result =
				    lpddr4_readreg(pd, LPDDR4_PHY_INDEP_REGS,
						   regnum,
						   (uint32_t *) (&regvalues->
								 denaliphyindepreg
								 [regnum]));
			}
		}
	}
	return result;
}

uint32_t lpddr4_readphyconfig(const lpddr4_privatedata * pd,
			      lpddr4_reginitdata * regvalues)
{
	uint32_t result;
	uint32_t regnum;

	result = lpddr4_readphyconfigsf(pd, regvalues);
	if (result == (uint32_t) CDN_EOK) {
		/* Iterate through PHY register numbers. */
		for (regnum = 0; regnum < LPDDR4_PHY_REG_COUNT; regnum++) {
			/* Check if the user has requested read (updateCtlReg=1) */
			if (regvalues->updatephyreg[regnum]) {
				result =
				    lpddr4_readreg(pd, LPDDR4_PHY_REGS, regnum,
						   (uint32_t *) (&regvalues->
								 denaliphyreg
								 [regnum]));
			}
		}
	}
	return result;
}

uint32_t lpddr4_getctlinterruptmask(const lpddr4_privatedata * pd,
				    uint64_t * mask)
{
	uint32_t result = 0U;
	uint64_t lowermask = 0U;

	result = lpddr4_getctlinterruptmasksf(pd, mask);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		/* Reading the lower mask register */
		lowermask =
		    (uint64_t) (CPS_FLD_READ
				(LPDDR4__INT_MASK_0__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__INT_MASK_0__REG))));
		/* Reading the upper mask register */
		*mask =
		    (uint64_t) (CPS_FLD_READ
				(LPDDR4__INT_MASK_1__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__INT_MASK_1__REG))));
		/* Concatenate both register informations */
		*mask = (uint64_t) ((*mask << WORD_SHIFT) | lowermask);
	}
	return result;
}

uint32_t lpddr4_setctlinterruptmask(const lpddr4_privatedata * pd,
				    const uint64_t * mask)
{
	uint32_t result;
	uint32_t regval = 0;
	const uint64_t ui64one = 1ULL;
	const uint32_t ui32irqcount = (uint32_t) LPDDR4_LOR_BITS + 1U;

	result = lpddr4_setctlinterruptmasksf(pd, mask);
	if ((result == (uint32_t) CDN_EOK) && (ui32irqcount < 64U)) {
		/* Return if the user given value is higher than the field width */
		if (*mask >= (ui64one << ui32irqcount)) {
			result = EINVAL;
		}
	}
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Extracting the lower 32 bits and writing to lower mask register */
		regval = (uint32_t) (*mask & WORD_MASK);
		regval =
		    CPS_FLD_WRITE(LPDDR4__INT_MASK_0__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__INT_MASK_0__REG)),
				  regval);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_MASK_0__REG), regval);

		/* Extracting the upper 32 bits and writing to upper mask register */
		regval = (uint32_t) ((*mask >> WORD_SHIFT) & WORD_MASK);
		regval =
		    CPS_FLD_WRITE(LPDDR4__INT_MASK_1__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__INT_MASK_1__REG)),
				  regval);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_MASK_1__REG), regval);
	}
	return result;
}

uint32_t lpddr4_checkctlinterrupt(const lpddr4_privatedata * pd,
				  lpddr4_ctlinterrupt intr, bool * irqstatus)
{
	uint32_t result;
	uint32_t ctlirqstatus = 0;
	uint32_t fieldshift = 0;

	/* NOTE:This function assume irq status is mentioned in NOT more than 2 registers.
	 * Value of 'interrupt' should be less than 64 */
	result = lpddr4_checkctlinterruptsf(pd, intr, irqstatus);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		if ((uint32_t) intr >= WORD_SHIFT) {
			ctlirqstatus =
			    CPS_REG_READ(&
					 (ctlregbase->
					  LPDDR4__INT_STATUS_1__REG));
			/* Reduce the shift value as we are considering upper register */
			fieldshift = (uint32_t) intr - ((uint32_t) WORD_SHIFT);
		} else {
			ctlirqstatus =
			    CPS_REG_READ(&
					 (ctlregbase->
					  LPDDR4__INT_STATUS_0__REG));
			/* The shift value remains same for lower interrupt register */
			fieldshift = (uint32_t) intr;
		}

		/* MISRA compliance (Shifting operation) check */
		if (fieldshift < WORD_SHIFT) {
			if (((ctlirqstatus >> fieldshift) & BIT_MASK) > 0U) {
				*irqstatus = true;
			} else {
				*irqstatus = false;
			}
		}
	}
	return result;
}

uint32_t lpddr4_ackctlinterrupt(const lpddr4_privatedata * pd,
				lpddr4_ctlinterrupt intr)
{
	uint32_t result = 0;
	uint32_t regval = 0;
	uint32_t localinterrupt = (uint32_t) intr;

	/* NOTE:This function assume irq status is mentioned in NOT more than 2 registers.
	 * Value of 'interrupt' should be less than 64 */
	result = lpddr4_ackctlinterruptsf(pd, intr);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Check if the requested bit is in upper register */
		if (localinterrupt > WORD_SHIFT) {
			localinterrupt =
			    (localinterrupt - (uint32_t) WORD_SHIFT);
			regval = ((uint32_t) BIT_MASK << localinterrupt);
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_1__REG),
				      regval);
		} else {
			regval = ((uint32_t) BIT_MASK << localinterrupt);
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_0__REG),
				      regval);
		}
	}

	return result;
}

uint32_t lpddr4_getphyindepinterruptmask(const lpddr4_privatedata * pd,
					 uint32_t * mask)
{
	uint32_t result;

	result = lpddr4_getphyindepinterruptmsf(pd, mask);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		/* Reading mask register */
		*mask =
		    CPS_FLD_READ(LPDDR4__PI_INT_MASK__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__PI_INT_MASK__REG)));
	}
	return result;
}

uint32_t lpddr4_setphyindepinterruptmask(const lpddr4_privatedata * pd,
					 const uint32_t * mask)
{
	uint32_t result;
	uint32_t regval = 0;
	const uint32_t ui32irqcount =
	    (uint32_t) LPDDR4_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT + 1U;

	result = lpddr4_setphyindepinterruptmsf(pd, mask);
	if ((result == (uint32_t) CDN_EOK) && (ui32irqcount < WORD_SHIFT)) {
		/* Return if the user given value is higher than the field width */
		if (*mask >= (1U << ui32irqcount)) {
			result = EINVAL;
		}
	}
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Writing to the user requested interrupt mask */
		regval =
		    CPS_FLD_WRITE(LPDDR4__PI_INT_MASK__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__PI_INT_MASK__REG)),
				  *mask);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PI_INT_MASK__REG), regval);
	}
	return result;
}

uint32_t lpddr4_checkphyindepinterrupt(const lpddr4_privatedata * pd,
				       lpddr4_phyindepinterrupt intr,
				       bool * irqstatus)
{
	uint32_t result = 0;
	uint32_t phyindepirqstatus = 0;

	result = lpddr4_checkphyindepinterrupsf(pd, intr, irqstatus);
	/* Confirming that the value of interrupt is less than register width */
	if ((result == (uint32_t) CDN_EOK) && ((uint32_t) intr < WORD_SHIFT)) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Reading the requested bit to check interrupt status */
		phyindepirqstatus =
		    CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INT_STATUS__REG));
		*irqstatus =
		    (((phyindepirqstatus >> (uint32_t) intr) & BIT_MASK) > 0U);
	}
	return result;
}

uint32_t lpddr4_ackphyindepinterrupt(const lpddr4_privatedata * pd,
				     lpddr4_phyindepinterrupt intr)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;
	uint32_t ui32shiftinterrupt = (uint32_t) intr;

	result = lpddr4_ackphyindepinterruptsf(pd, intr);
	/* Confirming that the value of interrupt is less than register width */
	if ((result == (uint32_t) CDN_EOK) && (ui32shiftinterrupt < WORD_SHIFT)) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Write 1 to the requested bit to ACk the interrupt */
		regval = ((uint32_t) BIT_MASK << ui32shiftinterrupt);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PI_INT_ACK__REG), regval);
	}

	return result;
}

/* Check for caTrainingError */
static void lpddr4_checkcatrainingerror(lpddr4_ctlregs * ctlregbase,
					lpddr4_debuginfo * debuginfo,
					bool * errfoundptr)
{

	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress =
	    (volatile uint32_t
	     *)(&(ctlregbase->LPDDR4__PHY_ADR_CALVL_OBS1_0__REG));
	errbitmask = (CA_TRAIN_RL) | (NIBBLE_MASK);
	/* PHY_ADR_CALVL_OBS1[4] – Right found
	   PHY_ADR_CALVL_OBS1[5] – left found
	   both the above fields should be high and below field should be zero.
	   PHY_ADR_CALVL_OBS1[3:0] – calvl_state
	 */
	for (snum = 0U; snum < ASLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != CA_TRAIN_RL) {
			debuginfo->catraingerror = true;
			*errfoundptr = true;
		}
		regaddress =
		    lpddr4_addoffset(regaddress, (uint32_t) SLICE_WIDTH);
	}
}

/* Check for  wrLvlError */
static void lpddr4_checkwrlvlerror(lpddr4_ctlregs * ctlregbase,
				   lpddr4_debuginfo * debuginfo,
				   bool * errfoundptr)
{

	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress =
	    (volatile uint32_t
	     *)(&(ctlregbase->LPDDR4__PHY_WRLVL_ERROR_OBS_0__REG));
	/* PHY_WRLVL_ERROR_OBS_X[1:0] should be zero */
	errbitmask = (BIT_MASK << 1) | (BIT_MASK);
	for (snum = 0U; snum < DSLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != 0U) {
			debuginfo->wrlvlerror = true;
			*errfoundptr = true;
		}
		regaddress =
		    lpddr4_addoffset(regaddress, (uint32_t) SLICE_WIDTH);
	}
}

/* Check for  GateLvlError */
static void lpddr4_checkgatelvlerror(lpddr4_ctlregs * ctlregbase,
				     lpddr4_debuginfo * debuginfo,
				     bool * errfoundptr)
{

	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress =
	    (volatile uint32_t
	     *)(&(ctlregbase->LPDDR4__PHY_GTLVL_STATUS_OBS_0__REG));
	/* PHY_GTLVL_STATUS_OBS[6] – gate_level min error
	 * PHY_GTLVL_STATUS_OBS[7] – gate_level max error
	 * All the above bit fields should be zero */
	errbitmask = GATE_LVL_ERROR_FIELDS;
	for (snum = 0U; snum < DSLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != 0U) {
			debuginfo->gatelvlerror = true;
			*errfoundptr = true;
		}
		regaddress =
		    lpddr4_addoffset(regaddress, (uint32_t) SLICE_WIDTH);
	}
}

/* Check for  ReadLvlError */
static void lpddr4_checkreadlvlerror(lpddr4_ctlregs * ctlregbase,
				     lpddr4_debuginfo * debuginfo,
				     bool * errfoundptr)
{

	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress =
	    (volatile uint32_t
	     *)(&(ctlregbase->LPDDR4__PHY_RDLVL_STATUS_OBS_0__REG));
	/* PHY_RDLVL_STATUS_OBS[23:16] – failed bits : should be zero.
	   PHY_RDLVL_STATUS_OBS[31:28] – rdlvl_state : should be zero */
	errbitmask = READ_LVL_ERROR_FIELDS;
	for (snum = 0U; snum < DSLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != 0U) {
			debuginfo->readlvlerror = true;
			*errfoundptr = true;
		}
		regaddress =
		    lpddr4_addoffset(regaddress, (uint32_t) SLICE_WIDTH);
	}
}

/* Check for  DqTrainingError */
static void lpddr4_checkdqtrainingerror(lpddr4_ctlregs * ctlregbase,
					lpddr4_debuginfo * debuginfo,
					bool * errfoundptr)
{

	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress =
	    (volatile uint32_t
	     *)(&(ctlregbase->LPDDR4__PHY_WDQLVL_STATUS_OBS_0__REG));
	/* PHY_WDQLVL_STATUS_OBS[26:18] should all be zero. */
	errbitmask = DQ_LVL_STATUS;
	for (snum = 0U; snum < DSLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != 0U) {
			debuginfo->dqtrainingerror = true;
			*errfoundptr = true;
		}
		regaddress =
		    lpddr4_addoffset(regaddress, (uint32_t) SLICE_WIDTH);
	}
}

/**
 * Internal Function:For checking errors in training/levelling sequence.
 * @param[in] pD Driver state info specific to this instance.
 * @param[in] debugInfo pointer to debug information.
 * @param[out] errFoundPtr pointer to return if error found.
 * @return CDN_EOK on success (Interrupt status high).
 * @return EINVAL checking or unmasking was not successful.
 */
static bool lpddr4_checklvlerrors(const lpddr4_privatedata * pd,
				  lpddr4_debuginfo * debuginfo, bool errfound)
{

	bool localerrfound = errfound;

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

	if (localerrfound == false) {
		/* Check for ca training error */
		lpddr4_checkcatrainingerror(ctlregbase, debuginfo,
					    &localerrfound);
	}

	if (localerrfound == false) {
		/* Check for Write leveling error */
		lpddr4_checkwrlvlerror(ctlregbase, debuginfo, &localerrfound);
	}

	if (localerrfound == false) {
		/* Check for Gate leveling error */
		lpddr4_checkgatelvlerror(ctlregbase, debuginfo, &localerrfound);
	}

	if (localerrfound == false) {
		/* Check for Read leveling error */
		lpddr4_checkreadlvlerror(ctlregbase, debuginfo, &localerrfound);
	}

	if (localerrfound == false) {
		/* Check for DQ training error */
		lpddr4_checkdqtrainingerror(ctlregbase, debuginfo,
					    &localerrfound);
	}
	return localerrfound;
}

static bool lpddr4_seterror(volatile uint32_t * reg, uint32_t errbitmask,
			    bool * errfoundptr, const uint32_t errorinfobits)
{

	uint32_t regval = 0U;

	/* Read the respective observation register */
	regval = CPS_REG_READ(reg);
	/* Compare the error bit values */
	if ((regval & errbitmask) != errorinfobits) {
		*errfoundptr = true;
	}
	return *errfoundptr;
}

static void lpddr4_seterrors(lpddr4_ctlregs * ctlregbase,
			     lpddr4_debuginfo * debuginfo, bool * errfoundptr)
{

	uint32_t errbitmask = (BIT_MASK << 0x1U) | (BIT_MASK);
	/* Check PLL observation registers for PLL lock errors */

	debuginfo->pllerror =
	    lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_PLL_OBS_0__REG),
			    errbitmask, errfoundptr, PLL_READY);
	if (*errfoundptr == false) {
		debuginfo->pllerror =
		    lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_PLL_OBS_1__REG),
				    errbitmask, errfoundptr, PLL_READY);
	}

	/* Check for IO Calibration errors */
	if (*errfoundptr == false) {
		debuginfo->iocaliberror =
		    lpddr4_seterror(&
				    (ctlregbase->
				     LPDDR4__PHY_CAL_RESULT_OBS_0__REG),
				    IO_CALIB_DONE, errfoundptr, IO_CALIB_DONE);
	}
	if (*errfoundptr == false) {
		debuginfo->iocaliberror =
		    lpddr4_seterror(&
				    (ctlregbase->
				     LPDDR4__PHY_CAL_RESULT2_OBS_0__REG),
				    IO_CALIB_DONE, errfoundptr, IO_CALIB_DONE);
	}
	if (*errfoundptr == false) {
		debuginfo->iocaliberror =
		    lpddr4_seterror(&
				    (ctlregbase->
				     LPDDR4__PHY_CAL_RESULT3_OBS_0__REG),
				    IO_CALIB_FIELD, errfoundptr,
				    IO_CALIB_STATE);
	}
}

static void lpddr4_setphysnapsettings(lpddr4_ctlregs * ctlregbase,
				      const bool errorfound)
{

	uint32_t snum = 0U;
	volatile uint32_t *regaddress;
	uint32_t regval = 0U;

	/* Setting SC_PHY_SNAP_OBS_REGS_x to get a snapshot */
	if (errorfound == false) {
		regaddress =
		    (volatile uint32_t
		     *)(&(ctlregbase->LPDDR4__SC_PHY_SNAP_OBS_REGS_0__REG));
		/* Iterate through each PHY Data Slice */
		for (snum = 0U; snum < DSLICE_NUM; snum++) {
			regval =
			    CPS_FLD_SET(LPDDR4__SC_PHY_SNAP_OBS_REGS_0__FLD,
					CPS_REG_READ(regaddress));
			CPS_REG_WRITE(regaddress, regval);
			regaddress =
			    lpddr4_addoffset(regaddress,
					     (uint32_t) SLICE_WIDTH);
		}
	}
}

static void lpddr4_setphyadrsnapsettings(lpddr4_ctlregs * ctlregbase,
					 const bool errorfound)
{

	uint32_t snum = 0U;
	volatile uint32_t *regaddress;
	uint32_t regval = 0U;

	/* Setting SC_PHY ADR_SNAP_OBS_REGS_x to get a snapshot */
	if (errorfound == false) {
		regaddress =
		    (volatile uint32_t
		     *)(&(ctlregbase->LPDDR4__SC_PHY_ADR_SNAP_OBS_REGS_0__REG));
		/* Iterate through each PHY Address Slice */
		for (snum = 0U; snum < ASLICE_NUM; snum++) {
			regval =
			    CPS_FLD_SET(LPDDR4__SC_PHY_ADR_SNAP_OBS_REGS_0__FLD,
					CPS_REG_READ(regaddress));
			CPS_REG_WRITE(regaddress, regval);
			regaddress =
			    lpddr4_addoffset(regaddress,
					     (uint32_t) SLICE_WIDTH);
		}
	}
}

static void lpddr4_setsettings(lpddr4_ctlregs * ctlregbase,
			       const bool errorfound)
{

	/* Calling functions to enable snap shots of OBS registers */
	lpddr4_setphysnapsettings(ctlregbase, errorfound);
	lpddr4_setphyadrsnapsettings(ctlregbase, errorfound);
}

static void lpddr4_setrxoffseterror(lpddr4_ctlregs * ctlregbase,
				    lpddr4_debuginfo * debuginfo,
				    bool * errorfound)
{

	volatile uint32_t *regaddress;
	uint32_t snum = 0U;
	uint32_t errbitmask = 0U;
	uint32_t regval = 0U;

	/* Check for rxOffsetError */
	if (*errorfound == false) {
		regaddress =
		    (volatile uint32_t
		     *)(&(ctlregbase->LPDDR4__PHY_RX_CAL_LOCK_OBS_0__REG));
		errbitmask = (RX_CAL_DONE) | (NIBBLE_MASK);
		/* PHY_RX_CAL_LOCK_OBS_x[4] – RX_CAL_DONE : should be high
		   phy_rx_cal_lock_obs_x[3:0] – RX_CAL_STATE : should be zero. */
		for (snum = 0U; snum < DSLICE_NUM; snum++) {
			regval =
			    CPS_FLD_READ(LPDDR4__PHY_RX_CAL_LOCK_OBS_0__FLD,
					 CPS_REG_READ(regaddress));
			if ((regval & errbitmask) != RX_CAL_DONE) {
				debuginfo->rxoffseterror = true;
				*errorfound = true;
			}
			regaddress =
			    lpddr4_addoffset(regaddress,
					     (uint32_t) SLICE_WIDTH);
		}
	}
}

uint32_t lpddr4_getdebuginitinfo(const lpddr4_privatedata * pd,
				 lpddr4_debuginfo * debuginfo)
{

	uint32_t result = 0U;
	bool errorfound = false;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_getdebuginitinfosf(pd, debuginfo);
	if (result == (uint32_t) CDN_EOK) {

		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		lpddr4_seterrors(ctlregbase, debuginfo, &errorfound);
		/* Function to setup Snap for OBS registers */
		lpddr4_setsettings(ctlregbase, errorfound);
		/* Function to check for Rx offset error */
		lpddr4_setrxoffseterror(ctlregbase, debuginfo, &errorfound);
		/* Function Check various levelling errors */
		errorfound = lpddr4_checklvlerrors(pd, debuginfo, errorfound);
	}

	if (errorfound == true) {
		result = (uint32_t) EPROTO;
	}

	return result;
}

static void readpdwakeup(const lpddr4_ctlfspnum * fspnum,
			 lpddr4_ctlregs * ctlregbase, uint32_t * cycles)
{

	/* Read the appropriate register, based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_PD_WAKEUP_F0__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_PD_WAKEUP_F0__REG)));
	} else if (*fspnum == LPDDR4_FSP_1) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_PD_WAKEUP_F1__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_PD_WAKEUP_F1__REG)));
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_PD_WAKEUP_F2__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_PD_WAKEUP_F2__REG)));
	}
}

static void readsrshortwakeup(const lpddr4_ctlfspnum * fspnum,
			      lpddr4_ctlregs * ctlregbase, uint32_t * cycles)
{

	/* Read the appropriate register, based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SR_SHORT_WAKEUP_F0__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SR_SHORT_WAKEUP_F0__REG)));
	} else if (*fspnum == LPDDR4_FSP_1) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SR_SHORT_WAKEUP_F1__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SR_SHORT_WAKEUP_F1__REG)));
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SR_SHORT_WAKEUP_F2__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SR_SHORT_WAKEUP_F2__REG)));
	}
}

static void readsrlongwakeup(const lpddr4_ctlfspnum * fspnum,
			     lpddr4_ctlregs * ctlregbase, uint32_t * cycles)
{

	/* Read the appropriate register, based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SR_LONG_WAKEUP_F0__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SR_LONG_WAKEUP_F0__REG)));
	} else if (*fspnum == LPDDR4_FSP_1) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SR_LONG_WAKEUP_F1__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SR_LONG_WAKEUP_F1__REG)));
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SR_LONG_WAKEUP_F2__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SR_LONG_WAKEUP_F2__REG)));
	}
}

static void readsrlonggatewakeup(const lpddr4_ctlfspnum * fspnum,
				 lpddr4_ctlregs * ctlregbase, uint32_t * cycles)
{

	/* Read the appropriate register, based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__REG)));
	} else if (*fspnum == LPDDR4_FSP_1) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__REG)));
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__REG)));
	}
}

static void readsrdpshortwakeup(const lpddr4_ctlfspnum * fspnum,
				lpddr4_ctlregs * ctlregbase, uint32_t * cycles)
{

	/* Read the appropriate register, based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__REG)));
	} else if (*fspnum == LPDDR4_FSP_1) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__REG)));
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__REG)));
	}
}

static void readsrdplongwakeup(const lpddr4_ctlfspnum * fspnum,
			       lpddr4_ctlregs * ctlregbase, uint32_t * cycles)
{

	/* Read the appropriate register, based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__REG)));
	} else if (*fspnum == LPDDR4_FSP_1) {
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__REG)));
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		*cycles =
		    CPS_FLD_READ(LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__REG)));
	}
}

static void readsrdplonggatewakeup(const lpddr4_ctlfspnum * fspnum,
				   lpddr4_ctlregs * ctlregbase,
				   uint32_t * cycles)
{

	/* Read the appropriate register, based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		*cycles =
		    CPS_FLD_READ
		    (LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__FLD,
		     CPS_REG_READ(&
				  (ctlregbase->
				   LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__REG)));
	} else if (*fspnum == LPDDR4_FSP_1) {
		*cycles =
		    CPS_FLD_READ
		    (LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__FLD,
		     CPS_REG_READ(&
				  (ctlregbase->
				   LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__REG)));
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		*cycles =
		    CPS_FLD_READ
		    (LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__FLD,
		     CPS_REG_READ(&
				  (ctlregbase->
				   LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__REG)));
	}

}

static void lpddr4_readlpiwakeuptime(lpddr4_ctlregs * ctlregbase,
				     const lpddr4_lpiwakeupparam *
				     lpiwakeupparam,
				     const lpddr4_ctlfspnum * fspnum,
				     uint32_t * cycles)
{

	/* Iterate through each of the Wake up parameter type */
	if (*lpiwakeupparam == LPDDR4_LPI_PD_WAKEUP_FN) {
		/* Calling appropriate function for register read */
		readpdwakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SR_SHORT_WAKEUP_FN) {
		readsrshortwakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SR_LONG_WAKEUP_FN) {
		readsrlongwakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SR_LONG_MCCLK_GATE_WAKEUP_FN) {
		readsrlonggatewakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SRPD_SHORT_WAKEUP_FN) {
		readsrdpshortwakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SRPD_LONG_WAKEUP_FN) {
		readsrdplongwakeup(fspnum, ctlregbase, cycles);
	} else {
		/* Default function (sanity function already confirmed the variable value) */
		readsrdplonggatewakeup(fspnum, ctlregbase, cycles);
	}
}

uint32_t lpddr4_getlpiwakeuptime(const lpddr4_privatedata * pd,
				 const lpddr4_lpiwakeupparam * lpiwakeupparam,
				 const lpddr4_ctlfspnum * fspnum,
				 uint32_t * cycles)
{

	uint32_t result = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_getlpiwakeuptimesf(pd, lpiwakeupparam, fspnum, cycles);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		lpddr4_readlpiwakeuptime(ctlregbase, lpiwakeupparam, fspnum,
					 cycles);
	}
	return result;
}

static void writepdwakeup(const lpddr4_ctlfspnum * fspnum,
			  lpddr4_ctlregs * ctlregbase, const uint32_t * cycles)
{

	uint32_t regval = 0U;
	/* Write to appropriate register ,based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_PD_WAKEUP_F0__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_PD_WAKEUP_F0__REG)),
				  *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F0__REG),
			      regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_PD_WAKEUP_F1__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_PD_WAKEUP_F1__REG)),
				  *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F1__REG),
			      regval);
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_PD_WAKEUP_F2__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_PD_WAKEUP_F2__REG)),
				  *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F2__REG),
			      regval);
	}
}

static void writesrshortwakeup(const lpddr4_ctlfspnum * fspnum,
			       lpddr4_ctlregs * ctlregbase,
			       const uint32_t * cycles)
{

	uint32_t regval = 0U;
	/* Write to appropriate register ,based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SR_SHORT_WAKEUP_F0__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SR_SHORT_WAKEUP_F0__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F0__REG),
			      regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SR_SHORT_WAKEUP_F1__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SR_SHORT_WAKEUP_F1__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F1__REG),
			      regval);
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SR_SHORT_WAKEUP_F2__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SR_SHORT_WAKEUP_F2__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F2__REG),
			      regval);
	}
}

static void writesrlongwakeup(const lpddr4_ctlfspnum * fspnum,
			      lpddr4_ctlregs * ctlregbase,
			      const uint32_t * cycles)
{

	uint32_t regval = 0U;
	/* Write to appropriate register ,based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_WAKEUP_F0__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SR_LONG_WAKEUP_F0__REG)),
				  *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F0__REG),
			      regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_WAKEUP_F1__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SR_LONG_WAKEUP_F1__REG)),
				  *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F1__REG),
			      regval);
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_WAKEUP_F2__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SR_LONG_WAKEUP_F2__REG)),
				  *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F2__REG),
			      regval);
	}
}

static void writesrlonggatewakeup(const lpddr4_ctlfspnum * fspnum,
				  lpddr4_ctlregs * ctlregbase,
				  const uint32_t * cycles)
{

	uint32_t regval = 0U;
	/* Write to appropriate register ,based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__REG),
			      regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__REG),
			      regval);
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__REG),
			      regval);
	}
}

static void writesrdpshortwakeup(const lpddr4_ctlfspnum * fspnum,
				 lpddr4_ctlregs * ctlregbase,
				 const uint32_t * cycles)
{

	uint32_t regval = 0U;
	/* Write to appropriate register ,based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__REG), regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__REG), regval);
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__REG), regval);
	}
}

static void writesrdplongwakeup(const lpddr4_ctlfspnum * fspnum,
				lpddr4_ctlregs * ctlregbase,
				const uint32_t * cycles)
{

	uint32_t regval = 0U;
	/* Write to appropriate register ,based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__REG), regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__REG), regval);
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		regval =
		    CPS_FLD_WRITE(LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__REG)),
				  *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__REG), regval);
	}
}

static void writesrdplonggatewakeup(const lpddr4_ctlfspnum * fspnum,
				    lpddr4_ctlregs * ctlregbase,
				    const uint32_t * cycles)
{

	uint32_t regval = 0U;
	/* Write to appropriate register ,based on user given frequency. */
	if (*fspnum == LPDDR4_FSP_0) {
		regval =
		    CPS_FLD_WRITE
		    (LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__FLD,
		     CPS_REG_READ(&
				  (ctlregbase->
				   LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__REG)),
		     *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__REG),
			      regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval =
		    CPS_FLD_WRITE
		    (LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__FLD,
		     CPS_REG_READ(&
				  (ctlregbase->
				   LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__REG)),
		     *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__REG),
			      regval);
	} else {
		/* Default register (sanity function already confirmed the variable value) */
		regval =
		    CPS_FLD_WRITE
		    (LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__FLD,
		     CPS_REG_READ(&
				  (ctlregbase->
				   LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__REG)),
		     *cycles);
		CPS_REG_WRITE(&
			      (ctlregbase->
			       LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__REG),
			      regval);
	}
}

static void lpddr4_writelpiwakeuptime(lpddr4_ctlregs * ctlregbase,
				      const lpddr4_lpiwakeupparam *
				      lpiwakeupparam,
				      const lpddr4_ctlfspnum * fspnum,
				      const uint32_t * cycles)
{

	/* Iterate through each of the Wake up parameter type */
	if (*lpiwakeupparam == LPDDR4_LPI_PD_WAKEUP_FN) {
		/* Calling appropriate function for register write */
		writepdwakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SR_SHORT_WAKEUP_FN) {
		writesrshortwakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SR_LONG_WAKEUP_FN) {
		writesrlongwakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SR_LONG_MCCLK_GATE_WAKEUP_FN) {
		writesrlonggatewakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SRPD_SHORT_WAKEUP_FN) {
		writesrdpshortwakeup(fspnum, ctlregbase, cycles);
	} else if (*lpiwakeupparam == LPDDR4_LPI_SRPD_LONG_WAKEUP_FN) {
		writesrdplongwakeup(fspnum, ctlregbase, cycles);
	} else {
		/* Default function (sanity function already confirmed the variable value) */
		writesrdplonggatewakeup(fspnum, ctlregbase, cycles);
	}
}

uint32_t lpddr4_setlpiwakeuptime(const lpddr4_privatedata * pd,
				 const lpddr4_lpiwakeupparam * lpiwakeupparam,
				 const lpddr4_ctlfspnum * fspnum,
				 const uint32_t * cycles)
{
	uint32_t result = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_setlpiwakeuptimesf(pd, lpiwakeupparam, fspnum, cycles);
	if (result == (uint32_t) CDN_EOK) {
		/* Return if the user given value is higher than the field width */
		if (*cycles > NIBBLE_MASK) {
			result = EINVAL;
		}
	}
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		lpddr4_writelpiwakeuptime(ctlregbase, lpiwakeupparam, fspnum,
					  cycles);
	}
	return result;
}

uint32_t lpddr4_geteccenable(const lpddr4_privatedata * pd,
			     lpddr4_eccenable * eccparam)
{
	uint32_t result = 0U;
	uint32_t fldval = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_geteccenablesf(pd, eccparam);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Reading the ECC_Enable field  from the register. */
		fldval =
		    CPS_FLD_READ(LPDDR4__ECC_ENABLE__FLD,
				 CPS_REG_READ(&
					      (ctlregbase->
					       LPDDR4__ECC_ENABLE__REG)));
		switch (fldval) {
		case 3:
			*eccparam = LPDDR4_ECC_ERR_DETECT_CORRECT;
			break;
		case 2:
			*eccparam = LPDDR4_ECC_ERR_DETECT;
			break;
		case 1:
			*eccparam = LPDDR4_ECC_ENABLED;
			break;
		default:
			/* Default ECC (Sanity function already confirmed the value to be in expected range.) */
			*eccparam = LPDDR4_ECC_DISABLED;
			break;
		}
	}
	return result;
}

uint32_t lpddr4_seteccenable(const lpddr4_privatedata * pd,
			     const lpddr4_eccenable * eccparam)
{

	uint32_t result = 0U;
	uint32_t regval = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_seteccenablesf(pd, eccparam);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Updating the ECC_Enable field based on the user given value. */
		regval =
		    CPS_FLD_WRITE(LPDDR4__ECC_ENABLE__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__ECC_ENABLE__REG)),
				  *eccparam);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__ECC_ENABLE__REG), regval);
	}
	return result;
}

uint32_t lpddr4_getreducmode(const lpddr4_privatedata * pd,
			     lpddr4_reducmode * mode)
{
	uint32_t result = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_getreducmodesf(pd, mode);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		/* Read the value of reduc parameter. */
		if (CPS_FLD_READ
		    (LPDDR4__REDUC__FLD,
		     CPS_REG_READ(&(ctlregbase->LPDDR4__REDUC__REG))) == 0U) {
			*mode = LPDDR4_REDUC_ON;
		} else {
			*mode = LPDDR4_REDUC_OFF;
		}
	}
	return result;
}

uint32_t lpddr4_setreducmode(const lpddr4_privatedata * pd,
			     const lpddr4_reducmode * mode)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_setreducmodesf(pd, mode);
	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		/* Setting to enable Half data path. */
		regval =
		    CPS_FLD_WRITE(LPDDR4__REDUC__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__REDUC__REG)), *mode);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__REDUC__REG), regval);
	}
	return result;
}

uint32_t lpddr4_getdbireadmode(const lpddr4_privatedata * pd, bool * on_off)
{

	uint32_t result = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_getdbireadmodesf(pd, on_off);

	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		/* Reading the field value from the register. */
		if (CPS_FLD_READ
		    (LPDDR4__RD_DBI_EN__FLD,
		     CPS_REG_READ(&(ctlregbase->LPDDR4__RD_DBI_EN__REG))) ==
		    0U) {
			*on_off = false;
		} else {
			*on_off = true;
		}
	}
	return result;
}

uint32_t lpddr4_getdbiwritemode(const lpddr4_privatedata * pd, bool * on_off)
{

	uint32_t result = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_getdbireadmodesf(pd, on_off);

	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		/* Reading the field value from the register. */
		if (CPS_FLD_READ
		    (LPDDR4__WR_DBI_EN__FLD,
		     CPS_REG_READ(&(ctlregbase->LPDDR4__WR_DBI_EN__REG))) ==
		    0U) {
			*on_off = false;
		} else {
			*on_off = true;
		}
	}
	return result;
}

uint32_t lpddr4_setdbimode(const lpddr4_privatedata * pd,
			   const lpddr4_dbimode * mode)
{

	uint32_t result = 0U;
	uint32_t regval = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_setdbimodesf(pd, mode);

	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Updating the appropriate field value based on the user given mode */
		if (*mode == LPDDR4_DBI_RD_ON) {
			regval =
			    CPS_FLD_WRITE(LPDDR4__RD_DBI_EN__FLD,
					  CPS_REG_READ(&
						       (ctlregbase->
							LPDDR4__RD_DBI_EN__REG)),
					  1U);
		} else if (*mode == LPDDR4_DBI_RD_OFF) {
			regval =
			    CPS_FLD_WRITE(LPDDR4__RD_DBI_EN__FLD,
					  CPS_REG_READ(&
						       (ctlregbase->
							LPDDR4__RD_DBI_EN__REG)),
					  0U);
		} else if (*mode == LPDDR4_DBI_WR_ON) {
			regval =
			    CPS_FLD_WRITE(LPDDR4__WR_DBI_EN__FLD,
					  CPS_REG_READ(&
						       (ctlregbase->
							LPDDR4__WR_DBI_EN__REG)),
					  1U);
		} else {
			/* Default field (Sanity function already confirmed the value to be in expected range.) */
			regval =
			    CPS_FLD_WRITE(LPDDR4__WR_DBI_EN__FLD,
					  CPS_REG_READ(&
						       (ctlregbase->
							LPDDR4__WR_DBI_EN__REG)),
					  0U);
		}
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__RD_DBI_EN__REG), regval);
	}
	return result;
}

uint32_t lpddr4_getrefreshrate(const lpddr4_privatedata * pd,
			       const lpddr4_ctlfspnum * fspnum,
			       uint32_t * cycles)
{
	uint32_t result = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_getrefreshratesf(pd, fspnum, cycles);

	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Selecting the appropriate register for the user requested Frequency */
		switch (*fspnum) {
		case LPDDR4_FSP_2:
			*cycles =
			    CPS_FLD_READ(LPDDR4__TREF_F2__FLD,
					 CPS_REG_READ(&
						      (ctlregbase->
						       LPDDR4__TREF_F2__REG)));
			break;
		case LPDDR4_FSP_1:
			*cycles =
			    CPS_FLD_READ(LPDDR4__TREF_F1__FLD,
					 CPS_REG_READ(&
						      (ctlregbase->
						       LPDDR4__TREF_F1__REG)));
			break;
		default:
			/* FSP_0 is considered as the default (sanity check already confirmed it as valid FSP) */
			*cycles =
			    CPS_FLD_READ(LPDDR4__TREF_F0__FLD,
					 CPS_REG_READ(&
						      (ctlregbase->
						       LPDDR4__TREF_F0__REG)));
			break;
		}
	}
	return result;
}

uint32_t lpddr4_setrefreshrate(const lpddr4_privatedata * pd,
			       const lpddr4_ctlfspnum * fspnum,
			       const uint32_t * cycles)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_setrefreshratesf(pd, fspnum, cycles);

	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;

		/* Selecting the appropriate register for the user requested Frequency */
		switch (*fspnum) {
		case LPDDR4_FSP_2:
			regval =
			    CPS_FLD_WRITE(LPDDR4__TREF_F2__FLD,
					  CPS_REG_READ(&
						       (ctlregbase->
							LPDDR4__TREF_F2__REG)),
					  *cycles);
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__TREF_F2__REG),
				      regval);
			break;
		case LPDDR4_FSP_1:
			regval =
			    CPS_FLD_WRITE(LPDDR4__TREF_F1__FLD,
					  CPS_REG_READ(&
						       (ctlregbase->
							LPDDR4__TREF_F1__REG)),
					  *cycles);
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__TREF_F1__REG),
				      regval);
			break;
		default:
			/* FSP_0 is considered as the default (sanity check already confirmed it as valid FSP) */
			regval =
			    CPS_FLD_WRITE(LPDDR4__TREF_F0__FLD,
					  CPS_REG_READ(&
						       (ctlregbase->
							LPDDR4__TREF_F0__REG)),
					  *cycles);
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__TREF_F0__REG),
				      regval);
			break;
		}
	}
	return result;
}

uint32_t lpddr4_refreshperchipselect(const lpddr4_privatedata * pd,
				     const uint32_t trefinterval)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	/* Calling Sanity Function to verify the input variables */
	result = lpddr4_refreshperchipselectsf(pd);

	if (result == (uint32_t) CDN_EOK) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *) pd->ctlbase;
		/* Setting tref_interval parameter to enable/disable Refresh per chip select. */
		regval =
		    CPS_FLD_WRITE(LPDDR4__TREF_INTERVAL__FLD,
				  CPS_REG_READ(&
					       (ctlregbase->
						LPDDR4__TREF_INTERVAL__REG)),
				  trefinterval);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__TREF_INTERVAL__REG),
			      regval);
	}
	return result;
}
