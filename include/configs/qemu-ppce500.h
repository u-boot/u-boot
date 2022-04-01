/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011-2014 Freescale Semiconductor, Inc.
 */

/*
 * Corenet DS style board configuration file
 */
#ifndef __QEMU_PPCE500_H
#define __QEMU_PPCE500_H

#define CONFIG_SYS_MPC85XX_NO_RESETVEC

#define CONFIG_SYS_RAMBOOT

#define CONFIG_ENABLE_36BIT_PHYS

/* Needed to fill the ccsrbar pointer */

/* Virtual address to CCSRBAR */
#define CONFIG_SYS_CCSRBAR		0xe0000000
/* Physical address should be a function call */
#ifndef __ASSEMBLY__
extern unsigned long long get_phys_ccsrbar_addr_early(void);
#define CONFIG_SYS_CCSRBAR_PHYS_HIGH (get_phys_ccsrbar_addr_early() >> 32)
#define CONFIG_SYS_CCSRBAR_PHYS_LOW get_phys_ccsrbar_addr_early()
#else
#define CONFIG_SYS_CCSRBAR_PHYS_HIGH 0x0
#define CONFIG_SYS_CCSRBAR_PHYS_LOW CONFIG_SYS_CCSRBAR
#endif

/* Virtual address range for PCI region maps */
#define CONFIG_SYS_PCI_MAP_START	0x80000000
#define CONFIG_SYS_PCI_MAP_END		0xe0000000

/* Virtual address to a temporary map if we need it (max 128MB) */
#define CONFIG_SYS_TMPVIRT		0xe8000000

/*
 * DDR Setup
 */
#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_SYS_BOOT_BLOCK		0x00000000	/* boot TLB */

#define CONFIG_HWCONFIG

#define CONFIG_SYS_INIT_RAM_ADDR		0x00100000
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH	0x0
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW	0x00100000
/* The assembler doesn't like typecast */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS \
	((CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#define CONFIG_SYS_INIT_RAM_SIZE		0x00004000

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

#define CONFIG_LBA48

/* RTC */
#define CONFIG_RTC_PT7C4338

/*
 * Environment
 */

#define CONFIG_LOADS_ECHO		/* echo on for serial download */

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

/*
 * Environment Configuration
 */
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_UBOOTPATH	"u-boot.bin"	/* U-Boot image on TFTP server*/

#endif	/* __QEMU_PPCE500_H */
