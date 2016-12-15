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

struct printf_info {
	char *bf;	/* Digit buffer */
	char zs;	/* non-zero if a digit has been written */
	char *outstr;	/* Next output position for sprintf() */

	/* Output a character */
	void (*putc)(struct printf_info *info, char ch);
};

void putc_normal(struct printf_info *info, char ch)
{
	putc(ch);
}

static void out(struct printf_info *info, char c)
{
	*info->bf++ = c;
}

static void out_dgt(struct printf_info *info, char dgt)
{
	out(info, dgt + (dgt < 10 ? '0' : 'a' - 10));
	info->zs = 1;
}

static void div_out(struct printf_info *info, unsigned int *num,
		    unsigned int div)
{
	unsigned char dgt = 0;

	while (*num >= div) {
		*num -= div;
		dgt++;
	}

	if (info->zs || dgt > 0)
		out_dgt(info, dgt);
}

int _vprintf(struct printf_info *info, const char *fmt, va_list va)
{
	char ch;
	char *p;
	unsigned int num;
	char buf[12];
	unsigned int div;

	while ((ch = *(fmt++))) {
		if (ch != '%') {
			info->putc(info, ch);
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
			info->bf = buf;
			p = info->bf;
			info->zs = 0;

			switch (ch) {
			case '\0':
				goto abort;
			case 'u':
			case 'd':
				num = va_arg(va, unsigned int);
				if (ch == 'd' && (int)num < 0) {
					num = -(int)num;
					out(info, '-');
				}
				if (!num) {
					out_dgt(info, 0);
				} else {
					for (div = 1000000000; div; div /= 10)
						div_out(info, &num, div);
				}
				break;
			case 'x':
				num = va_arg(va, unsigned int);
				if (!num) {
					out_dgt(info, 0);
				} else {
					for (div = 0x10000000; div; div /= 0x10)
						div_out(info, &num, div);
				}
				break;
			case 'c':
				out(info, (char)(va_arg(va, int)));
				break;
			case 's':
				p = va_arg(va, char*);
				break;
			case '%':
				out(info, '%');
			default:
				break;
			}

			*info->bf = 0;
			info->bf = p;
			while (*info->bf++ && width > 0)
				width--;
			while (width-- > 0)
				info->putc(info, lz ? '0' : ' ');
			if (p) {
				while ((ch = *p++))
					info->putc(info, ch);
			}
		}
	}

abort:
	return 0;
}

int vprintf(const char *fmt, va_list va)
{
	struct printf_info info;

	info.putc = putc_normal;
	return _vprintf(&info, fmt, va);
}

int printf(const char *fmt, ...)
{
	struct printf_info info;

	va_list va;
	int ret;

	info.putc = putc_normal;
	va_start(va, fmt);
	ret = _vprintf(&info, fmt, va);
	va_end(va);

	return ret;
}

static void putc_outstr(struct printf_info *info, char ch)
{
	*info->outstr++ = ch;
}

int sprintf(char *buf, const char *fmt, ...)
{
	struct printf_info info;
	va_list va;
	int ret;

	va_start(va, fmt);
	info.outstr = buf;
	info.putc = putc_outstr;
	ret = _vprintf(&info, fmt, va);
	va_end(va);
	*info.outstr = '\0';

	return ret;
}

/* Note that size is ignored */
int snprintf(char *buf, size_t size, const char *fmt, ...)
{
	struct printf_info info;
	va_list va;
	int ret;

	va_start(va, fmt);
	info.outstr = buf;
	info.putc = putc_outstr;
	ret = _vprintf(&info, fmt, va);
	va_end(va);
	*info.outstr = '\0';

	return ret;
}

void __assert_fail(const char *assertion, const char *file, unsigned line,
		   const char *function)
{
	/* This will not return */
	printf("%s:%u: %s: Assertion `%s' failed.", file, line, function,
	       assertion);
	hang();
}
