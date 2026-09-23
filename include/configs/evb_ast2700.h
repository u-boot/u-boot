/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/aspeed-common.h>

/* Extra ENV for Boot Command */
#define STR_HELPER(n)	#n
#define STR(n)		STR_HELPER(n)

#define CFG_SYS_UBOOT_BASE		CONFIG_TEXT_BASE

#define CFG_EXTRA_ENV_SETTINGS \
	"bootspi=fdt addr ${fdtspiaddr} && "				\
		"fdt header get fitsize totalsize && "			\
		"cp.b ${fdtspiaddr} ${loadaddr} ${fitsize} && "		\
		"bootm ${loadaddr}; "					\
		"echo Error loading kernel FIT image\0"			\
	"loadaddr=" STR(CONFIG_SYS_LOAD_ADDR) "\0"			\
	"bootside=a\0"							\
	"rootfs=rofs-a\0"						\
	"setmmcargs=setenv bootargs ${bootargs} "			\
		"rootwait root=PARTLABEL=${rootfs}\0"			\
	"boota=setenv bootpart 2; setenv rootfs rofs-a; "		\
		"run setmmcargs; "					\
		"ext4load mmc 0:${bootpart} ${loadaddr} fitImage && "	\
		"bootm ${loadaddr}; "					\
		"echo Error loading kernel FIT image\0"			\
	"bootb=setenv bootpart 3; setenv rootfs rofs-b; "		\
		"run setmmcargs; "					\
		"ext4load mmc 0:${bootpart} ${loadaddr} fitImage && "	\
		"bootm ${loadaddr}; "					\
		"echo Error loading kernel FIT image\0"			\
	"bootmmc=if test \"${bootside}\" = \"b\"; "			\
		"then run bootb; run boota; "				\
		"else run boota; run bootb; fi\0"			\
	"setufsargs=setenv bootargs ${bootargs} "			\
		"rootwait root=PARTLABEL=${rootfs}\0"			\
	"ufsboota=setenv bootpart 2; setenv rootfs rofs-a; "		\
		"run setufsargs; "					\
		"ext4load scsi 0:${bootpart} ${loadaddr} fitImage && "	\
		"bootm ${loadaddr}; "					\
		"echo Error loading kernel FIT image\0"			\
	"ufsbootb=setenv bootpart 3; setenv rootfs rofs-b; "		\
		"run setufsargs; "					\
		"ext4load scsi 0:${bootpart} ${loadaddr} fitImage && "	\
		"bootm ${loadaddr}; "					\
		"echo Error loading kernel FIT image\0"			\
	"bootufs=if test \"${bootside}\" = \"b\"; "			\
		"then run ufsbootb; run ufsboota; "			\
		"else run ufsboota; run ufsbootb; fi\0"			\
	"verify=no\0"							\
	""
#endif	/* __CONFIG_H */
