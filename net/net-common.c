// SPDX-License-Identifier: GPL-2.0

void copy_filename(char *dst, const char *src, int size)
{
	if (src && *src && (*src == '"')) {
		++src;
		--size;
	}

	while ((--size > 0) && src && *src && (*src != '"'))
		*dst++ = *src++;
	*dst = '\0';
}
