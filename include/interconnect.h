/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025 Linaro Limited
 */

#ifndef _INTERCONNECT_H
#define _INTERCONNECT_H

#include <linux/errno.h>

struct udevice;

/* macros for converting to icc units */
#define Bps_to_icc(x)	((x) / 1000)
#define kBps_to_icc(x)	(x)
#define MBps_to_icc(x)	((x) * 1000)
#define GBps_to_icc(x)	((x) * 1000 * 1000)
#define bps_to_icc(x)	(1)
#define kbps_to_icc(x)	((x) / 8 + ((x) % 8 ? 1 : 0))
#define Mbps_to_icc(x)	((x) * 1000 / 8)
#define Gbps_to_icc(x)	((x) * 1000 * 1000 / 8)

struct icc_path;

/**
 * of_icc_get - Get an Interconnect path from a DT node based on name
 *
 * This function will search for a path between two endpoints and return an
 * icc_path handle on success. Use icc_put() to release constraints when they
 * are not needed anymore.
 * If the interconnect API is disabled, NULL is returned and the consumer
 * drivers will still build. Drivers are free to handle this specifically,
 * but they don't have to.
 *
 * @dev:	The client device.
 * @name:	Name of the interconnect endpoint pair.
 * Return:	icc_path pointer on success or ERR_PTR() on error. NULL is returned
 * when the API is disabled or the "interconnects" DT property is missing.
 */
#if CONFIG_IS_ENABLED(INTERCONNECT)
struct icc_path *of_icc_get(struct udevice *dev, const char *name);
#else
static inline
struct icc_path *of_icc_get(struct udevice *dev, const char *name)
{
	return NULL;
}
#endif

/**
 * of_icc_get - Get an Interconnect path from a DT node based on index
 *
 * This function will search for a path between two endpoints and return an
 * icc_path handle on success. Use icc_put() to release constraints when they
 * are not needed anymore.
 * If the interconnect API is disabled, NULL is returned and the consumer
 * drivers will still build. Drivers are free to handle this specifically,
 * but they don't have to.
 *
 * @dev:	The client device.
 * @idx:	Index of the interconnect endpoint pair.
 * Return:	icc_path pointer on success or ERR_PTR() on error. NULL is returned
 * when the API is disabled or the "interconnects" DT property is missing.
 */
#if CONFIG_IS_ENABLED(INTERCONNECT)
struct icc_path *of_icc_get_by_index(struct udevice *dev, int idx);
#else
static inline
struct icc_path *of_icc_get_by_index(struct udevice *dev, int idx)
{
	return NULL;
}
#endif

/**
 * icc_put - release the reference to the Interconnect path.
 *
 * Use this function to release the constraints on a path when the path is
 * no longer needed. The constraints will be re-aggregated.
 *
 * @path:	An interconnect path
 * Return: 0 if OK, or a negative error code.
 */
#if CONFIG_IS_ENABLED(INTERCONNECT)
int icc_put(struct icc_path *path);
#else
static inline int icc_put(struct icc_path *path)
{
	return 0;
}
#endif

/**
 * icc_enable - Enable an Interconnect path.
 *
 * This will enable all the endpoints in the path, using the
 * bandwidth set by the `icc_set_bw()` call. Otherwise a zero
 * bandwidth will be set. Usually used after a call to `icc_disable()`.
 *
 * @path:	An interconnect path
 * Return: 0 if OK, or a negative error code. -ENOSYS if not implemented.
 */
#if CONFIG_IS_ENABLED(INTERCONNECT)
int icc_enable(struct icc_path *path);
#else
static inline int icc_enable(struct icc_path *path)
{
	return -ENOSYS;
}
#endif

/**
 * icc_disable - Disable an Interconnect path.
 *
 * This will disable all the endpoints in the path, effectively setting
 * a zero bandwidth. Calling `icc_enable()` will restore the bandwidth set
 * by calling `icc_set_bw()`.
 *
 * @path:	An interconnect path
 * Return: 0 if OK, or a negative error code. -ENOSYS if not implemented.
 */
#if CONFIG_IS_ENABLED(INTERCONNECT)
int icc_disable(struct icc_path *path);
#else
static inline int icc_disable(struct icc_path *path)
{
	return -ENOSYS;
}
#endif

/**
 * icc_set_bw - set bandwidth constraints on an interconnect path.
 *
 * This function is used by an interconnect consumer to express its own needs
 * in terms of bandwidth for a previously requested path between two endpoints.
 * The requests are aggregated and each node is updated accordingly. The entire
 * path is locked by a mutex to ensure that the set() is completed.
 * The @path can be NULL when the "interconnects" DT properties is missing,
 * which will mean that no constraints will be set.
 *
 * @path:	An interconnect path
 * @avg_bw:	Average bandwidth request in kBps
 * @peak_bw:	Peak bandwidth in request kBps
 * Return: 0 if OK, or a negative error code. -ENOSYS if not implemented.
 */
#if CONFIG_IS_ENABLED(INTERCONNECT)
int icc_set_bw(struct icc_path *path, u32 avg_bw, u32 peak_bw);
#else
static inline int icc_set_bw(struct icc_path *path, u32 avg_bw, u32 peak_bw)
{
	return -ENOSYS;
}
#endif

#endif
