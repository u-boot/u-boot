/*
 * DENX M53 configuration
 * Copyright (C) 2012-2013 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __M53EVK_CONFIG_H__
#define __M53EVK_CONFIG_H__

#define CONFIG_MX53
#define CONFIG_MXC_GPIO

#include <asm/arch/imx-regs.h>

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_REVISION_TAG
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FSL_CLK

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

/*
 * U-Boot Commands
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_FAT_WRITE

#define CONFIG_CMD_BMP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_CMD_SATA

/*
 * Memory configurations
 */
#define CONFIG_NR_DRAM_BANKS		2
#define PHYS_SDRAM_1			CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE		(gd->bd->bi_dram[0].size)
#define PHYS_SDRAM_2			CSD1_BASE_ADDR
#define PHYS_SDRAM_2_SIZE		(gd->bd->bi_dram[1].size)
#define PHYS_SDRAM_SIZE			(gd->ram_size)
#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)
#define CONFIG_SYS_MEMTEST_START	0x70000000
#define CONFIG_SYS_MEMTEST_END		0x8ff00000

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_TEXT_BASE		0x71000000

/*
 * U-Boot general configurations
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O buffer size */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
						/* Print buffer size */
#define CONFIG_SYS_MAXARGS	32		/* Max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
						/* Boot argument buffer size */
#define CONFIG_AUTO_COMPLETE			/* Command auto complete */
#define CONFIG_CMDLINE_EDITING			/* Command history etc */

/*
 * Serial Driver
 */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART2_BASE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/*
 * MMC Driver
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	1
#endif

/*
 * NAND
 */
#define CONFIG_ENV_SIZE			(16 * 1024)
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		NFC_BASE_ADDR_AXI
#define CONFIG_NAND_MXC
#define CONFIG_MXC_NAND_REGS_BASE	NFC_BASE_ADDR_AXI
#define CONFIG_MXC_NAND_IP_REGS_BASE	NFC_BASE_ADDR
#define CONFIG_SYS_NAND_LARGEPAGE
#define CONFIG_MXC_NAND_HWECC
#define CONFIG_SYS_NAND_USE_FLASH_BBT

/* Environment is in NAND */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_SECT_SIZE		(128 * 1024)
#define CONFIG_ENV_RANGE		(4 * CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_OFFSET		(8 * CONFIG_ENV_SECT_SIZE) /* 1 MiB */
#define CONFIG_ENV_OFFSET_REDUND	\
		(CONFIG_ENV_OFFSET + CONFIG_ENV_RANGE)

#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_RBTREE
#define CONFIG_LZO
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT			"nand0=mxc_nand"
#define MTDPARTS_DEFAULT			\
	"mtdparts=mxc_nand:"			\
		"1024k(u-boot),"		\
		"512k(env1),"			\
		"512k(env2),"			\
		"14m(boot),"			\
		"240m(data),"			\
		"-@2048k(UBI)"
#else
#define CONFIG_ENV_IS_NOWHERE
#endif

/*
 * Ethernet on SOC (FEC)
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR		0x0
#define CONFIG_MII
#define CONFIG_DISCOVER_PHY
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL
#define CONFIG_ETHPRIME			"FEC0"
#endif

/*
 * I2C
 */
#ifdef CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_RTC_BUS_NUM		1 /* I2C2 */
#endif

/*
 * RTC
 */
#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_M41T62
#define CONFIG_SYS_I2C_RTC_ADDR		0x68
#define CONFIG_SYS_M41T11_BASE_YEAR	2000
#endif

/*
 * USB
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX5
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_MCS7830
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_MXC_USB_PORT		1
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#endif

/*
 * SATA
 */
#ifdef CONFIG_CMD_SATA
#define CONFIG_DWC_AHSATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_BASE_ADDR
#define CONFIG_LBA48
#define CONFIG_LIBATA
#endif

/*
 * LCD
 */
