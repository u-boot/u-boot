/*
 * (C) Copyright 2004
 * Pantelis Antoniou, Intracom S.A. , panto@intracom.gr
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

/*
 * DSP test
 *
 * This test verifies the connection and performs a memory test
 * on any connected DSP(s). The meat of the work is done
 * in the board specific function.
 */

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_DSP

extern int board_post_dsp(int flags);

int dsp_post_test (int flags)
{
	return board_post_dsp(flags);
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_DSP */
