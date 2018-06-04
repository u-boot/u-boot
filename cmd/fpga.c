// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000, 2001
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

/*
 *  FPGA support
 */
#include <common.h>
#include <command.h>
#include <fpga.h>
#include <fs.h>
#include <malloc.h>

static long do_fpga_get_device(char *arg)
{
	long dev = FPGA_INVALID_DEVICE;
	char *devstr = env_get("fpga");

	if (devstr)
		/* Should be strtol to handle -1 cases */
		dev = simple_strtol(devstr, NULL, 16);

	if (arg)
		dev = simple_strtol(arg, NULL, 16);

	debug("%s: device = %ld\n", __func__, dev);

	return dev;
}

static int do_fpga_check_params(long *dev, long *fpga_data, size_t *data_size,
				cmd_tbl_t *cmdtp, int argc, char *const argv[])
{
	size_t local_data_size;
	long local_fpga_data;

	debug("%s %d, %d\n", __func__, argc, cmdtp->maxargs);

	if (argc != cmdtp->maxargs) {
		debug("fpga: incorrect parameters passed\n");
		return CMD_RET_USAGE;
	}

	*dev = do_fpga_get_device(argv[0]);

	local_fpga_data = simple_strtol(argv[1], NULL, 16);
	if (!local_fpga_data) {
		debug("fpga: zero fpga_data address\n");
		return CMD_RET_USAGE;
	}
	*fpga_data = local_fpga_data;

	local_data_size = simple_strtoul(argv[2], NULL, 16);
	if (!local_data_size) {
		debug("fpga: zero size\n");
		return CMD_RET_USAGE;
	}
	*data_size = local_data_size;

	return 0;
}

/* Local defines */
enum {
	FPGA_NONE = -1,
	FPGA_LOADS,
};

/*
 * Map op to supported operations.  We don't use a table since we
 * would just have to relocate it from flash anyway.
 */
static int fpga_get_op(char *opstr)
{
	int op = FPGA_NONE;

#if defined(CONFIG_CMD_FPGA_LOAD_SECURE)
	if (!strcmp("loads", opstr))
		op = FPGA_LOADS;
#endif

	return op;
}

/* ------------------------------------------------------------------------- */
/* command form:
 *   fpga <op> <device number> <data addr> <datasize>
 * where op is 'load', 'dump', or 'info'
 * If there is no device number field, the fpga environment variable is used.
 * If there is no data addr field, the fpgadata environment variable is used.
 * The info command requires no data address field.
 */
