// SPDX-License-Identifier: GPL-2.0+
/*
 * String functions
 *
 * Copyright (c) 2020 AKASHI Takahiro, Linaro Limited
 */

#include <common.h>
#include <charset.h>
#include <efi_loader.h>

/**
 * efi_create_indexed_name - create a string name with an index
 * @buffer:	Buffer
 * @name:	Name string
 * @index:	Index
 *
 * Create a utf-16 string with @name, appending @index.
 * For example, L"Capsule0001"
 *
 * The caller must ensure that the buffer has enough space for the resulting
 * string including the trailing L'\0'.
 *
 * Return: A pointer to the next position after the created string
 *	   in @buffer, or NULL otherwise
 */
u16 *efi_create_indexed_name(u16 *buffer, size_t buffer_size, const char *name,
			     unsigned int index)
{
	u16 *p = buffer;
	char index_buf[5];
	size_t size;

	size = (utf8_utf16_strlen(name) * sizeof(u16) +
		sizeof(index_buf) * sizeof(u16));
	if (buffer_size < size)
		return NULL;
	utf8_utf16_strcpy(&p, name);
	snprintf(index_buf, sizeof(index_buf), "%04X", index);
	utf8_utf16_strcpy(&p, index_buf);

	return p;
}
