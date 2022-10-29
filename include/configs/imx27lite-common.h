/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Heiko Schocher <hs@denx.de>
 *
 * based on:
 * Copyright (C) 2009 Ilya Yanok <yanok@emcraft.com>
 */

#ifndef __IMX27LITE_COMMON_CONFIG_H
#define __IMX27LITE_COMMON_CONFIG_H

/*
 * SoC Configuration
 */
#define CONFIG_MX27
#define CONFIG_MX27_CLK32	32768		/* OSC32K frequency */

/*
 * Lowlevel configuration
 */
#define SDRAM_ESDCFG_REGISTER_VAL(cas)	\
		(ESDCFG_TRC(10) |	\
		ESDCFG_TRCD(3) |	\
		ESDCFG_TCAS(cas) |	\
		ESDCFG_TRRD(1) |	\
		ESDCFG_TRAS(5) |	\
		ESDCFG_TWR |		\
		ESDCFG_TMRD(2) |	\
		ESDCFG_TRP(2) |		\
		ESDCFG_TXP(3))

#define SDRAM_ESDCTL_REGISTER_VAL	\
		(ESDCTL_PRCT(0) |	\
		 ESDCTL_BL |		\
		 ESDCTL_PWDT(0) |	\
		 ESDCTL_SREFR(3) |	\
		 ESDCTL_DSIZ_32 |	\
		 ESDCTL_COL10 |		\
		 ESDCTL_ROW13 |		\
		 ESDCTL_SDE)

#define SDRAM_ALL_VAL		0xf00

#define SDRAM_MODE_REGISTER_VAL	0x33	/* BL: 8, CAS: 3 */
#define SDRAM_EXT_MODE_REGISTER_VAL	0x1000000

#define MPCTL0_VAL	0x1ef15d5

#define SPCTL0_VAL	0x043a1c09

#define CSCR_VAL	0x33f08107

#define PCDR0_VAL	0x120470c3
#define PCDR1_VAL	0x03030303
#define PCCR0_VAL	0xffffffff
#define PCCR1_VAL	0xfffffffc

#define AIPI1_PSR0_VAL	0x20040304
#define AIPI1_PSR1_VAL	0xdffbfcfb
#define AIPI2_PSR0_VAL	0x07ffc200
#define AIPI2_PSR1_VAL	0xffffffff

/*
 * Memory Info
 */
/* memtest start address */
#define PHYS_SDRAM_1		0xA0000000	/* DDR Start */
#define PHYS_SDRAM_1_SIZE	0x08000000	/* DDR size 128MB */

/*
 * Serial Driver info
 */
#define CONFIG_MXC_UART_BASE	UART_BASE_ADDR(1)

/*
 * Flash & Environment
 */
/* Use buffered writes (~10x faster) */
/* Use hardware sector protection */
/* CS2 Base address */
#define PHYS_FLASH_1			0xc0000000
/* Flash Base for U-Boot */
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
/* Address and size of Redundant Environment Sector	*/

/*
 * Ethernet
 */
#define CONFIG_FEC_MXC_PHYADDR		0x1f

/*
 * MTD
 */

/*
 * NAND
 */
#define CONFIG_MXC_NAND_REGS_BASE	0xd8000000
#define CONFIG_SYS_NAND_BASE		0xd8000000
#define CONFIG_MXC_NAND_HWECC

/*
 * U-Boot general configuration
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=ttymxc0,${baudrate}\0"			\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addmisc=setenv bootargs ${bootargs}\0"				\
	"u-boot=" CONFIG_HOSTNAME "/u-boot.bin\0"		\
	"kernel_addr_r=a0800000\0"					\
	"bootfile=" CONFIG_HOSTNAME "/uImage\0"		\
	"rootpath=/opt/eldk-4.2-arm/arm\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"			\
		"run nfsargs addip addtty addmtd addmisc;"		\
		"bootm\0"						\
	"bootcmd=run net_nfs\0"						\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"update=protect off " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize};era " __stringify(CONFIG_SYS_MONITOR_BASE)\
		" +${filesize};cp.b ${fileaddr} "			\
		__stringify(CONFIG_SYS_MONITOR_BASE) " ${filesize}\0"	\
	"upd=run load update\0"						\

/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#endif /* __IMX27LITE_COMMON_CONFIG_H */
