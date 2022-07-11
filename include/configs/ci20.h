/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * CI20 configuration
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#ifndef __CONFIG_CI20_H__
#define __CONFIG_CI20_H__

/* Ingenic JZ4780 clock configuration. */
#define CONFIG_SYS_MHZ			1200
#define CONFIG_SYS_MIPS_TIMER_FREQ	(CONFIG_SYS_MHZ * 1000000)

/* Memory configuration */
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

/* NS16550-ish UARTs */
#define CONFIG_SYS_NS16550_CLK		48000000

/* Ethernet: davicom DM9000 */
#define CONFIG_DM9000_BASE		0xb6000000
#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE + 2)

/* Miscellaneous configuration options */

#endif /* __CONFIG_CI20_H__ */
