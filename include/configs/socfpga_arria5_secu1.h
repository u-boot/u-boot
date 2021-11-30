/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017-2020 Hitachi Power Grids
 *
 */
#ifndef __CONFIG_SOCFPGA_SECU1_H__
#define __CONFIG_SOCFPGA_SECU1_H__

#include <asm/arch/base_addr_ac5.h>
#include <linux/stringify.h>

#define CONFIG_HUSH_INIT_VAR
/* Eternal oscillator */
#define CONFIG_SYS_TIMER_RATE	40000000

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE	0x20000000	/* 512MiB on SECU1 */

/*
 * We use bootcounter in i2c nvram of the RTC (0x68)
 * The offset fopr the bootcounter is 0x9e, which are
 * the last two bytes of the 128 bytes large NVRAM in the
 * RTC which begin at address 0x20
 */
#define CONFIG_SYS_I2C_RTC_ADDR         0x68

/* Booting Linux */
#define CONFIG_BOOTFILE		"zImage"

#define CONFIG_BOOTCOMMAND	\
	"setenv bootcmd '"	\
		"bridge enable; "	\
		"if test ${bootnum} = \"b\"; " \
		      "then run _fpga_loadsafe; " \
		"else if test ${bootcount} -eq 4; then echo \"Switching copy...\"; setexpr x $bootnum % 2 && setexpr bootnum $x + 1; saveenv; fi; " \
		      "run _fpga_loaduser; " \
		"fi;" \
		"echo \"Booting bank $bootnum\" && run userload && run userboot;" \
	"' && " \
	"setenv altbootcmd 'setenv bootnum b && saveenv && boot;' && " \
	"saveenv && saveenv && boot;"

#define CONFIG_SYS_BOOTM_LEN		(64 << 20)

/* Environment settings */

/*
 * Autoboot
 *
 * After 45s of inactivity in the prompt, the board will reset.
 * Set 'bootretry' in the environment to -1 to disable this behavior
 */
#define CONFIG_BOOT_RETRY_TIME 45
#define CONFIG_RESET_TO_RETRY

/*
 * FPGA Remote Update related environment
 *
 * Note that since those commands access the FPGA, the HPS-to-FPGA
 * bridges MUST have been previously enabled (for example
 * with 'bridge enable').
 */
#define FPGA_RMTU_ENV \
	"rmtu_page=0xFF29000C\0" \
	"rmtu_reconfig=0xFF290018\0" \
	"fpga_safebase=0x0\0" \
	"fpga_userbase=0x2000000\0" \
	"_fpga_loaduser=echo Loading FPGA USER image..." \
		" && mw ${rmtu_page} ${fpga_userbase} && mw ${rmtu_reconfig} 1\0" \
	"_fpga_loadsafe=echo Loading FPGA SAFE image..." \
		" && mw ${rmtu_page} ${fpga_safebase} && mw ${rmtu_reconfig} 1\0" \

#define CONFIG_KM_NEW_ENV \
	"newenv=" \
		"nand erase 0x100000 0x40000\0"

#define CONFIG_KM_DEF_ENV_BOOTTARGETS \
	"release=" \
		"run newenv; reset\0" \
	"develop=" \
		"tftp 0x200000 scripts/develop-secu.txt && env import -t 0x200000 ${filesize} && saveenv && reset\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	FPGA_RMTU_ENV \
	CONFIG_KM_DEF_ENV_BOOTTARGETS \
	CONFIG_KM_NEW_ENV \
	"socfpga_legacy_reset_compat=1\0"	\
	"altbootcmd=run bootcmd;\0"	\
	"bootlimit=6\0"	\
	"bootnum=1\0" \
	"bootretry=" __stringify(CONFIG_BOOT_RETRY_TIME) "\0" \
	"fdt_addr=" __stringify(CONFIG_KM_FDT_ADDR) "\0" \
	"load=tftpboot ${loadaddr} u-boot-with-nand-spl.sfp\0" \
	"loadaddr=" __stringify(CONFIG_KM_KERNEL_ADDR) "\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"update=nand erase 0x0 0x00100000 && nand write ${loadaddr} 0x0 ${filesize}\0" \
	"userload=ubi part nand.ubi &&" \
		"ubi check rootfs$bootnum &&" \
		"ubi read $fdt_addr dtb$bootnum &&" \
		"ubi read $loadaddr kernel$bootnum\0" \
	"userboot=setenv bootargs " CONFIG_BOOTARGS \
		" ubi.mtd=1 ubi.block=0,rootfs$bootnum root=/dev/ubiblock0_$ubivolid"	\
		" ro rootfstype=squashfs init=sbin/preinit;" \
		"bootz ${loadaddr} - ${fdt_addr}\0" \
	"verify=y\0"

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#undef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS	60000

#endif	/* __CONFIG_SOCFPGA_SECU1_H__ */
