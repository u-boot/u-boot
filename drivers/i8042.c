/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* i8042.c - Intel 8042 keyboard driver routines */

/* includes */

#include <common.h>

#ifdef CONFIG_I8042_KBD

#include <i8042.h>

/* defines */

#ifdef CONFIG_CONSOLE_CURSOR
extern void console_cursor (int state);
static int blinkCount = CFG_CONSOLE_BLINK_COUNT;
static int cursor_state = 0;
#endif

/* locals */

static int  kbd_input    = -1;          /* no input yet */
static int  kbd_mapping  = KBD_US;      /* default US keyboard */
static int  kbd_flags    = NORMAL;      /* after reset */
static int  kbd_state    = 0;           /* unshift code */

static void kbd_conv_char (unsigned char scan_code);
static void kbd_led_set (void);
static void kbd_normal (unsigned char scan_code);
static void kbd_shift (unsigned char scan_code);
static void kbd_ctrl (unsigned char scan_code);
static void kbd_num (unsigned char scan_code);
static void kbd_caps (unsigned char scan_code);
static void kbd_scroll (unsigned char scan_code);
static void kbd_alt (unsigned char scan_code);
static int  kbd_input_empty (void);
static int  kbd_reset (void);

static unsigned char kbd_fct_map [144] =
    { /* kbd_fct_map table for scan code */
    0,   AS,   AS,   AS,   AS,   AS,   AS,   AS, /* scan  0- 7 */
   AS,   AS,   AS,   AS,   AS,   AS,   AS,   AS, /* scan  8- F */
   AS,   AS,   AS,   AS,   AS,   AS,   AS,   AS, /* scan 10-17 */
   AS,   AS,   AS,   AS,   AS,   CN,   AS,   AS, /* scan 18-1F */
   AS,   AS,   AS,   AS,   AS,   AS,   AS,   AS, /* scan 20-27 */
   AS,   AS,   SH,   AS,   AS,   AS,   AS,   AS, /* scan 28-2F */
   AS,   AS,   AS,   AS,   AS,   AS,   SH,   AS, /* scan 30-37 */
   AS,   AS,   CP,   0,    0,    0,    0,     0, /* scan 38-3F */
    0,   0,    0,    0,    0,    NM,   ST,   ES, /* scan 40-47 */
   ES,   ES,   ES,   ES,   ES,   ES,   ES,   ES, /* scan 48-4F */
   ES,   ES,   ES,   ES,   0,    0,    AS,    0, /* scan 50-57 */
    0,   0,    0,    0,    0,    0,    0,     0, /* scan 58-5F */
    0,   0,    0,    0,    0,    0,    0,     0, /* scan 60-67 */
    0,   0,    0,    0,    0,    0,    0,     0, /* scan 68-6F */
   AS,   0,    0,    AS,   0,    0,    AS,    0, /* scan 70-77 */
    0,   AS,   0,    0,    0,    AS,   0,     0, /* scan 78-7F */
   AS,   CN,   AS,   AS,   AK,   ST,   EX,   EX, /* enhanced   */
   AS,   EX,   EX,   AS,   EX,   AS,   EX,   EX  /* enhanced   */
    };

