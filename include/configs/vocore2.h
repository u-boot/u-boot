/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Mauro Condarelli <mc5686@mclink.it>
 */

#ifndef __VOCORE2_CONFIG_H__
#define __VOCORE2_CONFIG_H__

/* CPU */
#define CONFIG_SYS_MIPS_TIMER_FREQ	290000000

/* RAM */
#define CONFIG_SYS_SDRAM_BASE		0x80000000

#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

/* SPL */

#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE

/* Dummy value */
#define CONFIG_SYS_UBOOT_BASE		0

/* Serial SPL */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_CLK		40000000
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_COM3		0xb0000e00

/* RAM */

/* Environment settings */

#endif //__VOCORE2_CONFIG_H__