int do_fpga(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int op, dev = FPGA_INVALID_DEVICE;
	size_t data_size = 0;
	void *fpga_data = NULL;
	int rc = FPGA_FAIL;
#if defined(CONFIG_CMD_FPGA_LOAD_SECURE)
	struct fpga_secure_info fpga_sec_info;

	memset(&fpga_sec_info, 0, sizeof(fpga_sec_info));
#endif

	if (argc > 9 || argc < 2) {
		debug("%s: Too many or too few args (%d)\n", __func__, argc);
		return CMD_RET_USAGE;
	}

	op = fpga_get_op(argv[1]);

	switch (op) {
	case FPGA_NONE:
		printf("Unknown fpga operation \"%s\"\n", argv[1]);
		return CMD_RET_USAGE;
#if defined(CONFIG_CMD_FPGA_LOAD_SECURE)
	case FPGA_LOADS:
		if (argc < 7)
			return CMD_RET_USAGE;
		if (argc == 8)
			fpga_sec_info.userkey_addr = (u8 *)(uintptr_t)
						     simple_strtoull(argv[7],
								     NULL, 16);
		fpga_sec_info.encflag = (u8)simple_strtoul(argv[6], NULL, 16);
		fpga_sec_info.authflag = (u8)simple_strtoul(argv[5], NULL, 16);

		if (fpga_sec_info.authflag >= FPGA_NO_ENC_OR_NO_AUTH &&
		    fpga_sec_info.encflag >= FPGA_NO_ENC_OR_NO_AUTH) {
			puts("ERR: Use <fpga load> for NonSecure bitstream\n");
			return CMD_RET_USAGE;
		}

		if (fpga_sec_info.encflag == FPGA_ENC_USR_KEY &&
		    !fpga_sec_info.userkey_addr) {
			puts("ERR: User key not provided\n");
			return CMD_RET_USAGE;
		}

		argc = 5;
		break;
#endif
	default:
		break;
	}

	switch (argc) {
	case 5:		/* fpga <op> <dev> <data> <datasize> */
		data_size = simple_strtoul(argv[4], NULL, 16);
		if (!data_size) {
			puts("Zero data_size\n");
			return CMD_RET_USAGE;
		}
	case 4:		/* fpga <op> <dev> <data> */
		{
			fpga_data = (void *)simple_strtoul(argv[3], NULL, 16);
			debug("*  fpga: cmdline image address = 0x%08lx\n",
			      (ulong)fpga_data);
		}
		debug("%s: fpga_data = 0x%lx\n", __func__, (ulong)fpga_data);
		if (!fpga_data) {
			puts("Zero fpga_data address\n");
			return CMD_RET_USAGE;
		}
	case 3:		/* fpga <op> <dev | data addr> */
		dev = (int)simple_strtoul(argv[2], NULL, 16);
		debug("%s: device = %d\n", __func__, dev);
	}

	if (dev == FPGA_INVALID_DEVICE) {
		puts("FPGA device not specified\n");
		return CMD_RET_USAGE;
	}

	switch (op) {
#if defined(CONFIG_CMD_FPGA_LOAD_SECURE)
	case FPGA_LOADS:
		rc = fpga_loads(dev, fpga_data, data_size, &fpga_sec_info);
		break;
#endif

	default:
		printf("Unknown operation\n");
		return CMD_RET_USAGE;
	}
	return rc;
}

#if defined(CONFIG_CMD_FPGA_LOADFS)
static int do_fpga_loadfs(cmd_tbl_t *cmdtp, int flag, int argc,
			  char *const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;
	fpga_fs_info fpga_fsinfo;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	fpga_fsinfo.fstype = FS_TYPE_ANY;
	fpga_fsinfo.blocksize = (unsigned int)simple_strtoul(argv[3], NULL, 16);
	fpga_fsinfo.interface = argv[4];
	fpga_fsinfo.dev_part = argv[5];
	fpga_fsinfo.filename = argv[6];

	return fpga_fsload(dev, (void *)fpga_data, data_size, &fpga_fsinfo);
}
#endif

static int do_fpga_info(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	long dev = do_fpga_get_device(argv[0]);

	return fpga_info(dev);
}

static int do_fpga_dump(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_dump(dev, (void *)fpga_data, data_size);
}

static int do_fpga_load(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_load(dev, (void *)fpga_data, data_size, BIT_FULL);
}

static int do_fpga_loadb(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_loadbitstream(dev, (void *)fpga_data, data_size, BIT_FULL);
}

#if defined(CONFIG_CMD_FPGA_LOADP)
static int do_fpga_loadp(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_load(dev, (void *)fpga_data, data_size, BIT_PARTIAL);
}
#endif

#if defined(CONFIG_CMD_FPGA_LOADBP)
static int do_fpga_loadbp(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	size_t data_size = 0;
	long fpga_data, dev;
	int ret;

	ret = do_fpga_check_params(&dev, &fpga_data, &data_size,
				   cmdtp, argc, argv);
	if (ret)
		return ret;

	return fpga_loadbitstream(dev, (void *)fpga_data, data_size,
				  BIT_PARTIAL);
}
#endif

