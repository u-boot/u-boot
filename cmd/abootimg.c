// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020
 * Sam Protsenko <joe.skb7@gmail.com>
 */

#include <android_image.h>
#include <common.h>
#include <mapmem.h>

#define abootimg_addr() \
	(_abootimg_addr == -1 ? image_load_addr : _abootimg_addr)

/* Please use abootimg_addr() macro to obtain the boot image address */
static ulong _abootimg_addr = -1;

static int abootimg_get_ver(int argc, char * const argv[])
{
	const struct andr_img_hdr *hdr;
	int res = CMD_RET_SUCCESS;

	if (argc > 1)
		return CMD_RET_USAGE;

	hdr = map_sysmem(abootimg_addr(), sizeof(*hdr));
	if (android_image_check_header(hdr)) {
		printf("Error: Boot Image header is incorrect\n");
		res = CMD_RET_FAILURE;
		goto exit;
	}

	if (argc == 0)
		printf("%u\n", hdr->header_version);
	else
		env_set_ulong(argv[0], hdr->header_version);

exit:
	unmap_sysmem(hdr);
	return res;
}

static int abootimg_get_recovery_dtbo(int argc, char * const argv[])
{
	ulong addr;
	u32 size;

	if (argc > 2)
		return CMD_RET_USAGE;

	if (!android_image_get_dtbo(abootimg_addr(), &addr, &size))
		return CMD_RET_FAILURE;

	if (argc == 0) {
		printf("%lx\n", addr);
	} else {
		env_set_hex(argv[0], addr);
		if (argc == 2)
			env_set_hex(argv[1], size);
	}

	return CMD_RET_SUCCESS;
}

static int abootimg_get_dtb_load_addr(int argc, char * const argv[])
{
	const struct andr_img_hdr *hdr;
	int res = CMD_RET_SUCCESS;

	if (argc > 1)
		return CMD_RET_USAGE;

	hdr = map_sysmem(abootimg_addr(), sizeof(*hdr));
	if (android_image_check_header(hdr)) {
		printf("Error: Boot Image header is incorrect\n");
		res = CMD_RET_FAILURE;
		goto exit;
	}

	if (hdr->header_version < 2) {
		printf("Error: header_version must be >= 2 for this\n");
		res = CMD_RET_FAILURE;
		goto exit;
	}

	if (argc == 0)
		printf("%lx\n", (ulong)hdr->dtb_addr);
	else
		env_set_hex(argv[0], (ulong)hdr->dtb_addr);

exit:
	unmap_sysmem(hdr);
	return res;
}

static int abootimg_get_dtb_by_index(int argc, char * const argv[])
{
	const char *index_str;
	u32 num;
	char *endp;
	ulong addr;
	u32 size;

	if (argc < 1 || argc > 3)
		return CMD_RET_USAGE;

	index_str = argv[0] + strlen("--index=");
	if (index_str[0] == '\0') {
		printf("Error: Wrong index num\n");
		return CMD_RET_FAILURE;
	}

	num = simple_strtoul(index_str, &endp, 0);
	if (*endp != '\0') {
		printf("Error: Wrong index num\n");
		return CMD_RET_FAILURE;
	}

	if (!android_image_get_dtb_by_index(abootimg_addr(), num,
					    &addr, &size)) {
		return CMD_RET_FAILURE;
	}

	if (argc == 1) {
		printf("%lx\n", addr);
	} else {
		if (env_set_hex(argv[1], addr)) {
			printf("Error: Can't set [addr_var]\n");
			return CMD_RET_FAILURE;
		}

		if (argc == 3) {
			if (env_set_hex(argv[2], size)) {
				printf("Error: Can't set [size_var]\n");
				return CMD_RET_FAILURE;
			}
		}
	}

	return CMD_RET_SUCCESS;
}

static int abootimg_get_dtb(int argc, char * const argv[])
{
	if (argc < 1)
		return CMD_RET_USAGE;

	if (strstr(argv[0], "--index="))
		return abootimg_get_dtb_by_index(argc, argv);

	return CMD_RET_USAGE;
}

static int do_abootimg_addr(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	char *endp;
	ulong img_addr;

	if (argc != 2)
		return CMD_RET_USAGE;

	img_addr = simple_strtoul(argv[1], &endp, 16);
	if (*endp != '\0') {
		printf("Error: Wrong image address\n");
		return CMD_RET_FAILURE;
	}

	_abootimg_addr = img_addr;
	return CMD_RET_SUCCESS;
}

static int do_abootimg_get(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	const char *param;

	if (argc < 2)
		return CMD_RET_USAGE;

	param = argv[1];
	argc -= 2;
	argv += 2;
	if (!strcmp(param, "ver"))
		return abootimg_get_ver(argc, argv);
	else if (!strcmp(param, "recovery_dtbo"))
		return abootimg_get_recovery_dtbo(argc, argv);
	else if (!strcmp(param, "dtb_load_addr"))
		return abootimg_get_dtb_load_addr(argc, argv);
	else if (!strcmp(param, "dtb"))
		return abootimg_get_dtb(argc, argv);

	return CMD_RET_USAGE;
}

static int do_abootimg_dump(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "dtb")) {
		if (android_image_print_dtb_contents(abootimg_addr()))
			return CMD_RET_FAILURE;
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_abootimg_sub[] = {
	U_BOOT_CMD_MKENT(addr, 2, 1, do_abootimg_addr, "", ""),
	U_BOOT_CMD_MKENT(dump, 2, 1, do_abootimg_dump, "", ""),
	U_BOOT_CMD_MKENT(get, 5, 1, do_abootimg_get, "", ""),
};

static int do_abootimg(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	cmd_tbl_t *cp;

	cp = find_cmd_tbl(argv[1], cmd_abootimg_sub,
			  ARRAY_SIZE(cmd_abootimg_sub));

	/* Strip off leading 'abootimg' command argument */
	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	abootimg, CONFIG_SYS_MAXARGS, 0, do_abootimg,
	"manipulate Android Boot Image",
	"addr <addr>\n"
	"    - set the address in RAM where boot image is located\n"
	"      ($loadaddr is used by default)\n"
	"abootimg dump dtb\n"
	"    - print info for all DT blobs in DTB area\n"
	"abootimg get ver [varname]\n"
	"    - get header version\n"
	"abootimg get recovery_dtbo [addr_var [size_var]]\n"
	"    - get address and size (hex) of recovery DTBO area in the image\n"
	"      [addr_var]: variable name to contain DTBO area address\n"
	"      [size_var]: variable name to contain DTBO area size\n"
	"abootimg get dtb_load_addr [varname]\n"
	"    - get load address (hex) of DTB, from image header\n"
	"abootimg get dtb --index=<num> [addr_var [size_var]]\n"
	"    - get address and size (hex) of DT blob in the image by index\n"
	"      <num>: index number of desired DT blob in DTB area\n"
	"      [addr_var]: variable name to contain DT blob address\n"
	"      [size_var]: variable name to contain DT blob size"
);
