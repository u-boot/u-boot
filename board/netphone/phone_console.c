/*
 * (C) Copyright 2004 Intracom S.A.
 * Pantelis Antoniou <panto@intracom.gr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * phone_console.c
 *
 * A phone based console
 *
 * Virtual display of 80x24 characters.
 * The actual display is much smaller and panned to show the virtual one.
 * Input is made by a numeric keypad utilizing the input method of
 * mobile phones. Sorry no T9 lexicons...
 *
 */

#include <common.h>

#include <version.h>
#include <linux/types.h>
#include <devices.h>

#include <sed156x.h>

/*************************************************************************************************/

#define ROWS	24
#define COLS	80

#define REFRESH_HZ		(CFG_HZ/50)	/* refresh every 20ms */
#define BLINK_HZ		(CFG_HZ/2)	/* cursor blink every 500ms */

/*************************************************************************************************/

#define DISPLAY_BACKLIT_PORT	((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pcdat
#define DISPLAY_BACKLIT_MASK	0x0010

/*************************************************************************************************/

#define KP_STABLE_HZ		(CFG_HZ/100)	/* stable for 10ms */
#define KP_REPEAT_DELAY_HZ	(CFG_HZ/4)	/* delay before repeat 250ms */
#define KP_REPEAT_HZ		(CFG_HZ/20)	/* repeat every 50ms */
#define KP_FORCE_DELAY_HZ	(CFG_HZ/2)	/* key was force pressed */
#define KP_IDLE_DELAY_HZ	(CFG_HZ/2)	/* key was released and idle */

#if CONFIG_NETPHONE_VERSION == 1
#define KP_SPI_RXD_PORT (((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pcdat)
#define KP_SPI_RXD_MASK 0x0008

#define KP_SPI_TXD_PORT (((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pcdat)
#define KP_SPI_TXD_MASK 0x0004

#define KP_SPI_CLK_PORT (((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pcdat)
#define KP_SPI_CLK_MASK 0x0001
#elif CONFIG_NETPHONE_VERSION == 2
#define KP_SPI_RXD_PORT (((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pbdat)
#define KP_SPI_RXD_MASK 0x00000008

#define KP_SPI_TXD_PORT (((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pbdat)
#define KP_SPI_TXD_MASK 0x00000004

#define KP_SPI_CLK_PORT (((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pbdat)
#define KP_SPI_CLK_MASK 0x00000002
#endif

#define KP_CS_PORT	(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat)
#define KP_CS_MASK	0x00000010

#define KP_SPI_RXD() (KP_SPI_RXD_PORT & KP_SPI_RXD_MASK)

#define KP_SPI_TXD(x) \
	do { \
		if (x) \
			KP_SPI_TXD_PORT |=  KP_SPI_TXD_MASK; \
		else \
			KP_SPI_TXD_PORT &= ~KP_SPI_TXD_MASK; \
	} while(0)

#define KP_SPI_CLK(x) \
	do { \
		if (x) \
			KP_SPI_CLK_PORT |=  KP_SPI_CLK_MASK; \
		else \
			KP_SPI_CLK_PORT &= ~KP_SPI_CLK_MASK; \
	} while(0)

#define KP_SPI_CLK_TOGGLE() (KP_SPI_CLK_PORT ^= KP_SPI_CLK_MASK)

#define KP_SPI_BIT_DELAY()	/* no delay */

#define KP_CS(x) \
	do { \
		if (x) \
			KP_CS_PORT |=  KP_CS_MASK; \
		else \
			KP_CS_PORT &= ~KP_CS_MASK; \
	} while(0)

#define KP_ROWS 7
#define KP_COLS 4

#define KP_ROWS_MASK	((1 << KP_ROWS) - 1)
#define KP_COLS_MASK	((1 << KP_COLS) - 1)

#define SCAN		0
#define SCAN_FILTER	1
#define SCAN_COL	2
#define SCAN_COL_FILTER 3
#define PRESSED		4

