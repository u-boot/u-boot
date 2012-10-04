/*
 * This file implements recording of each stage of the boot process. It is
 * intended to implement timing of each stage, reporting this information
 * to the user and passing it to the OS for logging / further analysis.
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _BOOTSTAGE_H
#define _BOOTSTAGE_H

/* The number of boot stage records available for the user */
#ifndef CONFIG_BOOTSTAGE_USER_COUNT
#define CONFIG_BOOTSTAGE_USER_COUNT	20
#endif

/*
 * A list of boot stages that we know about. Each of these indicates the
 * state that we are at, and the action that we are about to perform. For
 * errors, we issue an error for an item when it fails. Therefore the
 * normal sequence is:
 *
 * progress action1
 * progress action2
 * progress action3
 *
 * and an error condition where action 3 failed would be:
 *
 * progress action1
 * progress action2
 * progress action3
 * error on action3
 */
enum bootstage_id {
	BOOTSTAGE_ID_START = 0,
	BOOTSTAGE_ID_CHECK_MAGIC,	/* Checking image magic */
	BOOTSTAGE_ID_CHECK_HEADER,	/* Checking image header */
	BOOTSTAGE_ID_CHECK_CHECKSUM,	/* Checking image checksum */
	BOOTSTAGE_ID_CHECK_ARCH,	/* Checking architecture */

	BOOTSTAGE_ID_CHECK_IMAGETYPE = 5,/* Checking image type */
	BOOTSTAGE_ID_DECOMP_IMAGE,	/* Decompressing image */
	BOOTSTAGE_ID_KERNEL_LOADED,	/* Kernel has been loaded */
	BOOTSTAGE_ID_DECOMP_UNIMPL = 7,	/* Odd decompression algorithm */
	BOOTSTAGE_ID_CHECK_BOOT_OS,	/* Calling OS-specific boot function */
	BOOTSTAGE_ID_BOOT_OS_RETURNED,	/* Tried to boot OS, but it returned */
	BOOTSTAGE_ID_CHECK_RAMDISK = 9,	/* Checking ram disk */

	BOOTSTAGE_ID_RD_MAGIC,		/* Checking ram disk magic */
	BOOTSTAGE_ID_RD_HDR_CHECKSUM,	/* Checking ram disk heder checksum */
	BOOTSTAGE_ID_RD_CHECKSUM,	/* Checking ram disk checksum */
	BOOTSTAGE_ID_COPY_RAMDISK = 12,	/* Copying ram disk into place */
	BOOTSTAGE_ID_RAMDISK,		/* Checking for valid ramdisk */
	BOOTSTAGE_ID_NO_RAMDISK,	/* No ram disk found (not an error) */

	BOOTSTAGE_ID_RUN_OS	= 15,	/* Exiting U-Boot, entering OS */

	BOOTSTAGE_ID_NEED_RESET = 30,
	BOOTSTAGE_ID_POST_FAIL,		/* Post failure */
	BOOTSTAGE_ID_POST_FAIL_R,	/* Post failure reported after reloc */

	/*
	 * This set is reported ony by x86, and the meaning is different. In
	 * this case we are reporting completion of a particular stage.
	 * This should probably change in he x86 code (which doesn't report
	 * errors in any case), but discussion this can perhaps wait until we
	 * have a generic board implementation.
	 */
	BOOTSTAGE_ID_BOARD_INIT_R,	/* We have relocated */
	BOOTSTAGE_ID_BOARD_GLOBAL_DATA,	/* Global data is set up */

	BOOTSTAGE_ID_BOARD_INIT_SEQ,	/* We completed the init sequence */
	BOOTSTAGE_ID_BOARD_FLASH,	/* We have configured flash banks */
	BOOTSTAGE_ID_BOARD_FLASH_37,	/* In case you didn't hear... */
	BOOTSTAGE_ID_BOARD_ENV,		/* Environment is relocated & ready */
	BOOTSTAGE_ID_BOARD_PCI,		/* PCI is up */

	BOOTSTAGE_ID_BOARD_INTERRUPTS,	/* Exceptions / interrupts ready */
	BOOTSTAGE_ID_BOARD_DONE,	/* Board init done, off to main loop */
	/* ^^^ here ends the x86 sequence */

	/* Boot stages related to loading a kernel from an IDE device */
	BOOTSTAGE_ID_IDE_START = 41,
	BOOTSTAGE_ID_IDE_ADDR,
	BOOTSTAGE_ID_IDE_BOOT_DEVICE,
	BOOTSTAGE_ID_IDE_TYPE,

	BOOTSTAGE_ID_IDE_PART,
	BOOTSTAGE_ID_IDE_PART_INFO,
	BOOTSTAGE_ID_IDE_PART_TYPE,
	BOOTSTAGE_ID_IDE_PART_READ,
	BOOTSTAGE_ID_IDE_FORMAT,

	BOOTSTAGE_ID_IDE_CHECKSUM,	/* 50 */
	BOOTSTAGE_ID_IDE_READ,

	/* Boot stages related to loading a kernel from an NAND device */
	BOOTSTAGE_ID_NAND_PART,
	BOOTSTAGE_ID_NAND_SUFFIX,
	BOOTSTAGE_ID_NAND_BOOT_DEVICE,
	BOOTSTAGE_ID_NAND_HDR_READ = 55,
	BOOTSTAGE_ID_NAND_AVAILABLE = 55,
	BOOTSTAGE_ID_NAND_TYPE = 57,
	BOOTSTAGE_ID_NAND_READ,

	/* Boot stages related to loading a kernel from an network device */
	BOOTSTAGE_ID_NET_CHECKSUM = 60,
	BOOTSTAGE_ID_NET_ETH_START = 64,
	BOOTSTAGE_ID_NET_ETH_INIT,

