/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#ifndef __THUNDERX_88XX_H__
#define __THUNDERX_88XX_H__

#define CONFIG_REMAKE_ELF

#define CONFIG_THUNDERX

#define CONFIG_SYS_64BIT

#define CONFIG_SYS_NO_FLASH


#define CONFIG_IDENT_STRING	\
	" for Cavium Thunder CN88XX ARM v8 Multi-Core"
#define CONFIG_BOOTP_VCI_STRING		"Diagnostics"

#define MEM_BASE			0x00500000

#define CONFIG_COREID_MASK             0xffffff

#define CONFIG_SYS_FULL_VA

#define CONFIG_SYS_LOWMEM_BASE		MEM_BASE

#define CONFIG_SYS_MEM_MAP		{{0x000000000000UL, 0x40000000000UL, \
					  PTL2_MEMTYPE(MT_NORMAL) |	     \
					  PTL2_BLOCK_NON_SHARE},	     \
					 {0x800000000000UL, 0x40000000000UL, \
					  PTL2_MEMTYPE(MT_DEVICE_NGNRNE) |   \
					  PTL2_BLOCK_NON_SHARE},	     \
					 {0x840000000000UL, 0x40000000000UL, \
					  PTL2_MEMTYPE(MT_DEVICE_NGNRNE) |   \
					  PTL2_BLOCK_NON_SHARE},	     \
					}

#define CONFIG_SYS_MEM_MAP_SIZE		3

#define CONFIG_SYS_VA_BITS		48
#define CONFIG_SYS_PTL2_BITS		42
#define CONFIG_SYS_BLOCK_SHIFT		29
#define CONFIG_SYS_PTL1_ENTRIES		64
#define CONFIG_SYS_PTL2_ENTRIES		8192

#define CONFIG_SYS_PGTABLE_SIZE		\
	((CONFIG_SYS_PTL1_ENTRIES + \
	  CONFIG_SYS_MEM_MAP_SIZE * CONFIG_SYS_PTL2_ENTRIES) * 8)
#define CONFIG_SYS_TCR_EL1_IPS_BITS	(5UL << 32)
#define CONFIG_SYS_TCR_EL2_IPS_BITS	(5 << 16)
#define CONFIG_SYS_TCR_EL3_IPS_BITS	(5 << 16)

/* Link Definitions */
#define CONFIG_SYS_TEXT_BASE		0x00500000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

/* SMP Spin Table Definitions */
#define CPU_RELEASE_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)


/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		(0x1800000)	/* 24MHz */


#define CONFIG_SYS_MEMTEST_START	MEM_BASE
#define CONFIG_SYS_MEMTEST_END		(MEM_BASE + PHYS_SDRAM_1_SIZE)

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 1024 * 1024)

/* PL011 Serial Configuration */

#define CONFIG_PL01X_SERIAL
#define CONFIG_PL011_CLOCK		24000000
#define CONFIG_CONS_INDEX		1

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			(0x801000000000)
#define GICR_BASE			(0x801000002000)
#define CONFIG_SYS_SERIAL0		0x87e024000000
#define CONFIG_SYS_SERIAL1		0x87e025000000

#define CONFIG_BAUDRATE			115200

/* Command line configuration */
#define CONFIG_MENU

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_PXE
#define CONFIG_BOOTP_PXE_CLIENTARCH	0x100

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		(MEM_BASE)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			(MEM_BASE)	  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		(0x80000000-MEM_BASE)	/* 2048 MB */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Initial environment variables */
#define UBOOT_IMG_HEAD_SIZE		0x40
/* C80000 - 0x40 */
#define CONFIG_EXTRA_ENV_SETTINGS	\
					"kernel_addr=08007ffc0\0"	\
					"fdt_addr=0x94C00000\0"		\
					"fdt_high=0x9fffffff\0"

#define CONFIG_BOOTARGS			\
					"console=ttyAMA0,115200n8 " \
					"earlycon=pl011,0x87e024000000 " \
					"debug maxcpus=48 rootwait rw "\
					"root=/dev/sda2 coherent_pool=16M"
#define CONFIG_BOOTDELAY		5

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING		1
#define CONFIG_SYS_MAXARGS		64		/* max command args */
#define CONFIG_NO_RELOCATION		1
#define CONFIG_LIB_RAND
#define PLL_REF_CLK			50000000	/* 50 MHz */
#define NS_PER_REF_CLK_TICK		(1000000000/PLL_REF_CLK)

#endif /* __THUNDERX_88XX_H__ */
