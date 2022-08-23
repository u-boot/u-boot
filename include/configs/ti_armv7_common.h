/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ti_armv7_common.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * The various ARMv7 SoCs from TI all share a number of IP blocks when
 * implementing a given feature.  Rather than define these in every
 * board or even SoC common file, we define a common file to be re-used
 * in all cases.  While technically true that some of these details are
 * configurable at the board design, they are common throughout SoC
 * reference platforms as well as custom designs and become de facto
 * standards.
 */

#ifndef __CONFIG_TI_ARMV7_COMMON_H__
#define __CONFIG_TI_ARMV7_COMMON_H__

/*
 * We setup defaults based on constraints from the Linux kernel, which should
 * also be safe elsewhere.  We have the default load at 32MB into DDR (for
 * the kernel), FDT above 128MB (the maximum location for the end of the
 * kernel), and the ramdisk 512KB above that (allowing for hopefully never
 * seen large trees).  We say all of this must be within the first 256MB
 * as that will normally be within the kernel lowmem and thus visible via
 * bootm_size and we only run on platforms with 256MB or more of memory.
 *
 * As a temporary storage for DTBO blobs (which should be applied into DTB
 * blob), we use the location 15.5 MB above the ramdisk. If someone wants to
 * use ramdisk bigger than 15.5 MB, then DTBO can be loaded and applied to DTB
 * blob before loading the ramdisk, as DTBO location is only used as a temporary
 * storage, and can be re-used after 'fdt apply' command is done.
 */
#define DEFAULT_LINUX_BOOT_ENV \
	"loadaddr=0x82000000\0" \
	"kernel_addr_r=0x82000000\0" \
	"fdtaddr=0x88000000\0" \
	"dtboaddr=0x89000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"fdtoverlay_addr_r=0x89000000\0" \
	"rdaddr=0x88080000\0" \
	"ramdisk_addr_r=0x88080000\0" \
	"scriptaddr=0x80000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"bootm_size=0x10000000\0" \
	"boot_fdt=try\0"

#define DEFAULT_FIT_TI_ARGS \
	"boot_fit=0\0" \
	"addr_fit=0x90000000\0" \
	"name_fit=fitImage\0" \
	"update_to_fit=setenv loadaddr ${addr_fit}; setenv bootfile ${name_fit}\0" \
	"get_overlaystring=" \
		"for overlay in $name_overlays;" \
		"do;" \
		"setenv overlaystring ${overlaystring}'#'${overlay};" \
		"done;\0" \
	"run_fit=bootm ${addr_fit}#conf-${fdtfile}${overlaystring}\0" \

/*
 * DDR information.  If the CONFIG_NR_DRAM_BANKS is not defined,
 * we say (for simplicity) that we have 1 bank, always, even when
 * we have more.  We always start at 0x80000000, and we place the
 * initial stack pointer in our SRAM. Otherwise, we can define
 * CONFIG_NR_DRAM_BANKS before including this file.
 */
#define CONFIG_SYS_SDRAM_BASE		0x80000000

#ifndef CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_INIT_SP_ADDR         (NON_SECURE_SRAM_END - \
						GENERATED_GBL_DATA_SIZE)
#endif

/* Timer information. */
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/* If DM_I2C, enable non-DM I2C support */

/*
 * The following are general good-enough settings for U-Boot.  We set a
 * large malloc pool as we generally have a lot of DDR, and we opt for
 * function over binary size in the main portion of U-Boot as this is
 * generally easily constrained later if needed.  We enable the config
 * options that give us information in the environment about what board
 * we are on so we do not need to rely on the command prompt.  We set a
 * console baudrate of 115200 and use the default baud rate table.
 */

/* As stated above, the following choices are optional. */