#define KP_F1	0	/* leftmost dot (tab)	*/
#define KP_F2	1	/* middle left dot	*/
#define KP_F3	2	/* up			*/
#define KP_F4	3	/* middle right dot	*/
#define KP_F5	4	/* rightmost dot	*/
#define KP_F6	5	/* C			*/
#define KP_F7	6	/* left			*/
#define KP_F8	7	/* down			*/
#define KP_F9	8	/* right		*/
#define KP_F10	9	/* enter		*/
#define KP_F11	10	/* R			*/
#define KP_F12	11	/* save			*/
#define KP_F13	12	/* redial		*/
#define KP_F14	13	/* speaker		*/
#define KP_F15	14	/* unused		*/
#define KP_F16	15	/* unused		*/

#define KP_RELEASE		-1	/* key depressed				*/
#define KP_FORCE		-2	/* key was pressed for more than force hz	*/
#define KP_IDLE			-3	/* key was released and idle			*/

#define KP_1	'1'
#define KP_2	'2'
#define KP_3	'3'
#define KP_4	'4'
#define KP_5	'5'
#define KP_6	'6'
#define KP_7	'7'
#define KP_8	'8'
#define KP_9	'9'
#define KP_0	'0'
#define KP_STAR '*'
#define KP_HASH '#'

/*************************************************************************************************/

static int curs_disabled;
static int curs_col, curs_row;
static int disp_col, disp_row;

static int width, height;

/* the simulated vty buffer */
static char vty_buf[ROWS * COLS];
static char last_visible_buf[ROWS * COLS];	/* worst case */
static char *last_visible_curs_ptr;
static int last_visible_curs_rev;
static int blinked_state;
static int last_input_mode;
static int refresh_time;
static int blink_time;
static char last_fast_punct;

/*************************************************************************************************/

#define IM_SMALL	0
#define IM_CAPITAL	1
#define IM_NUMBER	2

static int input_mode;
static char fast_punct;
static int tab_indicator;
static const char *fast_punct_list = ",.:;*";

static const char *input_mode_txt[] = { "abc", "ABC", "123" };

static const char *punct = ".,!;?'\"-()@/:_+&%*=<>$[]{}\\~^#|";
static const char *whspace = " 0\n";
/* per mode character select (for 2-9) */
static const char *digits_sel[2][8] = {
	{	/* small */
		"abc2",					/* 2 */
		"def3",					/* 3 */
		"ghi4",					/* 4 */
		"jkl5",					/* 5 */
		"mno6",					/* 6 */
		"pqrs7",				/* 7 */
		"tuv8",					/* 8 */
		"wxyz9",				/* 9 */
	}, {	/* capital */
		"ABC2",					/* 2 */
		"DEF3",					/* 3 */
		"GHI4",					/* 4 */
		"JKL5",					/* 5 */
		"MNO6",					/* 6 */
		"PQRS7",				/* 7 */
		"TUV8",					/* 8 */
		"WXYZ9",				/* 9 */
	}
};

/*****************************************************************************/

static void update(void);
static void ensure_visible(int col, int row, int dx, int dy);

static void console_init(void)
{
	curs_disabled = 0;
	curs_col = 0;
	curs_row = 0;

	disp_col = 0;
	disp_row = 0;

	input_mode = IM_SMALL;
	fast_punct = ',';
	last_fast_punct = '\0';
	refresh_time = REFRESH_HZ;
	blink_time = BLINK_HZ;

	memset(vty_buf, ' ', sizeof(vty_buf));

	memset(last_visible_buf, ' ', sizeof(last_visible_buf));
	last_visible_curs_ptr = NULL;
	last_input_mode = -1;
	last_visible_curs_rev = 0;

	blinked_state = 0;

	sed156x_init();
	width = sed156x_text_width;
	height = sed156x_text_height - 1;

	tab_indicator = 0;
}

/*****************************************************************************/

void phone_putc(const char c);

/*****************************************************************************/

static int  queued_char = -1;
static int  enabled = 0;

/*****************************************************************************/

/* flush buffers */
int phone_start(void)
{
	console_init();

	update();
	sed156x_sync();

	enabled = 1;
	queued_char = 'U' - '@';

	/* backlit on */
	DISPLAY_BACKLIT_PORT &= ~DISPLAY_BACKLIT_MASK;

	return 0;
}

int phone_stop(void)
{
	enabled = 0;

	sed156x_clear();
	sed156x_sync();

	/* backlit off */
	DISPLAY_BACKLIT_PORT |= DISPLAY_BACKLIT_MASK;

	return 0;
}

