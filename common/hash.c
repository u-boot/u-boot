/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <common.h>
#include <command.h>
#include <hash.h>
#include <sha1.h>
#include <sha256.h>

/*
 * These are the hash algorithms we support. Chips which support accelerated
 * crypto could perhaps add named version of these algorithms here.
 */
static struct hash_algo hash_algo[] = {
#ifdef CONFIG_SHA1
	{
		"SHA1",
		SHA1_SUM_LEN,
		sha1_csum_wd,
		CHUNKSZ_SHA1,
	},
#endif
#ifdef CONFIG_SHA256
	{
		"SHA256",
		SHA256_SUM_LEN,
		sha256_csum_wd,
		CHUNKSZ_SHA256,
	},
#endif
};

/**
 * store_result: Store the resulting sum to an address or variable
 *
 * @algo:		Hash algorithm being used
 * @sum:		Hash digest (algo->digest_size bytes)
 * @dest:		Destination, interpreted as a hex address if it starts
 *			with * or otherwise as an environment variable.
 */
static void store_result(struct hash_algo *algo, const u8 *sum,
			 const char *dest)
{
	unsigned int i;

	if (*dest == '*') {
		u8 *ptr;

		ptr = (u8 *)simple_strtoul(dest + 1, NULL, 16);
		memcpy(ptr, sum, algo->digest_size);
	} else {
		char str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		char *str_ptr = str_output;

		for (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		str_ptr = '\0';
		setenv(dest, str_output);
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
 * @return 0 if ok, non-zero on error
 */
static int parse_verify_sum(struct hash_algo *algo, char *verify_str, u8 *vsum)
{
	if (*verify_str == '*') {
		u8 *ptr;

		ptr = (u8 *)simple_strtoul(verify_str + 1, NULL, 16);
		memcpy(vsum, ptr, algo->digest_size);
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
		if (!strcasecmp(name, hash_algo[i].name))
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

int hash_command(const char *algo_name, int verify, cmd_tbl_t *cmdtp, int flag,
		 int argc, char * const argv[])
{
	struct hash_algo *algo;
	ulong addr, len;
	u8 output[HASH_MAX_DIGEST_SIZE];
	u8 vsum[HASH_MAX_DIGEST_SIZE];

	if (argc < 2)
		return CMD_RET_USAGE;

	algo = find_hash_algo(algo_name);
	if (!algo) {
		printf("Unknown hash algorithm '%s'\n", algo_name);
		return CMD_RET_USAGE;
	}
	addr = simple_strtoul(*argv++, NULL, 16);
	len = simple_strtoul(*argv++, NULL, 16);
	argc -= 2;

	if (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
		puts("HASH_MAX_DIGEST_SIZE exceeded\n");
		return 1;
	}

	algo->hash_func_ws((const unsigned char *)addr, len, output,
			   algo->chunk_size);

	/* Try to avoid code bloat when verify is not needed */
#ifdef CONFIG_HASH_VERIFY
	if (verify) {
#else
	if (0) {
#endif
		if (!argc)
			return CMD_RET_USAGE;
		if (parse_verify_sum(algo, *argv, vsum)) {
			printf("ERROR: %s does not contain a valid %s sum\n",
				*argv, algo->name);
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

		if (argc)
			store_result(algo, output, *argv);
	}

	return 0;
}
