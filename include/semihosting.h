/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ifndef _SEMIHOSTING_H
#define _SEMIHOSTING_H

/**
 * enum smh_open_mode - Numeric file modes for use with smh_open()
 * MODE_READ: 'r'
 * MODE_BINARY: 'b'
 * MODE_PLUS: '+'
 * MODE_WRITE: 'w'
 * MODE_APPEND: 'a'
 *
 * These modes represent the mode string used by fopen(3) in a form which can
 * be passed to smh_open(). These do NOT correspond directly to %O_RDONLY,
 * %O_CREAT, etc; see fopen(3) for details. In particular, @MODE_PLUS
 * effectively results in adding %O_RDWR, and @MODE_WRITE will add %O_TRUNC.
 * For compatibility, @MODE_BINARY should be added when opening non-text files
 * (such as images).
 */
enum smh_open_mode {
	MODE_READ	= 0x0,
	MODE_BINARY	= 0x1,
	MODE_PLUS	= 0x2,
	MODE_WRITE	= 0x4,
	MODE_APPEND	= 0x8,
};

long smh_open(const char *fname, enum smh_open_mode mode);
long smh_read(long fd, void *memp, size_t len);
long smh_close(long fd);
long smh_flen(long fd);

#endif /* _SEMIHOSTING_H */
