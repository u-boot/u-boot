/*
 * (C) Copyright 2014
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __STV0991_DEFS_H__
#define __STV0991_DEFS_H__
#include <asm/arch/stv0991_periph.h>

extern int stv0991_pinmux_config(enum periph_id);
extern int clock_setup(enum periph_clock);

#endif

