/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * CPU and Board Configuration Options
 */
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_SERVERIP

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */

/*
 * Print Buffer Size
 */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/*
 * max number of command args
 */
#define CONFIG_SYS_MAXARGS	16

/*
 * Boot Argument Buffer Size
 */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * Size of malloc() pool
 * 512kB is suggested, (CONFIG_ENV_SIZE + 128 * 1024) was not enough
 */
#define CONFIG_SYS_MALLOC_LEN   (512 << 10)

/* DT blob (fdt) address */
#define CONFIG_SYS_FDT_BASE		0x000f0000

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2
#define PHYS_SDRAM_0	0x00000000		/* SDRAM Bank #1 */
#define PHYS_SDRAM_1	\
	(PHYS_SDRAM_0 + PHYS_SDRAM_0_SIZE)	/* SDRAM Bank #2 */
#define PHYS_SDRAM_0_SIZE	0x20000000	/* 512 MB */
#define PHYS_SDRAM_1_SIZE	0x20000000	/* 512 MB */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_0

/*
 * Serial console configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#ifndef CONFIG_DM_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#endif
#define CONFIG_SYS_NS16550_CLK		19660800

/* Init Stack Pointer */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000000 - \
					GENERATED_GBL_DATA_SIZE)

/*
 * Load address and memory test area should agree with
 * arch/riscv/config.mk. Be careful not to overwrite U-Boot itself.
 */
#define CONFIG_SYS_LOAD_ADDR		0x100000	/* SDRAM */

/*
 * memtest works on 512 MB in DRAM
 */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_0
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_0 + PHYS_SDRAM_0_SIZE)

/* environments */
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0
#define CONFIG_ENV_SPI_MAX_HZ		50000000
#define CONFIG_ENV_SPI_MODE		0
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OVERWRITE

/* SPI FLASH */
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED		1000000
#define CONFIG_SF_DEFAULT_MODE		0

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */

/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)
/* Increase max gunzip size */
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)

#endif /* __CONFIG_H */
