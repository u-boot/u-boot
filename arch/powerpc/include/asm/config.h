/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_CONFIG_H_
#define _ASM_CONFIG_H_

#ifdef CONFIG_MPC85xx
#include <asm/config_mpc85xx.h>
#endif

#ifdef CONFIG_MPC86xx
#include <asm/config_mpc86xx.h>
#endif

#ifdef CONFIG_MPC83xx
#endif

#ifndef HWCONFIG_BUFFER_SIZE
  #define HWCONFIG_BUFFER_SIZE 256
#endif

/* CONFIG_HARD_SPI triggers SPI bus initialization in PowerPC */
#if defined(CONFIG_MPC8XXX_SPI) || defined(CONFIG_FSL_ESPI)
# ifndef CONFIG_HARD_SPI
#  define CONFIG_HARD_SPI
# endif
#endif

#define CONFIG_LMB
#define CONFIG_SYS_BOOT_RAMDISK_HIGH
#define CONFIG_SYS_BOOT_GET_CMDLINE
#define CONFIG_SYS_BOOT_GET_KBD

#ifndef CONFIG_MAX_MEM_MAPPED
#if	defined(CONFIG_E500)		|| \
	defined(CONFIG_MPC86xx)		|| \
	defined(CONFIG_E300)
#define CONFIG_MAX_MEM_MAPPED	((phys_size_t)2 << 30)
#else
#define CONFIG_MAX_MEM_MAPPED	(256 << 20)
#endif
#endif

/* Check if boards need to enable FSL DMA engine for SDRAM init */
#if !defined(CONFIG_FSL_DMA) && defined(CONFIG_DDR_ECC)
#if (defined(CONFIG_MPC83xx) && defined(CONFIG_DDR_ECC_INIT_VIA_DMA)) || \
	((defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)) && \
	!defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER))
#define CONFIG_FSL_DMA
#endif
#endif

/*
 * Provide a default boot page translation virtual address that lines up with
 * Freescale's default e500 reset page.
 */
#if (defined(CONFIG_E500) && defined(CONFIG_MP))
#ifndef CONFIG_BPTR_VIRT_ADDR
#define CONFIG_BPTR_VIRT_ADDR	0xfffff000
#endif
#endif

/* Since so many PPC SOCs have a semi-common LBC, define this here */
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx) || \
	defined(CONFIG_MPC83xx)
#if !defined(CONFIG_FSL_IFC)
#define CONFIG_FSL_LBC
#endif
#endif

/* The TSEC driver uses the PHYLIB infrastructure */
#ifndef CONFIG_PHYLIB
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_PHYLIB

#include <config_phylib_all_drivers.h>
#endif /* TSEC_ENET */
#endif /* !CONFIG_PHYLIB */

/* The FMAN driver uses the PHYLIB infrastructure */
#if defined(CONFIG_FMAN_ENET)
#define CONFIG_PHYLIB
#endif

/* All PPC boards must swap IDE bytes */
#define CONFIG_IDE_SWAP_IO

#if defined(CONFIG_DM_SERIAL)
/*
 * TODO: Convert this to a clock driver exists that can give us the UART
 * clock here.
 */
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#endif

#endif /* _ASM_CONFIG_H_ */
