/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CLK_H_
#define _CLK_H_

#include <linux/types.h>

/**
 * A clock is a hardware signal that oscillates autonomously at a specific
 * frequency and duty cycle. Most hardware modules require one or more clock
 * signal to drive their operation. Clock signals are typically generated
 * externally to the HW module consuming them, by an entity this API calls a
 * clock provider. This API provides a standard means for drivers to enable and
 * disable clocks, and to set the rate at which they oscillate.
 *
 * A driver that implements UCLASS_CLOCK is a clock provider. A provider will
 * often implement multiple separate clocks, since the hardware it manages
 * often has this capability. clock_uclass.h describes the interface which
 * clock providers must implement.
 *
 * Clock consumers/clients are the HW modules driven by the clock signals. This
 * header file describes the API used by drivers for those HW modules.
 */

struct udevice;

/**
 * struct clk - A handle to (allowing control of) a single clock.
 *
 * Clients provide storage for clock handles. The content of the structure is
 * managed solely by the clock API and clock drivers. A clock struct is
 * initialized by "get"ing the clock struct. The clock struct is passed to all
 * other clock APIs to identify which clock signal to operate upon.
 *
 * @dev: The device which implements the clock signal.
 * @id: The clock signal ID within the provider.
 *
 * Currently, the clock API assumes that a single integer ID is enough to
 * identify and configure any clock signal for any clock provider. If this
 * assumption becomes invalid in the future, the struct could be expanded to
 * either (a) add more fields to allow clock providers to store additional
 * information, or (b) replace the id field with an opaque pointer, which the
 * provider would dynamically allocated during its .of_xlate op, and process
 * during is .request op. This may require the addition of an extra op to clean
 * up the allocation.
 */
struct clk {
	struct udevice *dev;
	/*
	 * Written by of_xlate. We assume a single id is enough for now. In the
	 * future, we might add more fields here.
	 */
	unsigned long id;
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
struct phandle_2_cell;
int clk_get_by_index_platdata(struct udevice *dev, int index,
			      struct phandle_2_cell *cells, struct clk *clk);

/**
 * clock_get_by_index - Get/request a clock by integer index.
 *
 * This looks up and requests a clock. The index is relative to the client
 * device; each device is assumed to have n clocks associated with it somehow,
 * and this function finds and requests one of them. The mapping of client
 * device clock indices to provider clocks may be via device-tree properties,
 * board-provided mapping tables, or some other mechanism.
 *
 * @dev:	The client device.
 * @index:	The index of the clock to request, within the client's list of
 *		clocks.
 * @clock	A pointer to a clock struct to initialize.
 * @return 0 if OK, or a negative error code.
 */
int clk_get_by_index(struct udevice *dev, int index, struct clk *clk);

/**
 * clock_get_by_name - Get/request a clock by name.
 *
 * This looks up and requests a clock. The name is relative to the client
 * device; each device is assumed to have n clocks associated with it somehow,
 * and this function finds and requests one of them. The mapping of client
 * device clock names to provider clocks may be via device-tree properties,
 * board-provided mapping tables, or some other mechanism.
 *
 * @dev:	The client device.
 * @name:	The name of the clock to request, within the client's list of
 *		clocks.
 * @clock:	A pointer to a clock struct to initialize.
 * @return 0 if OK, or a negative error code.
 */
int clk_get_by_name(struct udevice *dev, const char *name, struct clk *clk);
#else
static inline int clk_get_by_index(struct udevice *dev, int index,
				   struct clk *clk)
{
	return -ENOSYS;
}

static int clk_get_by_name(struct udevice *dev, const char *name,
			   struct clk *clk)
{
	return -ENOSYS;
}
#endif

/**
 * clk_request - Request a clock by provider-specific ID.
 *
 * This requests a clock using a provider-specific ID. Generally, this function
 * should not be used, since clk_get_by_index/name() provide an interface that
 * better separates clients from intimate knowledge of clock providers.
 * However, this function may be useful in core SoC-specific code.
 *
 * @dev:	The clock provider device.
 * @clock:	A pointer to a clock struct to initialize. The caller must
 *		have already initialized any field in this struct which the
 *		clock provider uses to identify the clock.
 * @return 0 if OK, or a negative error code.
 */
int clk_request(struct udevice *dev, struct clk *clk);

/**
 * clock_free - Free a previously requested clock.
 *
 * @clock:	A clock struct that was previously successfully requested by
 *		clk_request/get_by_*().
 * @return 0 if OK, or a negative error code.
 */
int clk_free(struct clk *clk);

/**
 * clk_get_rate() - Get current clock rate.
 *
 * @clk:	A clock struct that was previously successfully requested by
 *		clk_request/get_by_*().
 * @return clock rate in Hz, or -ve error code.
 */
ulong clk_get_rate(struct clk *clk);

/**
 * clk_set_rate() - Set current clock rate.
 *
 * @clk:	A clock struct that was previously successfully requested by
 *		clk_request/get_by_*().
 * @rate:	New clock rate in Hz.
 * @return new rate, or -ve error code.
 */
ulong clk_set_rate(struct clk *clk, ulong rate);

/**
 * clk_enable() - Enable (turn on) a clock.
 *
 * @clk:	A clock struct that was previously successfully requested by
 *		clk_request/get_by_*().
 * @return zero on success, or -ve error code.
 */
int clk_enable(struct clk *clk);

/**
 * clk_disable() - Disable (turn off) a clock.
 *
 * @clk:	A clock struct that was previously successfully requested by
 *		clk_request/get_by_*().
 * @return zero on success, or -ve error code.
 */
int clk_disable(struct clk *clk);

int soc_clk_dump(void);

#endif
