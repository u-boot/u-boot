/*
 * (C) Copyright 2005
 * STMicroelectronics.
 * Configuration settings for the "Nomadik Hardware Kit" NHK-8815,
 * the evaluation board for the Nomadik 8815 System on Chip.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <nomadik.h>

#define CONFIG_NOMADIK_8815	/* cpu variant */

#define CONFIG_SKIP_LOWLEVEL_INIT /* we have already been loaded to RAM */

/* commands */
#include <config_cmd_default.h>

#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NFS
/* There is no NOR flash, so undefine these commands */
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_SYS_NO_FLASH
/* There is NAND storage */
#define CONFIG_NAND_NOMADIK
#define CONFIG_CMD_JFFS2

/* user interface */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT		"Nomadik> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Arg Buffer Size */
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_LOAD_ADDR	0x800000	/* default load address */
#define CONFIG_SYS_LOADS_BAUD_CHANGE

/* boot config */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_TAG
#define CONFIG_BOOTDELAY	1
#define CONFIG_BOOTARGS	"root=/dev/ram0 console=ttyAMA1,115200n8 init=linuxrc"
#define CONFIG_BOOTCOMMAND	"fsload 0x100000 kernel.uimg;" \
				" fsload 0x800000 initrd.gz.uimg;" \
				" bootm 0x100000 0x800000"

/* memory-related information */
#define CONFIG_NR_DRAM_BANKS	2
#define PHYS_SDRAM_1		0x00000000	/* DDR-SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000	/* 64 MB */
#define PHYS_SDRAM_2		0x08000000	/* SDR-SDRAM BANK #2*/
#define PHYS_SDRAM_2_SIZE	0x04000000	/* 64 MB */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_SIZE	(PHYS_SDRAM_1_SIZE + PHYS_SDRAM_2_SIZE)
/* The IPL loads us at 0, tell so to u-boot. Put stack pointer 1M into RAM */
#define CONFIG_SYS_TEXT_BASE    0x00000000
#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_TEXT_BASE + (1<<20))

#define CONFIG_SYS_MEMTEST_START	0x00000000
#define CONFIG_SYS_MEMTEST_END		0x0FFFFFFF
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 256 * 1024)

#define CONFIG_BOARD_LATE_INIT	/* call board_late_init during start up */

/* timing informazion */
#define CONFIG_SYS_TIMERBASE	0x101E2000

/* serial port (PL011) configuration */
#define CONFIG_PL011_SERIAL
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CFG_SERIAL0		0x101FD000
#define CFG_SERIAL1		0x101FB000

#define CONFIG_PL01x_PORTS	{ (void *)CFG_SERIAL0, (void *)CFG_SERIAL1 }
#define CONFIG_PL011_CLOCK	48000000

/* i2c, for the port extenders (uses gpio.c in board directory) */
#ifndef __ASSEMBLY__
#include <asm/arch/gpio.h>
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define	CONFIG_SYS_I2C_SOFT	1	/* I2C bit-banged	*/
#define I2C_SOFT_DEFS
#define CONFIG_SYS_I2C_SOFT_SPEED	400000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x7F
#define __SDA			63
#define __SCL			62
#define I2C_SDA(x)		nmk_gpio_set(__SDA, x)
#define I2C_SCL(x)		nmk_gpio_set(__SCL, x)
#define I2C_READ		(nmk_gpio_get(__SDA)!=0)
#define I2C_ACTIVE		nmk_gpio_dir(__SDA, 1)
#define I2C_TRISTATE		nmk_gpio_dir(__SDA, 0)
#define I2C_DELAY     (udelay(2))
#endif /* __ASSEMBLY__ */

/* Ethernet */
#define PCI_MEMORY_VADDR	0xe8000000
#define PCI_IO_VADDR		0xee000000
#define __io(a)			((void __iomem *)(PCI_IO_VADDR + (a)))
#define __mem_isa(a)		((a) + PCI_MEMORY_VADDR)

#define CONFIG_SMC91111	/* Using SMC91c111*/
#define CONFIG_SMC91111_BASE	0x34000300
#undef  CONFIG_SMC91111_EXT_PHY	/* Internal PHY */
#define CONFIG_SMC_USE_32_BIT
#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_IP_DEFRAG	/* Allows faster download, TFTP and NFS */
#define CONFIG_TFTP_BLOCKSIZE	4096
#define CONFIG_NFS_READ_SIZE	4096

/* Storage information: onenand and nand */
#define CONFIG_CMD_ONENAND
#define CONFIG_MTD_ONENAND_VERIFY_WRITE
#define CONFIG_SYS_ONENAND_BASE		0x30000000

#define CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000 /* SMPS0n */

/*
 * Filesystem information
 *
 * Since U-Boot has been loaded to RAM by vendor code, we could use
 * either or both OneNand and Nand. However, we need to know where the
 * filesystem lives. Comments below report vendor-selected partitions
 */
#ifdef CONFIG_BOOT_ONENAND
   /* Partition				Size	Start
    * XloaderTOC + X-Loader		256KB	0x00000000
    * Memory init function		256KB	0x00040000
    * U-Boot + env			2MB	0x00080000
    * Sysimage (kernel + ramdisk)	4MB	0x00280000
    * JFFS2 Root filesystem		22MB	0x00680000
    * JFFS2 User Data			227.5MB	0x01C80000
    */
#   define CONFIG_JFFS2_DEV		"onenand0"
#   define CONFIG_JFFS2_PART_SIZE	0x01600000
#   define CONFIG_JFFS2_PART_OFFSET	0x00680000
#   define CONFIG_ENV_IS_IN_ONENAND
#   define CONFIG_ENV_SIZE		0x20000 /* 128 Kb - one sector */
#   define CONFIG_ENV_ADDR		(0x00280000 - CONFIG_ENV_SIZE)

#else /*  BOOT_NAND */
   /* Partition				Size	Start
    * XloaderTOC + X-Loader		256KB	0x00000000
    * Memory init function		256KB	0x00040000
    * U-Boot + env			2MB	0x00080000
    * Kernel Image			3MB	0x00280000
    * JFFS2 Root filesystem		22MB	0x00580000
    * JFFS2 User Data			100.5MB	0x01b80000
    */
#   define CONFIG_JFFS2_DEV		"nand0"
#   define CONFIG_JFFS2_NAND		1 /* For the jffs2 support*/
#   define CONFIG_JFFS2_PART_SIZE	0x01600000
#   define CONFIG_JFFS2_PART_OFFSET	0x00580000
#   define CONFIG_ENV_IS_IN_NAND
#   define CONFIG_ENV_SIZE		0x20000 /* 128 Kb - one sector */
#   define CONFIG_ENV_OFFSET		(0x00280000 - CONFIG_ENV_SIZE)

#endif /* CONFIG_BOOT_ONENAND */

/* this is needed to make hello_world.c and other stuff happy */
#define CONFIG_SYS_MAX_FLASH_SECT	512
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#endif /* __CONFIG_H */
