/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Compass Electronics Group, LLC
 */

#ifndef __BEACON_RZG2M_H
#define __BEACON_RZG2M_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

#undef CONFIG_EXTRA_ENV_SETTINGS

#define CONFIG_EXTRA_ENV_SETTINGS		\
	"usb_pgood_delay=2000\0"	\
	"script=boot.scr\0" \
	"image=Image\0" \
	"console=ttySC0,115200\0" \
	"fdt_addr=0x48000000\0" \
	"loadaddr=0x48080000\0" \
	"boot_fdt=try\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"initrd_addr=0x43800000\0"		\
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"mmcrootpart=2\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcrootpart} uuid\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console} " \
	" root=PARTUUID=${uuid} rootwait rw ${optargs}\0" \
	"loadbootscript=load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=load mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run finduuid; run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"booti ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"else " \
			"echo wait for boot; " \
		"fi;\0" \
	"netargs=setenv bootargs ${jh_clk} console=${console} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs;  " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${loadaddr} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"booti ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"else " \
			"booti; " \
		"fi;\0"

#undef CONFIG_BOOTCOMMAND

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
	   "if run loadbootscript; then " \
		   "run bootscript; " \
	   "else " \
		   "if run loadimage; then " \
			   "run mmcboot; " \
		   "else run netboot; " \
		   "fi; " \
	   "fi; " \
	"else booti ${loadaddr} - ${fdt_addr}; fi"

#endif /* __BEACON_RZG2M_H */