/* We set the max number of command args high to avoid HUSH bugs. */
#define CONFIG_SYS_MAXARGS		64

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		1024
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define EEPROM_PROGRAMMING \
	"eeprom_dump=i2c dev 0; " \
		"i2c md 0x50 0x00.2 20; " \
		"\0" \
	"eeprom_blank=i2c dev 0; " \
		"i2c mw 0x50 0x00.2 ff; " \
		"i2c mw 0x50 0x01.2 ff; " \
		"i2c mw 0x50 0x02.2 ff; " \
		"i2c mw 0x50 0x03.2 ff; " \
		"i2c mw 0x50 0x04.2 ff; " \
		"i2c mw 0x50 0x05.2 ff; " \
		"i2c mw 0x50 0x06.2 ff; " \
		"i2c mw 0x50 0x07.2 ff; " \
		"i2c mw 0x50 0x08.2 ff; " \
		"i2c mw 0x50 0x09.2 ff; " \
		"i2c mw 0x50 0x0a.2 ff; " \
		"i2c mw 0x50 0x0b.2 ff; " \
		"i2c mw 0x50 0x0c.2 ff; " \
		"i2c mw 0x50 0x0d.2 ff; " \
		"i2c mw 0x50 0x0e.2 ff; " \
		"i2c mw 0x50 0x0f.2 ff; " \
		"i2c mw 0x50 0x10.2 ff; " \
		"i2c mw 0x50 0x11.2 ff; " \
		"i2c mw 0x50 0x12.2 ff; " \
		"i2c mw 0x50 0x13.2 ff; " \
		"i2c mw 0x50 0x14.2 ff; " \
		"i2c mw 0x50 0x15.2 ff; " \
		"i2c mw 0x50 0x16.2 ff; " \
		"i2c mw 0x50 0x17.2 ff; " \
		"i2c mw 0x50 0x18.2 ff; " \
		"i2c mw 0x50 0x19.2 ff; " \
		"i2c mw 0x50 0x1a.2 ff; " \
		"i2c mw 0x50 0x1b.2 ff; " \
		"i2c mw 0x50 0x1c.2 ff; " \
		"i2c mw 0x50 0x1d.2 ff; " \
		"i2c mw 0x50 0x1e.2 ff; " \
		"i2c mw 0x50 0x1f.2 ff; " \
		"\0" \
	"eeprom_bbb_header=i2c dev 0; " \
		"i2c mw 0x50 0x00.2 aa; " \
		"i2c mw 0x50 0x01.2 55; " \
		"i2c mw 0x50 0x02.2 33; " \
		"i2c mw 0x50 0x03.2 ee; " \
		"i2c mw 0x50 0x04.2 41; " \
		"i2c mw 0x50 0x05.2 33; " \
		"i2c mw 0x50 0x06.2 33; " \
		"i2c mw 0x50 0x07.2 35; " \
		"i2c mw 0x50 0x08.2 42; " \
		"i2c mw 0x50 0x09.2 4e; " \
		"i2c mw 0x50 0x0a.2 4c; " \
		"i2c mw 0x50 0x0b.2 54; " \
		"\0" \
	"eeprom_bbbl_footer= " \
		"i2c mw 0x50 0x0c.2 42; " \
		"i2c mw 0x50 0x0d.2 4c; " \
		"i2c mw 0x50 0x0e.2 41; " \
		"i2c mw 0x50 0x0f.2 32; " \
		"\0" \
	"eeprom_bbbw_footer= " \
		"i2c mw 0x50 0x0c.2 42; " \
		"i2c mw 0x50 0x0d.2 57; " \
		"i2c mw 0x50 0x0e.2 41; " \
		"i2c mw 0x50 0x0f.2 35; " \
		"\0" \
	"eeprom_bbgg_footer= " \
		"i2c mw 0x50 0x0c.2 47; " \
		"i2c mw 0x50 0x0d.2 47; " \
		"i2c mw 0x50 0x0e.2 31; " \
		"i2c mw 0x50 0x0f.2 41; " \
		"\0" \
	"eeprom_pocketbeagle= " \
		"i2c mw 0x50 0x00.2 aa; " \
		"i2c mw 0x50 0x01.2 55; " \
		"i2c mw 0x50 0x02.2 33; " \
		"i2c mw 0x50 0x03.2 ee; " \
		"i2c mw 0x50 0x04.2 41; " \
		"i2c mw 0x50 0x05.2 33; " \
		"i2c mw 0x50 0x06.2 33; " \
		"i2c mw 0x50 0x07.2 35; " \
		"i2c mw 0x50 0x08.2 50; " \
		"i2c mw 0x50 0x09.2 42; " \
		"i2c mw 0x50 0x0a.2 47; " \
		"i2c mw 0x50 0x0b.2 4c; " \
		"i2c mw 0x50 0x0c.2 30; " \
		"i2c mw 0x50 0x0d.2 30; " \
		"i2c mw 0x50 0x0e.2 41; " \
		"i2c mw 0x50 0x0f.2 32; " \
		"\0" \
	"eeprom_beaglelogic= " \
		"i2c mw 0x50 0x00.2 aa; " \
		"i2c mw 0x50 0x01.2 55; " \
		"i2c mw 0x50 0x02.2 33; " \
		"i2c mw 0x50 0x03.2 ee; " \
		"i2c mw 0x50 0x04.2 41; " \
		"i2c mw 0x50 0x05.2 33; " \
		"i2c mw 0x50 0x06.2 33; " \
		"i2c mw 0x50 0x07.2 35; " \
		"i2c mw 0x50 0x08.2 42; " \
		"i2c mw 0x50 0x09.2 4c; " \
		"i2c mw 0x50 0x0a.2 47; " \
		"i2c mw 0x50 0x0b.2 43; " \
		"i2c mw 0x50 0x0c.2 30; " \
		"i2c mw 0x50 0x0d.2 30; " \
		"i2c mw 0x50 0x0e.2 30; " \
		"i2c mw 0x50 0x0f.2 41; " \
		"\0" \

