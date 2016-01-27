/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fsl_validate.h>
#include <fsl_sfp.h>

#ifdef CONFIG_LS102XA
#include <asm/arch/immap_ls102xa.h>
#endif

#if defined(CONFIG_MPC85xx)
#define CONFIG_DCFG_ADDR	CONFIG_SYS_MPC85xx_GUTS_ADDR
#else
#define CONFIG_DCFG_ADDR	CONFIG_SYS_FSL_GUTS_ADDR
#endif

#ifdef CONFIG_SYS_FSL_CCSR_GUR_LE
#define gur_in32(a)       in_le32(a)
#else
#define gur_in32(a)       in_be32(a)
#endif

/* Check the Boot Mode. If Secure, return 1 else return 0 */
int fsl_check_boot_mode_secure(void)
{
	uint32_t val;
	struct ccsr_sfp_regs *sfp_regs = (void *)(CONFIG_SYS_SFP_ADDR);
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_DCFG_ADDR);

	val = sfp_in32(&sfp_regs->ospr) & ITS_MASK;
	if (val == ITS_MASK)
		return 1;

#if defined(CONFIG_FSL_CORENET) || !defined(CONFIG_MPC85xx)
	/* For PBL based platforms check the SB_EN bit in RCWSR */
	val = gur_in32(&gur->rcwsr[RCW_SB_EN_REG_INDEX - 1]) & RCW_SB_EN_MASK;
	if (val == RCW_SB_EN_MASK)
		return 1;
#endif

#if defined(CONFIG_MPC85xx) && !defined(CONFIG_FSL_CORENET)
	/* For Non-PBL Platforms, check the Device Status register 2*/
	val = gur_in32(&gur->pordevsr2) & MPC85xx_PORDEVSR2_SBC_MASK;
	if (val != MPC85xx_PORDEVSR2_SBC_MASK)
		return 1;

#endif
	return 0;
}

int fsl_setenv_chain_of_trust(void)
{
	/* Check Boot Mode
	 * If Boot Mode is Non-Secure, no changes are required
	 */
	if (fsl_check_boot_mode_secure() == 0)
		return 0;

	/* If Boot mode is Secure, set the environment variables
	 * bootdelay = 0 (To disable Boot Prompt)
	 * bootcmd = CONFIG_CHAIN_BOOT_CMD (Validate and execute Boot script)
	 */
	setenv("bootdelay", "0");
	setenv("bootcmd", CONFIG_CHAIN_BOOT_CMD);
	return 0;
}