void phone_puts(const char *s)
{
	int count = strlen(s);

	while (count--)
		phone_putc(*s++);
}

int phone_tstc(void)
{
	return queued_char >= 0 ? 1 : 0;
}

int phone_getc(void)
{
	int r;

	if (queued_char < 0)
		return -1;

	r = queued_char;
	queued_char = -1;

	return r;
}

/*****************************************************************************/

int drv_phone_init(void)
{
	device_t console_dev;

	console_init();

	memset(&console_dev, 0, sizeof(console_dev));
	strcpy(console_dev.name, "phone");
	console_dev.ext = DEV_EXT_VIDEO;	/* Video extensions */
	console_dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	console_dev.start = phone_start;
	console_dev.stop = phone_stop;
	console_dev.putc = phone_putc;	/* 'putc' function */
	console_dev.puts = phone_puts;	/* 'puts' function */
	console_dev.tstc = phone_tstc;	/* 'tstc' function */
	console_dev.getc = phone_getc;	/* 'getc' function */

	if (device_register(&console_dev) == 0)
		return 1;

	return 0;
}

static int use_me;

int drv_phone_use_me(void)
{
	return use_me;
}

static void kp_do_poll(void);

void phone_console_do_poll(void)
{
	int i, x, y;

	kp_do_poll();

	if (enabled) {
		/* do the blink */
		blink_time -= PHONE_CONSOLE_POLL_HZ;
		if (blink_time <= 0) {
			blink_time += BLINK_HZ;
			if (last_visible_curs_ptr) {
				i = last_visible_curs_ptr - last_visible_buf;
				x = i % width; y = i / width;
				sed156x_reverse_at(x, y, 1);
				last_visible_curs_rev ^= 1;
			}
		}

		/* do the refresh */
		refresh_time -= PHONE_CONSOLE_POLL_HZ;
		if (refresh_time <= 0) {
			refresh_time += REFRESH_HZ;
			sed156x_sync();
		}
	}

}

static int last_scancode = -1;
static int forced_scancode = 0;
static int input_state = -1;
static int input_scancode = -1;
static int input_selected_char = -1;
static char input_covered_char;

static void putchar_at_cursor(char c)
{
	vty_buf[curs_row * COLS + curs_col] = c;
	ensure_visible(curs_col, curs_row, 1, 1);
}

static char getchar_at_cursor(void)
{
	return vty_buf[curs_row * COLS + curs_col];
}

static void queue_input_char(char c)
{
	if (c <= 0)
		return;

	queued_char = c;
}

static void terminate_input(void)
{
	if (input_state < 0)
		return;

	if (input_selected_char >= 0)
		queue_input_char(input_selected_char);

	input_state = -1;
	input_selected_char = -1;
	putchar_at_cursor(input_covered_char);

	curs_disabled = 0;
	blink_time = BLINK_HZ;
	update();
}

