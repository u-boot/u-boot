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

/* i8042.h - Intel 8042 keyboard driver header */

#ifndef _I8042_H_
#define _I8042_H_

#ifdef __I386__
#include <common.h>
#include <asm/io.h>
#define in8(p) inb(p)
#define out8(p,v) outb(v,p)
#endif

/* defines */

#define I8042_DATA_REG      (CFG_ISA_IO + 0x0060)    /* keyboard i/o buffer */
#define I8042_STATUS_REG    (CFG_ISA_IO + 0x0064)    /* keyboard status read */
#define I8042_COMMAND_REG   (CFG_ISA_IO + 0x0064)    /* keyboard ctrl write */

#define KBD_US              0        /* default US layout */
#define KBD_GER             1        /* german layout */

#define KBD_TIMEOUT         1000     /* 1 sec */
#define KBD_RESET_TRIES     3

#define AS                  0        /* normal character index */
#define SH                  1        /* shift index */
#define CN                  2        /* control index */
#define NM                  3        /* numeric lock index */
#define AK                  4        /* right alt key */
#define CP                  5        /* capslock index */
#define ST                  6        /* stop output index */
#define EX                  7        /* extended code index */
#define ES                  8        /* escape and extended code index */

#define NORMAL              0x0000    /* normal key */
#define STP                 0x0001    /* scroll lock stop output*/
#define NUM                 0x0002    /* numeric lock */
#define CAPS                0x0004    /* capslock */
#define SHIFT               0x0008    /* shift */
#define CTRL                0x0010    /* control*/
#define EXT                 0x0020    /* extended scan code 0xe0 */
#define ESC                 0x0040    /* escape key press */
#define E1                  0x0080    /* extended scan code 0xe1 */
#define BRK                 0x0100    /* make break flag for keyboard */
#define ALT                 0x0200    /* right alt */

/* exports */

int i8042_kbd_init(void);
int i8042_tstc(void);
int i8042_getc(void);

#endif /* _I8042_H_ */
