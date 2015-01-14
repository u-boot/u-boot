/*
 *
 * Command for encapsulating/decapsulating blob of memory.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * blob_decap() - Decapsulate the data as a blob
 * @key_mod:	- Pointer to key modifier/key
 * @src:	- Address of data to be decapsulated
 * @dst:	- Address of data to be decapsulated
 * @len:	- Size of data to be decapsulated
 *
 * Returns zero on success,and negative on error.
 */
__weak int blob_decap(u8 *key_mod, u8 *src, u8 *dst, u32 len)
{
	return 0;
}

/**
 * blob_encap() - Encapsulate the data as a blob
 * @key_mod:	- Pointer to key modifier/key
 * @src:	- Address of data to be encapsulated
 * @dst:	- Address of data to be encapsulated
 * @len:	- Size of data to be encapsulated
 *
 * Returns zero on success,and negative on error.
 */
__weak int blob_encap(u8 *key_mod, u8 *src, u8 *dst, u32 len)
{
	return 0;
}

/**
 * do_blob() - Handle the "blob" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_blob(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	uint32_t key_addr, src_addr, dst_addr, len;
	uint8_t *km_ptr, *src_ptr, *dst_ptr;
	int enc, ret = 0;

	if (argc != 6)
		return CMD_RET_USAGE;

	if (!strncmp(argv[1], "enc", 3))
		enc = 1;
	else if (!strncmp(argv[1], "dec", 3))
		enc = 0;
	else
		return CMD_RET_USAGE;

	src_addr = simple_strtoul(argv[2], NULL, 16);
	dst_addr = simple_strtoul(argv[3], NULL, 16);
	len = simple_strtoul(argv[4], NULL, 16);
	key_addr = simple_strtoul(argv[5], NULL, 16);

	km_ptr = (uint8_t *)key_addr;
	src_ptr = (uint8_t *)src_addr;
	dst_ptr = (uint8_t *)dst_addr;

	if (enc)
		ret = blob_encap(km_ptr, src_ptr, dst_ptr, len);
	else
		ret = blob_decap(km_ptr, src_ptr, dst_ptr, len);

	return ret;
}

/***************************************************/
static char blob_help_text[] =
	"enc src dst len km - Encapsulate and create blob of data\n"
	"                          $len bytes long at address $src and\n"
	"                          store the result at address $dst.\n"
	"                          $km is the 16 byte key modifier\n"
	"                          is also required for generation/use as\n"
	"                          key for cryptographic operation. Key\n"
	"                          modifier should be 16 byte long.\n"
	"blob dec src dst len km - Decapsulate the  blob of data at address\n"
	"                          $src and store result of $len byte at\n"
	"                          addr $dst.\n"
	"                          $km is the 16 byte key modifier\n"
	"                          is also required for generation/use as\n"
	"                          key for cryptographic operation. Key\n"
	"                          modifier should be 16 byte long.\n";

U_BOOT_CMD(
	blob, 6, 1, do_blob,
	"Blob encapsulation/decryption",
	blob_help_text
);
