/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CPU_H
#define __CPU_H

/**
 * struct cpu_platdata - platform data for a CPU
 *
 * This can be accessed with dev_get_parent_platdata() for any UCLASS_CPU
 * device.
 *
 * @cpu_id:	Platform-specific way of identifying the CPU.
 */
struct cpu_platdata {
	int cpu_id;
};

/* CPU features - mostly just a placeholder for now */
enum {
	CPU_FEAT_L1_CACHE	= 0,	/* Supports level 1 cache */
	CPU_FEAT_MMU		= 1,	/* Supports virtual memory */

	CPU_FEAT_COUNT,
};

/**
 * struct cpu_info - Information about a CPU
 *
 * @cpu_freq:	Current CPU frequency in Hz
 * @features:	Flags for supported CPU features
 */
struct cpu_info {
	ulong cpu_freq;
	ulong features;
};

struct cpu_ops {
	/**
	 * get_desc() - Get a description string for a CPU
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @buf:	Buffer to place string
	 * @size:	Size of string space
	 * @return 0 if OK, -ENOSPC if buffer is too small, other -ve on error
	 */
	int (*get_desc)(struct udevice *dev, char *buf, int size);

	/**
	 * get_info() - Get information about a CPU
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @info:	Returns CPU info
	 * @return 0 if OK, -ve on error
	 */
	int (*get_info)(struct udevice *dev, struct cpu_info *info);
};

#define cpu_get_ops(dev)        ((struct cpu_ops *)(dev)->driver->ops)

/**
 * cpu_get_desc() - Get a description string for a CPU
 *
 * @dev:	Device to check (UCLASS_CPU)
 * @buf:	Buffer to place string
 * @size:	Size of string space
 * @return 0 if OK, -ENOSPC if buffer is too small, other -ve on error
 */
int cpu_get_desc(struct udevice *dev, char *buf, int size);

/**
 * cpu_get_info() - Get information about a CPU
 *
 * @dev:	Device to check (UCLASS_CPU)
 * @info:	Returns CPU info
 * @return 0 if OK, -ve on error
 */
int cpu_get_info(struct udevice *dev, struct cpu_info *info);

#endif
