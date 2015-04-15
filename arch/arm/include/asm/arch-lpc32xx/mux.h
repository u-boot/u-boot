/*
 * LPC32xx MUX interface
 *
 * (C) Copyright 2015  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/**
 * MUX register map for LPC32xx
 */

struct mux_regs {
	u32 p_mux_set;
	u32 p_mux_clr;
	u32 p_mux_state;
};
