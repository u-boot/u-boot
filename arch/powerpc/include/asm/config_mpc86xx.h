/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_MPC86xx_CONFIG_H_
#define _ASM_MPC86xx_CONFIG_H_

/* SoC specific defines for Freescale MPC86xx processors */

#if defined(CONFIG_MPC8610)
#define CONFIG_MAX_CPUS			1
#define CONFIG_SYS_FSL_NUM_LAWS		10

#elif defined(CONFIG_MPC8641)
#define CONFIG_MAX_CPUS			2
#define CONFIG_SYS_FSL_NUM_LAWS		10

#else
#error Processor type not defined for this platform
#endif

#endif /* _ASM_MPC85xx_CONFIG_H_ */
