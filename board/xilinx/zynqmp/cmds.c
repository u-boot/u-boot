/*
 * (C) Copyright 2018 Xilinx, Inc.
 * Siva Durga Prasad Paladugu <siva.durga.paladugu@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

static int zynqmp_verify_secure(u8 *key_ptr, u8 *src_ptr, u32 len)
{
	int ret;
	u32 src_lo, src_hi;
	u32 key_lo = 0;
	u32 key_hi = 0;
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u64 addr;

	if ((ulong)src_ptr != ALIGN((ulong)src_ptr,
				    CONFIG_SYS_CACHELINE_SIZE)) {
		printf("Failed: source address not aligned:%p\n", src_ptr);
		return -EINVAL;
	}

	src_lo = lower_32_bits((ulong)src_ptr);
	src_hi = upper_32_bits((ulong)src_ptr);
	flush_dcache_range((ulong)src_ptr, (ulong)(src_ptr + len));

	if (key_ptr) {
		key_lo = lower_32_bits((ulong)key_ptr);
		key_hi = upper_32_bits((ulong)key_ptr);
		flush_dcache_range((ulong)key_ptr,
				   (ulong)(key_ptr + KEY_PTR_LEN));
	}

	ret = invoke_smc(ZYNQMP_SIP_SVC_PM_SECURE_IMG_LOAD, src_lo, src_hi,
			 key_lo, key_hi, ret_payload);
	if (ret) {
		printf("Failed: secure op status:0x%x\n", ret);
	} else {
		addr = (u64)ret_payload[1] << 32 | ret_payload[2];
		printf("Verified image at 0x%llx\n", addr);
		env_set_hex("zynqmp_verified_img_addr", addr);
	}

	return ret;
}

/**
 * do_zynqmp - Handle the "zynqmp" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Processes the zynqmp specific commands
 *
 * Return: return 0 on success and CMD_RET_USAGE incase of misuse and error
 */
static int do_zynqmp(cmd_tbl_t *cmdtp, int flag, int argc,
		     char *const argv[])
{
	u64 src_addr;
	u32 len;
	u8 *key_ptr = NULL;
	u8 *src_ptr;
	int ret;

	if (argc > 5 || argc < 4 || strncmp(argv[1], "secure", 6))
		return CMD_RET_USAGE;

	src_addr = simple_strtoull(argv[2], NULL, 16);

	len = simple_strtoul(argv[3], NULL, 16);

	if (argc > 4)
		key_ptr = (uint8_t *)(uintptr_t)simple_strtoull(argv[4],
								NULL, 16);

	src_ptr = (uint8_t *)(uintptr_t)src_addr;

	ret = zynqmp_verify_secure(key_ptr, src_ptr, len);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

/***************************************************/
#ifdef CONFIG_SYS_LONGHELP
static char zynqmp_help_text[] =
	"secure src len [key_addr] - verifies secure images of $len bytes\n"
	"                            long at address $src. Optional key_addr\n"
	"                            can be specified if user key needs to\n"
	"                            be used for decryption\n";
#endif

U_BOOT_CMD(
	zynqmp, 5, 1, do_zynqmp,
	"Verify and load secure images",
	zynqmp_help_text
)
