/*
 * Copyright (C) 2014 Gateworks Corporation
 * Copyright (C) 2011-2012 Freescale Semiconductor, Inc.
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/spl.h>
#include <spl.h>

#if defined(CONFIG_MX6)
/* determine boot device from SRC_SBMR1 (BOOT_CFG[4:1]) or SRC_GPR9 register */
u32 spl_boot_device(void)
{
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned int gpr10_boot = readl(&psrc->gpr10) & (1 << 28);
	unsigned reg = gpr10_boot ? readl(&psrc->gpr9) : readl(&psrc->sbmr1);

	/* BOOT_CFG1[7:4] - see IMX6DQRM Table 8-8 */
	switch ((reg & 0x000000FF) >> 4) {
	 /* EIM: See 8.5.1, Table 8-9 */
	case 0x0:
		/* BOOT_CFG1[3]: NOR/OneNAND Selection */
		if ((reg & 0x00000008) >> 3)
			return BOOT_DEVICE_ONENAND;
		else
			return BOOT_DEVICE_NOR;
		break;
	/* SATA: See 8.5.4, Table 8-20 */
	case 0x2:
		return BOOT_DEVICE_SATA;
	/* Serial ROM: See 8.5.5.1, Table 8-22 */
	case 0x3:
		/* BOOT_CFG4[2:0] */
		switch ((reg & 0x07000000) >> 24) {
		case 0x0 ... 0x4:
			return BOOT_DEVICE_SPI;
		case 0x5 ... 0x7:
			return BOOT_DEVICE_I2C;
		}
		break;
	/* SD/eSD: 8.5.3, Table 8-15  */
	case 0x4:
	case 0x5:
		return BOOT_DEVICE_MMC1;
	/* MMC/eMMC: 8.5.3 */
	case 0x6:
	case 0x7:
		return BOOT_DEVICE_MMC1;
	/* NAND Flash: 8.5.2 */
	case 0x8 ... 0xf:
		return BOOT_DEVICE_NAND;
	}
	return BOOT_DEVICE_NONE;
}
#endif

#if defined(CONFIG_SPL_MMC_SUPPORT)
/* called from spl_mmc to see type of boot mode for storage (RAW or FAT) */
u32 spl_boot_mode(void)
{
	switch (spl_boot_device()) {
	/* for MMC return either RAW or FAT mode */
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
#if defined(CONFIG_SPL_FAT_SUPPORT)
		return MMCSD_MODE_FS;
#elif defined(CONFIG_SUPPORT_EMMC_BOOT)
		return MMCSD_MODE_EMMCBOOT;
#else
		return MMCSD_MODE_RAW;
#endif
		break;
	default:
		puts("spl: ERROR:  unsupported device\n");
		hang();
	}
}
#endif
