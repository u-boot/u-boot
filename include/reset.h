/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _RESET_H
#define _RESET_H

#include <linux/errno.h>

/**
 * A reset is a hardware signal indicating that a HW module (or IP block, or
 * sometimes an entire off-CPU chip) reset all of its internal state to some
 * known-good initial state. Drivers will often reset HW modules when they
 * begin execution to ensure that hardware correctly responds to all requests,
 * or in response to some error condition. Reset signals are often controlled
 * externally to the HW module being reset, by an entity this API calls a reset
 * controller. This API provides a standard means for drivers to request that
 * reset controllers set or clear reset signals.
 *
 * A driver that implements UCLASS_RESET is a reset controller or provider. A
 * controller will often implement multiple separate reset signals, since the
 * hardware it manages often has this capability. reset-uclass.h describes the
 * interface which reset controllers must implement.
 *
 * Reset consumers/clients are the HW modules affected by reset signals. This
 * header file describes the API used by drivers for those HW modules.
 */

struct udevice;

/**
 * struct reset_ctl - A handle to (allowing control of) a single reset signal.
 *
 * Clients provide storage for reset control handles. The content of the
 * structure is managed solely by the reset API and reset drivers. A reset
 * control struct is initialized by "get"ing the reset control struct. The
 * reset control struct is passed to all other reset APIs to identify which
 * reset signal to operate upon.
 *
 * @dev: The device which implements the reset signal.
 * @id: The reset signal ID within the provider.
 *
 * Currently, the reset API assumes that a single integer ID is enough to
 * identify and configure any reset signal for any reset provider. If this
 * assumption becomes invalid in the future, the struct could be expanded to
 * either (a) add more fields to allow reset providers to store additional
 * information, or (b) replace the id field with an opaque pointer, which the
 * provider would dynamically allocated during its .of_xlate op, and process
 * during is .request op. This may require the addition of an extra op to clean
 * up the allocation.
 */
struct reset_ctl {
	struct udevice *dev;
	/*
	 * Written by of_xlate. We assume a single id is enough for now. In the
	 * future, we might add more fields here.
	 */
	unsigned long id;
};

#ifdef CONFIG_DM_RESET
/**
 * reset_get_by_index - Get/request a reset signal by integer index.
 *
 * This looks up and requests a reset signal. The index is relative to the
 * client device; each device is assumed to have n reset signals associated
 * with it somehow, and this function finds and requests one of them. The
 * mapping of client device reset signal indices to provider reset signals may
 * be via device-tree properties, board-provided mapping tables, or some other
 * mechanism.
 *
 * @dev:	The client device.
 * @index:	The index of the reset signal to request, within the client's
 *		list of reset signals.
 * @reset_ctl	A pointer to a reset control struct to initialize.
 * @return 0 if OK, or a negative error code.
 */
int reset_get_by_index(struct udevice *dev, int index,
		       struct reset_ctl *reset_ctl);

/**
 * reset_get_by_name - Get/request a reset signal by name.
 *
 * This looks up and requests a reset signal. The name is relative to the
 * client device; each device is assumed to have n reset signals associated
 * with it somehow, and this function finds and requests one of them. The
 * mapping of client device reset signal names to provider reset signal may be
 * via device-tree properties, board-provided mapping tables, or some other
 * mechanism.
 *
 * @dev:	The client device.
 * @name:	The name of the reset signal to request, within the client's
 *		list of reset signals.
 * @reset_ctl:	A pointer to a reset control struct to initialize.
 * @return 0 if OK, or a negative error code.
 */
int reset_get_by_name(struct udevice *dev, const char *name,
		      struct reset_ctl *reset_ctl);

/**
 * reset_free - Free a previously requested reset signal.
 *
 * @reset_ctl:	A reset control struct that was previously successfully
 *		requested by reset_get_by_*().
 * @return 0 if OK, or a negative error code.
 */
int reset_free(struct reset_ctl *reset_ctl);

/**
 * reset_assert - Assert a reset signal.
 *
 * This function will assert the specified reset signal, thus resetting the
 * affected HW module(s). Depending on the reset controller hardware, the reset
 * signal will either stay asserted until reset_deassert() is called, or the
 * hardware may autonomously clear the reset signal itself.
 *
 * @reset_ctl:	A reset control struct that was previously successfully
 *		requested by reset_get_by_*().
 * @return 0 if OK, or a negative error code.
 */
int reset_assert(struct reset_ctl *reset_ctl);

/**
 * reset_deassert - Deassert a reset signal.
 *
 * This function will deassert the specified reset signal, thus releasing the
 * affected HW modules() from reset, and allowing them to continue normal
 * operation.
 *
 * @reset_ctl:	A reset control struct that was previously successfully
 *		requested by reset_get_by_*().
 * @return 0 if OK, or a negative error code.
 */
int reset_deassert(struct reset_ctl *reset_ctl);

#else
static inline int reset_get_by_index(struct udevice *dev, int index,
				     struct reset_ctl *reset_ctl)
{
	return -ENOTSUPP;
}

static inline int reset_get_by_name(struct udevice *dev, const char *name,
				    struct reset_ctl *reset_ctl)
{
	return -ENOTSUPP;
}

static inline int reset_free(struct reset_ctl *reset_ctl)
{
	return 0;
}

static inline int reset_assert(struct reset_ctl *reset_ctl)
{
	return 0;
}

static inline int reset_deassert(struct reset_ctl *reset_ctl)
{
	return 0;
}
#endif

#endif