#if defined(CONFIG_CMD_FPGA_LOADMK)
static int do_fpga_loadmk(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	size_t data_size = 0;
	void *fpga_data = NULL;
#if defined(CONFIG_FIT)
	const char *fit_uname = NULL;
	ulong fit_addr;
#endif
	ulong dev = do_fpga_get_device(argv[0]);
	char *datastr = env_get("fpgadata");

	if (datastr)
		fpga_data = (void *)simple_strtoul(datastr, NULL, 16);

	if (argc == 2) {
#if defined(CONFIG_FIT)
		if (fit_parse_subimage(argv[1], (ulong)fpga_data,
				       &fit_addr, &fit_uname)) {
			fpga_data = (void *)fit_addr;
			debug("*  fpga: subimage '%s' from FIT image ",
			      fit_uname);
			debug("at 0x%08lx\n", fit_addr);
		} else
#endif
		{
			fpga_data = (void *)simple_strtoul(argv[1], NULL, 16);
			debug("*  fpga: cmdline image address = 0x%08lx\n",
			      (ulong)fpga_data);
		}
		debug("%s: fpga_data = 0x%lx\n", __func__, (ulong)fpga_data);
		if (!fpga_data) {
			puts("Zero fpga_data address\n");
			return CMD_RET_USAGE;
		}
	}

	switch (genimg_get_format(fpga_data)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	case IMAGE_FORMAT_LEGACY:
	{
		image_header_t *hdr = (image_header_t *)fpga_data;
		ulong data;
		u8 comp;

		comp = image_get_comp(hdr);
		if (comp == IH_COMP_GZIP) {
#if defined(CONFIG_GZIP)
			ulong image_buf = image_get_data(hdr);
			ulong image_size = ~0UL;

			data = image_get_load(hdr);

			if (gunzip((void *)data, ~0UL, (void *)image_buf,
				   &image_size) != 0) {
				puts("GUNZIP: error\n");
				return 1;
			}
			data_size = image_size;
#else
			puts("Gunzip image is not supported\n");
			return 1;
#endif
		} else {
			data = (ulong)image_get_data(hdr);
			data_size = image_get_data_size(hdr);
		}
		return fpga_load(dev, (void *)data, data_size,
				  BIT_FULL);
	}
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
	{
		const void *fit_hdr = (const void *)fpga_data;
		int noffset;
		const void *fit_data;

		if (!fit_uname) {
			puts("No FIT subimage unit name\n");
			return 1;
		}

		if (!fit_check_format(fit_hdr)) {
			puts("Bad FIT image format\n");
			return 1;
		}

		/* get fpga component image node offset */
		noffset = fit_image_get_node(fit_hdr, fit_uname);
		if (noffset < 0) {
			printf("Can't find '%s' FIT subimage\n", fit_uname);
			return 1;
		}

		/* verify integrity */
		if (!fit_image_verify(fit_hdr, noffset)) {
			puts("Bad Data Hash\n");
			return 1;
		}

		/* get fpga subimage data address and length */
		if (fit_image_get_data(fit_hdr, noffset, &fit_data,
				       &data_size)) {
			puts("Fpga subimage data not found\n");
			return 1;
		}

		return fpga_load(dev, fit_data, data_size, BIT_FULL);
	}
#endif
	default:
		puts("** Unknown image type\n");
		return FPGA_FAIL;
	}
}
#endif

static cmd_tbl_t fpga_commands[] = {
	U_BOOT_CMD_MKENT(info, 1, 1, do_fpga_info, "", ""),
	U_BOOT_CMD_MKENT(dump, 3, 1, do_fpga_dump, "", ""),
	U_BOOT_CMD_MKENT(load, 3, 1, do_fpga_load, "", ""),
	U_BOOT_CMD_MKENT(loadb, 3, 1, do_fpga_loadb, "", ""),
#if defined(CONFIG_CMD_FPGA_LOADP)
	U_BOOT_CMD_MKENT(loadp, 3, 1, do_fpga_loadp, "", ""),
#endif
#if defined(CONFIG_CMD_FPGA_LOADBP)
	U_BOOT_CMD_MKENT(loadbp, 3, 1, do_fpga_loadbp, "", ""),
#endif
#if defined(CONFIG_CMD_FPGA_LOADFS)
	U_BOOT_CMD_MKENT(loadfs, 7, 1, do_fpga_loadfs, "", ""),
#endif
#if defined(CONFIG_CMD_FPGA_LOADMK)
	U_BOOT_CMD_MKENT(loadmk, 2, 1, do_fpga_loadmk, "", ""),
#endif
};