#define BBAI_EEPROM_PROGRAMMING \
	"eeprom_dump=i2c dev 0; " \
		"i2c md 0x50 0x00.2 20; " \
		"\0" \
	"eeprom_blank=i2c dev 0; " \
		"i2c mw 0x50 0x00.2 ff; " \
		"i2c mw 0x50 0x01.2 ff; " \
		"i2c mw 0x50 0x02.2 ff; " \
		"i2c mw 0x50 0x03.2 ff; " \
		"i2c mw 0x50 0x04.2 ff; " \
		"i2c mw 0x50 0x05.2 ff; " \
		"i2c mw 0x50 0x06.2 ff; " \
		"i2c mw 0x50 0x07.2 ff; " \
		"i2c mw 0x50 0x08.2 ff; " \
		"i2c mw 0x50 0x09.2 ff; " \
		"i2c mw 0x50 0x0a.2 ff; " \
		"i2c mw 0x50 0x0b.2 ff; " \
		"i2c mw 0x50 0x0c.2 ff; " \
		"i2c mw 0x50 0x0d.2 ff; " \
		"i2c mw 0x50 0x0e.2 ff; " \
		"i2c mw 0x50 0x0f.2 ff; " \
		"i2c mw 0x50 0x10.2 ff; " \
		"i2c mw 0x50 0x11.2 ff; " \
		"i2c mw 0x50 0x12.2 ff; " \
		"i2c mw 0x50 0x13.2 ff; " \
		"i2c mw 0x50 0x14.2 ff; " \
		"i2c mw 0x50 0x15.2 ff; " \
		"i2c mw 0x50 0x16.2 ff; " \
		"i2c mw 0x50 0x17.2 ff; " \
		"i2c mw 0x50 0x18.2 ff; " \
		"i2c mw 0x50 0x19.2 ff; " \
		"i2c mw 0x50 0x1a.2 ff; " \
		"i2c mw 0x50 0x1b.2 ff; " \
		"i2c mw 0x50 0x1c.2 ff; " \
		"i2c mw 0x50 0x1d.2 ff; " \
		"i2c mw 0x50 0x1e.2 ff; " \
		"i2c mw 0x50 0x1f.2 ff; " \
		"i2c md 0x50 0x00.2 20; " \
		"\0" \
	"eeprom_x15_c=i2c dev 0; " \
		"i2c mw 0x50 0x00.2 aa; " \
		"i2c mw 0x50 0x01.2 55; " \
		"i2c mw 0x50 0x02.2 33; " \
		"i2c mw 0x50 0x03.2 ee; " \
		"i2c mw 0x50 0x04.2 42; " \
		"i2c mw 0x50 0x05.2 42; " \
		"i2c mw 0x50 0x06.2 52; " \
		"i2c mw 0x50 0x07.2 44; " \
		"i2c mw 0x50 0x08.2 58; " \
		"i2c mw 0x50 0x09.2 31; " \
		"i2c mw 0x50 0x0a.2 35; " \
		"i2c mw 0x50 0x0b.2 5f; " \
		"i2c mw 0x50 0x0c.2 43; " \
		"i2c mw 0x50 0x0d.2 2e; " \
		"i2c mw 0x50 0x0e.2 30; " \
		"i2c mw 0x50 0x0f.2 30; " \
		"i2c mw 0x50 0x10.2 79; " \
		"i2c mw 0x50 0x11.2 79; " \
		"i2c mw 0x50 0x12.2 77; " \
		"i2c mw 0x50 0x13.2 77; " \
		"i2c mw 0x50 0x14.2 50; " \
		"i2c mw 0x50 0x15.2 58; " \
		"i2c mw 0x50 0x16.2 31; " \
		"i2c mw 0x50 0x17.2 35; " \
		"i2c mw 0x50 0x18.2 6e; " \
		"i2c mw 0x50 0x19.2 6e; " \
		"i2c mw 0x50 0x1a.2 6e; " \
		"i2c mw 0x50 0x1b.2 6e; " \
		"i2c mw 0x50 0x1c.2 ff; " \
		"i2c mw 0x50 0x1d.2 ff; " \
		"i2c mw 0x50 0x1e.2 ff; " \
		"i2c mw 0x50 0x1f.2 ff; " \
		"i2c md 0x50 0x00.2 20; " \
		"\0" \
	"eeprom_bbai_a2=i2c dev 0; " \
		"i2c mw 0x50 0x00.2 aa; " \
		"i2c mw 0x50 0x01.2 55; " \
		"i2c mw 0x50 0x02.2 33; " \
		"i2c mw 0x50 0x03.2 ee; " \
		"i2c mw 0x50 0x04.2 42; " \
		"i2c mw 0x50 0x05.2 42; " \
		"i2c mw 0x50 0x06.2 4F; " \
		"i2c mw 0x50 0x07.2 4E; " \
		"i2c mw 0x50 0x08.2 45; " \
		"i2c mw 0x50 0x09.2 2D; " \
		"i2c mw 0x50 0x0a.2 41; " \
		"i2c mw 0x50 0x0b.2 49; " \
		"i2c mw 0x50 0x0c.2 30; " \
		"i2c mw 0x50 0x0d.2 30; " \
		"i2c mw 0x50 0x0e.2 41; " \
		"i2c mw 0x50 0x0f.2 32; " \
		"i2c mw 0x50 0x10.2 57; " \
		"i2c mw 0x50 0x11.2 57; " \
		"i2c mw 0x50 0x12.2 59; " \
		"i2c mw 0x50 0x13.2 59; " \
		"i2c mw 0x50 0x14.2 53; " \
		"i2c mw 0x50 0x15.2 42; " \
		"i2c mw 0x50 0x16.2 41; " \
		"i2c mw 0x50 0x17.2 49; " \
		"i2c mw 0x50 0x18.2 30; " \
		"i2c mw 0x50 0x19.2 30; " \
		"i2c mw 0x50 0x1a.2 30; " \
		"i2c mw 0x50 0x1b.2 30; " \
		"i2c mw 0x50 0x1c.2 30; " \
		"i2c mw 0x50 0x1d.2 30; " \
		"i2c mw 0x50 0x1e.2 ff; " \
		"i2c mw 0x50 0x1f.2 ff; " \
		"i2c md 0x50 0x00.2 20; " \
		"\0" \

