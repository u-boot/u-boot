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

/*
 * This code in here may execute before the DRAM is initialised, so
 * we should make sure that it doesn't touch BSS, which some boards
 * put in DRAM.
 */
static char *bf __attribute__ ((section(".data")));
static char zs __attribute__ ((section(".data")));

/* Current position in sprintf() output string */
static char *outstr __attribute__ ((section(".data")));

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

int _vprintf(const char *fmt, va_list va, void (*putc)(const char ch))
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
			bool lz = false;
			int width = 0;

			ch = *(fmt++);
			if (ch == '0') {
				ch = *(fmt++);
				lz = 1;
			}

			if (ch >= '0' && ch <= '9') {
				width = 0;
				while (ch >= '0' && ch <= '9') {
					width = (width * 10) + ch - '0';
					ch = *fmt++;
				}
			}
			bf = buf;
			p = bf;
			zs = 0;

			switch (ch) {
			case '\0':
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
			while (*bf++ && width > 0)
				width--;
			while (width-- > 0)
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

int vprintf(const char *fmt, va_list va)
{
	return _vprintf(fmt, va, putc);
}

int printf(const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = _vprintf(fmt, va, putc);
	va_end(va);

	return ret;
}

static void putc_outstr(char ch)
{
	*outstr++ = ch;
}

int sprintf(char *buf, const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	outstr = buf;
	ret = _vprintf(fmt, va, putc_outstr);
	va_end(va);
	*outstr = '\0';

	return ret;
}

/* Note that size is ignored */
int snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	outstr = buf;
	ret = _vprintf(fmt, va, putc_outstr);
	va_end(va);
	*outstr = '\0';

	return ret;
}