	BOOTSTAGE_ID_NET_START = 80,
	BOOTSTAGE_ID_NET_NETLOOP_OK,
	BOOTSTAGE_ID_NET_LOADED,
	BOOTSTAGE_ID_NET_DONE_ERR,
	BOOTSTAGE_ID_NET_DONE,

	/*
	 * Boot stages related to loading a FIT image. Some of these are a
	 * bit wonky.
	 */
	BOOTSTAGE_ID_FIT_FORMAT = 100,
	BOOTSTAGE_ID_FIT_NO_UNIT_NAME,
	BOOTSTAGE_ID_FIT_UNIT_NAME,
	BOOTSTAGE_ID_FIT_CONFIG,
	BOOTSTAGE_ID_FIT_CHECK_SUBIMAGE,
	BOOTSTAGE_ID_FIT_CHECK_HASH = 104,

	BOOTSTAGE_ID_FIT_CHECK_ARCH,
	BOOTSTAGE_ID_FIT_CHECK_KERNEL,
	BOOTSTAGE_ID_FIT_CHECKED,

	BOOTSTAGE_ID_FIT_KERNEL_INFO_ERR = 107,
	BOOTSTAGE_ID_FIT_KERNEL_INFO,
	BOOTSTAGE_ID_FIT_TYPE,

	BOOTSTAGE_ID_FIT_COMPRESSION,
	BOOTSTAGE_ID_FIT_OS,
	BOOTSTAGE_ID_FIT_LOADADDR,
	BOOTSTAGE_ID_OVERWRITTEN,

	BOOTSTAGE_ID_FIT_RD_FORMAT = 120,
	BOOTSTAGE_ID_FIT_RD_FORMAT_OK,
	BOOTSTAGE_ID_FIT_RD_NO_UNIT_NAME,
	BOOTSTAGE_ID_FIT_RD_UNIT_NAME,
	BOOTSTAGE_ID_FIT_RD_SUBNODE,

	BOOTSTAGE_ID_FIT_RD_CHECK,
	BOOTSTAGE_ID_FIT_RD_HASH = 125,
	BOOTSTAGE_ID_FIT_RD_CHECK_ALL,
	BOOTSTAGE_ID_FIT_RD_GET_DATA,
	BOOTSTAGE_ID_FIT_RD_CHECK_ALL_OK = 127,
	BOOTSTAGE_ID_FIT_RD_GET_DATA_OK,
	BOOTSTAGE_ID_FIT_RD_LOAD,

	BOOTSTAGE_ID_IDE_FIT_READ = 140,
	BOOTSTAGE_ID_IDE_FIT_READ_OK,

	BOOTSTAGE_ID_NAND_FIT_READ = 150,
	BOOTSTAGE_ID_NAND_FIT_READ_OK,

	/*
	 * These boot stages are new, higher level, and not directly related
	 * to the old boot progress numbers. They are useful for recording
	 * rough boot timing information.
	 */
	BOOTSTAGE_ID_AWAKE,
	BOOTSTAGE_ID_START_UBOOT_F,
	BOOTSTAGE_ID_START_UBOOT_R,
	BOOTSTAGE_ID_USB_START,
	BOOTSTAGE_ID_ETH_START,
	BOOTSTAGE_ID_BOOTP_START,
	BOOTSTAGE_ID_BOOTP_STOP,
	BOOTSTAGE_ID_BOOTM_START,
	BOOTSTAGE_ID_BOOTM_HANDOFF,
	BOOTSTAGE_ID_MAIN_LOOP,
	BOOTSTAGE_KERNELREAD_START,
	BOOTSTAGE_KERNELREAD_STOP,

	BOOTSTAGE_ID_CPU_AWAKE,
	BOOTSTAGE_ID_MAIN_CPU_AWAKE,
	BOOTSTAGE_ID_MAIN_CPU_READY,

	/* a few spare for the user, from here */
	BOOTSTAGE_ID_USER,
	BOOTSTAGE_ID_COUNT = BOOTSTAGE_ID_USER + CONFIG_BOOTSTAGE_USER_COUNT,
	BOOTSTAGE_ID_ALLOC,
};

/*
 * Return the time since boot in microseconds, This is needed for bootstage
 * and should be defined in CPU- or board-specific code. If undefined then
 * millisecond resolution will be used (the standard get_timer()).
 */
ulong timer_get_boot_us(void);

#ifndef CONFIG_SPL_BUILD
/*
 * Board code can implement show_boot_progress() if needed.
 *
 * @param val	Progress state (enum bootstage_id), or -id if an error
 *		has occurred.
 */
void show_boot_progress(int val);
#else
#define show_boot_progress(val) do {} while (0)
#endif

#if defined(CONFIG_BOOTSTAGE) && !defined(CONFIG_SPL_BUILD)
/* This is the full bootstage implementation */

/*
 * Mark a time stamp for the current boot stage.
 */
ulong bootstage_mark(enum bootstage_id id);

ulong bootstage_error(enum bootstage_id id);

ulong bootstage_mark_name(enum bootstage_id id, const char *name);

/* Print a report about boot time */
void bootstage_report(void);

#else
/*
 * This is a dummy implementation which just calls show_boot_progress(),
 * and won't even do that unless CONFIG_SHOW_BOOT_PROGRESS is defined
 */

static inline ulong bootstage_mark(enum bootstage_id id)
{
	show_boot_progress(id);
	return 0;
}

static inline ulong bootstage_error(enum bootstage_id id)
{
	show_boot_progress(-id);
	return 0;
}

static inline ulong bootstage_mark_name(enum bootstage_id id, const char *name)
{
	return 0;
}


#endif /* CONFIG_BOOTSTAGE */

#endif