#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_IPUV3
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASHIMAGE_GUARD
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(2 << 20)
#define CONFIG_IPUV3_CLK		200000000
#endif

/*
 * Boot Linux
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTFILE		"fitImage"
#define CONFIG_BOOTARGS		"console=ttymxc1,115200"
#define CONFIG_LOADADDR		0x70800000
#define CONFIG_BOOTCOMMAND	"run mmc_mmc"
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/*
 * NAND SPL
 */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TARGET		"u-boot-with-nand-spl.imx"
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_TEXT_BASE		0x70008000
#define CONFIG_SPL_PAD_TO		0x8000
#define CONFIG_SPL_STACK		0x70004000

#define CONFIG_SYS_NAND_U_BOOT_OFFS	CONFIG_SPL_PAD_TO
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_SIZE		(256 * 1024 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0

/*
 * Extra Environments
 */
#define CONFIG_PREBOOT		"run try_bootscript"
#define CONFIG_HOSTNAME		m53evk

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"consdev=ttymxc1\0"						\
	"baudrate=115200\0"						\
	"bootscript=boot.scr\0"						\
	"bootdev=/dev/mmcblk0p1\0"					\
	"rootdev=/dev/mmcblk0p2\0"					\
	"netdev=eth0\0"							\
	"rootpath=/opt/eldk-5.5/armv7a-hf/rootfs-qte-sdk\0"		\
	"kernel_addr_r=0x72000000\0"					\
	"addcons="							\
		"setenv bootargs ${bootargs} "				\
		"console=${consdev},${baudrate}\0"			\
	"addip="							\
		"setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
			"${netmask}:${hostname}:${netdev}:off\0"	\
	"addmisc="							\
		"setenv bootargs ${bootargs} ${miscargs}\0"		\
	"adddfltmtd="							\
		"if test \"x${mtdparts}\" == \"x\" ; then "		\
			"mtdparts default ; "				\
		"fi\0"							\
	"addmtd="							\
		"run adddfltmtd ; "					\
		"setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addargs=run addcons addmtd addmisc\0"				\
	"mmcload="							\
		"mmc rescan ; "						\
		"load mmc 0:1 ${kernel_addr_r} ${bootfile}\0"		\
	"ubiload="							\
		"ubi part UBI ; ubifsmount ubi0:rootfs ; "		\
		"ubifsload ${kernel_addr_r} /boot/${bootfile}\0"	\
	"netload="							\
		"tftp ${kernel_addr_r} ${hostname}/${bootfile}\0"	\
	"miscargs=nohlt panic=1\0"					\
	"mmcargs=setenv bootargs root=${rootdev} rw rootwait\0"		\
	"ubiargs="							\
		"setenv bootargs ubi.mtd=5 "				\
		"root=ubi0:rootfs rootfstype=ubifs\0"			\
	"nfsargs="							\
		"setenv bootargs root=/dev/nfs rw "			\
			"nfsroot=${serverip}:${rootpath},v3,tcp\0"	\
	"mmc_mmc="							\
		"run mmcload mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"mmc_ubi="							\
		"run mmcload ubiargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"mmc_nfs="							\
		"run mmcload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"ubi_mmc="							\
		"run ubiload mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"ubi_ubi="							\
		"run ubiload ubiargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"ubi_nfs="							\
		"run ubiload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_mmc="							\
		"run netload mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_ubi="							\
		"run netload ubiargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_nfs="							\
		"run netload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"try_bootscript="						\
		"mmc rescan;"						\
		"if test -e mmc 0:1 ${bootscript} ; then "		\
		"if load mmc 0:1 ${kernel_addr_r} ${bootscript};"	\
		"then ; "						\
			"echo Running bootscript... ; "			\
			"source ${kernel_addr_r} ; "			\
		"fi ; "							\
		"fi\0"

#endif	/* __M53EVK_CONFIG_H__ */
