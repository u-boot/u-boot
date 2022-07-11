/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#ifndef __IMXRT1050_EVK_H
#define __IMXRT1050_EVK_H

#include <asm/arch/imx-regs.h>

#define ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE	1

#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(32 * 1024 * 1024)

#define DMAMEM_SZ_ALL			(1 * 1024 * 1024)
#define DMAMEM_BASE			(PHYS_SDRAM + PHYS_SDRAM_SIZE - \
					 DMAMEM_SZ_ALL)

#ifdef CONFIG_DM_VIDEO
#define CONFIG_EXTRA_ENV_SETTINGS \
		"stdin=serial\0" \
		"stdout=serial,vidconsole\0" \
		"stderr=serial,vidconsole\0"
#endif

/*
 * Configuration of the external SDRAM memory
 */

#define CONFIG_SYS_UBOOT_START		0x800023FD

#endif /* __IMXRT1050_EVK_H */
