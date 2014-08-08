/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#define I8042_DATA_REG      (CONFIG_SYS_ISA_IO + 0x0060)    /* keyboard i/o buffer */
#define I8042_STATUS_REG    (CONFIG_SYS_ISA_IO + 0x0064)    /* keyboard status read */
#define I8042_COMMAND_REG   (CONFIG_SYS_ISA_IO + 0x0064)    /* keyboard ctrl write */

enum {
	/* Output register (I8042_DATA_REG) has data for system */
	I8042_STATUS_OUT_DATA	= 1 << 0,
	I8042_STATUS_IN_DATA	= 1 << 1,
};

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

/**
 * Flush all buffer from keyboard controller to host.
 */
void i8042_flush(void);

/**
 * Disables the keyboard so that key strokes no longer generate scancodes to
 * the host.
 *
 * @return 0 if ok, -1 if keyboard input was found while disabling
 */
int i8042_disable(void);

struct stdio_dev;

int i8042_kbd_init(void);
int i8042_tstc(struct stdio_dev *dev);
int i8042_getc(struct stdio_dev *dev);

#endif /* _I8042_H_ */