static unsigned char kbd_key_map [2][5][144] =
    {
    { /* US keyboard */
    { /* unshift code */
    0,  0x1b,   '1',   '2',   '3',   '4',   '5',   '6',    /* scan  0- 7 */
  '7',   '8',   '9',   '0',   '-',   '=',  0x08,  '\t',    /* scan  8- F */
  'q',   'w',   'e',   'r',   't',   'y',   'u',   'i',    /* scan 10-17 */
  'o',   'p',   '[',   ']',  '\r',   CN,    'a',   's',    /* scan 18-1F */
  'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',    /* scan 20-27 */
 '\'',   '`',   SH,   '\\',   'z',   'x',   'c',   'v',    /* scan 28-2F */
  'b',   'n',   'm',   ',',   '.',   '/',   SH,    '*',    /* scan 30-37 */
  ' ',   ' ',   CP,      0,     0,     0,     0,     0,    /* scan 38-3F */
    0,     0,     0,     0,     0,   NM,    ST,    '7',    /* scan 40-47 */
  '8',   '9',   '-',   '4',   '5',   '6',   '+',   '1',    /* scan 48-4F */
  '2',   '3',   '0',   '.',     0,     0,     0,     0,    /* scan 50-57 */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 58-5F */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 60-67 */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 68-6F */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 70-77 */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 78-7F */
  '\r',   CN,   '/',   '*',   ' ',    ST,   'F',   'A',    /* extended */
    0,   'D',   'C',     0,   'B',     0,    '@',  'P'     /* extended */
    },
    { /* shift code */
    0,  0x1b,   '!',   '@',   '#',   '$',   '%',   '^',    /* scan  0- 7 */
  '&',   '*',   '(',   ')',   '_',   '+',  0x08,  '\t',    /* scan  8- F */
  'Q',   'W',   'E',   'R',   'T',   'Y',   'U',   'I',    /* scan 10-17 */
  'O',   'P',   '{',   '}',  '\r',   CN,    'A',   'S',    /* scan 18-1F */
  'D',   'F',   'G',   'H',   'J',   'K',   'L',   ':',    /* scan 20-27 */
  '"',   '~',   SH,    '|',   'Z',   'X',   'C',   'V',    /* scan 28-2F */
  'B',   'N',   'M',   '<',   '>',   '?',   SH,    '*',    /* scan 30-37 */
  ' ',   ' ',   CP,      0,     0,     0,     0,     0,    /* scan 38-3F */
    0,     0,     0,     0,     0,   NM,    ST,    '7',    /* scan 40-47 */
  '8',   '9',   '-',   '4',   '5',   '6',   '+',   '1',    /* scan 48-4F */
  '2',   '3',   '0',   '.',     0,     0,     0,     0,    /* scan 50-57 */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 58-5F */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 60-67 */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 68-6F */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 70-77 */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 78-7F */
  '\r',   CN,   '/',   '*',   ' ',    ST,   'F',   'A',    /* extended */
    0,   'D',   'C',     0,   'B',     0,   '@',   'P'     /* extended */
    },
    { /* control code */
 0xff,  0x1b,  0xff,  0x00,  0xff,  0xff,  0xff,  0xff,    /* scan  0- 7 */
 0x1e,  0xff,  0xff,  0xff,  0x1f,  0xff,  0xff,  '\t',    /* scan  8- F */
 0x11,  0x17,  0x05,  0x12,  0x14,  0x19,  0x15,  0x09,    /* scan 10-17 */
 0x0f,  0x10,  0x1b,  0x1d,  '\r',   CN,   0x01,  0x13,    /* scan 18-1F */
 0x04,  0x06,  0x07,  0x08,  0x0a,  0x0b,  0x0c,  0xff,    /* scan 20-27 */
 0xff,  0x1c,   SH,   0xff,  0x1a,  0x18,  0x03,  0x16,    /* scan 28-2F */
 0x02,  0x0e,  0x0d,  0xff,  0xff,  0xff,   SH,   0xff,    /* scan 30-37 */
 0xff,  0xff,   CP,   0xff,  0xff,  0xff,  0xff,  0xff,    /* scan 38-3F */
 0xff,  0xff,  0xff,  0xff,  0xff,   NM,    ST,   0xff,    /* scan 40-47 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,    /* scan 48-4F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,    /* scan 50-57 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,    /* scan 58-5F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,    /* scan 60-67 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,    /* scan 68-6F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,    /* scan 70-77 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,    /* scan 78-7F */
  '\r',   CN,   '/',   '*',   ' ',    ST,  0xff,  0xff,    /* extended */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff     /* extended */
    },
    { /* non numeric code */
    0,  0x1b,   '1',   '2',   '3',   '4',   '5',   '6',    /* scan  0- 7 */
  '7',   '8',   '9',   '0',   '-',   '=',  0x08,  '\t',    /* scan  8- F */
  'q',   'w',   'e',   'r',   't',   'y',   'u',   'i',    /* scan 10-17 */
  'o',   'p',   '[',   ']',  '\r',   CN,    'a',   's',    /* scan 18-1F */
  'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',    /* scan 20-27 */
 '\'',   '`',   SH,   '\\',   'z',   'x',   'c',   'v',    /* scan 28-2F */
  'b',   'n',   'm',   ',',   '.',   '/',   SH,    '*',    /* scan 30-37 */
  ' ',   ' ',   CP,      0,     0,     0,     0,     0,    /* scan 38-3F */
    0,     0,     0,     0,     0,   NM,    ST,    'w',    /* scan 40-47 */
  'x',   'y',   'l',   't',   'u',   'v',   'm',   'q',    /* scan 48-4F */
  'r',   's',   'p',   'n',     0,     0,     0,     0,    /* scan 50-57 */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 58-5F */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 60-67 */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 68-6F */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 70-77 */
    0,     0,     0,     0,     0,     0,     0,     0,    /* scan 78-7F */
  '\r',   CN,   '/',   '*',   ' ',    ST,   'F',   'A',    /* extended */
    0,   'D',   'C',     0,   'B',     0,    '@',  'P'     /* extended */
    },
    { /* right alt mode - not used in US keyboard */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan  0 - 7 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan  8 - F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 10 -17 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 18 -1F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 20 -27 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 28 -2F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 30 -37 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 38 -3F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 40 -47 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 48 -4F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 50 -57 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 58 -5F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 60 -67 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 68 -6F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 70 -77 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 78 -7F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* extended    */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff  /* extended    */
    }
    },
    { /* german keyboard */
    { /* unshift code */
    0,  0x1b,   '1',   '2',   '3',   '4',   '5',   '6', /* scan  0- 7 */
  '7',   '8',   '9',   '0',  0xe1,  '\'',  0x08,  '\t', /* scan  8- F */
  'q',   'w',   'e',   'r',   't',   'z',   'u',   'i', /* scan 10-17 */
  'o',   'p',  0x81,   '+',  '\r',   CN,    'a',   's', /* scan 18-1F */
  'd',   'f',   'g',   'h',   'j',   'k',   'l',  0x94, /* scan 20-27 */
 0x84,   '^',   SH,    '#',   'y',   'x',   'c',   'v', /* scan 28-2F */
  'b',   'n',   'm',   ',',   '.',   '-',   SH,    '*', /* scan 30-37 */
  ' ',   ' ',   CP,      0,     0,     0,     0,     0, /* scan 38-3F */
    0,     0,     0,     0,     0,   NM,    ST,    '7', /* scan 40-47 */
  '8',   '9',   '-',   '4',   '5',   '6',   '+',   '1', /* scan 48-4F */
  '2',   '3',   '0',   ',',     0,     0,   '<',     0, /* scan 50-57 */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 58-5F */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 60-67 */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 68-6F */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 70-77 */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 78-7F */
  '\r',   CN,   '/',   '*',   ' ',    ST,   'F',   'A', /* extended */
    0,   'D',   'C',     0,   'B',     0,    '@',  'P'  /* extended */
    },
    { /* shift code */
    0,  0x1b,   '!',   '"',  0x15,   '$',   '%',   '&', /* scan  0- 7 */
  '/',   '(',   ')',   '=',   '?',   '`',  0x08,  '\t', /* scan  8- F */
  'Q',   'W',   'E',   'R',   'T',   'Z',   'U',   'I', /* scan 10-17 */
  'O',   'P',  0x9a,   '*',  '\r',   CN,    'A',   'S', /* scan 18-1F */
  'D',   'F',   'G',   'H',   'J',   'K',   'L',  0x99, /* scan 20-27 */
 0x8e,  0xf8,   SH,   '\'',   'Y',   'X',   'C',   'V', /* scan 28-2F */
  'B',   'N',   'M',   ';',   ':',   '_',   SH,    '*', /* scan 30-37 */
  ' ',   ' ',   CP,      0,     0,     0,     0,     0, /* scan 38-3F */
    0,     0,     0,     0,     0,   NM,    ST,    '7', /* scan 40-47 */
  '8',   '9',   '-',   '4',   '5',   '6',   '+',   '1', /* scan 48-4F */
  '2',   '3',   '0',   ',',     0,     0,   '>',     0, /* scan 50-57 */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 58-5F */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 60-67 */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 68-6F */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 70-77 */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 78-7F */
  '\r',   CN,   '/',   '*',   ' ',    ST,   'F',   'A', /* extended */
    0,   'D',   'C',     0,   'B',     0,   '@',   'P'  /* extended */
    },
    { /* control code */
 0xff,  0x1b,  0xff,  0x00,  0xff,  0xff,  0xff,  0xff, /* scan  0- 7 */
 0x1e,  0xff,  0xff,  0xff,  0x1f,  0xff,  0xff,  '\t', /* scan  8- F */
 0x11,  0x17,  0x05,  0x12,  0x14,  0x19,  0x15,  0x09, /* scan 10-17 */
 0x0f,  0x10,  0x1b,  0x1d,  '\r',   CN,   0x01,  0x13, /* scan 18-1F */
 0x04,  0x06,  0x07,  0x08,  0x0a,  0x0b,  0x0c,  0xff, /* scan 20-27 */
 0xff,  0x1c,   SH,   0xff,  0x1a,  0x18,  0x03,  0x16, /* scan 28-2F */
 0x02,  0x0e,  0x0d,  0xff,  0xff,  0xff,   SH,   0xff, /* scan 30-37 */
 0xff,  0xff,   CP,   0xff,  0xff,  0xff,  0xff,  0xff, /* scan 38-3F */
 0xff,  0xff,  0xff,  0xff,  0xff,   NM,    ST,   0xff, /* scan 40-47 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 48-4F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 50-57 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 58-5F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 60-67 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 68-6F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 70-77 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 78-7F */
  '\r',   CN,   '/',   '*',   ' ',    ST,  0xff,  0xff, /* extended */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff  /* extended */
    },
    { /* non numeric code */
    0,  0x1b,   '1',   '2',   '3',   '4',   '5',   '6', /* scan  0- 7 */
  '7',   '8',   '9',   '0',  0xe1,  '\'',  0x08,  '\t', /* scan  8- F */
  'q',   'w',   'e',   'r',   't',   'z',   'u',   'i', /* scan 10-17 */
  'o',   'p',  0x81,   '+',  '\r',   CN,    'a',   's', /* scan 18-1F */
  'd',   'f',   'g',   'h',   'j',   'k',   'l',  0x94, /* scan 20-27 */
 0x84,   '^',   SH,      0,   'y',   'x',   'c',   'v', /* scan 28-2F */
  'b',   'n',   'm',   ',',   '.',   '-',   SH,    '*', /* scan 30-37 */
  ' ',   ' ',   CP,      0,     0,     0,     0,     0, /* scan 38-3F */
    0,     0,     0,     0,     0,   NM,    ST,    'w', /* scan 40-47 */
  'x',   'y',   'l',   't',   'u',   'v',   'm',   'q', /* scan 48-4F */
  'r',   's',   'p',   'n',     0,     0,   '<',     0, /* scan 50-57 */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 58-5F */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 60-67 */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 68-6F */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 70-77 */
    0,     0,     0,     0,     0,     0,     0,     0, /* scan 78-7F */
  '\r',   CN,   '/',   '*',   ' ',    ST,   'F',   'A', /* extended */
    0,   'D',   'C',     0,   'B',     0,    '@',  'P'  /* extended */
    },
    { /* Right alt mode - is used in German keyboard */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan  0 - 7 */
  '{',   '[',   ']',   '}',  '\\',  0xff,  0xff,  0xff, /* scan  8 - F */
  '@',  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 10 -17 */
 0xff,  0xff,  0xff,   '~',  0xff,  0xff,  0xff,  0xff, /* scan 18 -1F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 20 -27 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 28 -2F */
 0xff,  0xff,  0xe6,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 30 -37 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 38 -3F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 40 -47 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 48 -4F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,   '|',  0xff, /* scan 50 -57 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 58 -5F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 60 -67 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 68 -6F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 70 -77 */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* scan 78 -7F */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, /* extended    */
 0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff  /* extended    */
    }
    }
    };