#define EEWIKI_BOOT \
	"boot=${devtype} dev ${mmcdev}; " \
		"if ${devtype} rescan; then " \
			"gpio set 54;" \
			"setenv bootpart ${mmcdev}:1; " \
			"if test -e ${devtype} ${bootpart} /etc/fstab; then " \
				"setenv mmcpart 1;" \
			"fi; " \
			"echo Checking for: /uEnv.txt ...;" \
			"if test -e ${devtype} ${bootpart} /uEnv.txt; then " \
				"if run loadbootenv; then " \
					"gpio set 55;" \
					"echo Loaded environment from /uEnv.txt;" \
					"run importbootenv;" \
				"fi;" \
				"echo Checking if uenvcmd is set ...;" \
				"if test -n ${uenvcmd}; then " \
					"gpio set 56;" \
					"echo Running uenvcmd ...;" \
					"run uenvcmd;" \
				"fi;" \
			"fi; " \
			"echo Checking for: /boot/uEnv.txt ...;" \
			"for i in 1 2 3 4 5 6 7 ; do " \
				"setenv mmcpart ${i};" \
				"setenv bootpart ${mmcdev}:${mmcpart};" \
				"if test -e ${devtype} ${bootpart} /boot/uEnv.txt; then " \
					"gpio set 55;" \
					"load ${devtype} ${bootpart} ${loadaddr} /boot/uEnv.txt;" \
					"env import -t ${loadaddr} ${filesize};" \
					"echo Loaded environment from /boot/uEnv.txt;" \
					"if test -n ${dtb}; then " \
						"echo debug: [dtb=${dtb}] ... ;" \
						"setenv fdtfile ${dtb};" \
						"echo Using: dtb=${fdtfile} ...;" \
					"fi;" \
					"echo Checking if uname_r is set in /boot/uEnv.txt...;" \
					"if test -n ${uname_r}; then " \
						"gpio set 56; " \
						"setenv oldroot /dev/mmcblk${mmcdev}p${mmcpart};" \
						"echo Running uname_boot ...;" \
						"run uname_boot;" \
					"fi;" \
				"fi;" \
			"done;" \
		"fi;\0" \

#define BBAI_MMC_BOOT \
	"mmc_boot=${devtype} dev ${mmcdev}; ${devtype} part; " \
		"if ${devtype} rescan; then " \
			"echo Scanning ${devtype} device ${mmcdev};" \
			"setenv bootpart ${mmcdev}:1; " \
			"echo Checking for: /uEnv.txt ...;" \
			"if test -e ${devtype} ${bootpart} /uEnv.txt; then " \
				"load ${devtype} ${bootpart} ${loadaddr} /uEnv.txt;" \
				"env import -t ${loadaddr} ${filesize};" \
				"echo Loaded environment from /uEnv.txt;" \
				"echo Checking if uenvcmd is set ...;" \
				"if test -n ${uenvcmd}; then " \
					"echo Running uenvcmd ...;" \
					"run uenvcmd;" \
				"fi;" \
			"fi; " \
			"echo Checking for: /boot/uEnv.txt ...;" \
			"for i in 1 2 3 4 5 6 7 ; do " \
				"setenv mmcpart ${i};" \
				"setenv bootpart ${mmcdev}:${mmcpart};" \
				"if test -e ${devtype} ${bootpart} /boot/uEnv.txt; then " \
					"load ${devtype} ${bootpart} ${loadaddr} /boot/uEnv.txt;" \
					"env import -t ${loadaddr} ${filesize};" \
					"echo Loaded environment from /boot/uEnv.txt;" \
					"if test -n ${dtb}; then " \
						"setenv fdtfile ${dtb};" \
						"echo debug: [dtb=${fdtfile}] ...;" \
					"fi;" \
					"echo Checking if uname_r is set in /boot/uEnv.txt ...;" \
					"if test -n ${uname_r}; then " \
						"echo debug: [uname_r=${uname_r}] ...;" \
						"setenv oldroot /dev/mmcblk${mmcdev}p${mmcpart};" \
						"run uname_boot;" \
					"fi;" \
				"fi;" \
			"done;" \
		"fi;\0" \

