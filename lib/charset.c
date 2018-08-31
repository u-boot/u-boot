// SPDX-License-Identifier: GPL-2.0+
/*
 *  charset conversion utils
 *
 *  Copyright (c) 2017 Rob Clark
 */

#include <charset.h>
#include <malloc.h>

s32 utf8_get(const char **src)
{
	s32 code = 0;
	unsigned char c;

	if (!src || !*src)
		return -1;
	if (!**src)
		return 0;
	c = **src;
	if (c >= 0x80) {
		++*src;
		if (!**src)
			return -1;
		/*
		 * We do not expect a continuation byte (0x80 - 0xbf).
		 * 0x80 is coded as 0xc2 0x80, so we cannot have less then 0xc2
		 * here.
		 * The highest code point is 0x10ffff which is coded as
		 * 0xf4 0x8f 0xbf 0xbf. So we cannot have a byte above 0xf4.
		 */
		if (c < 0xc2 || code > 0xf4)
			return -1;
		if (c >= 0xe0) {
			if (c >= 0xf0) {
				/* 0xf0 - 0xf4 */
				c &= 0x07;
				code = c << 18;
				c = **src;
				++*src;
				if (!**src)
					return -1;
				if (c < 0x80 || c > 0xbf)
					return -1;
				c &= 0x3f;
			} else {
				/* 0xe0 - 0xef */
				c &= 0x0f;
			}
			code += c << 12;
			if ((code >= 0xD800 && code <= 0xDFFF) ||
			    code >= 0x110000)
				return -1;
			c = **src;
			++*src;
			if (!**src)
				return -1;
			if (c < 0x80 || c > 0xbf)
				return -1;
		}
		/* 0xc0 - 0xdf or continuation byte (0x80 - 0xbf) */
		c &= 0x3f;
		code += c << 6;
		c = **src;
		if (c < 0x80 || c > 0xbf)
			return -1;
		c &= 0x3f;
	}
	code += c;
	++*src;
	return code;
}

int utf8_put(s32 code, char **dst)
{
	if (!dst || !*dst)
		return -1;
	if ((code >= 0xD800 && code <= 0xDFFF) || code >= 0x110000)
		return -1;
	if (code <= 0x007F) {
		**dst = code;
	} else {
		if (code <= 0x07FF) {
			**dst = code >> 6 | 0xC0;
		} else {
			if (code < 0x10000) {
				**dst = code >> 12 | 0xE0;
			} else {
				**dst = code >> 18 | 0xF0;
				++*dst;
				**dst = (code >> 12 & 0x3F) | 0x80;
			}
			++*dst;
			**dst = (code >> 6 & 0x3F) | 0x80;
		}
		++*dst;
		**dst = (code & 0x3F) | 0x80;
	}
	++*dst;
	return 0;
}

size_t utf8_utf16_strnlen(const char *src, size_t count)
{
	size_t len = 0;

	for (; *src && count; --count)  {
		s32 code = utf8_get(&src);

		if (!code)
			break;
		if (code < 0) {
			/* Reserve space for a replacement character */
			len += 1;
		} else if (code < 0x10000) {
			len += 1;
		} else {
			len += 2;
		}
	}
	return len;
}

int utf8_utf16_strncpy(u16 **dst, const char *src, size_t count)
{
	if (!src || !dst || !*dst)
		return -1;

	for (; count && *src; --count) {
		s32 code = utf8_get(&src);

		if (code < 0)
			code = '?';
		utf16_put(code, dst);
	}
	**dst = 0;
	return 0;
}

s32 utf16_get(const u16 **src)
{
	s32 code, code2;

	if (!src || !*src)
		return -1;
	if (!**src)
		return 0;
	code = **src;
	++*src;
	if (code >= 0xDC00 && code <= 0xDFFF)
		return -1;
	if (code >= 0xD800 && code <= 0xDBFF) {
		if (!**src)
			return -1;
		code &= 0x3ff;
		code <<= 10;
		code += 0x10000;
		code2 = **src;
		++*src;
		if (code2 <= 0xDC00 || code2 >= 0xDFFF)
			return -1;
		code2 &= 0x3ff;
		code += code2;
	}
	return code;
}

int utf16_put(s32 code, u16 **dst)
{
	if (!dst || !*dst)
		return -1;
	if ((code >= 0xD800 && code <= 0xDFFF) || code >= 0x110000)
		return -1;
	if (code < 0x10000) {
		**dst = code;
	} else {
		code -= 0x10000;
		**dst = code >> 10 | 0xD800;
		++*dst;
		**dst = (code & 0x3ff) | 0xDC00;
	}
	++*dst;
	return 0;
}

size_t utf16_strnlen(const u16 *src, size_t count)
{
	size_t len = 0;

	for (; *src && count; --count)  {
		s32 code = utf16_get(&src);

		if (!code)
			break;
		/*
		 * In case of an illegal sequence still reserve space for a
		 * replacement character.
		 */
		++len;
	}
	return len;
}