static void handle_enabled_scancode(int scancode)
{
	char c;
	int new_disp_col, new_disp_row;
	const char *sel;


	switch (scancode) {

			/* key was released */
		case KP_RELEASE:
			forced_scancode = 0;
			break;

			/* key was forced */
		case KP_FORCE:

			switch (last_scancode) {
				case '#':
					if (input_mode == IM_NUMBER) {
						input_mode = IM_CAPITAL;
						/* queue backspace to erase # */
						queue_input_char('\b');
					} else {
						input_mode = IM_NUMBER;
						fast_punct = '*';
					}
					update();
					break;

				case '0': case '1':
				case '2': case '3': case '4': case '5':
				case '6': case '7': case '8': case '9':

					if (input_state < 0)
						break;

					input_selected_char = last_scancode;
					putchar_at_cursor((char)input_selected_char);
					terminate_input();

					break;

				default:
					break;
			}

			break;

			/* release and idle */
		case KP_IDLE:
			input_scancode = -1;
			if (input_state < 0)
				break;
			terminate_input();
			break;

			/* change input mode */
		case '#':
			if (last_scancode == '#')	/* no repeat */
				break;

			if (input_mode == IM_NUMBER) {
				input_scancode = scancode;
				input_state = 0;
				input_selected_char = scancode;
				input_covered_char = getchar_at_cursor();
				putchar_at_cursor((char)input_selected_char);
				terminate_input();
				break;
			}

			if (input_mode == IM_SMALL)
				input_mode = IM_CAPITAL;
			else
				input_mode = IM_SMALL;

			update();
			break;

		case '*':
			/* no repeat */
			if (last_scancode == scancode)
				break;

			if (input_state >= 0)
				terminate_input();

			input_scancode = fast_punct;
			input_state = 0;
			input_selected_char = input_scancode;
			input_covered_char = getchar_at_cursor();
			putchar_at_cursor((char)input_selected_char);
			terminate_input();

			break;

		case '0': case '1':
		case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':

			/* no repeat */
			if (last_scancode == scancode)
				break;

			if (input_mode == IM_NUMBER) {
				input_scancode = scancode;
				input_state = 0;
				input_selected_char = scancode;
				input_covered_char = getchar_at_cursor();
				putchar_at_cursor((char)input_selected_char);
				terminate_input();
				break;
			}

			if (input_state >= 0 && input_scancode != scancode)
				terminate_input();

			if (input_state < 0) {
				curs_disabled = 1;
				input_scancode = scancode;
				input_state = 0;
				input_covered_char = getchar_at_cursor();
			} else
				input_state++;

			if (scancode == '0')
				sel = whspace;
			else if (scancode == '1')
				sel = punct;
			else
				sel = digits_sel[input_mode][scancode - '2'];
			c = *(sel + input_state);
			if (c == '\0') {
				input_state = 0;
				c = *sel;
			}

			input_selected_char = (int)c;
			putchar_at_cursor((char)input_selected_char);
			update();

			break;

			/* move visible display */
		case KP_F3: case KP_F8: case KP_F7: case KP_F9:

			new_disp_col = disp_col;
			new_disp_row = disp_row;

			switch (scancode) {
					/* up */
				case KP_F3:
					if (new_disp_row <= 0)
						break;
					new_disp_row--;
					break;

					/* down */
				case KP_F8:
					if (new_disp_row >= ROWS - height)
						break;
					new_disp_row++;
					break;

					/* left */
				case KP_F7:
					if (new_disp_col <= 0)
						break;
					new_disp_col--;
					break;

					/* right */
				case KP_F9:
					if (new_disp_col >= COLS - width)
						break;
					new_disp_col++;
					break;
			}

			/* no change? */
			if (disp_col == new_disp_col && disp_row == new_disp_row)
				break;

			disp_col = new_disp_col;
			disp_row = new_disp_row;
			update();

			break;

		case KP_F6:	/* backspace */
			/* inputing something; no backspace sent, just cancel input */
			if (input_state >= 0) {
				input_selected_char = -1;	/* cancel */
				terminate_input();
				break;
			}
			queue_input_char('\b');
			break;

		case KP_F10:	/* enter */
			/* inputing something; first cancel input */
			if (input_state >= 0)
				terminate_input();
			queue_input_char('\r');
			break;

		case KP_F11:	/* R -> Ctrl-C (abort) */
			if (input_state >= 0)
				terminate_input();
			queue_input_char('C' - 'Q');	/* ctrl-c */
			break;

		case KP_F5:	/* F% -> Ctrl-U (clear line) */
			if (input_state >= 0)
				terminate_input();
			queue_input_char('U' - 'Q');	/* ctrl-c */
			break;


		case KP_F1:	/* tab */
			/* inputing something; first cancel input */
			if (input_state >= 0)
				terminate_input();
			queue_input_char('\t');
			break;

		case KP_F2:	/* change fast punct */
			sel = strchr(fast_punct_list, fast_punct);
			if (sel == NULL)
				sel = &fast_punct_list[0];
			sel++;
			if (*sel == '\0')
				sel = &fast_punct_list[0];
			fast_punct = *sel;
			update();
			break;


	}

	if (scancode != KP_FORCE && scancode != KP_IDLE)	/* don't record forced or idle scancode */
		last_scancode = scancode;
}

static void scancode_action(int scancode)
{
#if 0
	if (scancode == KP_RELEASE)
		printf(" RELEASE\n");
	else if (scancode == KP_FORCE)
		printf(" FORCE\n");
	else if (scancode == KP_IDLE)
		printf(" IDLE\n");
	else if (scancode < 32)
		printf(" F%d", scancode + 1);
	else
		printf(" %c", (char)scancode);
	printf("\n");
#endif

	if (enabled) {
		handle_enabled_scancode(scancode);
		return;
	}

	if (scancode == KP_FORCE && last_scancode == '*')
		use_me = 1;

	last_scancode = scancode;
}