#define EEWIKI_UNAME_BOOT \
	"uname_boot="\
		"setenv bootdir /boot; " \
		"setenv bootfile vmlinuz-${uname_r}; " \
		"if test -e ${devtype} ${bootpart} ${bootdir}/${bootfile}; then " \
			"echo loading ${bootdir}/${bootfile} ...; "\
			"run loadimage;" \
			"setenv fdtdir /boot/dtbs/${uname_r}; " \
			"echo debug: [enable_uboot_overlays=${enable_uboot_overlays}] ... ;" \
			"if test -n ${enable_uboot_overlays}; then " \
				"echo debug: [enable_uboot_cape_universal=${enable_uboot_cape_universal}] ... ;" \
				"if test -n ${enable_uboot_cape_universal}; then " \
					"echo debug: [uboot_base_dtb_univ=${uboot_base_dtb_univ}] ... ;" \
					"if test -n ${uboot_base_dtb_univ}; then " \
						"echo uboot_overlays: [uboot_base_dtb=${uboot_base_dtb_univ}] ... ;" \
						"if test -e ${devtype} ${bootpart} ${fdtdir}/${uboot_base_dtb_univ}; then " \
							"setenv fdtfile ${uboot_base_dtb_univ};" \
							"echo uboot_overlays: Switching too: dtb=${fdtfile} ...;" \
							"setenv cape_uboot bone_capemgr.uboot_capemgr_enabled=1; " \
						"else " \
							"echo debug: unable to find [${uboot_base_dtb_univ}] using [${uboot_base_dtb}] instead ... ;" \
							"echo debug: [uboot_base_dtb_univ=${uboot_base_dtb}] ... ;" \
							"if test -n ${uboot_base_dtb}; then " \
								"echo uboot_overlays: [uboot_base_dtb=${uboot_base_dtb}] ... ;" \
								"if test -e ${devtype} ${bootpart} ${fdtdir}/${uboot_base_dtb}; then " \
									"setenv fdtfile ${uboot_base_dtb};" \
									"echo uboot_overlays: Switching too: dtb=${fdtfile} ...;" \
								"fi;" \
							"fi;" \
						"fi;" \
					"fi;" \
				"else " \
					"echo debug: [uboot_base_dtb_univ=${uboot_base_dtb}] ... ;" \
					"if test -n ${uboot_base_dtb}; then " \
						"echo uboot_overlays: [uboot_base_dtb=${uboot_base_dtb}] ... ;" \
						"if test -e ${devtype} ${bootpart} ${fdtdir}/${uboot_base_dtb}; then " \
							"setenv fdtfile ${uboot_base_dtb};" \
							"echo uboot_overlays: Switching too: dtb=${fdtfile} ...;" \
						"fi;" \
					"fi;" \
				"fi;" \
			"fi;" \
			"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
				"run loadfdt;" \
			"else " \
				"setenv fdtdir /usr/lib/linux-image-${uname_r}; " \
				"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
					"run loadfdt;" \
				"else " \
					"setenv fdtdir /lib/firmware/${uname_r}/device-tree; " \
					"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
						"run loadfdt;" \
					"else " \
						"setenv fdtdir /boot/dtb-${uname_r}; " \
						"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
							"run loadfdt;" \
						"else " \
							"setenv fdtdir /boot/dtbs; " \
							"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
								"run loadfdt;" \
							"else " \
								"setenv fdtdir /boot/dtb; " \
								"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
									"run loadfdt;" \
								"else " \
									"setenv fdtdir /boot; " \
									"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
										"run loadfdt;" \
									"else " \
										"if test -e ${devtype} ${bootpart} ${fdtfile}; then " \
											"run loadfdt;" \
										"else " \
											"echo; echo unable to find [dtb=${fdtfile}] did you name it correctly? ...; " \
											"run failumsboot;" \
										"fi;" \
									"fi;" \
								"fi;" \
							"fi;" \
						"fi;" \
					"fi;" \
				"fi;" \
			"fi; " \
			"if test -n ${enable_uboot_overlays}; then " \
				"extension scan;" \
				"setenv fdt_buffer 0x60000;" \
				"if test -n ${uboot_fdt_buffer}; then " \
					"setenv fdt_buffer ${uboot_fdt_buffer};" \
				"fi;" \
				"echo uboot_overlays: [fdt_buffer=${fdt_buffer}] ... ;" \
				"if test -n ${uboot_silicon}; then " \
					"setenv uboot_overlay ${uboot_silicon}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_model}; then " \
					"setenv uboot_overlay ${uboot_model}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${disable_uboot_overlay_adc}; then " \
					"echo uboot_overlays: uboot loading of [BB-ADC-00A0.dtbo] disabled by /boot/uEnv.txt [disable_uboot_overlay_adc=1]...;" \
				"else " \
					"setenv uboot_overlay BB-ADC-00A0.dtbo; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr0}; then " \
					"if test -n ${disable_uboot_overlay_addr0}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_overlay_addr0}] disabled by /boot/uEnv.txt [disable_uboot_overlay_addr0=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_overlay_addr0}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr1}; then " \
					"if test -n ${disable_uboot_overlay_addr1}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_overlay_addr1}] disabled by /boot/uEnv.txt [disable_uboot_overlay_addr1=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_overlay_addr1}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr2}; then " \
					"if test -n ${disable_uboot_overlay_addr2}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_overlay_addr2}] disabled by /boot/uEnv.txt [disable_uboot_overlay_addr2=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_overlay_addr2}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr3}; then " \
					"if test -n ${disable_uboot_overlay_addr3}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_overlay_addr3}] disabled by /boot/uEnv.txt [disable_uboot_overlay_addr3=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_overlay_addr3}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr4}; then " \
					"setenv uboot_overlay ${uboot_overlay_addr4}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr5}; then " \
					"setenv uboot_overlay ${uboot_overlay_addr5}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr6}; then " \
					"setenv uboot_overlay ${uboot_overlay_addr6}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr7}; then " \
					"setenv uboot_overlay ${uboot_overlay_addr7}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_emmc}; then " \
					"if test -n ${disable_uboot_overlay_emmc}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_emmc}] disabled by /boot/uEnv.txt [disable_uboot_overlay_emmc=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_emmc}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_video}; then " \
					"if test -n ${disable_uboot_overlay_video}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_video}] disabled by /boot/uEnv.txt [disable_uboot_overlay_video=1]...;" \
					"else " \
						"if test -n ${disable_uboot_overlay_audio}; then " \
							"echo uboot_overlays: uboot loading of [${uboot_video}] disabled by /boot/uEnv.txt [disable_uboot_overlay_audio=1]...;" \
							"setenv uboot_overlay ${uboot_video_naudio}; " \
							"run virtualloadoverlay;" \
						"else " \
							"setenv uboot_overlay ${uboot_video}; " \
							"run virtualloadoverlay;" \
						"fi;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_wireless}; then " \
					"if test -n ${disable_uboot_overlay_wireless}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_wireless}] disabled by /boot/uEnv.txt [disable_uboot_overlay_wireless=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_wireless}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_overlay_pru}; then " \
					"setenv uboot_overlay ${uboot_overlay_pru}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_overlay_pru_add}; then " \
					"setenv uboot_overlay ${uboot_overlay_pru_add}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${dtb_overlay}; then " \
					"setenv uboot_overlay ${dtb_overlay}; " \
					"echo uboot_overlays: [dtb_overlay=${uboot_overlay}] ... ;" \
					"run virtualloadoverlay;" \
				"fi;" \
			"else " \
				"echo uboot_overlays: add [enable_uboot_overlays=1] to /boot/uEnv.txt to enable...;" \
			"fi;" \
			"if test -n ${uboot_detected_capes}; then " \
				"echo uboot_overlays: [uboot_detected_capes=${uboot_detected_capes_addr0}${uboot_detected_capes_addr1}${uboot_detected_capes_addr2}${uboot_detected_capes_addr3}] ... ;" \
				"setenv uboot_detected_capes uboot_detected_capes=${uboot_detected_capes_addr0}${uboot_detected_capes_addr1}${uboot_detected_capes_addr2}${uboot_detected_capes_addr3}; " \
			"fi;" \
			"setenv rdfile initrd.img-${uname_r}; " \
			"if test -e ${devtype} ${bootpart} ${bootdir}/${rdfile}; then " \
				"echo loading ${bootdir}/${rdfile} ...; "\
				"run loadrd;" \
				"if test -n ${netinstall_enable}; then " \
					"run args_netinstall; run message;" \
					"echo debug: [${bootargs}] ... ;" \
					"echo debug: [bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}] ... ;" \
					"bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}; " \
				"fi;" \
				"if test -n ${uenv_root}; then " \
					"run args_uenv_root;" \
					"echo debug: [${bootargs}] ... ;" \
					"echo debug: [bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}] ... ;" \
					"bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}; " \
				"fi;" \
				"if test -n ${uuid}; then " \
					"run args_mmc_uuid;" \
					"echo debug: [${bootargs}] ... ;" \
					"echo debug: [bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}] ... ;" \
					"bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}; " \
				"fi;" \
				"run args_mmc_old;" \
				"echo debug: [${bootargs}] ... ;" \
				"echo debug: [bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}] ... ;" \
				"bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}; " \
			"else " \
				"if test -n ${uenv_root}; then " \
					"run args_uenv_root;" \
					"echo debug: [${bootargs}] ... ;" \
					"echo debug: [bootz ${loadaddr} - ${fdtaddr}] ... ;" \
					"bootz ${loadaddr} - ${fdtaddr}; " \
				"fi;" \
				"run args_mmc_old;" \
				"echo debug: [${bootargs}] ... ;" \
				"echo debug: [bootz ${loadaddr} - ${fdtaddr}] ... ;" \
				"bootz ${loadaddr} - ${fdtaddr}; " \
			"fi;" \
		"fi;\0" \

