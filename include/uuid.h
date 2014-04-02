/*
 * Copyright (C) 2014 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __UUID_H__
#define __UUID_H__

enum {
	UUID_STR_FORMAT_STD,
	UUID_STR_FORMAT_GUID
};

#define UUID_STR_LEN		36
#define UUID_BIN_LEN		16

int uuid_str_valid(const char *uuid);
int uuid_str_to_bin(char *uuid_str, unsigned char *uuid_bin, int str_format);
void uuid_bin_to_str(unsigned char *uuid_bin, char *uuid_str, int str_format);
#endif
