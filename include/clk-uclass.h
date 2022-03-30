/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef _CLK_UCLASS_H
#define _CLK_UCLASS_H

/* See clk.h for background documentation. */

#include <clk.h>

struct ofnode_phandle_args;

/**
 * struct clk_ops - The functions that a clock driver must implement.
 * @of_xlate: Translate a client's device-tree (OF) clock specifier.
 * @request: Request a translated clock.
 * @rfree: Free a previously requested clock.
 * @round_rate: Adjust a rate to the exact rate a clock can provide.
 * @get_rate: Get current clock rate.
 * @set_rate: Set current clock rate.
 * @set_parent: Set current clock parent
 * @enable: Enable a clock.
 * @disable: Disable a clock.
 *
 * The individual methods are described more fully below.
 */
struct clk_ops {
	int (*of_xlate)(struct clk *clock,
			struct ofnode_phandle_args *args);
	int (*request)(struct clk *clock);
	void (*rfree)(struct clk *clock);
	ulong (*round_rate)(struct clk *clk, ulong rate);
	ulong (*get_rate)(struct clk *clk);
	ulong (*set_rate)(struct clk *clk, ulong rate);
	int (*set_parent)(struct clk *clk, struct clk *parent);
	int (*enable)(struct clk *clk);
	int (*disable)(struct clk *clk);
};

#if 0 /* For documentation only */
/**
 * of_xlate() - Translate a client's device-tree (OF) clock specifier.
 * @clock:	The clock struct to hold the translation result.
 * @args:	The clock specifier values from device tree.
 *
 * The clock core calls this function as the first step in implementing
 * a client's clk_get_by_*() call.
 *
 * If this function pointer is set to NULL, the clock core will use a
 * default implementation, which assumes #clock-cells = <1>, and that
 * the DT cell contains a simple integer clock ID.
 *
 * At present, the clock API solely supports device-tree. If this
 * changes, other xxx_xlate() functions may be added to support those
 * other mechanisms.
 *
 * Return: 0 if OK, or a negative error code.
 */
int of_xlate(struct clk *clock, struct ofnode_phandle_args *args);

/**
 * request() - Request a translated clock.
 * @clock:	The clock struct to request; this has been fille in by
 *		a previoux xxx_xlate() function call, or by the caller
 *		of clk_request().
 *
 * The clock core calls this function as the second step in
 * implementing a client's clk_get_by_*() call, following a successful
 * xxx_xlate() call, or as the only step in implementing a client's
 * clk_request() call.
 *
 * Return: 0 if OK, or a negative error code.
 */
int request(struct clk *clock);

/**
 * rfree() - Free a previously requested clock.
 * @clock:	The clock to free.
 *
 * Free any resources allocated in request().
 */
void rfree(struct clk *clock);

/**
 * round_rate() - Adjust a rate to the exact rate a clock can provide.
 * @clk:	The clock to manipulate.
 * @rate:	Desidered clock rate in Hz.
 *
 * Return: rounded rate in Hz, or -ve error code.
 */
ulong round_rate(struct clk *clk, ulong rate);

/**
 * get_rate() - Get current clock rate.
 * @clk:	The clock to query.
 *
 * Return: clock rate in Hz, or -ve error code
 */
ulong get_rate(struct clk *clk);

/**
 * set_rate() - Set current clock rate.
 * @clk:	The clock to manipulate.
 * @rate:	New clock rate in Hz.
 *
 * Return: new rate, or -ve error code.
 */
ulong set_rate(struct clk *clk, ulong rate);

/**
 * set_parent() - Set current clock parent
 * @clk:        The clock to manipulate.
 * @parent:     New clock parent.
 *
 * Return: zero on success, or -ve error code.
 */
int set_parent(struct clk *clk, struct clk *parent);

/**
 * enable() - Enable a clock.
 * @clk:	The clock to manipulate.
 *
 * Return: zero on success, or -ve error code.
 */
int enable(struct clk *clk);

/**
 * disable() - Disable a clock.
 * @clk:	The clock to manipulate.
 *
 * Return: zero on success, or -ve error code.
 */
int disable(struct clk *clk);
#endif

#endif
