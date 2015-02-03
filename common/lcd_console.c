/*
 * (C) Copyright 2001-2014
 * DENX Software Engineering -- wd@denx.de
 * Compulab Ltd - http://compulab.co.il/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <video_font.h>		/* Get font data, width and height */

#define CONSOLE_ROW_SIZE	(VIDEO_FONT_HEIGHT * lcd_line_length)
#define CONSOLE_ROW_FIRST	lcd_console_address
#define CONSOLE_SIZE		(CONSOLE_ROW_SIZE * console_rows)

static short console_curr_col;
static short console_curr_row;
static short console_cols;
static short console_rows;
static void *lcd_console_address;

void lcd_init_console(void *address, int rows, int cols)
{
	console_curr_col = 0;
	console_curr_row = 0;
	console_cols = cols;
	console_rows = rows;
	lcd_console_address = address;
}

void lcd_set_col(short col)
{
	console_curr_col = col;
}

void lcd_set_row(short row)
{
	console_curr_row = row;
}

void lcd_position_cursor(unsigned col, unsigned row)
{
	console_curr_col = min_t(short, col, console_cols - 1);
	console_curr_row = min_t(short, row, console_rows - 1);
}

int lcd_get_screen_rows(void)
{
	return console_rows;
}

int lcd_get_screen_columns(void)
{
	return console_cols;
}

static void lcd_drawchars(ushort x, ushort y, uchar *str, int count)
{
	uchar *dest;
	ushort row;
	int fg_color, bg_color;

	dest = (uchar *)(lcd_console_address +
			 y * lcd_line_length + x * NBITS(LCD_BPP) / 8);

	for (row = 0; row < VIDEO_FONT_HEIGHT; ++row, dest += lcd_line_length) {
		uchar *s = str;
		int i;
#if LCD_BPP == LCD_COLOR16
		ushort *d = (ushort *)dest;
#elif LCD_BPP == LCD_COLOR32
		u32 *d = (u32 *)dest;
#else
		uchar *d = dest;
#endif

		fg_color = lcd_getfgcolor();
		bg_color = lcd_getbgcolor();
		for (i = 0; i < count; ++i) {
			uchar c, bits;

			c = *s++;
			bits = video_fontdata[c * VIDEO_FONT_HEIGHT + row];

			for (c = 0; c < 8; ++c) {
				*d++ = (bits & 0x80) ? fg_color : bg_color;
				bits <<= 1;
			}
		}
	}
}

static inline void lcd_putc_xy(ushort x, ushort y, uchar c)
{
	lcd_drawchars(x, y, &c, 1);
}

static void console_scrollup(void)
{
	const int rows = CONFIG_CONSOLE_SCROLL_LINES;
	int bg_color = lcd_getbgcolor();

	/* Copy up rows ignoring those that will be overwritten */
	memcpy(CONSOLE_ROW_FIRST,
	       lcd_console_address + CONSOLE_ROW_SIZE * rows,
	       CONSOLE_SIZE - CONSOLE_ROW_SIZE * rows);

	/* Clear the last rows */
#if (LCD_BPP != LCD_COLOR32)
	memset(lcd_console_address + CONSOLE_SIZE - CONSOLE_ROW_SIZE * rows,
	       bg_color, CONSOLE_ROW_SIZE * rows);
#else
	u32 *ppix = lcd_console_address +
		    CONSOLE_SIZE - CONSOLE_ROW_SIZE * rows;
	u32 i;
	for (i = 0;
	    i < (CONSOLE_ROW_SIZE * rows) / NBYTES(panel_info.vl_bpix);
	    i++) {
		*ppix++ = bg_color;
	}
#endif
	lcd_sync();
	console_curr_row -= rows;
}

static inline void console_back(void)
{
	if (--console_curr_col < 0) {
		console_curr_col = console_cols - 1;
		if (--console_curr_row < 0)
			console_curr_row = 0;
	}

	lcd_putc_xy(console_curr_col * VIDEO_FONT_WIDTH,
		    console_curr_row * VIDEO_FONT_HEIGHT, ' ');
}

static inline void console_newline(void)
{
	console_curr_col = 0;

	/* Check if we need to scroll the terminal */
	if (++console_curr_row >= console_rows)
		console_scrollup();
	else
		lcd_sync();
}

void lcd_putc(const char c)
{
	if (!lcd_is_enabled) {
		serial_putc(c);

		return;
	}

	switch (c) {
	case '\r':
		console_curr_col = 0;

		return;
	case '\n':
		console_newline();

		return;
	case '\t':	/* Tab (8 chars alignment) */
		console_curr_col +=  8;
		console_curr_col &= ~7;

		if (console_curr_col >= console_cols)
			console_newline();

		return;
	case '\b':
		console_back();

		return;
	default:
		lcd_putc_xy(console_curr_col * VIDEO_FONT_WIDTH,
			    console_curr_row * VIDEO_FONT_HEIGHT, c);
		if (++console_curr_col >= console_cols)
			console_newline();
	}
}

void lcd_puts(const char *s)
{
	if (!lcd_is_enabled) {
		serial_puts(s);

		return;
	}

	while (*s)
		lcd_putc(*s++);

	lcd_sync();
}

void lcd_printf(const char *fmt, ...)
{
	va_list args;
	char buf[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	lcd_puts(buf);
}

static int do_lcd_setcursor(cmd_tbl_t *cmdtp, int flag, int argc,
			    char *const argv[])
{
	unsigned int col, row;

	if (argc != 3)
		return CMD_RET_USAGE;

	col = simple_strtoul(argv[1], NULL, 10);
	row = simple_strtoul(argv[2], NULL, 10);
	lcd_position_cursor(col, row);

	return 0;
}

static int do_lcd_puts(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	lcd_puts(argv[1]);

	return 0;
}

U_BOOT_CMD(
	setcurs, 3,	1,	do_lcd_setcursor,
	"set cursor position within screen",
	"    <col> <row> in character"
);

U_BOOT_CMD(
	lcdputs, 2,	1,	do_lcd_puts,
	"print string on lcd-framebuffer",
	"    <string>"
);

