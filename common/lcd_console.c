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
#define CONSOLE_ROW_FIRST	cons.lcd_address
#define CONSOLE_SIZE		(CONSOLE_ROW_SIZE * cons.rows)

struct console_t {
	short curr_col, curr_row;
	short cols, rows;
	void *lcd_address;
};
static struct console_t cons;

void lcd_init_console(void *address, int rows, int cols)
{
	memset(&cons, 0, sizeof(cons));
	cons.cols = cols;
	cons.rows = rows;
	cons.lcd_address = address;

}

void lcd_set_col(short col)
{
	cons.curr_col = col;
}

void lcd_set_row(short row)
{
	cons.curr_row = row;
}

void lcd_position_cursor(unsigned col, unsigned row)
{
	cons.curr_col = min_t(short, col, cons.cols - 1);
	cons.curr_row = min_t(short, row, cons.rows - 1);
}

int lcd_get_screen_rows(void)
{
	return cons.rows;
}

int lcd_get_screen_columns(void)
{
	return cons.cols;
}

static void lcd_putc_xy(ushort x, ushort y, char c)
{
	uchar *dest;
	ushort row;
	int fg_color = lcd_getfgcolor();
	int bg_color = lcd_getbgcolor();
	int i;

	dest = (uchar *)(cons.lcd_address +
			 y * lcd_line_length + x * NBITS(LCD_BPP) / 8);

	for (row = 0; row < VIDEO_FONT_HEIGHT; ++row, dest += lcd_line_length) {
#if LCD_BPP == LCD_COLOR16
		ushort *d = (ushort *)dest;
#elif LCD_BPP == LCD_COLOR32
		u32 *d = (u32 *)dest;
#else
		uchar *d = dest;
#endif
		uchar bits;
		bits = video_fontdata[c * VIDEO_FONT_HEIGHT + row];

		for (i = 0; i < 8; ++i) {
			*d++ = (bits & 0x80) ? fg_color : bg_color;
			bits <<= 1;
		}
	}
}

static void console_scrollup(void)
{
	const int rows = CONFIG_CONSOLE_SCROLL_LINES;
	int bg_color = lcd_getbgcolor();

	/* Copy up rows ignoring those that will be overwritten */
	memcpy(CONSOLE_ROW_FIRST,
	       cons.lcd_address + CONSOLE_ROW_SIZE * rows,
	       CONSOLE_SIZE - CONSOLE_ROW_SIZE * rows);

	/* Clear the last rows */
#if (LCD_BPP != LCD_COLOR32)
	memset(lcd_console_address + CONSOLE_SIZE - CONSOLE_ROW_SIZE * rows,
	       bg_color, CONSOLE_ROW_SIZE * rows);
#else
	u32 *ppix = cons.lcd_address +
		    CONSOLE_SIZE - CONSOLE_ROW_SIZE * rows;
	u32 i;
	for (i = 0;
	    i < (CONSOLE_ROW_SIZE * rows) / NBYTES(panel_info.vl_bpix);
	    i++) {
		*ppix++ = bg_color;
	}
#endif
	lcd_sync();
	cons.curr_row -= rows;
}

static inline void console_back(void)
{
	if (--cons.curr_col < 0) {
		cons.curr_col = cons.cols - 1;
		if (--cons.curr_row < 0)
			cons.curr_row = 0;
	}

	lcd_putc_xy(cons.curr_col * VIDEO_FONT_WIDTH,
		    cons.curr_row * VIDEO_FONT_HEIGHT, ' ');
}

static inline void console_newline(void)
{
	cons.curr_col = 0;

	/* Check if we need to scroll the terminal */
	if (++cons.curr_row >= cons.rows)
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
		cons.curr_col = 0;
		return;
	case '\n':
		console_newline();

		return;
	case '\t':	/* Tab (8 chars alignment) */
		cons.curr_col +=  8;
		cons.curr_col &= ~7;

		if (cons.curr_col >= cons.cols)
			console_newline();

		return;
	case '\b':
		console_back();

		return;
	default:
		lcd_putc_xy(cons.curr_col * VIDEO_FONT_WIDTH,
			    cons.curr_row * VIDEO_FONT_HEIGHT, c);
		if (++cons.curr_col >= cons.cols)
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

