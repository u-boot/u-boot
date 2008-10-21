/* most of this is taken from the file */
/* hal/powerpc/cogent/current/src/hal_diag.c in the */
/* Cygnus eCos source. Here is the copyright notice: */
/* */
/*============================================================================= */
/* */
/*      hal_diag.c */
/* */
/*      HAL diagnostic output code */
/* */
/*============================================================================= */
/*####COPYRIGHTBEGIN#### */
/* */
/* ------------------------------------------- */
/* The contents of this file are subject to the Cygnus eCos Public License */
/* Version 1.0 (the "License"); you may not use this file except in */
/* compliance with the License.  You may obtain a copy of the License at */
/* http://sourceware.cygnus.com/ecos */
/* */
/* Software distributed under the License is distributed on an "AS IS" */
/* basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the */
/* License for the specific language governing rights and limitations under */
/* the License. */
/* */
/* The Original Code is eCos - Embedded Cygnus Operating System, released */
/* September 30, 1998. */
/* */
/* The Initial Developer of the Original Code is Cygnus.  Portions created */
/* by Cygnus are Copyright (C) 1998,1999 Cygnus Solutions.  All Rights Reserved. */
/* ------------------------------------------- */
/* */
/*####COPYRIGHTEND#### */
/*============================================================================= */
/*#####DESCRIPTIONBEGIN#### */
/* */
/* Author(s):    nickg, jskov */
/* Contributors: nickg, jskov */
/* Date:         1999-03-23 */
/* Purpose:      HAL diagnostic output */
/* Description:  Implementations of HAL diagnostic output support. */
/* */
/*####DESCRIPTIONEND#### */
/* */
/*============================================================================= */

/*----------------------------------------------------------------------------- */
/* Cogent board specific LCD code */

#include <common.h>
#include <stdarg.h>
#include <board/cogent/lcd.h>

static char lines[2][LCD_LINE_LENGTH+1];
static int curline;
static int linepos;
static int heartbeat_active;
/* make the next two strings exactly LCD_LINE_LENGTH (16) chars long */
/* pad to the right with spaces if necessary */
static char init_line0[LCD_LINE_LENGTH+1] = "U-Boot Cogent  ";
static char init_line1[LCD_LINE_LENGTH+1] = "mjj, 11 Aug 2000";

static inline unsigned char
lcd_read_status(cma_mb_lcd *clp)
{
    /* read the Busy Status Register */
    return (cma_mb_reg_read(&clp->lcd_bsr));
}

static inline void
lcd_wait_not_busy(cma_mb_lcd *clp)
{
    /*
     * wait for not busy
     * Note: It seems that the LCD isn't quite ready to process commands
     * when it clears the BUSY flag. Reading the status address an extra
     * time seems to give it enough breathing room.
     */

    while (lcd_read_status(clp) & LCD_STAT_BUSY)
	;

    (void)lcd_read_status(clp);
}

static inline void
lcd_write_command(cma_mb_lcd *clp, unsigned char cmd)
{
    lcd_wait_not_busy(clp);

    /* write the Command Register */
    cma_mb_reg_write(&clp->lcd_cmd, cmd);
}

static inline void
lcd_write_data(cma_mb_lcd *clp, unsigned char data)
{
    lcd_wait_not_busy(clp);

    /* write the Current Character Register */
    cma_mb_reg_write(&clp->lcd_ccr, data);
}

static inline void
lcd_dis(int addr, char *string)
{
    cma_mb_lcd *clp = (cma_mb_lcd *)CMA_MB_LCD_BASE;
    int pos, linelen;

    linelen = LCD_LINE_LENGTH;
    if (heartbeat_active && addr == LCD_LINE0)
	linelen--;

    lcd_write_command(clp, LCD_CMD_ADD + addr);
    for (pos = 0; *string != '\0' && pos < linelen; pos++)
	lcd_write_data(clp, *string++);
}

void
lcd_init(void)
{
    cma_mb_lcd *clp = (cma_mb_lcd *)CMA_MB_LCD_BASE;
    int i;

    /* configure the lcd for 8 bits/char, 2 lines and 5x7 dot matrix */
    lcd_write_command(clp, LCD_CMD_MODE);

    /* turn the LCD display on */
    lcd_write_command(clp, LCD_CMD_DON);

    curline = 0;
    linepos = 0;

    for (i = 0; i < LCD_LINE_LENGTH; i++) {
	lines[0][i] = init_line0[i];
	lines[1][i] = init_line1[i];
    }

    lines[0][LCD_LINE_LENGTH] = lines[1][LCD_LINE_LENGTH] = 0;

    lcd_dis(LCD_LINE0, lines[0]);
    lcd_dis(LCD_LINE1, lines[1]);

    printf("HD44780 2 line x %d char display\n", LCD_LINE_LENGTH);
}

void
lcd_write_char(const char c)
{
    int i, linelen;

    /* ignore CR */
    if (c == '\r')
	return;

    linelen = LCD_LINE_LENGTH;
    if (heartbeat_active && curline == 0)
	linelen--;

    if (c == '\n') {
	lcd_dis(LCD_LINE0, &lines[curline^1][0]);
	lcd_dis(LCD_LINE1, &lines[curline][0]);

	/* Do a line feed */
	curline ^= 1;
	linelen = LCD_LINE_LENGTH;
	if (heartbeat_active && curline == 0)
	    linelen--;
	linepos = 0;

	for (i = 0; i < linelen; i++)
	    lines[curline][i] = ' ';

	return;
    }

    /* Only allow to be output if there is room on the LCD line */
    if (linepos < linelen)
	lines[curline][linepos++] = c;
}

void
lcd_flush(void)
{
    lcd_dis(LCD_LINE1, &lines[curline][0]);
}

void
lcd_write_string(const char *s)
{
    char *p;

    for (p = (char *)s; *p != '\0'; p++)
	lcd_write_char(*p);
}

void
lcd_printf(const char *fmt, ...)
{
    va_list args;
    char buf[CONFIG_SYS_PBSIZE];

    va_start(args, fmt);
    (void)vsprintf(buf, fmt, args);
    va_end(args);

    lcd_write_string(buf);
}

void
lcd_heartbeat(void)
{
    cma_mb_lcd *clp = (cma_mb_lcd *)CMA_MB_LCD_BASE;
#if 0
    static char rotchars[] = { '|', '/', '-', '\\' };
#else
    /* HD44780 Rom Code A00 has no backslash */
    static char rotchars[] = { '|', '/', '-', '\315' };
#endif
    static int rotator_index = 0;

    heartbeat_active = 1;

    /* write the address */
    lcd_write_command(clp, LCD_CMD_ADD + LCD_LINE0 + (LCD_LINE_LENGTH - 1));

    /* write the next char in the sequence */
    lcd_write_data(clp, rotchars[rotator_index]);

    if (++rotator_index >= (sizeof rotchars / sizeof rotchars[0]))
	rotator_index = 0;
}

#ifdef CONFIG_SHOW_ACTIVITY
void board_show_activity (ulong timestamp)
{
#ifdef CONFIG_STATUS_LED
	if ((timestamp % (CONFIG_SYS_HZ / 2) == 0)
		lcd_heartbeat ();
#endif
}

void show_activity(int arg)
{
}
#endif