static int do_fpga_wrapper(cmd_tbl_t *cmdtp, int flag, int argc,
			   char *const argv[])
{
	cmd_tbl_t *fpga_cmd;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	fpga_cmd = find_cmd_tbl(argv[1], fpga_commands,
				ARRAY_SIZE(fpga_commands));

	/* This should be removed when all functions are converted */
	if (!fpga_cmd)
		return do_fpga(cmdtp, flag, argc, argv);

	/* FIXME This can't be reached till all functions are converted */
	if (!fpga_cmd) {
		debug("fpga: non existing command\n");
		return CMD_RET_USAGE;
	}

	argc -= 2;
	argv += 2;

	if (argc > fpga_cmd->maxargs) {
		debug("fpga: more parameters passed\n");
		return CMD_RET_USAGE;
	}

	ret = fpga_cmd->cmd(fpga_cmd, flag, argc, argv);

	return cmd_process_error(fpga_cmd, ret);
}

#if defined(CONFIG_CMD_FPGA_LOADFS) || defined(CONFIG_CMD_FPGA_LOAD_SECURE)
U_BOOT_CMD(fpga, 9, 1, do_fpga_wrapper,
#else
U_BOOT_CMD(fpga, 6, 1, do_fpga_wrapper,
#endif
	   "loadable FPGA image support",
	   "[operation type] [device number] [image address] [image size]\n"
	   "fpga operations:\n"
	   "  dump\t[dev] [address] [size]\tLoad device to memory buffer\n"
	   "  info\t[dev]\t\t\tlist known device information\n"
	   "  load\t[dev] [address] [size]\tLoad device from memory buffer\n"
#if defined(CONFIG_CMD_FPGA_LOADP)
	   "  loadp\t[dev] [address] [size]\t"
	   "Load device from memory buffer with partial bitstream\n"
#endif
	   "  loadb\t[dev] [address] [size]\t"
	   "Load device from bitstream buffer (Xilinx only)\n"
#if defined(CONFIG_CMD_FPGA_LOADBP)
	   "  loadbp\t[dev] [address] [size]\t"
	   "Load device from bitstream buffer with partial bitstream"
	   "(Xilinx only)\n"
#endif
#if defined(CONFIG_CMD_FPGA_LOADFS)
	   "Load device from filesystem (FAT by default) (Xilinx only)\n"
	   "  loadfs [dev] [address] [image size] [blocksize] <interface>\n"
	   "        [<dev[:part]>] <filename>\n"
#endif
#if defined(CONFIG_CMD_FPGA_LOADMK)
	   "  loadmk [dev] [address]\tLoad device generated with mkimage"
#if defined(CONFIG_FIT)
	   "\n"
	   "\tFor loadmk operating on FIT format uImage address must include\n"
	   "\tsubimage unit name in the form of addr:<subimg_uname>"
#endif
#endif
#if defined(CONFIG_CMD_FPGA_LOAD_SECURE)
	   "Load encrypted bitstream (Xilinx only)\n"
	   "  loads [dev] [address] [size] [auth-OCM-0/DDR-1/noauth-2]\n"
	   "        [enc-devkey(0)/userkey(1)/nenc(2) [Userkey address]\n"
	   "Loads the secure bistreams(authenticated/encrypted/both\n"
	   "authenticated and encrypted) of [size] from [address].\n"
	   "The auth-OCM/DDR flag specifies to perform authentication\n"
	   "in OCM or in DDR. 0 for OCM, 1 for DDR, 2 for no authentication.\n"
	   "The enc flag specifies which key to be used for decryption\n"
	   "0-device key, 1-user key, 2-no encryption.\n"
	   "The optional Userkey address specifies from which address key\n"
	   "has to be used for decryption if user key is selected.\n"
	   "NOTE: the sceure bitstream has to be created using xilinx\n"
	   "bootgen tool only.\n"
#endif
);
