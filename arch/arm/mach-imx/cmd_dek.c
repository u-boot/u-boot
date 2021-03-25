// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2015 Freescale Semiconductor, Inc.
 *
 * Command for encapsulating DEK blob
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <fsl_sec.h>
#include <asm/arch/clock.h>
#include <mapmem.h>
#include <tee.h>

/**
* blob_dek() - Encapsulate the DEK as a blob using CAM's Key
* @src: - Address of data to be encapsulated
* @dst: - Desination address of encapsulated data
* @len: - Size of data to be encapsulated
*
* Returns zero on success,and negative on error.
*/
#ifdef CONFIG_IMX_CAAM_DEK_ENCAP
static int blob_encap_dek(u32 src_addr, u32 dst_addr, u32 len)
{
	u8 *src_ptr, *dst_ptr;

	src_ptr = map_sysmem(src_addr, len / 8);
	dst_ptr = map_sysmem(dst_addr, BLOB_SIZE(len / 8));

	hab_caam_clock_enable(1);

	u32 out_jr_size = sec_in32(CONFIG_SYS_FSL_JR0_ADDR +
				   FSL_CAAM_ORSR_JRa_OFFSET);
	if (out_jr_size != FSL_CAAM_MAX_JR_SIZE)
		sec_init();

	if (!((len == 128) | (len == 192) | (len == 256))) {
		debug("Invalid DEK size. Valid sizes are 128, 192 and 256b\n");
		return -1;
	}

	len /= 8;
	return blob_dek(src_ptr, dst_ptr, len);
}
#endif /* CONFIG_IMX_CAAM_DEK_ENCAP */

#ifdef CONFIG_IMX_OPTEE_DEK_ENCAP

#define PTA_DEK_BLOB_PTA_UUID {0xef477737, 0x0db1, 0x4a9d, \
	{0x84, 0x37, 0xf2, 0xf5, 0x35, 0xc0, 0xbd, 0x92} }

#define OPTEE_BLOB_HDR_SIZE		8

static int blob_encap_dek(u32 src_addr, u32 dst_addr, u32 len)
{
	struct udevice *dev = NULL;
	struct tee_shm *shm_input, *shm_output;
	struct tee_open_session_arg arg = {0};
	struct tee_invoke_arg arg_func = {0};
	const struct tee_optee_ta_uuid uuid = PTA_DEK_BLOB_PTA_UUID;
	struct tee_param param[4] = {0};
	int ret;

	/* Get tee device */
	dev = tee_find_device(NULL, NULL, NULL, NULL);
	if (!dev) {
		printf("Cannot get OP-TEE device\n");
		return -1;
	}

	/* Set TA UUID */
	tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);

	/* Open TA session */
	ret = tee_open_session(dev, &arg, 0, NULL);
	if (ret < 0) {
		printf("Cannot open session with PTA Blob 0x%X\n", ret);
		return -1;
	}

	/* Allocate shared input and output buffers for TA */
	ret = tee_shm_register(dev, (void *)(ulong)src_addr, len / 8, 0x0, &shm_input);
	if (ret < 0) {
		printf("Cannot register input shared memory 0x%X\n", ret);
		goto error;
	}

	ret = tee_shm_register(dev, (void *)(ulong)dst_addr,
			       BLOB_SIZE(len / 8) + OPTEE_BLOB_HDR_SIZE,
			       0x0, &shm_output);
	if (ret < 0) {
		printf("Cannot register output shared memory 0x%X\n", ret);
		goto error;
	}

	param[0].u.memref.shm	= shm_input;
	param[0].u.memref.size	= shm_input->size;
	param[0].attr		= TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[1].u.memref.shm	= shm_output;
	param[1].u.memref.size	= shm_output->size;
	param[1].attr		= TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	param[2].attr		= TEE_PARAM_ATTR_TYPE_NONE;
	param[3].attr		= TEE_PARAM_ATTR_TYPE_NONE;

	arg_func.func = 0;
	arg_func.session = arg.session;

	/* Generate DEK blob */
	arg_func.session = arg.session;
	ret = tee_invoke_func(dev, &arg_func, 4, param);
	if (ret < 0)
		printf("Cannot generate Blob with PTA DEK Blob 0x%X\n", ret);

error:
	/* Free shared memory */
	tee_shm_free(shm_input);
	tee_shm_free(shm_output);

	/* Close session */
	ret = tee_close_session(dev, arg.session);
	if (ret < 0)
		printf("Cannot close session with PTA DEK Blob 0x%X\n", ret);

	return ret;
}
#endif /* CONFIG_IMX_OPTEE_DEK_ENCAP */

/**
 * do_dek_blob() - Handle the "dek_blob" command-line command
 * @cmdtp:  Command data struct pointer
 * @flag:   Command flag
 * @argc:   Command-line argument count
 * @argv:   Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_dek_blob(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	uint32_t src_addr, dst_addr, len;

	if (argc != 4)
		return CMD_RET_USAGE;

	src_addr = simple_strtoul(argv[1], NULL, 16);
	dst_addr = simple_strtoul(argv[2], NULL, 16);
	len = simple_strtoul(argv[3], NULL, 10);

	return blob_encap_dek(src_addr, dst_addr, len);
}

/***************************************************/
static char dek_blob_help_text[] =
	"src dst len            - Encapsulate and create blob of data\n"
	"                         $len bits long at address $src and\n"
	"                         store the result at address $dst.\n";

U_BOOT_CMD(
	dek_blob, 4, 1, do_dek_blob,
	"Data Encryption Key blob encapsulation",
	dek_blob_help_text
);