static unsigned char ext_key_map [] =
    {
    0x1c,   /* keypad enter */
    0x1d,   /* right control */
    0x35,   /* keypad slash */
    0x37,   /* print screen */
    0x38,   /* right alt */
    0x46,   /* break */
    0x47,   /* editpad home */
    0x48,   /* editpad up */
    0x49,   /* editpad pgup */
    0x4b,   /* editpad left */
    0x4d,   /* editpad right */
    0x4f,   /* editpad end */
    0x50,   /* editpad dn */
    0x51,   /* editpad pgdn */
    0x52,   /* editpad ins */
    0x53,   /* editpad del */
    0x00    /* map end */
    };

/*******************************************************************************
 *
 * i8042_kbd_init - reset keyboard and init state flags
 */
int i8042_kbd_init (void)
{
    int keymap, try;
    char *penv;

    /* Init keyboard device (default US layout) */
    keymap = KBD_US;
    if ((penv = getenv ("keymap")) != NULL)
    {
	if (strncmp (penv, "de", 3) == 0)
	keymap = KBD_GER;
    }

    for (try = 0; try < KBD_RESET_TRIES; try++)
    {
	if (kbd_reset() == 0)
	{
	    kbd_mapping   = keymap;
	    kbd_flags     = NORMAL;
	    kbd_state     = 0;
	    kbd_led_set();
	    return 0;
	    }
    }
    return -1;
}


