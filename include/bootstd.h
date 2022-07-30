/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Standard U-Boot boot framework
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __bootstd_h
#define __bootstd_h

struct udevice;

/**
 * struct bootstd_priv - priv data for the bootstd driver
 *
 * This is attached to the (only) bootstd device, so there is only one instance
 * of this struct. It provides overall information about bootdevs and bootflows.
 *
 * @prefixes: NULL-terminated list of prefixes to use for bootflow filenames,
 *	e.g. "/", "/boot/"; NULL if none
 * @bootdev_order: Order to use for bootdevs (or NULL if none), with each item
 *	being a bootdev label, e.g. "mmc2", "mmc1";
 * @cur_bootdev: Currently selected bootdev (for commands)
 * @cur_bootflow: Currently selected bootflow (for commands)
 * @glob_head: Head for the global list of all bootflows across all bootdevs
 * @bootmeth_count: Number of bootmeth devices in @bootmeth_order
 * @bootmeth_order: List of bootmeth devices to use, in order, NULL-terminated
 * @vbe_bootmeth: Currently selected VBE bootmeth, NULL if none
 */
struct bootstd_priv {
	const char **prefixes;
	const char **bootdev_order;
	struct udevice *cur_bootdev;
	struct bootflow *cur_bootflow;
	struct list_head glob_head;
	int bootmeth_count;
	struct udevice **bootmeth_order;
	struct udevice *vbe_bootmeth;
};

/**
 * bootstd_get_bootdev_order() - Get the boot-order list
 *
 * This reads the boot order, e.g. {"mmc0", "mmc2", NULL}
 *
 * The list is alloced by the bootstd driver so should not be freed. That is the
 * reason for all the const stuff in the function signature
 *
 * Return: list of string points, terminated by NULL; or NULL if no boot order
 */
const char *const *const bootstd_get_bootdev_order(struct udevice *dev);

/**
 * bootstd_get_prefixes() - Get the filename-prefixes list
 *
 * This reads the prefixes, e.g. {"/", "/bpot", NULL}
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
 * bootstd_clear_glob() - Clear the global list of bootflows
 *
 * This removes all bootflows globally and across all bootdevs.
 */
void bootstd_clear_glob(void);

#endif