/**************************************************************************************/

/* update the display; make sure to update only the differences */
static void update(void)
{
	int i;
	char *s, *e, *t, *r, *b, *cp;

	if (input_mode != last_input_mode)
		sed156x_output_at(sed156x_text_width - 3, sed156x_text_height - 1, input_mode_txt[input_mode], 3);

	if (tab_indicator == 0) {
		sed156x_output_at(0, sed156x_text_height - 1, "\\t", 2);
		tab_indicator = 1;
	}

	if (fast_punct != last_fast_punct)
		sed156x_output_at(4, sed156x_text_height - 1, &fast_punct, 1);

	if (curs_disabled ||
		curs_col < disp_col || curs_col >= (disp_col + width) ||
		curs_row < disp_row || curs_row >= (disp_row + height)) {
		cp = NULL;
	} else
		cp = last_visible_buf + (curs_row - disp_row) * width + (curs_col - disp_col);


	/* printf("(%d,%d) (%d,%d) %s\n", curs_col, curs_row, disp_col, disp_row, cp ? "YES" : "no"); */

	/* clear previous cursor */
	if (last_visible_curs_ptr && last_visible_curs_rev == 0) {
		i = last_visible_curs_ptr - last_visible_buf;
		sed156x_reverse_at(i % width, i / width, 1);
	}

	b = vty_buf + disp_row * COLS + disp_col;
	t = last_visible_buf;
	for (i = 0; i < height; i++) {
		s = b;
		e = b + width;
		/* update only the differences */
		do {
			while (s < e && *s == *t) {
				s++;
				t++;
			}
			if (s == e)	/* no more */
				break;

			/* find run */
			r = s;
			while (s < e && *s != *t)
				*t++ = *s++;

			/* and update */
			sed156x_output_at(r - b, i, r, s - r);

		} while (s < e);

		b += COLS;
	}

	/* set cursor */
	if (cp) {
		last_visible_curs_ptr = cp;
		i = last_visible_curs_ptr - last_visible_buf;
		sed156x_reverse_at(i % width, i / width, 1);
		last_visible_curs_rev = 0;
	} else {
		last_visible_curs_ptr = NULL;
	}

	last_input_mode = input_mode;
	last_fast_punct = fast_punct;
}

/* ensure visibility; the trick is to minimize the screen movement */
static void ensure_visible(int col, int row, int dx, int dy)
{
	int x1, y1, x2, y2, a1, b1, a2, b2;

	/* clamp visible region */
	if (col < 0) {
		dx -= col;
		col = 0;
		if (dx <= 0)
			dx = 1;
	}

	if (row < 0) {
		dy -= row;
		row = 0;
		if (dy <= 0)
			dy = 1;
	}

	if (col + dx > COLS)
		dx = COLS - col;

	if (row + dy > ROWS)
		dy = ROWS - row;


	/* move to easier to use vars */
	x1 = disp_col;	 y1 = disp_row;
	x2 = x1 + width; y2 = y1 + height;
	a1 = col;	 b1 = row;
	a2 = a1 + dx;	 b2 = b1 + dy;

	/* printf("(%d,%d) - (%d,%d) : (%d, %d) - (%d, %d)\n", x1, y1, x2, y2, a1, b1, a2, b2); */

	if (a2 > x2) {
		/* move to the right */
		x2 = a2;
		x1 = x2 - width;
		if (x1 < 0) {
			x1 = 0;
			x2 = width;
		}
	} else if (a1 < x1) {
		/* move to the left */
		x1 = a1;
		x2 = x1 + width;
		if (x2 > COLS) {
			x2 = COLS;
			x1 = x2 - width;
		}
	}

	if (b2 > y2) {
		/* move down */
		y2 = b2;
		y1 = y2 - height;
		if (y1 < 0) {
			y1 = 0;
			y2 = height;
		}
	} else if (b1 < y1) {
		/* move up */
		y1 = b1;
		y2 = y1 + width;
		if (y2 > ROWS) {
			y2 = ROWS;
			y1 = y2 - height;
		}
	}

	/* printf("(%d,%d) - (%d,%d) : (%d, %d) - (%d, %d)\n", x1, y1, x2, y2, a1, b1, a2, b2); */

	/* no movement? */
	if (disp_col == x1 && disp_row == y1)
		return;

	disp_col = x1;
	disp_row = y1;
}