/*******************************************************************************
 *
 * i8042_tstc - test if keyboard input is available
 *              option: cursor blinking if called in a loop
 */
int i8042_tstc (void)
{
    unsigned char scan_code = 0;

#ifdef CONFIG_CONSOLE_CURSOR
    if (--blinkCount == 0)
    {
	cursor_state ^= 1;
	console_cursor (cursor_state);
	blinkCount = CFG_CONSOLE_BLINK_COUNT;
	udelay (10);
    }
#endif

    if ((in8 (I8042_STATUS_REG) & 0x01) == 0)
	return 0;
    else
    {
	scan_code = in8 (I8042_DATA_REG);
	if (scan_code == 0xfa)
	    return 0;

	kbd_conv_char(scan_code);

	if (kbd_input != -1)
	    return 1;
    }
    return 0;
}


/*******************************************************************************
 *
 * i8042_getc - wait till keyboard input is available
 *              option: turn on/off cursor while waiting
 */
int i8042_getc (void)
{
    int ret_chr;
    unsigned char scan_code;

    while (kbd_input == -1)
    {
	while ((in8 (I8042_STATUS_REG) & 0x01) == 0)
	{
#ifdef CONFIG_CONSOLE_CURSOR
	    if (--blinkCount==0)
	    {
		cursor_state ^= 1;
		console_cursor (cursor_state);
		blinkCount = CFG_CONSOLE_BLINK_COUNT;
	    }
	    udelay (10);
#endif
	}

	scan_code = in8 (I8042_DATA_REG);

	if (scan_code != 0xfa)
	kbd_conv_char (scan_code);
    }
    ret_chr = kbd_input;
    kbd_input = -1;
    return ret_chr;
}


