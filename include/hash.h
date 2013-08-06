/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _HASH_H
#define _HASH_H

#if defined(CONFIG_SHA1SUM_VERIFY) || defined(CONFIG_CRC32_VERIFY)
#define CONFIG_HASH_VERIFY
#endif

struct hash_algo {
	const char *name;			/* Name of algorithm */
	int digest_size;			/* Length of digest */
	/**
	 * hash_func_ws: Generic hashing function
	 *
	 * This is the generic prototype for a hashing function. We only
	 * have the watchdog version at present.
	 *
	 * @input:	Input buffer
	 * @ilen:	Input buffer length
	 * @output:	Checksum result (length depends on algorithm)
	 * @chunk_sz:	Trigger watchdog after processing this many bytes
	 */
	void (*hash_func_ws)(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);
	int chunk_size;				/* Watchdog chunk size */
};

/*
 * Maximum digest size for all algorithms we support. Having this value
 * avoids a malloc() or C99 local declaration in common/cmd_hash.c.
 */
#define HASH_MAX_DIGEST_SIZE	32

enum {
	HASH_FLAG_VERIFY	= 1 << 0,	/* Enable verify mode */
	HASH_FLAG_ENV		= 1 << 1,	/* Allow env vars */
};

/**
 * hash_command: Process a hash command for a particular algorithm
 *
 * This common function is used to implement specific hash commands.
 *
 * @algo_name:		Hash algorithm being used (lower case!)
 * @flags:		Flags value (HASH_FLAG_...)
 * @cmdtp:		Pointer to command table entry
 * @flag:		Some flags normally 0 (see CMD_FLAG_.. above)
 * @argc:		Number of arguments (arg 0 must be the command text)
 * @argv:		Arguments
 */
int hash_command(const char *algo_name, int flags, cmd_tbl_t *cmdtp, int flag,
		 int argc, char * const argv[]);

/**
 * hash_block() - Hash a block according to the requested algorithm
 *
 * The caller probably knows the hash length for the chosen algorithm, but
 * in order to provide a general interface, and output_size parameter is
 * provided.
 *
 * @algo_name:		Hash algorithm to use
 * @data:		Data to hash
 * @len:		Lengh of data to hash in bytes
 * @output:		Place to put hash value
 * @output_size:	On entry, pointer to the number of bytes available in
 *			output. On exit, pointer to the number of bytes used.
 *			If NULL, then it is assumed that the caller has
 *			allocated enough space for the hash. This is possible
 *			since the caller is selecting the algorithm.
 * @return 0 if ok, -ve on error: -EPROTONOSUPPORT for an unknown algorithm,
 * -ENOSPC if the output buffer is not large enough.
 */
int hash_block(const char *algo_name, const void *data, unsigned int len,
	       uint8_t *output, int *output_size);

#endif
