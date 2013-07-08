/*
 * Copyright 2010 Sergey Poselenov, Emcraft Systems, <sposelenov@emcraft.com>
 * Copyright 2010 Ilya Yanok, Emcraft Systems, <yanok@emcraft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <led-display.h>
#include <asm/io.h>

#ifdef CONFIG_CMD_DISPLAY
#define CWORD_CLEAR	0x80
#define CLEAR_DELAY	(110 * 2)
#define DISPLAY_SIZE	8

static int pos; /* Current display position */

/* Handle different display commands */
void display_set(int cmd)
{
	if (cmd & DISPLAY_CLEAR) {
		out_8((unsigned char *)CONFIG_SYS_DISP_CWORD, CWORD_CLEAR);
		udelay(1000 * CLEAR_DELAY);
	}

	if (cmd & DISPLAY_HOME) {
		pos = 0;
	}
}

/*
 * Display a character at the current display position.
 * Characters beyond the display size are ignored.
 */
int display_putc(char c)
{
	if (pos >= DISPLAY_SIZE)
		return -1;

	out_8((unsigned char *)CONFIG_SYS_DISP_CHR_RAM + pos++, c);

	return c;
}
#endif