#define BBAI_UNAME_BOOT \
	"uname_boot="\
		"setenv bootdir /boot; " \
		"setenv bootfile vmlinuz-${uname_r}; " \
		"if test -e ${devtype} ${bootpart} ${bootdir}/${bootfile}; then " \
			"echo loading ${bootdir}/${bootfile} ...; "\
			"run loadimage;" \
			"setenv fdtdir /boot/dtbs/${uname_r}; " \
			"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
				"run loadfdt;" \
			"else " \
				"setenv fdtdir /usr/lib/linux-image-${uname_r}; " \
				"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
					"run loadfdt;" \
				"else " \
					"setenv fdtdir /lib/firmware/${uname_r}/device-tree; " \
					"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
						"run loadfdt;" \
					"else " \
						"setenv fdtdir /boot/dtb-${uname_r}; " \
						"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
							"run loadfdt;" \
						"else " \
							"setenv fdtdir /boot/dtbs; " \
							"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
								"run loadfdt;" \
							"else " \
								"setenv fdtdir /boot/dtb; " \
								"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
									"run loadfdt;" \
								"else " \
									"setenv fdtdir /boot; " \
									"if test -e ${devtype} ${bootpart} ${fdtdir}/${fdtfile}; then " \
										"run loadfdt;" \
									"else " \
										"echo; echo unable to find ${fdtfile} ...; echo booting legacy ...;"\
										"run args_mmc;" \
										"echo debug: [${bootargs}] ... ;" \
										"echo debug: [bootz ${loadaddr}] ... ;" \
										"bootz ${loadaddr}; " \
									"fi;" \
								"fi;" \
							"fi;" \
						"fi;" \
					"fi;" \
				"fi;" \
			"fi; " \
			"if test -n ${enable_uboot_overlays}; then " \
				"setenv fdt_buffer 0x60000;" \
				"if test -n ${uboot_fdt_buffer}; then " \
					"setenv fdt_buffer ${uboot_fdt_buffer};" \
				"fi;" \
				"echo uboot_overlays: [fdt_buffer=${fdt_buffer}] ... ;" \
				"if test -n ${uboot_overlay_addr0}; then " \
					"if test -n ${disable_uboot_overlay_addr0}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_overlay_addr0}] disabled by /boot/uEnv.txt [disable_uboot_overlay_addr0=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_overlay_addr0}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr1}; then " \
					"if test -n ${disable_uboot_overlay_addr1}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_overlay_addr1}] disabled by /boot/uEnv.txt [disable_uboot_overlay_addr1=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_overlay_addr1}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr2}; then " \
					"if test -n ${disable_uboot_overlay_addr2}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_overlay_addr2}] disabled by /boot/uEnv.txt [disable_uboot_overlay_addr2=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_overlay_addr2}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr3}; then " \
					"if test -n ${disable_uboot_overlay_addr3}; then " \
						"echo uboot_overlays: uboot loading of [${uboot_overlay_addr3}] disabled by /boot/uEnv.txt [disable_uboot_overlay_addr3=1]...;" \
					"else " \
						"setenv uboot_overlay ${uboot_overlay_addr3}; " \
						"run virtualloadoverlay;" \
					"fi;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr4}; then " \
					"setenv uboot_overlay ${uboot_overlay_addr4}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr5}; then " \
					"setenv uboot_overlay ${uboot_overlay_addr5}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr6}; then " \
					"setenv uboot_overlay ${uboot_overlay_addr6}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_overlay_addr7}; then " \
					"setenv uboot_overlay ${uboot_overlay_addr7}; " \
					"run virtualloadoverlay;" \
				"fi;" \
				"if test -n ${uboot_overlay_pru}; then " \
					"setenv uboot_overlay ${uboot_overlay_pru}; " \
					"run virtualloadoverlay;" \
				"fi;" \
			"else " \
				"echo uboot_overlays: add [enable_uboot_overlays=1] to /boot/uEnv.txt to enable...;" \
			"fi;" \
			"if test -n ${uboot_detected_capes}; then " \
				"echo uboot_overlays: [uboot_detected_capes=${uboot_detected_capes_addr0}${uboot_detected_capes_addr1}${uboot_detected_capes_addr2}${uboot_detected_capes_addr3}] ... ;" \
				"setenv uboot_detected_capes uboot_detected_capes=${uboot_detected_capes_addr0}${uboot_detected_capes_addr1}${uboot_detected_capes_addr2}${uboot_detected_capes_addr3}; " \
			"fi;" \
			"setenv rdfile initrd.img-${uname_r}; " \
			"if test -e ${devtype} ${bootpart} ${bootdir}/${rdfile}; then " \
				"echo loading ${bootdir}/${rdfile} ...; "\
				"run loadrd;" \
				"if test -n ${netinstall_enable}; then " \
					"run args_netinstall; run message;" \
					"echo debug: [${bootargs}] ... ;" \
					"echo debug: [bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}] ... ;" \
					"bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}; " \
				"fi;" \
				"if test -n ${uenv_root}; then " \
					"run args_uenv_root;" \
					"echo debug: [${bootargs}] ... ;" \
					"echo debug: [bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}] ... ;" \
					"bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}; " \
				"fi;" \
				"if test -n ${uuid}; then " \
					"run args_mmc_uuid;" \
					"echo debug: [${bootargs}] ... ;" \
					"echo debug: [bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}] ... ;" \
					"bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}; " \
				"else " \
					"run args_mmc_old;" \
					"echo debug: [${bootargs}] ... ;" \
					"echo debug: [bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}] ... ;" \
					"bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}; " \
				"fi;" \
			"else " \
				"if test -n ${uenv_root}; then " \
					"run args_uenv_root;" \
					"echo debug: [${bootargs}] ... ;" \
					"echo debug: [bootz ${loadaddr} - ${fdtaddr}] ... ;" \
					"bootz ${loadaddr} - ${fdtaddr}; " \
				"fi;" \
				"run args_mmc_old;" \
				"echo debug: [${bootargs}] ... ;" \
				"echo debug: [bootz ${loadaddr} - ${fdtaddr}] ... ;" \
				"bootz ${loadaddr} - ${fdtaddr}; " \
			"fi;" \
		"fi;\0" \

