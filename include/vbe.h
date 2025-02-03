/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Verified Boot for Embedded (VBE) support
 * See doc/develop/vbe.rst
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __VBE_H
#define __VBE_H

#include <linux/types.h>

/**
 * enum vbe_phase_t - current phase of VBE
 *
 * VBE operates in two distinct phases. In VPL it has to choose which firmware
 * to run (SPL, U-Boot, OP-TEE, etc.). It then carries on running until it gets
 * to U-Boot, where it decides which OS to run
 *
 * @VBE_PHASE_FIRMWARE: Selecting the firmware to run
 * @VBE_PHASE_OS: Selecting the Operating System to run
 */
enum vbe_phase_t {
	VBE_PHASE_FIRMWARE,
	VBE_PHASE_OS,
};

/**
 * enum vbe_pick_t - indicates which firmware is picked
 *
 * @VBEFT_A: Firmware A
 * @VBEFT_B: Firmware B
 * @VBEFT_RECOVERY: Recovery firmware
 */
enum vbe_pick_t {
	VBEP_A,
	VBEP_B,
	VBEP_RECOVERY,
};

/**
 * struct vbe_handoff - information about VBE progress
 *
 * @offset: Offset of the FIT to use for SPL onwards
 * @size: Size of the area containing the FIT
 * @phases: Indicates which phases used the VBE bootmeth (1 << PHASE_...)
 * @pick: Indicates which firmware pick was used (enum vbe_pick_t)
 */
struct vbe_handoff {
	ulong offset;
	ulong size;
	u8 phases;
	u8 pick;
};

/**
 * vbe_phase() - get current VBE phase
 *
 * Returns: Current VBE phase
 */
static inline enum vbe_phase_t vbe_phase(void)
{
	if (IS_ENABLED(CONFIG_XPL_BUILD))
		return VBE_PHASE_FIRMWARE;

	return VBE_PHASE_OS;
}

/**
 * vbe_list() - List the VBE bootmeths
 *
 * This shows a list of the VBE bootmeth devices
 *
 * @return 0 (always)
 */
int vbe_list(void);

/**
 * vbe_find_by_any() - Find a VBE bootmeth by name or sequence
 *
 * @name: name (e.g. "vbe-simple"), or sequence ("2") to find
 * @devp: returns the device found, on success
 * Return: 0 if OK, -ve on error
 */
int vbe_find_by_any(const char *name, struct udevice **devp);

/**
 * vbe_find_first_device() - Find the first VBE bootmeth
 *
 * @devp: Returns first available VBE bootmeth, or NULL if none
 * Returns: 0 (always)
 */
int vbe_find_first_device(struct udevice **devp);

/**
 * vbe_find_next_device() - Find the next available VBE bootmeth
 *
 * @devp: Previous device to start from. Returns next available VBE bootmeth,
 * or NULL if none
 * Returns: 0 (always)
 */
int vbe_find_next_device(struct udevice **devp);

#endif
