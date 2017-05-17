/*
 * Ugly header containing required header files. This could  be adjusted
 * so that including asm/arch/hardware includes the correct file.
 *
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_PPC_H
#define __ASM_PPC_H

#ifndef __ASSEMBLY__

#if defined(CONFIG_8xx)
#include <asm/8xx_immap.h>
#if defined(CONFIG_MPC859) || defined(CONFIG_MPC859T) || \
	defined(CONFIG_MPC866) ||  defined(CONFIG_MPC866P)
# define CONFIG_MPC866_FAMILY 1
#elif defined(CONFIG_MPC885)
# define CONFIG_MPC885_FAMILY   1
#endif
#if defined(CONFIG_MPC860) || defined(CONFIG_MPC860T) || \
	defined(CONFIG_MPC866_FAMILY) || defined(CONFIG_MPC885_FAMILY)
# define CONFIG_MPC86x 1
#endif
#elif defined(CONFIG_5xx)
#include <asm/5xx_immap.h>
#elif defined(CONFIG_MPC5xxx)
#include <mpc5xxx.h>
#elif defined(CONFIG_MPC512X)
#include <asm/immap_512x.h>
#elif defined(CONFIG_MPC8260)
#if defined(CONFIG_MPC8247) || defined(CONFIG_MPC8272)
#define CONFIG_MPC8272_FAMILY	1
#endif
#include <asm/immap_8260.h>
#endif
#ifdef CONFIG_MPC86xx
#include <mpc86xx.h>
#include <asm/immap_86xx.h>
#endif
#ifdef CONFIG_MPC85xx
#include <mpc85xx.h>
#include <asm/immap_85xx.h>
#endif
#ifdef CONFIG_MPC83xx
#include <mpc83xx.h>
#include <asm/immap_83xx.h>
#endif
#ifdef	CONFIG_4xx
#include <asm/ppc4xx.h>
#endif
#ifdef CONFIG_SOC_DA8XX
#include <asm/arch/hardware.h>
#endif
#ifdef CONFIG_FSL_LSCH3
#include <asm/arch/immap_lsch3.h>
#endif
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#endif

#endif /* !__ASSEMBLY__ */

#endif