/**************************************************************************************/

static void newline(void)
{
	curs_col = 0;
	if (curs_row + 1 < ROWS)
		curs_row++;
	else {
		memmove(vty_buf, vty_buf + COLS, COLS * (ROWS - 1));
		memset(vty_buf + (ROWS - 1) * COLS, ' ', COLS);
	}
}

void phone_putc(const char c)
{
	int i;

	if (input_mode != -1) {
		input_selected_char = -1;
		terminate_input();
	}

	curs_disabled = 1;
	update();

	blink_time = BLINK_HZ;

	switch (c) {
		case '\a':		/* ignore bell		  */
		case '\r':		/* ignore carriage return */
			break;

		case '\n':		/* next line */
			newline();
			ensure_visible(curs_col, curs_row, 1, 1);
			break;

		case 9: /* tab 8 */
			/* move to tab */
			i = curs_col;
			i |=  0x0008;
			i &= ~0x0007;

			if (i < COLS)
				curs_col = i;
			else
				newline();

			ensure_visible(curs_col, curs_row, 1, 1);
			break;

		case 8:		/* backspace */
			if (curs_col <= 0)
				break;
			curs_col--;

			/* make sure that we see a couple of characters before */
			if (curs_col > 4)
				ensure_visible(curs_col - 4, curs_row, 4, 1);
			else
				ensure_visible(curs_col, curs_row, 1, 1);

			break;

		default:		/* draw the char */
			putchar_at_cursor(c);

			/*
			 * check for newline
			 */
			if (curs_col + 1 < COLS)
				curs_col++;
			else
				newline();

			ensure_visible(curs_col, curs_row, 1, 1);

			break;
	}

	curs_disabled = 0;
	blink_time = BLINK_HZ;
	update();
}

/**************************************************************************************/

static inline unsigned int kp_transfer(unsigned int val)
{
	unsigned int rx;
	int b;

	rx = 0; b = 8;
	while (--b >= 0) {
		KP_SPI_TXD(val & 0x80);
		val <<= 1;
		KP_SPI_CLK_TOGGLE();
		KP_SPI_BIT_DELAY();
		rx <<= 1;
		if (KP_SPI_RXD())
			rx |= 1;
		KP_SPI_CLK_TOGGLE();
		KP_SPI_BIT_DELAY();
	}

	return rx;
}

unsigned int kp_data_transfer(unsigned int val)
{
	KP_SPI_CLK(1);
	KP_CS(0);
	val = kp_transfer(val);
	KP_CS(1);

	return val;
}

