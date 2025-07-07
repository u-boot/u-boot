/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Standard U-Boot boot framework
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __bootstd_h
#define __bootstd_h

#include <alist.h>
#include <dm/ofnode_decl.h>
#include <linux/types.h>

struct udevice;

/**
 * struct bootstd_priv - priv data for the bootstd driver
 *
 * This is attached to the (only) bootstd device, so there is only one instance
 * of this struct. It provides overall information about bootdevs and bootflows.
 *
 * TODO(sjg@chromium.org): Convert prefixes, bootdev_order and env_order to use
 *	alist
 *
 * @prefixes: NULL-terminated list of prefixes to use for bootflow filenames,
 *	e.g. "/", "/boot/"; NULL if none
 * @bootdev_order: Order to use for bootdevs (or NULL if none), with each item
 *	being a bootdev label, e.g. "mmc2", "mmc1" (NULL terminated)
 * @env_order: Order as specified by the boot_targets env var (or NULL if none),
 *	with each item being a bootdev label, e.g. "mmc2", "mmc1" (NULL
 *	terminated)
 * @cur_bootdev: Currently selected bootdev (for commands)
 * @cur_bootflow: Currently selected bootflow (for commands)
 * @bootflows: (struct bootflow) Global list of all bootflows across all
 *	bootdevs
 * @bootmeth_count: Number of bootmeth devices in @bootmeth_order
 * @bootmeth_order: List of bootmeth devices to use, in order, NULL-terminated
 * @vbe_bootmeth: Currently selected VBE bootmeth, NULL if none
 * @theme: Node containing the theme information
 * @hunters_used: Bitmask of used hunters, indexed by their position in the
 * linker list. The bit is set if the hunter has been used already
 */
struct bootstd_priv {
	const char **prefixes;
	const char **bootdev_order;
	const char **env_order;
	struct udevice *cur_bootdev;
	struct bootflow *cur_bootflow;
	struct alist bootflows;
	int bootmeth_count;
	struct udevice **bootmeth_order;
	struct udevice *vbe_bootmeth;
	ofnode theme;
	uint hunters_used;
};

/**
 * bootstd_get_bootdev_order() - Get the boot-order list
 *
 * This reads the boot order, e.g. {"mmc0", "mmc2", NULL}
 *
 * The list is alloced by the bootstd driver so should not be freed. That is the
 * reason for all the const stuff in the function signature
 *
 * @dev: bootstd device
 * @okp: returns true if OK, false if out of memory
 * Return: list of string pointers, terminated by NULL; or NULL if no boot
 * order. Note that this returns NULL in the case of an empty list
 */
const char *const *const bootstd_get_bootdev_order(struct udevice *dev,
						   bool *okp);

/**
 * bootstd_get_prefixes() - Get the filename-prefixes list
 *
 * This reads the prefixes, e.g. {"/", "/boot", NULL}
 *
 * The list is alloced by the bootstd driver so should not be freed. That is the
 * reason for all the const stuff in the function signature
 *
 * Return: list of string points, terminated by NULL; or NULL if no boot order
 */
const char *const *const bootstd_get_prefixes(struct udevice *dev);

/**
 * bootstd_get_priv() - Get the (single) state for the bootstd system
 *
 * The state holds a global list of all bootflows that have been found.
 *
 * Return: 0 if OK, -ve if the uclass does not exist
 */
int bootstd_get_priv(struct bootstd_priv **stdp);

/**
 * bootstd_try_priv() - Try to get the (single) state for the bootstd system
 *
 * The state holds a global list of all bootflows that have been found. This
 * function returns the state if available, but takes care not to create the
 * device (or uclass) if it doesn't exist.
 *
 * This function is safe to use in the 'unbind' path. It will always return NULL
 * unless the bootstd device is probed and ready, e.g. bootstd_get_priv() has
 * previously been called.
 *
 * TODO(sjg@chromium.org): Consider adding a bootstd pointer to global_data
 *
 * Return: pointer if the device exists, else NULL
 */
struct bootstd_priv *bootstd_try_priv(void);

/**
 * bootstd_clear_glob() - Clear the global list of bootflows
 *
 * This removes all bootflows globally and across all bootdevs.
 */
void bootstd_clear_glob(void);

/**
 * bootstd_prog_boot() - Run standard boot in a fully programmatic mode
 *
 * Attempts to boot without making any use of U-Boot commands
 *
 * Returns: -ve error value (does not return except on failure to boot)
 */
int bootstd_prog_boot(void);

/**
 * bootstd_add_bootflow() - Add a bootflow to the global list
 *
 * All fields in @bflow must be set up. Note that @bflow->dev is used to add the
 * bootflow to that device.
 *
 * The bootflow is also added to the global list of all bootflows
 *
 * @dev: Bootdev device to add to
 * @bflow: Bootflow to add. Note that fields within bflow must be allocated
 *	since this function takes over ownership of these. This functions makes
 *	a copy of @bflow itself (without allocating its fields again), so the
 *	caller must dispose of the memory used by the @bflow pointer itself
 * Return: element number in the list, if OK, -ENOMEM if out of memory
 */
int bootstd_add_bootflow(struct bootflow *bflow);

/**
 * bootstd_clear_bootflows_for_bootdev() - Clear bootflows from a bootdev
 *
 * Each bootdev maintains a list of discovered bootflows. This provides a
 * way to clear it. These bootflows are removed from the global list too.
 *
 * @dev: bootdev device to update
 */
int bootstd_clear_bootflows_for_bootdev(struct udevice *dev);

#endif
