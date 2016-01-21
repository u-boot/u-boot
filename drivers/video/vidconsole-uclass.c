/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2001-2015
 * DENX Software Engineering -- wd@denx.de
 * Compulab Ltd - http://compulab.co.il/
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <video.h>
#include <video_console.h>
#include <video_font.h>		/* Get font data, width and height */

/* By default we scroll by a single line */
#ifndef CONFIG_CONSOLE_SCROLL_LINES
#define CONFIG_CONSOLE_SCROLL_LINES 1
#endif

int vidconsole_putc_xy(struct udevice *dev, uint x, uint y, char ch)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->putc_xy)
		return -ENOSYS;
	return ops->putc_xy(dev, x, y, ch);
}

int vidconsole_move_rows(struct udevice *dev, uint rowdst, uint rowsrc,
			 uint count)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->move_rows)
		return -ENOSYS;
	return ops->move_rows(dev, rowdst, rowsrc, count);
}

int vidconsole_set_row(struct udevice *dev, uint row, int clr)
{
	struct vidconsole_ops *ops = vidconsole_get_ops(dev);

	if (!ops->set_row)
		return -ENOSYS;
	return ops->set_row(dev, row, clr);
}

/* Move backwards one space */
static void vidconsole_back(struct udevice *dev)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);

	if (--priv->curr_col < 0) {
		priv->curr_col = priv->cols - 1;
		if (--priv->curr_row < 0)
			priv->curr_row = 0;
	}

	vidconsole_putc_xy(dev, priv->curr_col * VIDEO_FONT_WIDTH,
			   priv->curr_row * VIDEO_FONT_HEIGHT, ' ');
}

/* Move to a newline, scrolling the display if necessary */
static void vidconsole_newline(struct udevice *dev)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);
	const int rows = CONFIG_CONSOLE_SCROLL_LINES;
	int i;

	priv->curr_col = 0;

	/* Check if we need to scroll the terminal */
	if (++priv->curr_row >= priv->rows) {
		vidconsole_move_rows(dev, 0, rows, priv->rows - rows);
		for (i = 0; i < rows; i++)
			vidconsole_set_row(dev, priv->rows - i - 1,
					   vid_priv->colour_bg);
		priv->curr_row -= rows;
	}
	video_sync(dev->parent);
}

int vidconsole_put_char(struct udevice *dev, char ch)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	int ret;

	switch (ch) {
	case '\r':
		priv->curr_col = 0;
		break;
	case '\n':
		vidconsole_newline(dev);
		break;
	case '\t':	/* Tab (8 chars alignment) */
		priv->curr_col +=  8;
		priv->curr_col &= ~7;

		if (priv->curr_col >= priv->cols)
			vidconsole_newline(dev);
		break;
	case '\b':
		vidconsole_back(dev);
		break;
	default:
		/*
		 * Failure of this function normally indicates an unsupported
		 * colour depth. Check this and return an error to help with
		 * diagnosis.
		 */
		ret = vidconsole_putc_xy(dev,
					 priv->curr_col * VIDEO_FONT_WIDTH,
					 priv->curr_row * VIDEO_FONT_HEIGHT,
					 ch);
		if (ret)
			return ret;
		if (++priv->curr_col >= priv->cols)
			vidconsole_newline(dev);
		break;
	}

	return 0;
}

static void vidconsole_putc(struct stdio_dev *sdev, const char ch)
{
	struct udevice *dev = sdev->priv;

	vidconsole_put_char(dev, ch);
}

static void vidconsole_puts(struct stdio_dev *sdev, const char *s)
{
	struct udevice *dev = sdev->priv;

	while (*s)
		vidconsole_put_char(dev, *s++);
}

/* Set up the number of rows and colours (rotated drivers override this) */
static int vidconsole_pre_probe(struct udevice *dev)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);

	priv->rows = vid_priv->ysize / VIDEO_FONT_HEIGHT;
	priv->cols = vid_priv->xsize / VIDEO_FONT_WIDTH;

	return 0;
}

/* Register the device with stdio */
static int vidconsole_post_probe(struct udevice *dev)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);
	struct stdio_dev *sdev = &priv->sdev;
	int ret;

	strlcpy(sdev->name, dev->name, sizeof(sdev->name));
	sdev->flags = DEV_FLAGS_OUTPUT;
	sdev->putc = vidconsole_putc;
	sdev->puts = vidconsole_puts;
	sdev->priv = dev;
	ret = stdio_register(sdev);
	if (ret)
		return ret;

	return 0;
}

UCLASS_DRIVER(vidconsole) = {
	.id		= UCLASS_VIDEO_CONSOLE,
	.name		= "vidconsole0",
	.pre_probe	= vidconsole_pre_probe,
	.post_probe	= vidconsole_post_probe,
	.per_device_auto_alloc_size	= sizeof(struct vidconsole_priv),
};

void vidconsole_position_cursor(struct udevice *dev, unsigned col, unsigned row)
{
	struct vidconsole_priv *priv = dev_get_uclass_priv(dev);

	priv->curr_col = min_t(short, col, priv->cols - 1);
	priv->curr_row = min_t(short, row, priv->rows - 1);
}

static int do_video_setcursor(cmd_tbl_t *cmdtp, int flag, int argc,
			      char *const argv[])
{
	unsigned int col, row;
	struct udevice *dev;

	if (argc != 3)
		return CMD_RET_USAGE;

	uclass_first_device(UCLASS_VIDEO_CONSOLE, &dev);
	if (!dev)
		return CMD_RET_FAILURE;
	col = simple_strtoul(argv[1], NULL, 10);
	row = simple_strtoul(argv[2], NULL, 10);
	vidconsole_position_cursor(dev, col, row);

	return 0;
}

static int do_video_puts(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct udevice *dev;
	const char *s;

	if (argc != 2)
		return CMD_RET_USAGE;

	uclass_first_device(UCLASS_VIDEO_CONSOLE, &dev);
	if (!dev)
		return CMD_RET_FAILURE;
	for (s = argv[1]; *s; s++)
		vidconsole_put_char(dev, *s);

	return 0;
}

U_BOOT_CMD(
	setcurs, 3,	1,	do_video_setcursor,
	"set cursor position within screen",
	"    <col> <row> in character"
);

U_BOOT_CMD(
	lcdputs, 2,	1,	do_video_puts,
	"print string on video framebuffer",
	"    <string>"
);
