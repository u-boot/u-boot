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

/* FEMA 162B 16 character x 2 line LCD */

/* status register bit definitions */
#define LCD_STAT_BUSY	0x80	/* 1 = display busy */
#define LCD_STAT_ADD	0x7F	/* bits 0-6 return current display address */

/* command register definitions */
#define LCD_CMD_RST	0x01	/* clear entire display and reset display addr */
#define LCD_CMD_HOME	0x02	/* reset display address and reset any shifting */
#define LCD_CMD_ECL	0x04	/* move cursor left one pos on next data write */
#define LCD_CMD_ESL	0x05	/* shift display left one pos on next data write */
#define LCD_CMD_ECR	0x06	/* move cursor right one pos on next data write */
#define LCD_CMD_ESR	0x07	/* shift disp right one pos on next data write */
#define LCD_CMD_DOFF	0x08	/* display off, cursor off, blinking off */
#define LCD_CMD_BL	0x09	/* blink character at current cursor position */
#define LCD_CMD_CUR	0x0A	/* enable cursor on */
#define LCD_CMD_DON	0x0C	/* turn display on */
#define LCD_CMD_CL	0x10	/* move cursor left one position */
#define LCD_CMD_SL	0x14	/* shift display left one position */
#define LCD_CMD_CR	0x18	/* move cursor right one position */
#define LCD_CMD_SR	0x1C	/* shift display right one position */
#define LCD_CMD_MODE	0x38	/* sets 8 bits, 2 lines, 5x7 characters */
#define LCD_CMD_ACG	0x40	/* bits 0-5 sets character generator address */
#define LCD_CMD_ADD	0x80	/* bits 0-6 sets display data addr to line 1 + */

/* LCD status values */
#define LCD_OK		0x00
#define LCD_ERR 	0x01

#define LCD_LINE0	0x00
#define LCD_LINE1	0x40

#define LCD_LINE_LENGTH	16

extern void lcd_init(void);
extern void lcd_write_char(const char);
extern void lcd_flush(void);
extern void lcd_write_string(const char *);
extern void lcd_printf(const char *, ...);
