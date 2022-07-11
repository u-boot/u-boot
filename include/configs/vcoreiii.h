/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef __VCOREIII_H
#define __VCOREIII_H

#include <linux/sizes.h>

/* Onboard devices */

#define CONFIG_SYS_INIT_SP_OFFSET       0x400000

#if defined(CONFIG_SOC_LUTON) || defined(CONFIG_SOC_SERVAL)
#define CPU_CLOCK_RATE			416666666 /* Clock for the MIPS core */
#define CONFIG_SYS_MIPS_TIMER_FREQ	208333333
#else
#define CPU_CLOCK_RATE			500000000 /* Clock for the MIPS core */
#define CONFIG_SYS_MIPS_TIMER_FREQ	(CPU_CLOCK_RATE / 2)
#endif
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_MIPS_TIMER_FREQ

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#if defined(CONFIG_DDRTYPE_H5TQ1G63BFA) || defined(CONFIG_DDRTYPE_MT47H128M8HQ)
#define CONFIG_SYS_SDRAM_SIZE		(128 * SZ_1M)
#elif defined(CONFIG_DDRTYPE_MT41J128M16HA) || defined(CONFIG_DDRTYPE_MT41K128M16JT)
#define CONFIG_SYS_SDRAM_SIZE		(256 * SZ_1M)
#elif defined(CONFIG_DDRTYPE_H5TQ4G63MFR) || defined(CONFIG_DDRTYPE_MT41K256M16)
#define CONFIG_SYS_SDRAM_SIZE		(512 * SZ_1M)
#else
#error Unknown DDR size - please add!
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"loadaddr=0x81000000\0"						\
	"spi_image_off=0x00100000\0"					\
	"console=ttyS0,115200\0"					\
	"setup=setenv bootargs console=${console} ${mtdparts}"		\
	"${bootargs_extra}\0"						\
	"spiboot=run setup; sf probe; sf read ${loadaddr}"		\
	"${spi_image_off} 0x600000; bootm ${loadaddr}\0"		\
	"ubootfile=u-boot.bin\0"					\
	"update=sf probe;mtdparts;dhcp ${loadaddr} ${ubootfile};"	\
	"sf erase UBoot 0x100000;"					\
	"sf write ${loadaddr} UBoot  ${filesize}\0"			\
	"bootcmd=run spiboot\0"						\
	""
#endif				/* __VCOREIII_H */
