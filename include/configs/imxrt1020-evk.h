/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#ifndef __IMXRT1020_EVK_H
#define __IMXRT1020_EVK_H

#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_INIT_SP_ADDR		0x20240000

#ifdef CONFIG_SUPPORT_SPL
#define CONFIG_SYS_LOAD_ADDR		0x20209000
#else
#define CONFIG_SYS_LOAD_ADDR		0x80000000
#define CONFIG_LOADADDR			0x80000000
#endif

#define CONFIG_SYS_FSL_ERRATUM_ESDHC135		1
#define ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE	1

#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(32 * 1024 * 1024)

#define DMAMEM_SZ_ALL			(1 * 1024 * 1024)
#define DMAMEM_BASE			(PHYS_SDRAM + PHYS_SDRAM_SIZE - \
					 DMAMEM_SZ_ALL)

/*
 * Configuration of the external SDRAM memory
 */
#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)

/* For SPL */
#ifdef CONFIG_SUPPORT_SPL
#define CONFIG_SPL_STACK		CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_SPL_LEN		0x00008000
#define CONFIG_SYS_UBOOT_START		0x800023FD
#endif
/* For SPL ends */

#endif /* __IMXRT1020_EVK_H */
