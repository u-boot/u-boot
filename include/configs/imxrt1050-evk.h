/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#ifndef __IMXRT1050_EVK_H
#define __IMXRT1050_EVK_H

#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_INIT_SP_ADDR		0x20280000

#define CONFIG_SYS_FSL_ERRATUM_ESDHC135		1
#define ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE	1

#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(32 * 1024 * 1024)

#define DMAMEM_SZ_ALL			(1 * 1024 * 1024)
#define DMAMEM_BASE			(PHYS_SDRAM + PHYS_SDRAM_SIZE - \
					 DMAMEM_SZ_ALL)

#ifdef CONFIG_DM_VIDEO
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO

#define CONFIG_EXTRA_ENV_SETTINGS \
		"stdin=serial\0" \
		"stdout=serial,vidconsole\0" \
		"stderr=serial,vidconsole\0"
#endif

/*
 * Configuration of the external SDRAM memory
 */

/* For SPL */
#ifdef CONFIG_SUPPORT_SPL
#define CONFIG_SPL_STACK		CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_SPL_LEN		0x00008000
#define CONFIG_SYS_UBOOT_START		0x800023FD
#endif
/* For SPL ends */

#endif /* __IMXRT1050_EVK_H */
