/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _KBD_H_
#define _KBD_H_

extern int kbd_testc(void);
extern int kbd_getc(void);
extern void kbd_interrupt(void);
extern char *kbd_initialize(void);

unsigned char kbd_is_init(void);
#define KBD_INTERRUPT 1
#endif
