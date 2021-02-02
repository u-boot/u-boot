/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#ifndef __SLIMBOOTLOADER_CONFIG_H__
#define __SLIMBOOTLOADER_CONFIG_H__

#include <configs/x86-common.h>

#define CONFIG_STD_DEVICES_SETTINGS		\
	"stdin=serial,i8042-kbd,usbkbd\0"	\
	"stdout=serial\0"			\
	"stderr=serial\0"

/*
 * Override CONFIG_EXTRA_ENV_SETTINGS in x86-common.h
 */
#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS		\
	CONFIG_STD_DEVICES_SETTINGS		\
	"netdev=eth0\0"				\
	"consoledev=ttyS0\0"			\
	"ramdiskaddr=0x4000000\0"		\
	"ramdiskfile=initrd\0"			\
	"bootdev=usb\0"				\
	"bootdevnum=0\0"			\
	"bootdevpart=0\0"			\
	"bootfsload=fatload\0"			\
	"bootusb=setenv bootdev usb; boot\0"	\
	"bootscsi=setenv bootdev scsi; boot\0"	\
	"bootmmc=setenv bootdev mmc; boot\0"	\
	"bootargs=console=ttyS0,115200 console=tty0\0"

/*
 * Override CONFIG_BOOTCOMMAND in x86-common.h
 */
#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND						\
	"if test ${bootdev} = \"usb\"; then ${bootdev} start; fi; "	\
	"if test ${bootdev} = \"scsi\"; then ${bootdev} scan; fi; "	\
	"${bootdev} info; "						\
	"${bootfsload} ${bootdev} ${bootdevnum}:${bootdevpart} "	\
	"${loadaddr} ${bootfile}; "					\
	"${bootfsload} ${bootdev} ${bootdevnum}:${bootdevpart} "	\
	"${ramdiskaddr} ${ramdiskfile}; "				\
	"zboot ${loadaddr} 0 ${ramdiskaddr} ${filesize}"

#endif /* __SLIMBOOTLOADER_CONFIG_H__ */
