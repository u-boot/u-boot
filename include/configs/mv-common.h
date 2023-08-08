/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

/*
 * This file contains Marvell Board Specific common defincations.
 * This file should be included in board config header file.
 *
 * It supports common definations for Kirkwood platform
 * TBD: support for Orion5X platforms
 */

#ifndef _MV_COMMON_H
#define _MV_COMMON_H

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * Custom CONFIG_SYS_TEXT_BASE can be done in <board>.h
 */

/* additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE	0x00000000

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#if !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_COM1		MV_UART_CONSOLE_BASE
#endif

/* auto boot */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs  */
#define CONFIG_INITRD_TAG	1	/* enable INITRD tag */
#define CONFIG_SETUP_MEMORY_TAGS 1	/* enable memory tag */

#define	CONFIG_SYS_CBSIZE	1024	/* Console I/O Buff Size */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(1024 * 1024 * 4) /* 4MiB for malloc() */

/*
 * Other required minimal configurations
 */
#define CONFIG_SYS_LOAD_ADDR	0x00800000	/* default load adr- 8M */
#define CONFIG_SYS_MEMTEST_START 0x00800000	/* 8M */
#define CONFIG_SYS_MEMTEST_END	0x00ffffff	/*(_16M -1) */
#define CONFIG_SYS_RESET_ADDRESS 0xffff0000	/* Rst Vector Adr */
#define CONFIG_SYS_MAXARGS	32	/* max number of command args */

/* ====> Include platform Common Definitions */
#include <asm/arch/config.h>

/* ====> Include driver Common Definitions */
/*
 * Common NAND configuration
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE     1
#endif

/* Default Env vars */
#define CONFIG_IPADDR			0.0.0.0	/* In order to cause an error */
#define CONFIG_SERVERIP			0.0.0.0	/* In order to cause an error */
#define CONFIG_NETMASK			255.255.255.0
#define CONFIG_GATEWAYIP		10.4.50.254
#define CONFIG_HAS_ETH1
#define CONFIG_ETHPRIME			"eth0"
#define CONFIG_ROOTPATH                 "/srv/nfs/" /* Default Dir for NFS */
#define CONFIG_ENV_OVERWRITE		/* ethaddr can be reprogrammed */
#define CONFIG_EXTRA_ENV_SETTINGS	"bootcmd=run get_images; " \
						"run set_bootargs; " \
						"bootz $kernel_addr_r " \
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
					"ramdisk_addr_r=0x8000000\0"	\
					"ramfs_name=-\0"		\
					"cpuidle=cpuidle.off=1\0"	\
					"fdt_name=fdt.dtb\0"		\
					"netdev=eth0\0"			\
					"ethaddr=00:51:82:11:22:00\0"	\
					"eth1addr=00:51:82:11:22:01\0"	\
					"eth2addr=00:51:82:11:22:02\0"	\
					"eth3addr=00:51:82:11:22:03\0"	\
					"image_name=zImage\0"		\
					"get_ramfs=if test \"${ramfs_name}\"" \
						" != \"-\"; then setenv " \
						"ramdisk_addr_r 0x8000000; " \
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
						"$cpuidle\0"

/*
 * PCI
 */
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE     1

#endif /* _MV_COMMON_H */
