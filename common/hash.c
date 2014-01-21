/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <hw_sha.h>
#include <hash.h>
#include <sha1.h>
#include <sha256.h>
#include <asm/io.h>
#include <asm/errno.h>

/*
 * These are the hash algorithms we support. Chips which support accelerated
 * crypto could perhaps add named version of these algorithms here. Note that
 * algorithm names must be in lower case.
 */
static struct hash_algo hash_algo[] = {
	/*
	 * CONFIG_SHA_HW_ACCEL is defined if hardware acceleration is
	 * available.
	 */
#ifdef CONFIG_SHA_HW_ACCEL
	{
		"sha1",
		SHA1_SUM_LEN,
		hw_sha1,
		CHUNKSZ_SHA1,
	}, {
		"sha256",
		SHA256_SUM_LEN,
		hw_sha256,
		CHUNKSZ_SHA256,
	},
#endif
	/*
	 * This is CONFIG_CMD_SHA1SUM instead of CONFIG_SHA1 since otherwise
	 * it bloats the code for boards which use SHA1 but not the 'hash'
	 * or 'sha1sum' commands.
	 */
#ifdef CONFIG_CMD_SHA1SUM
	{
		"sha1",
		SHA1_SUM_LEN,
		sha1_csum_wd,
		CHUNKSZ_SHA1,
	},
#define MULTI_HASH
#endif
#ifdef CONFIG_SHA256
	{
		"sha256",
		SHA256_SUM_LEN,
		sha256_csum_wd,
		CHUNKSZ_SHA256,
	},
#define MULTI_HASH
#endif
	{
		"crc32",
		4,
		crc32_wd_buf,
		CHUNKSZ_CRC32,
	},
};

#if defined(CONFIG_HASH_VERIFY) || defined(CONFIG_CMD_HASH)
#define MULTI_HASH
#endif

/* Try to minimize code size for boards that don't want much hashing */
#ifdef MULTI_HASH
#define multi_hash()	1
#else
#define multi_hash()	0
#endif

/**
 * store_result: Store the resulting sum to an address or variable
 *
 * @algo:		Hash algorithm being used
 * @sum:		Hash digest (algo->digest_size bytes)
 * @dest:		Destination, interpreted as a hex address if it starts
 *			with * (or allow_env_vars is 0) or otherwise as an
 *			environment variable.
 * @allow_env_vars:	non-zero to permit storing the result to an
 *			variable environment
 */
static void store_result(struct hash_algo *algo, const u8 *sum,
			 const char *dest, int allow_env_vars)
{
	unsigned int i;
	int env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	if (allow_env_vars) {
		if (*dest == '*')
			dest++;
		else
			env_var = 1;
	}

	if (env_var) {
		char str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		char *str_ptr = str_output;

		for (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		str_ptr = '\0';
		setenv(dest, str_output);
	} else {
		ulong addr;
		void *buf;

		addr = simple_strtoul(dest, NULL, 16);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(buf, sum, algo->digest_size);
		unmap_sysmem(buf);
	}
}

/**
 * parse_verify_sum: Parse a hash verification parameter
 *
 * @algo:		Hash algorithm being used
 * @verify_str:		Argument to parse. If it starts with * then it is
 *			interpreted as a hex address containing the hash.
 *			If the length is exactly the right number of hex digits
 *			for the digest size, then we assume it is a hex digest.
 *			Otherwise we assume it is an environment variable, and
 *			look up its value (it must contain a hex digest).
 * @vsum:		Returns binary digest value (algo->digest_size bytes)
 * @allow_env_vars:	non-zero to permit storing the result to an environment
 *			variable. If 0 then verify_str is assumed to be an
 *			address, and the * prefix is not expected.
 * @return 0 if ok, non-zero on error
 */
static int parse_verify_sum(struct hash_algo *algo, char *verify_str, u8 *vsum,
			    int allow_env_vars)
{
	int env_var = 0;

	/* See comment above in store_result() */
	if (allow_env_vars) {
		if (*verify_str == '*')
			verify_str++;
		else
			env_var = 1;
	}

	if (env_var) {
		ulong addr;
		void *buf;

		addr = simple_strtoul(verify_str, NULL, 16);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(vsum, buf, algo->digest_size);
	} else {
		unsigned int i;
		char *vsum_str;
		int digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		if (strlen(verify_str) == digits)
			vsum_str = verify_str;
		else {
			vsum_str = getenv(verify_str);
			if (vsum_str == NULL || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				return 1;
			}
		}

		for (i = 0; i < algo->digest_size; i++) {
			char *nullp = vsum_str + (i + 1) * 2;
			char end = *nullp;

			*nullp = '\0';
			vsum[i] = simple_strtoul(vsum_str + (i * 2), NULL, 16);
			*nullp = end;
		}
	}
	return 0;
}

static struct hash_algo *find_hash_algo(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		if (!strcmp(name, hash_algo[i].name))
			return &hash_algo[i];
	}

	return NULL;
}

static void show_hash(struct hash_algo *algo, ulong addr, ulong len,
		      u8 *output)
{
	int i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	for (i = 0; i < algo->digest_size; i++)
		printf("%02x", output[i]);
}

int hash_block(const char *algo_name, const void *data, unsigned int len,
	       uint8_t *output, int *output_size)
{
	struct hash_algo *algo;

	algo = find_hash_algo(algo_name);
	if (!algo) {
		debug("Unknown hash algorithm '%s'\n", algo_name);
		return -EPROTONOSUPPORT;
	}
	if (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		return -ENOSPC;
	}
	if (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	return 0;
}

int hash_command(const char *algo_name, int flags, cmd_tbl_t *cmdtp, int flag,
		 int argc, char * const argv[])
{
	ulong addr, len;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(*argv++, NULL, 16);
	len = simple_strtoul(*argv++, NULL, 16);

	if (multi_hash()) {
		struct hash_algo *algo;
		u8 output[HASH_MAX_DIGEST_SIZE];
		u8 vsum[HASH_MAX_DIGEST_SIZE];
		void *buf;

		algo = find_hash_algo(algo_name);
		if (!algo) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			return CMD_RET_USAGE;
		}
		argc -= 2;

		if (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			return 1;
		}

		buf = map_sysmem(addr, len);
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#ifdef CONFIG_HASH_VERIFY
		if (flags & HASH_FLAG_VERIFY) {
#else
		if (0) {
#endif
			if (!argc)
				return CMD_RET_USAGE;
			if (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				return 1;
			}
			if (memcmp(output, vsum, algo->digest_size) != 0) {
				int i;

				show_hash(algo, addr, len, output);
				printf(" != ");
				for (i = 0; i < algo->digest_size; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				return 1;
			}
		} else {
			show_hash(algo, addr, len, output);
			printf("\n");

			if (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

	/* Horrible code size hack for boards that just want crc32 */
	} else {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (const uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		if (argc >= 3) {
			ptr = (ulong *)simple_strtoul(argv[0], NULL, 16);
			*ptr = crc;
		}
	}

	return 0;
}
