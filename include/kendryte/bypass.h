/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */
#ifndef K210_BYPASS_H
#define K210_BYPASS_H

struct clk;

struct k210_bypass {
	struct clk clk;
	struct clk **children; /* Clocks to reparent */
	struct clk **saved_parents; /* Parents saved over en-/dis-able */
	struct clk *bypassee; /* Clock to bypass */
	const struct clk_ops *bypassee_ops; /* Ops of the bypass clock */
	struct clk *alt; /* Clock to set children to when bypassing */
	size_t child_count;
};

#define to_k210_bypass(_clk) container_of(_clk, struct k210_bypass, clk)

int k210_bypass_set_children(struct clk *clk, struct clk **children,
			     size_t child_count);
struct clk *k210_register_bypass_struct(const char *name,
					const char *parent_name,
					struct k210_bypass *bypass);
struct clk *k210_register_bypass(const char *name, const char *parent_name,
				 struct clk *bypassee,
				 const struct clk_ops *bypassee_ops,
				 struct clk *alt);
#endif /* K210_BYPASS_H */
