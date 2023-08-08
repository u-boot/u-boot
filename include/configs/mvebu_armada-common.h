/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_MVEBU_ARMADA_H
#define _CONFIG_MVEBU_ARMADA_H

#include <asm/arch/soc.h>

/* Additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE		0x00000000

/* Console */
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, \
					  115200, 230400, 460800, 921600 }

/* Default Env vars */
#define CONFIG_IPADDR			0.0.0.0	/* In order to cause an error */
#define CONFIG_SERVERIP			0.0.0.0	/* In order to cause an error */
#define CONFIG_NETMASK			255.255.255.0
#define CONFIG_GATEWAYIP		10.4.50.254
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_ETHPRIME			"eth0"
#define CONFIG_ROOTPATH			"/srv/nfs/" /* Default Dir for NFS */
#define CONFIG_ENV_OVERWRITE		/* ethaddr can be reprogrammed */
#define CONFIG_EXTRA_ENV_SETTINGS	"bootcmd=run get_images; " \
						"run set_bootargs; " \
						"booti $kernel_addr_r " \
						"$ramdisk_addr_r " \
						"$fdt_addr_r\0" \
					"extra_params=pci=pcie_bus_safe\0" \
					"kernel_addr_r=0x7000000\0"	\
					"initrd_addr=0xa00000\0"	\
					"initrd_size=0x2000000\0"	\
					"fdt_addr_r=0x6f00000\0"	\
					"loadaddr=0x6000000\0"		\
					"fdt_high=0xffffffffffffffff\0"	\
					"hostname=marvell\0"		\
					"ramdisk_addr_r=0x9000000\0"	\
					"ramfs_name=-\0"		\
					"cpuidle=cpuidle.off=1\0"	\
					"fdt_name=fdt.dtb\0"		\
					"netdev=eth0\0"			\
					"ethaddr=00:51:82:11:22:00\0"	\
					"eth1addr=00:51:82:11:22:01\0"	\
					"eth2addr=00:51:82:11:22:02\0"	\
					"eth3addr=00:51:82:11:22:03\0"	\
					"eth4addr=00:51:82:11:22:04\0"	\
					"image_name=Image\0"		\
					"get_ramfs=if test \"${ramfs_name}\"" \
						" != \"-\"; then setenv " \
						"ramdisk_addr_r $ramdisk_addr_r; " \
						"tftpboot $ramdisk_addr_r " \
						"$ramfs_name; else setenv " \
						"ramdisk_addr_r -;fi\0"	\
					"get_images=tftpboot $kernel_addr_r " \
						"$image_name; tftpboot " \
						"$fdt_addr_r $fdt_name; " \
						"run get_ramfs\0"	\
					"console=" CONFIG_DEFAULT_CONSOLE "\0"\
					"root=root=/dev/nfs rw\0"	\
					"set_bootargs=setenv bootargs $console"\
						" $root ip=$ipaddr:$serverip:" \
						"$gatewayip:$netmask:$hostname"\
						":$netdev:none nfsroot="\
						"$serverip:$rootpath,tcp,v3 " \
						"$extra_params " \
						"$cpuidle\0" \
					"usb_pgood_delay=5000"

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs  */
#define CONFIG_INITRD_TAG		/* enable INITRD tag */
#define CONFIG_SETUP_MEMORY_TAGS	/* enable memory tag */

#define	CONFIG_SYS_CBSIZE	1024	/* Console I/O Buff Size */
#define	CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE \
		+ sizeof(CONFIG_SYS_PROMPT) + 16)	/* Print Buff */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(9 << 20) /* 16MiB for malloc() */

/*
 * Other required minimal configurations
 */
#define CONFIG_SYS_LOAD_ADDR	0x00800000	/* default load adr- 8M */
#define CONFIG_SYS_MEMTEST_START 0x00800000	/* 8M */
#define CONFIG_SYS_MEMTEST_END	0x00ffffff	/*(_16M -1) */
#define CONFIG_SYS_RESET_ADDRESS 0xffff0000	/* Rst Vector Adr */
#define CONFIG_SYS_MAXARGS	32	/* max number of command args */

#define CONFIG_SYS_ALT_MEMTEST

/* End of 16M scrubbed by training in bootrom */
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_TEXT_BASE + 0xFF0000)

/*
 * SPI Flash configuration
 */
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0

/* SPI NOR flash default params, used by sf commands */
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED

#ifdef CONFIG_MVEBU_NAND_BOOT
/* In case of NAND, we want to start the environment on page boundary
 * Linux DTS uses 2MB partition size for u-boot.
 */
#define CONFIG_ENV_SIZE			(256 << 10)
#define CONFIG_ENV_OFFSET		(0x200000 - CONFIG_ENV_SIZE)

#else /* !CONFIG_MVEBU_NAND_BOOT */
/*
 * Assume minimum flash/eMMC boot partition size of 4MB
 * and save the environment at the end of the boot device
 * There are some exclusions from this rule, for instance EspressoBIN board.
 * The eMMC device found on some EspressoBIN V7 boards has 2MB boot partition.
 */
#define CONFIG_ENV_SIZE			(64 << 10) /* 64KiB */
#define CONFIG_ENV_SECT_SIZE		(64 << 10) /* 64KiB sectors */

#if defined(CONFIG_ENV_IS_IN_MMC) && \
((defined(CONFIG_TARGET_MVEBU_ARMADA_37XX) && \
	defined(CONFIG_MV88E6XXX_SWITCH))  || \
defined(CONFIG_TARGET_OCTEONTX2_CN913x) && (CONFIG_MVEBU_BOOT_PART != 0))
/* A3700 + MV88E6XXX == EspressoBIN board OR
 * CN913x CRB that boots from eMMC partition BOOT0/BOOT1 (ID 1/2)
 */
#define CONFIG_ENV_OFFSET		(0x200000 - CONFIG_ENV_SIZE)
#else
#define CONFIG_ENV_OFFSET		(0x400000 - CONFIG_ENV_SIZE)
#endif
#endif /* CONFIG_MVEBU_NAND_BOOT */

/*
 * Device ID and Boot partition defined in external script
 */
#define CONFIG_SYS_MMC_ENV_DEV		(CONFIG_MVEBU_BOOT_DEVICE)
#define CONFIG_SYS_MMC_ENV_PART		(CONFIG_MVEBU_BOOT_PART)

/*
 * SATA/SCSI/AHCI configuration
 */
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_LBA48
#define CONFIG_SYS_64BIT_LBA

#define CONFIG_SYS_SCSI_MAX_SCSI_ID	2
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)

/*
 * Ethernet Driver configuration
 */
#define CONFIG_ARP_TIMEOUT	200
#define CONFIG_NET_RETRY_COUNT	50

/*
 * The EEPROM ST M24C64 has 32 byte page write mode and takes up to 10 msec.
 */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

#endif /* _CONFIG_MVEBU_ARMADA_H */