/*
 * When we have SPI, NOR or NAND flash we expect to be making use of
 * mtdparts, both for ease of use in U-Boot and for passing information
 * on to the Linux kernel.
 */

/*
 * Our platforms make use of SPL to initalize the hardware (primarily
 * memory) enough for full U-Boot to be loaded. We make use of the general
 * SPL framework found under common/spl/.  Given our generally common memory
 * map, we set a number of related defaults and sizes here.
 */
#if !defined(CONFIG_NOR_BOOT) && \
	!(defined(CONFIG_QSPI_BOOT) && defined(CONFIG_AM43XX))

/*
 * We also support Falcon Mode so that the Linux kernel can be booted
 * directly from SPL. This is not currently available on HS devices.
 */

/*
 * Place the image at the start of the ROM defined image space (per
 * CONFIG_SPL_TEXT_BASE and we limit our size to the ROM-defined
 * downloaded image area minus 1KiB for scratch space.  We initalize DRAM as
 * soon as we can so that we can place stack, malloc and BSS there.  We load
 * U-Boot itself into memory at 0x80800000 for legacy reasons (to not conflict
 * with older SPLs).  We have our BSS be placed 2MiB after this, to allow for
 * the default Linux kernel address of 0x80008000 to work with most sized
 * kernels, in the Falcon Mode case.  We have the SPL malloc pool at the end
 * of the BSS area.  We suggest that the stack be placed at 32MiB after the
 * start of DRAM to allow room for all of the above (handled in Kconfig).
 */
