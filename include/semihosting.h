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

/**
 * smh_open() - Open a file on the host
 * @fname: The name of the file to open
 * @mode: The mode to use when opening the file
 *
 * Return: Either a file descriptor or a negative error on failure
 */
long smh_open(const char *fname, enum smh_open_mode mode);

/**
 * smh_read() - Read data from a file
 * @fd: A file descriptor returned from smh_open()
 * @memp: Pointer to a buffer of memory of at least @len bytes
 * @len: The number of bytes to read
 *
 * Return:
 * * The number of bytes read on success, with 0 indicating %EOF
 * * A negative error on failure
 */
long smh_read(long fd, void *memp, size_t len);

/**
 * smh_close() - Close an open file
 * @fd: A file descriptor returned from smh_open()
 *
 * Return: 0 on success or negative error on failure
 */
long smh_close(long fd);

/**
 * smh_flen() - Get the length of a file
 * @fd: A file descriptor returned from smh_open()
 *
 * Return: The length of the file, in bytes, or a negative error on failure
 */
long smh_flen(long fd);

#endif /* _SEMIHOSTING_H */
