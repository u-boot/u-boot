/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
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
	/*
	 * hash_init: Create the context for progressive hashing
	 *
	 * @algo: Pointer to the hash_algo struct
	 * @ctxp: Pointer to the pointer of the context for hashing
	 * @return 0 if ok, -1 on error
	 */
	int (*hash_init)(struct hash_algo *algo, void **ctxp);
	/*
	 * hash_update: Perform hashing on the given buffer
	 *
	 * The context is freed by this function if an error occurs.
	 *
	 * @algo: Pointer to the hash_algo struct
	 * @ctx: Pointer to the context for hashing
	 * @buf: Pointer to the buffer being hashed
	 * @size: Size of the buffer being hashed
	 * @is_last: 1 if this is the last update; 0 otherwise
	 * @return 0 if ok, -1 on error
	 */
	int (*hash_update)(struct hash_algo *algo, void *ctx, const void *buf,
			   unsigned int size, int is_last);
	/*
	 * hash_finish: Write the hash result to the given buffer
	 *
	 * The context is freed by this function.
	 *
	 * @algo: Pointer to the hash_algo struct
	 * @ctx: Pointer to the context for hashing
	 * @dest_buf: Pointer to the buffer for the result
	 * @size: Size of the buffer for the result
	 * @return 0 if ok, -ENOSPC if size of the result buffer is too small
	 *   or -1 on other errors
	 */
	int (*hash_finish)(struct hash_algo *algo, void *ctx, void *dest_buf,
			   int size);
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

/**
 * hash_lookup_algo() - Look up the hash_algo struct for an algorithm
 *
 * The function returns the pointer to the struct or -EPROTONOSUPPORT if the
 * algorithm is not available.
 *
 * @algo_name: Hash algorithm to look up
 * @algop: Pointer to the hash_algo struct if found
 *
 * @return 0 if ok, -EPROTONOSUPPORT for an unknown algorithm.
 */
int hash_lookup_algo(const char *algo_name, struct hash_algo **algop);
#endif
