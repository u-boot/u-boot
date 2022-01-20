/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _BOOTM_H
#define _BOOTM_H

#include <image.h>

struct cmd_tbl;

#define BOOTM_ERR_RESET		(-1)
#define BOOTM_ERR_OVERLAP		(-2)
#define BOOTM_ERR_UNIMPLEMENTED	(-3)

/*
 *  Continue booting an OS image; caller already has:
 *  - copied image header to global variable `header'
 *  - checked header magic number, checksums (both header & image),
 *  - verified image architecture (PPC) and type (KERNEL or MULTI),
 *  - loaded (first part of) image to header load address,
 *  - disabled interrupts.
 *
 * @flag: Flags indicating what to do (BOOTM_STATE_...)
 * @argc: Number of arguments. Note that the arguments are shifted down
 *	 so that 0 is the first argument not processed by U-Boot, and
 *	 argc is adjusted accordingly. This avoids confusion as to how
 *	 many arguments are available for the OS.
 * @images: Pointers to os/initrd/fdt
 * Return: 1 on error. On success the OS boots so this function does
 * not return.
 */
typedef int boot_os_fn(int flag, int argc, char *const argv[],
			bootm_headers_t *images);

extern boot_os_fn do_bootm_linux;
extern boot_os_fn do_bootm_vxworks;

int do_bootelf(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

boot_os_fn *bootm_os_get_boot_func(int os);

#if defined(CONFIG_FIT_SIGNATURE)
int bootm_host_load_images(const void *fit, int cfg_noffset);
#endif

int boot_selected_os(int argc, char *const argv[], int state,
		     bootm_headers_t *images, boot_os_fn *boot_fn);

ulong bootm_disable_interrupts(void);

/* This is a special function used by booti/bootz */
int bootm_find_images(int flag, int argc, char *const argv[], ulong start,
		      ulong size);

int do_bootm_states(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[], int states, bootm_headers_t *images,
		    int boot_progress);

void arch_preboot_os(void);

/*
 * boards should define this to disable devices when EFI exits from boot
 * services.
 *
 * TODO(sjg@chromium.org>): Update this to use driver model's device_remove().
 */
void board_quiesce_devices(void);

/**
 * switch_to_non_secure_mode() - switch to non-secure mode
 */
void switch_to_non_secure_mode(void);

/* Flags to control bootm_process_cmdline() */
enum bootm_cmdline_t {
	BOOTM_CL_SILENT	= 1 << 0,	/* Do silent console processing */
	BOOTM_CL_SUBST	= 1 << 1,	/* Do substitution */

	BOOTM_CL_ALL	= 3,		/* All substitutions */
};

/**
 * arch_preboot_os() - arch specific configuration before booting
 */
void arch_preboot_os(void);

/**
 * board_preboot_os() - board specific configuration before booting
 */
void board_preboot_os(void);

/*
 * bootm_process_cmdline() - Process fix-ups for the command line
 *
 * This handles:
 *
 *  - making Linux boot silently if requested ('silent_linux' envvar)
 *  - performing substitutions in the command line ('bootargs_subst' envvar)
 *
 * @maxlen must provide enough space for the string being processed plus the
 * resulting string
 *
 * @buf: buffer holding commandline string to adjust
 * @maxlen: Maximum length of buffer at @buf (including \0)
 * @flags: Flags to control what happens (see bootm_cmdline_t)
 * Return: 0 if OK, -ENOMEM if out of memory, -ENOSPC if the commandline is too
 *	long
 */
int bootm_process_cmdline(char *buf, int maxlen, int flags);

/**
 * bootm_process_cmdline_env() - Process fix-ups for the command line
 *
 * Updates the 'bootargs' envvar as required. This handles:
 *
 *  - making Linux boot silently if requested ('silent_linux' envvar)
 *  - performing substitutions in the command line ('bootargs_subst' envvar)
 *
 * @flags: Flags to control what happens (see bootm_cmdline_t)
 * Return: 0 if OK, -ENOMEM if out of memory
 */
int bootm_process_cmdline_env(int flags);

#endif