unsigned int kp_get_col_mask(unsigned int row_mask)
{
	unsigned int val, col_mask;

	val = 0x80 | (row_mask & 0x7F);
	(void)kp_data_transfer(val);
#if CONFIG_NETPHONE_VERSION == 1
	col_mask = kp_data_transfer(val) & 0x0F;
#elif CONFIG_NETPHONE_VERSION == 2
	col_mask = ((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat & 0x0f;
	/* XXX FUCK FUCK FUCK FUCK FUCK!!!! */
	col_mask = ((col_mask & 0x08) >> 3) |	/* BKBR1 */
		   ((col_mask & 0x04) << 1) |	/* BKBR2 */
		    (col_mask & 0x02) |		/* BKBR3 */
		   ((col_mask & 0x01) << 2);	/* BKBR4 */

#endif
	/* printf("col_mask(row_mask = 0x%x) -> col_mask = 0x%x\n", row_mask, col_mask); */

	return col_mask;
}

/**************************************************************************************/

static const int kp_scancodes[KP_ROWS * KP_COLS] = {
	KP_F1,	 KP_F3,	  KP_F4,  KP_F2,
	KP_F6,	 KP_F8,	  KP_F9,  KP_F7,
	KP_1,	 KP_3,	  KP_F11, KP_2,
	KP_4,	 KP_6,	  KP_F12, KP_5,
	KP_7,	 KP_9,	  KP_F13, KP_8,
	KP_STAR, KP_HASH, KP_F14, KP_0,
	KP_F5,	 KP_F15,  KP_F16, KP_F10,
};

static const int kp_repeats[KP_ROWS * KP_COLS] = {
	0, 1, 0, 0,
	0, 1, 1, 1,
	1, 1, 0, 1,
	1, 1, 0, 1,
	1, 1, 0, 1,
	1, 1, 0, 1,
	0, 0, 0, 1,
};

static int kp_state = SCAN;
static int kp_last_col_mask;
static int kp_cur_row, kp_cur_col;
static int kp_scancode;
static int kp_stable;
static int kp_repeat;
static int kp_repeat_time;
static int kp_force_time;
static int kp_idle_time;

static void kp_do_poll(void)
{
	unsigned int col_mask;
	int col;

	switch (kp_state) {
		case SCAN:
			if (kp_idle_time > 0) {
				kp_idle_time -= PHONE_CONSOLE_POLL_HZ;
				if (kp_idle_time <= 0)
					scancode_action(KP_IDLE);
			}

			col_mask = kp_get_col_mask(KP_ROWS_MASK);
			if (col_mask == KP_COLS_MASK)
				break;	/* nothing */
			kp_last_col_mask = col_mask;
			kp_stable = 0;
			kp_state = SCAN_FILTER;
			break;

		case SCAN_FILTER:
			col_mask = kp_get_col_mask(KP_ROWS_MASK);
			if (col_mask != kp_last_col_mask) {
				kp_state = SCAN;
				break;
			}

			kp_stable += PHONE_CONSOLE_POLL_HZ;
			if (kp_stable < KP_STABLE_HZ)
				break;

			kp_cur_row = 0;
			kp_stable = 0;
			kp_state = SCAN_COL;

			(void)kp_get_col_mask(1 << kp_cur_row);
			break;

		case SCAN_COL:
			col_mask = kp_get_col_mask(1 << kp_cur_row);
			if (col_mask == KP_COLS_MASK) {
				if (++kp_cur_row >= KP_ROWS) {
					kp_state = SCAN;
					break;
				}
				kp_get_col_mask(1 << kp_cur_row);
				break;
			}
			kp_last_col_mask = col_mask;
			kp_stable = 0;
			kp_state = SCAN_COL_FILTER;
			break;

		case SCAN_COL_FILTER:
			col_mask = kp_get_col_mask(1 << kp_cur_row);
			if (col_mask != kp_last_col_mask || col_mask == KP_COLS_MASK) {
				kp_state = SCAN;
				break;
			}

			kp_stable += PHONE_CONSOLE_POLL_HZ;
			if (kp_stable < KP_STABLE_HZ)
				break;

			for (col = 0; col < KP_COLS; col++)
				if ((col_mask & (1 << col)) == 0)
					break;
			kp_cur_col = col;
			kp_state = PRESSED;
			kp_scancode = kp_scancodes[kp_cur_row * KP_COLS + kp_cur_col];
			kp_repeat = kp_repeats[kp_cur_row * KP_COLS + kp_cur_col];

			if (kp_repeat)
				kp_repeat_time = KP_REPEAT_DELAY_HZ;
			kp_force_time = KP_FORCE_DELAY_HZ;

			scancode_action(kp_scancode);

			break;

		case PRESSED:
			col_mask = kp_get_col_mask(1 << kp_cur_row);
			if (col_mask != kp_last_col_mask) {
				kp_state = SCAN;
				scancode_action(KP_RELEASE);
				kp_idle_time = KP_IDLE_DELAY_HZ;
				break;
			}

			if (kp_repeat) {
				kp_repeat_time -= PHONE_CONSOLE_POLL_HZ;
				if (kp_repeat_time <= 0) {
					kp_repeat_time += KP_REPEAT_HZ;
					scancode_action(kp_scancode);
				}
			}

			if (kp_force_time > 0) {
				kp_force_time -= PHONE_CONSOLE_POLL_HZ;
				if (kp_force_time <= 0)
					scancode_action(KP_FORCE);
			}

			break;
	}
}

/**************************************************************************************/

int drv_phone_is_idle(void)
{
	return kp_state == SCAN;
}