#ifndef CONFIG_SPL_BSS_START_ADDR
#define CONFIG_SPL_BSS_START_ADDR	0x80a00000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */
#endif
#ifndef CONFIG_SYS_SPL_MALLOC_START
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_BSS_START_ADDR + \
					 CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_8M
#endif
#ifndef CONFIG_SPL_MAX_SIZE
#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)
#endif


/* FAT sd card locations. */
#ifndef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"
#endif

#ifdef CONFIG_SPL_OS_BOOT
/* FAT */
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME		"uImage"
#define CONFIG_SPL_FS_LOAD_ARGS_NAME		"args"

/* RAW SD card / eMMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x1500  /* address 0x2A0000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0x200   /* 256KiB */
#endif

/* General parts of the framework, required. */

#ifdef CONFIG_MTD_RAW_NAND
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#endif
#endif /* !CONFIG_NOR_BOOT */

/* Generic Environment Variables */

#ifdef CONFIG_CMD_NET
#define NETARGS \
	"static_ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}" \
		"::off\0" \
	"nfsopts=nolock\0" \
	"rootpath=/export/rootfs\0" \
	"netloadimage=tftp ${loadaddr} ${bootfile}\0" \
	"netloadfdt=tftp ${fdtaddr} ${fdtfile}\0" \
	"netargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} rw " \
		"ip=dhcp\0" \
	"netboot=echo Booting from network ...; " \
		"setenv autoload no; " \
		"dhcp; " \
		"run netloadimage; " \
		"run netloadfdt; " \
		"run netargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0"
#else
#define NETARGS ""
#endif

#endif	/* __CONFIG_TI_ARMV7_COMMON_H__ */
