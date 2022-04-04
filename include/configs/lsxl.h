/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
 */

#ifndef _CONFIG_LSXL_H
#define _CONFIG_LSXL_H

/*
 * General configuration options
 */

#include "mv-common.h"

/* loading initramfs images without uimage header */

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_SPI_FLASH
#define CONFIG_SYS_MAX_FLASH_SECT	8
#endif

/*
 * Default environment variables
 */

#if defined(CONFIG_LSXHL)
#define CONFIG_FDTFILE "kirkwood-lsxhl.dtb"
#elif defined(CONFIG_LSCHLV2)
#define CONFIG_FDTFILE "kirkwood-lschlv2.dtb"
#else
#error "Unsupported board"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"bootsource=legacy\0"						\
	"hdpart=0:1\0"							\
	"kernel_addr=0x00800000\0"					\
	"ramdisk_addr=0x01000000\0"					\
	"fdt_addr=0x00ff0000\0"						\
	"bootcmd_legacy=sata init "					\
		"&& load sata ${hdpart} ${kernel_addr} /uImage.buffalo "\
		"&& load sata ${hdpart} ${ramdisk_addr} /initrd.buffalo "\
		"&& bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"bootcmd_net=bootp ${kernel_addr} vmlinuz "			\
		"&& tftpboot ${ramdisk_addr} initrd.img "		\
		"&& setenv ramdisk_len ${filesize} "			\
		"&& tftpboot ${fdt_addr} " CONFIG_FDTFILE " "		\
		"&& bootz ${kernel_addr} "				\
			"${ramdisk_addr}:${ramdisk_len} ${fdt_addr}\0"	\
	"bootcmd_hdd=sata init "					\
		"&& load sata ${hdpart} ${kernel_addr} /vmlinuz "	\
		"&& load sata ${hdpart} ${ramdisk_addr} /initrd.img "	\
		"&& setenv ramdisk_len ${filesize} "			\
		"&& load sata ${hdpart} ${fdt_addr} /dtb "		\
		"&& bootz ${kernel_addr} "				\
			"${ramdisk_addr}:${ramdisk_len} ${fdt_addr}\0"	\
	"bootcmd_usb=usb start "					\
		"&& load usb 0:1 ${kernel_addr} /vmlinuz "		\
		"&& load usb 0:1 ${ramdisk_addr} /initrd.img "		\
		"&& setenv ramdisk_len ${filesize} "			\
		"&& load usb 0:1 ${fdt_addr} " CONFIG_FDTFILE " "	\
		"&& bootz ${kernel_addr} "				\
			"${ramdisk_addr}:${ramdisk_len} ${fdt_addr}\0"	\
	"bootcmd_rescue=run config_nc_dhcp; run nc\0"			\
	"eraseenv=sf probe 0 "						\
		"&& sf erase " __stringify(CONFIG_ENV_OFFSET)		\
			" +" __stringify(CONFIG_ENV_SIZE) "\0"		\
	"config_nc_dhcp=setenv autoload_old ${autoload}; "		\
		"setenv autoload no "					\
		"&& bootp "						\
		"&& setenv ncip "					\
		"&& setenv autoload ${autoload_old}; "			\
		"setenv autoload_old\0"					\
	"standard_env=setenv ipaddr; setenv netmask; setenv serverip; "	\
		"setenv ncip; setenv gatewayip; setenv ethact; "	\
		"setenv bootfile; setenv dnsip; "			\
		"setenv bootsource legacy; run ser\0"			\
	"restore_env=run standard_env; saveenv; reset\0"		\
	"ser=setenv stdin serial; setenv stdout serial; "		\
		"setenv stderr serial\0"				\
	"nc=setenv stdin nc; setenv stdout nc; setenv stderr nc\0"	\
	"stdin=serial\0"						\
	"stdout=serial\0"						\
	"stderr=serial\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS		{0, 1} /* enable port 1 only */
#define CONFIG_PHY_BASE_ADR		7
#endif /* CONFIG_CMD_NET */

#ifdef CONFIG_SATA
#define CONFIG_SYS_64BIT_LBA
#define CONFIG_LBA48
#endif

#endif /* _CONFIG_LSXL_H */
