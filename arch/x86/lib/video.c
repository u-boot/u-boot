/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
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

#include <common.h>
#include <pci.h>
#include <stdio_dev.h>
#include <i8042.h>
#include <asm/ptrace.h>
#include <asm/realmode.h>
#include <asm/io.h>
#include <asm/pci.h>

/* basic textmode I/O from linux kernel */
static char *vidmem = (char *)0xb8000;
static int vidport;
static int lines, cols;
static int orig_x, orig_y;

static void beep(int dur)
{
	int i;

	outb_p(3, 0x61);
	for (i = 0; i < 10*dur; i++)
		udelay(1000);

	outb_p(0, 0x61);
}

static void scroll(void)
{
	int i;

	memcpy(vidmem, vidmem + cols * 2, (lines - 1) * cols * 2);
	for (i = (lines - 1) * cols * 2; i < lines * cols * 2; i += 2)
		vidmem[i] = ' ';
}

static void __video_putc(const char c, int *x, int *y)
{
	if (c == '\n') {
		(*x) = 0;
		if (++(*y) >= lines) {
			scroll();
			(*y)--;
		}
	} else if (c == '\b') {
		if ((*x) != 0) {
			--(*x);
			vidmem[((*x) + cols * (*y)) * 2] = ' ';
		}
	} else if (c == '\r') {
		(*x) = 0;

	} else if (c == '\a') {
		beep(3);

	} else if (c == '\t') {
		__video_putc(' ', x, y);
		__video_putc(' ', x, y);
		__video_putc(' ', x, y);
		__video_putc(' ', x, y);
		__video_putc(' ', x, y);
		__video_putc(' ', x, y);
		__video_putc(' ', x, y);
		__video_putc(' ', x, y);
	} else if (c == '\v') {
		switch ((*x) % 8) {
		case 0:
			__video_putc(' ', x, y);
		case 7:
			__video_putc(' ', x, y);
		case 6:
			__video_putc(' ', x, y);
		case 5:
			__video_putc(' ', x, y);
		case 4:
			__video_putc(' ', x, y);
		case 3:
			__video_putc(' ', x, y);
		case 2:
			__video_putc(' ', x, y);
		case 1:
			__video_putc(' ', x, y);
		}
	} else if (c == '\f') {
		int i;
		for (i = 0; i < lines * cols * 2; i += 2)
			vidmem[i] = 0;
		(*x) = 0;
		(*y) = 0;
	} else {
		vidmem[((*x) + cols * (*y)) * 2] = c;
		if (++(*x) >= cols) {
			(*x) = 0;
			if (++(*y) >= lines) {
				scroll();
				(*y)--;
			}
		}
	}
}

static void video_putc(const char c)
{
	int x,y,pos;

	x = orig_x;
	y = orig_y;

	__video_putc(c, &x, &y);

	orig_x = x;
	orig_y = y;

	pos = (x + cols * y) * 2;	/* Update cursor position */
	outb_p(14, vidport);
	outb_p(0xff & (pos >> 9), vidport+1);
	outb_p(15, vidport);
	outb_p(0xff & (pos >> 1), vidport+1);
}

static void video_puts(const char *s)
{
	int x,y,pos;
	char c;

	x = orig_x;
	y = orig_y;

	while ((c = *s++) != '\0')
		__video_putc(c, &x, &y);

	orig_x = x;
	orig_y = y;

	pos = (x + cols * y) * 2;	/* Update cursor position */
	outb_p(14, vidport);
	outb_p(0xff & (pos >> 9), vidport+1);
	outb_p(15, vidport);
	outb_p(0xff & (pos >> 1), vidport+1);
}

int video_init(void)
{
	u16 pos;

	static struct stdio_dev vga_dev;
	static struct stdio_dev kbd_dev;

	vidmem = (char *) 0xb8000;
	vidport = 0x3d4;

	lines = 25;
	cols = 80;

	outb_p(14, vidport);
	pos = inb_p(vidport+1);
	pos <<= 8;
	outb_p(15, vidport);
	pos |= inb_p(vidport+1);

	orig_x = pos%cols;
	orig_y = pos/cols;

#if 0
	printf("pos %x %d %d\n", pos, orig_x, orig_y);
#endif
	if (orig_y > lines)
		orig_x = orig_y =0;

	memset(&vga_dev, 0, sizeof(vga_dev));
	strcpy(vga_dev.name, "vga");
	vga_dev.ext   = 0;
	vga_dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_SYSTEM;
	vga_dev.putc  = video_putc;        /* 'putc' function */
	vga_dev.puts  = video_puts;        /* 'puts' function */
	vga_dev.tstc  = NULL;              /* 'tstc' function */
	vga_dev.getc  = NULL;              /* 'getc' function */

	if (stdio_register(&vga_dev) == 0)
		return 1;

	if (i8042_kbd_init())
		return 1;

	memset(&kbd_dev, 0, sizeof(kbd_dev));
	strcpy(kbd_dev.name, "kbd");
	kbd_dev.ext   = 0;
	kbd_dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	kbd_dev.putc  = NULL;        /* 'putc' function */
	kbd_dev.puts  = NULL;        /* 'puts' function */
	kbd_dev.tstc  = i8042_tstc;  /* 'tstc' function */
	kbd_dev.getc  = i8042_getc;  /* 'getc' function */

	if (stdio_register(&kbd_dev) == 0)
		return 1;

	return 0;
}


int drv_video_init(void)
{
	if (video_bios_init())
		return 1;

	return video_init();
}