/******************************************************************************/

static void kbd_conv_char (unsigned char scan_code)
{
    if (scan_code == 0xe0)
    {
	kbd_flags |= EXT;
	return;
    }

    /* if high bit of scan_code, set break flag */
    if (scan_code & 0x80)
	kbd_flags |=  BRK;
    else
	kbd_flags &= ~BRK;

    if ((scan_code == 0xe1) || (kbd_flags & E1))
    {
	if (scan_code == 0xe1)
	{
	    kbd_flags ^= BRK;     /* reset the break flag */
	    kbd_flags ^= E1;      /* bitwise EXOR with E1 flag */
	}
	return;
    }

    scan_code &= 0x7f;

    if (kbd_flags & EXT)
    {
	int i;

	kbd_flags ^= EXT;
	for (i=0; ext_key_map[i]; i++)
	{
	    if (ext_key_map[i] == scan_code)
	    {
		scan_code = 0x80 + i;
		break;
	    }
	}
	/* not found ? */
	if (!ext_key_map[i])
	    return;
    }

    switch (kbd_fct_map [scan_code])
    {
    case AS:  kbd_normal (scan_code);
	break;
    case SH:  kbd_shift (scan_code);
	break;
    case CN:  kbd_ctrl (scan_code);
	break;
    case NM:  kbd_num (scan_code);
	break;
    case CP:  kbd_caps (scan_code);
	break;
    case ST:  kbd_scroll (scan_code);
	break;
    case AK:  kbd_alt (scan_code);
	break;
    }
    return;
}


