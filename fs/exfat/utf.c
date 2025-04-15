/*
	utf.c (13.09.09)
	exFAT file system implementation library.

	Free exFAT implementation.
	Copyright (C) 2010-2023  Andrew Nayenko

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "exfat.h"
#include <errno.h>

static char* wchar_to_utf8(char* output, u32 wc, size_t outsize)
{
	if (wc <= 0x7f)
	{
		if (outsize < 1)
			return NULL;
		*output++ = (char) wc;
	}
	else if (wc <= 0x7ff)
	{
		if (outsize < 2)
			return NULL;
		*output++ = 0xc0 | (wc >> 6);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else if (wc <= 0xffff)
	{
		if (outsize < 3)
			return NULL;
		*output++ = 0xe0 | (wc >> 12);
		*output++ = 0x80 | ((wc >> 6) & 0x3f);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else if (wc <= 0x1fffff)
	{
		if (outsize < 4)
			return NULL;
		*output++ = 0xf0 | (wc >> 18);
		*output++ = 0x80 | ((wc >> 12) & 0x3f);
		*output++ = 0x80 | ((wc >> 6) & 0x3f);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else if (wc <= 0x3ffffff)
	{
		if (outsize < 5)
			return NULL;
		*output++ = 0xf8 | (wc >> 24);
		*output++ = 0x80 | ((wc >> 18) & 0x3f);
		*output++ = 0x80 | ((wc >> 12) & 0x3f);
		*output++ = 0x80 | ((wc >> 6) & 0x3f);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else if (wc <= 0x7fffffff)
	{
		if (outsize < 6)
			return NULL;
		*output++ = 0xfc | (wc >> 30);
		*output++ = 0x80 | ((wc >> 24) & 0x3f);
		*output++ = 0x80 | ((wc >> 18) & 0x3f);
		*output++ = 0x80 | ((wc >> 12) & 0x3f);
		*output++ = 0x80 | ((wc >> 6) & 0x3f);
		*output++ = 0x80 | (wc & 0x3f);
	}
	else
		return NULL;

	return output;
}

static const le16_t* utf16_to_wchar(const le16_t* input, u32* wc,
		size_t insize)
{
	if ((le16_to_cpu(input[0]) & 0xfc00) == 0xd800)
	{
		if (insize < 2 || (le16_to_cpu(input[1]) & 0xfc00) != 0xdc00)
			return NULL;
		*wc = ((u32) (le16_to_cpu(input[0]) & 0x3ff) << 10);
		*wc |= (le16_to_cpu(input[1]) & 0x3ff);
		*wc += 0x10000;
		return input + 2;
	}
	else
	{
		*wc = le16_to_cpu(*input);
		return input + 1;
	}
}

int exfat_utf16_to_utf8(char* output, const le16_t* input, size_t outsize,
		size_t insize)
{
	const le16_t* iptr = input;
	const le16_t* iend = input + insize;
	char* optr = output;
	const char* oend = output + outsize;
	u32 wc;

	while (iptr < iend)
	{
		iptr = utf16_to_wchar(iptr, &wc, iend - iptr);
		if (iptr == NULL)
		{
			exfat_error("illegal UTF-16 sequence");
			return -EILSEQ;
		}
		optr = wchar_to_utf8(optr, wc, oend - optr);
		if (optr == NULL)
		{
			exfat_error("name is too long");
			return -ENAMETOOLONG;
		}
		if (wc == 0)
			return 0;
	}
	if (optr >= oend)
	{
		exfat_error("name is too long");
		return -ENAMETOOLONG;
	}
	*optr = '\0';
	return 0;
}

static const char* utf8_to_wchar(const char* input, u32* wc,
		size_t insize)
{
	size_t size;
	size_t i;

	if (insize == 0)
		exfat_bug("no input for utf8_to_wchar");

	if ((input[0] & 0x80) == 0)
	{
		*wc = (u32) input[0];
		return input + 1;
	}
	else if ((input[0] & 0xe0) == 0xc0)
	{
		*wc = ((u32) input[0] & 0x1f) << 6;
		size = 2;
	}
	else if ((input[0] & 0xf0) == 0xe0)
	{
		*wc = ((u32) input[0] & 0x0f) << 12;
		size = 3;
	}
	else if ((input[0] & 0xf8) == 0xf0)
	{
		*wc = ((u32) input[0] & 0x07) << 18;
		size = 4;
	}
	else if ((input[0] & 0xfc) == 0xf8)
	{
		*wc = ((u32) input[0] & 0x03) << 24;
		size = 5;
	}
	else if ((input[0] & 0xfe) == 0xfc)
	{
		*wc = ((u32) input[0] & 0x01) << 30;
		size = 6;
	}
	else
		return NULL;

	if (insize < size)
		return NULL;

	/* the first byte is handled above */
	for (i = 1; i < size; i++)
	{
		if ((input[i] & 0xc0) != 0x80)
			return NULL;
		*wc |= (input[i] & 0x3f) << ((size - i - 1) * 6);
	}

	return input + size;
}

static le16_t* wchar_to_utf16(le16_t* output, u32 wc, size_t outsize)
{
	if (wc <= 0xffff) /* if character is from BMP */
	{
		if (outsize == 0)
			return NULL;
		output[0] = cpu_to_le16(wc);
		return output + 1;
	}
	if (outsize < 2)
		return NULL;
	wc -= 0x10000;
	output[0] = cpu_to_le16(0xd800 | ((wc >> 10) & 0x3ff));
	output[1] = cpu_to_le16(0xdc00 | (wc & 0x3ff));
	return output + 2;
}

int exfat_utf8_to_utf16(le16_t* output, const char* input, size_t outsize,
		size_t insize)
{
	const char* iptr = input;
	const char* iend = input + insize;
	le16_t* optr = output;
	const le16_t* oend = output + outsize;
	u32 wc;

	while (iptr < iend)
	{
		iptr = utf8_to_wchar(iptr, &wc, iend - iptr);
		if (iptr == NULL)
		{
			exfat_error("illegal UTF-8 sequence");
			return -EILSEQ;
		}
		optr = wchar_to_utf16(optr, wc, oend - optr);
		if (optr == NULL)
		{
			exfat_error("name is too long");
			return -ENAMETOOLONG;
		}
		if (wc == 0)
			break;
	}
	if (optr >= oend)
	{
		exfat_error("name is too long");
		return -ENAMETOOLONG;
	}
	*optr = cpu_to_le16(0);
	return 0;
}

size_t exfat_utf16_length(const le16_t* str)
{
	size_t i = 0;

	while (le16_to_cpu(str[i]))
		i++;
	return i;
}
