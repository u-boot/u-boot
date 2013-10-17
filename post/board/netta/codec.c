/*
 * (C) Copyright 2004
 * Pantelis Antoniou, Intracom S.A. , panto@intracom.gr
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

/*
 * CODEC test
 *
 * This test verifies the connection and performs a memory test
 * on any connected codec(s). The meat of the work is done
 * in the board specific function.
 */

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_CODEC

extern int board_post_codec(int flags);

int codec_post_test (int flags)
{
	return board_post_codec(flags);
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_CODEC */