/******************************************************************************/

static void kbd_normal (unsigned char scan_code)
{
    unsigned char chr;

    if ((kbd_flags & BRK) == NORMAL)
    {
       chr = kbd_key_map [kbd_mapping][kbd_state][scan_code];
       if ((chr == 0xff) || (chr == 0x00))
	{
	    return;
	}

	/* if caps lock convert upper to lower */
	if (((kbd_flags & CAPS) == CAPS) && (chr >= 'a' && chr <= 'z'))
       {
	   chr -= 'a' - 'A';
       }
       kbd_input = chr;
    }
}


/******************************************************************************/

static void kbd_shift (unsigned char scan_code)
{
    if ((kbd_flags & BRK) == BRK)
    {
	kbd_state = AS;
	kbd_flags &= (~SHIFT);
    }
    else
    {
       kbd_state = SH;
       kbd_flags |= SHIFT;
    }
}


/******************************************************************************/

static void kbd_ctrl (unsigned char scan_code)
{
    if ((kbd_flags & BRK) == BRK)
    {
       kbd_state = AS;
       kbd_flags &= (~CTRL);
    }
    else
    {
       kbd_state = CN;
       kbd_flags |= CTRL;
    }
}


/******************************************************************************/

static void kbd_caps (unsigned char scan_code)
{
    if ((kbd_flags & BRK) == NORMAL)
    {
       kbd_flags ^= CAPS;
       kbd_led_set ();            /* update keyboard LED */
    }
}


/******************************************************************************/

static void kbd_num (unsigned char scan_code)
{
    if ((kbd_flags & BRK) == NORMAL)
    {
       kbd_flags ^= NUM;
       kbd_state = (kbd_flags & NUM) ? AS : NM;
       kbd_led_set ();            /* update keyboard LED */
    }
}


/******************************************************************************/

static void kbd_scroll (unsigned char scan_code)
{
    if ((kbd_flags & BRK) == NORMAL)
    {
	kbd_flags ^= STP;
	kbd_led_set ();            /* update keyboard LED */
	if (kbd_flags & STP)
	    kbd_input = 0x13;
	else
	    kbd_input = 0x11;
    }
}

/******************************************************************************/

static void kbd_alt (unsigned char scan_code)
{
    if ((kbd_flags & BRK) == BRK)
    {
	kbd_state = AS;
	kbd_flags &= (~ALT);
    }
    else
    {
	kbd_state = AK;
	kbd_flags &= ALT;
    }
}


/******************************************************************************/

static void kbd_led_set (void)
{
    kbd_input_empty();
    out8 (I8042_DATA_REG, 0xed);        /* SET LED command */
    kbd_input_empty();
    out8 (I8042_DATA_REG, (kbd_flags & 0x7));    /* LED bits only */
}


/******************************************************************************/

static int kbd_input_empty (void)
{
    int kbdTimeout = KBD_TIMEOUT;

    /* wait for input buf empty */
    while ((in8 (I8042_STATUS_REG) & 0x02) && kbdTimeout--)
	udelay(1000);

    return kbdTimeout;
}

/******************************************************************************/

static int kbd_reset (void)
{
    if (kbd_input_empty() == 0)
	return -1;

    out8 (I8042_DATA_REG, 0xff);

    udelay(250000);

    if (kbd_input_empty() == 0)
	return -1;

    out8 (I8042_DATA_REG, 0x60);

    if (kbd_input_empty() == 0)
	return -1;

    out8 (I8042_DATA_REG, 0x45);


    if (kbd_input_empty() == 0)
	return -1;

    out8 (I8042_COMMAND_REG, 0xae);

    if (kbd_input_empty() == 0)
	return -1;

    return 0;
}

#endif /* CONFIG_I8042_KBD */