size_t utf16_utf8_strnlen(const u16 *src, size_t count)
{
	size_t len = 0;

	for (; *src && count; --count)  {
		s32 code = utf16_get(&src);

		if (!code)
			break;
		if (code < 0)
			/* Reserve space for a replacement character */
			len += 1;
		else if (code < 0x80)
			len += 1;
		else if (code < 0x800)
			len += 2;
		else if (code < 0x10000)
			len += 3;
		else
			len += 4;
	}
	return len;
}

int utf16_utf8_strncpy(char **dst, const u16 *src, size_t count)
{
	if (!src || !dst || !*dst)
		return -1;

	for (; count && *src; --count) {
		s32 code = utf16_get(&src);

		if (code < 0)
			code = '?';
		utf8_put(code, dst);
	}
	**dst = 0;
	return 0;
}


size_t u16_strlen(const u16 *in)
{
	size_t i;
	for (i = 0; in[i]; i++);
	return i;
}

size_t u16_strnlen(const u16 *in, size_t count)
{
	size_t i;
	for (i = 0; count-- && in[i]; i++);
	return i;
}

uint16_t *utf16_strcpy(uint16_t *dest, const uint16_t *src)
{
	uint16_t *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;

}

uint16_t *utf16_strdup(const uint16_t *s)
{
	uint16_t *new;

	if (!s)
		return NULL;
	new = malloc((u16_strlen(s) + 1) * 2);
	if (!new)
		return NULL;
	utf16_strcpy(new, s);
	return new;
}

/* Convert UTF-16 to UTF-8.  */
uint8_t *utf16_to_utf8(uint8_t *dest, const uint16_t *src, size_t size)
{
	uint32_t code_high = 0;

	while (size--) {
		uint32_t code = *src++;

		if (code_high) {
			if (code >= 0xDC00 && code <= 0xDFFF) {
				/* Surrogate pair.  */
				code = ((code_high - 0xD800) << 10) + (code - 0xDC00) + 0x10000;

				*dest++ = (code >> 18) | 0xF0;
				*dest++ = ((code >> 12) & 0x3F) | 0x80;
				*dest++ = ((code >> 6) & 0x3F) | 0x80;
				*dest++ = (code & 0x3F) | 0x80;
			} else {
				/* Error...  */
				*dest++ = '?';
				/* *src may be valid. Don't eat it.  */
				src--;
			}

			code_high = 0;
		} else {
			if (code <= 0x007F) {
				*dest++ = code;
			} else if (code <= 0x07FF) {
				*dest++ = (code >> 6) | 0xC0;
				*dest++ = (code & 0x3F) | 0x80;
			} else if (code >= 0xD800 && code <= 0xDBFF) {
				code_high = code;
				continue;
			} else if (code >= 0xDC00 && code <= 0xDFFF) {
				/* Error... */
				*dest++ = '?';
			} else if (code < 0x10000) {
				*dest++ = (code >> 12) | 0xE0;
				*dest++ = ((code >> 6) & 0x3F) | 0x80;
				*dest++ = (code & 0x3F) | 0x80;
			} else {
				*dest++ = (code >> 18) | 0xF0;
				*dest++ = ((code >> 12) & 0x3F) | 0x80;
				*dest++ = ((code >> 6) & 0x3F) | 0x80;
				*dest++ = (code & 0x3F) | 0x80;
			}
		}
	}

	return dest;
}

uint16_t *utf8_to_utf16(uint16_t *dest, const uint8_t *src, size_t size)
{
	while (size--) {
		int extension_bytes;
		uint32_t code;

		extension_bytes = 0;
		if (*src <= 0x7f) {
			code = *src++;
			/* Exit on zero byte */
			if (!code)
				size = 0;
		} else if (*src <= 0xbf) {
			/* Illegal code */
			code = '?';
		} else if (*src <= 0xdf) {
			code = *src++ & 0x1f;
			extension_bytes = 1;
		} else if (*src <= 0xef) {
			code = *src++ & 0x0f;
			extension_bytes = 2;
		} else if (*src <= 0xf7) {
			code = *src++ & 0x07;
			extension_bytes = 3;
		} else {
			/* Illegal code */
			code = '?';
		}

		for (; extension_bytes && size; --size, --extension_bytes) {
			if ((*src & 0xc0) == 0x80) {
				code <<= 6;
				code |= *src++ & 0x3f;
			} else {
				/* Illegal code */
				code = '?';
				++src;
				--size;
				break;
			}
		}

		if (code < 0x10000) {
			*dest++ = code;
		} else {
			/*
			 * Simplified expression for
			 * (((code - 0x10000) >> 10) & 0x3ff) | 0xd800
			 */
			*dest++ = (code >> 10) + 0xd7c0;
			*dest++ = (code & 0x3ff) | 0xdc00;
		}
	}
	return dest;
}
