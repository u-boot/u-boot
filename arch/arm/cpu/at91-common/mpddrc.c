/*
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/atmel_mpddrc.h>

static inline void atmel_mpddr_op(int mode, u32 ram_address)
{
	struct atmel_mpddr *mpddr = (struct atmel_mpddr *)ATMEL_BASE_MPDDRC;

	writel(mode, &mpddr->mr);
	writel(0, ram_address);
}

int ddr2_init(const unsigned int ram_address,
	      const struct atmel_mpddr *mpddr_value)
{
	struct atmel_mpddr *mpddr = (struct atmel_mpddr *)ATMEL_BASE_MPDDRC;
	u32 ba_off, cr;

	/* Compute bank offset according to NC in configuration register */
	ba_off = (mpddr_value->cr & ATMEL_MPDDRC_CR_NC_MASK) + 9;
	if (!(mpddr_value->cr & ATMEL_MPDDRC_CR_DECOD_INTERLEAVED))
		ba_off += ((mpddr->cr & ATMEL_MPDDRC_CR_NR_MASK) >> 2) + 11;

	ba_off += (mpddr_value->md & ATMEL_MPDDRC_MD_DBW_MASK) ? 1 : 2;

	/* Program the memory device type into the memory device register */
	writel(mpddr_value->md, &mpddr->md);

	/* Program the configuration register */
	writel(mpddr_value->cr, &mpddr->cr);

	/* Program the timing register */
	writel(mpddr_value->tpr0, &mpddr->tpr0);
	writel(mpddr_value->tpr1, &mpddr->tpr1);
	writel(mpddr_value->tpr2, &mpddr->tpr2);

	/* Issue a NOP command */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_NOP_CMD, ram_address);

	/* A 200 us is provided to precede any signal toggle */
	udelay(200);

	/* Issue a NOP command */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_NOP_CMD, ram_address);

	/* Issue an all banks precharge command */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_PRCGALL_CMD, ram_address);

	/* Issue an extended mode register set(EMRS2) to choose operation */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_EXT_LMR_CMD,
		       ram_address + (0x2 << ba_off));

	/* Issue an extended mode register set(EMRS3) to set EMSR to 0 */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_EXT_LMR_CMD,
		       ram_address + (0x3 << ba_off));

	/*
	 * Issue an extended mode register set(EMRS1) to enable DLL and
	 * program D.I.C (output driver impedance control)
	 */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_EXT_LMR_CMD,
		       ram_address + (0x1 << ba_off));

	/* Enable DLL reset */
	cr = readl(&mpddr->cr);
	writel(cr | ATMEL_MPDDRC_CR_DLL_RESET_ENABLED, &mpddr->cr);

	/* A mode register set(MRS) cycle is issued to reset DLL */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_LMR_CMD, ram_address);

	/* Issue an all banks precharge command */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_PRCGALL_CMD, ram_address);

	/* Two auto-refresh (CBR) cycles are provided */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_RFSH_CMD, ram_address);
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_RFSH_CMD, ram_address);

	/* Disable DLL reset */
	cr = readl(&mpddr->cr);
	writel(cr & (~ATMEL_MPDDRC_CR_DLL_RESET_ENABLED), &mpddr->cr);

	/* A mode register set (MRS) cycle is issued to disable DLL reset */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_LMR_CMD, ram_address);

	/* Set OCD calibration in default state */
	cr = readl(&mpddr->cr);
	writel(cr | ATMEL_MPDDRC_CR_OCD_DEFAULT, &mpddr->cr);

	/*
	 * An extended mode register set (EMRS1) cycle is issued
	 * to OCD default value
	 */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_EXT_LMR_CMD,
		       ram_address + (0x1 << ba_off));

	 /* OCD calibration mode exit */
	cr = readl(&mpddr->cr);
	writel(cr & (~ATMEL_MPDDRC_CR_OCD_DEFAULT), &mpddr->cr);

	/*
	 * An extended mode register set (EMRS1) cycle is issued
	 * to enable OCD exit
	 */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_EXT_LMR_CMD,
		       ram_address + (0x1 << ba_off));

	/* A nornal mode command is provided */
	atmel_mpddr_op(ATMEL_MPDDRC_MR_MODE_NORMAL_CMD, ram_address);

	/* Perform a write access to any DDR2-SDRAM address */
	writel(0, ram_address);

	/* Write the refresh rate */
	writel(mpddr_value->rtr, &mpddr->rtr);

	return 0;
}
