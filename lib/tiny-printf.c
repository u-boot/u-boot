/*
 * Tiny printf version for SPL
 *
 * Copied from:
 * http://www.sparetimelabs.com/printfrevisited/printfrevisited.php
 *
 * Copyright (C) 2004,2008  Kustaa Nyholm
 *
 * SPDX-License-Identifier:	LGPL-2.1+
 */

#include <common.h>
#include <stdarg.h>
#include <serial.h>

static char *bf;
static char zs;

static void out(char c)
{
	*bf++ = c;
}

static void out_dgt(char dgt)
{
	out(dgt + (dgt < 10 ? '0' : 'a' - 10));
	zs = 1;
}

static void div_out(unsigned int *num, unsigned int div)
{
	unsigned char dgt = 0;

	while (*num >= div) {
		*num -= div;
		dgt++;
	}

	if (zs || dgt > 0)
		out_dgt(dgt);
}

int vprintf(const char *fmt, va_list va)
{
	char ch;
	char *p;
	unsigned int num;
	char buf[12];
	unsigned int div;

	while ((ch = *(fmt++))) {
		if (ch != '%') {
			putc(ch);
		} else {
			char lz = 0;
			char w = 0;

			ch = *(fmt++);
			if (ch == '0') {
				ch = *(fmt++);
				lz = 1;
			}

			if (ch >= '0' && ch <= '9') {
				w = 0;
				while (ch >= '0' && ch <= '9') {
					w = (w * 10) + ch - '0';
					ch = *fmt++;
				}
			}
			bf = buf;
			p = bf;
			zs = 0;

			switch (ch) {
			case 0:
				goto abort;
			case 'u':
			case 'd':
				num = va_arg(va, unsigned int);
				if (ch == 'd' && (int)num < 0) {
					num = -(int)num;
					out('-');
				}
				if (!num) {
					out_dgt(0);
				} else {
					for (div = 1000000000; div; div /= 10)
						div_out(&num, div);
				}
				break;
			case 'x':
				num = va_arg(va, unsigned int);
				if (!num) {
					out_dgt(0);
				} else {
					for (div = 0x10000000; div; div /= 0x10)
						div_out(&num, div);
				}
				break;
			case 'c':
				out((char)(va_arg(va, int)));
				break;
			case 's':
				p = va_arg(va, char*);
				break;
			case '%':
				out('%');
			default:
				break;
			}

			*bf = 0;
			bf = p;
			while (*bf++ && w > 0)
				w--;
			while (w-- > 0)
				putc(lz ? '0' : ' ');
			if (p) {
				while ((ch = *p++))
					putc(ch);
			}
		}
	}

abort:
	return 0;
}

int printf(const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = vprintf(fmt, va);
	va_end(va);

	return ret;
}
