/*
 * (C) Copyright 2000, 2001
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 *  FPGA support
 */
#include <common.h>
#include <command.h>
#include <fpga.h>
#include <fs.h>
#include <malloc.h>

/* Local functions */
static int fpga_get_op(char *opstr);

/* Local defines */
#define FPGA_NONE   -1
#define FPGA_INFO   0
#define FPGA_LOAD   1
#define FPGA_LOADB  2
#define FPGA_DUMP   3
#define FPGA_LOADMK 4
#define FPGA_LOADP  5
#define FPGA_LOADBP 6
#define FPGA_LOADFS 7

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
	char *devstr = getenv("fpga");
	char *datastr = getenv("fpgadata");
	int rc = FPGA_FAIL;
	int wrong_parms = 0;
#if defined(CONFIG_FIT)
	const char *fit_uname = NULL;
	ulong fit_addr;
#endif
#if defined(CONFIG_CMD_FPGA_LOADFS)
	fpga_fs_info fpga_fsinfo;
	fpga_fsinfo.fstype = FS_TYPE_ANY;
#endif

	if (devstr)
		dev = (int) simple_strtoul(devstr, NULL, 16);
	if (datastr)
		fpga_data = (void *)simple_strtoul(datastr, NULL, 16);

	switch (argc) {
#if defined(CONFIG_CMD_FPGA_LOADFS)
	case 9:
		fpga_fsinfo.blocksize = (unsigned int)
					     simple_strtoul(argv[5], NULL, 16);
		fpga_fsinfo.interface = argv[6];
		fpga_fsinfo.dev_part = argv[7];
		fpga_fsinfo.filename = argv[8];
#endif
	case 5:		/* fpga <op> <dev> <data> <datasize> */
		data_size = simple_strtoul(argv[4], NULL, 16);

	case 4:		/* fpga <op> <dev> <data> */
#if defined(CONFIG_FIT)
		if (fit_parse_subimage(argv[3], (ulong)fpga_data,
				       &fit_addr, &fit_uname)) {
			fpga_data = (void *)fit_addr;
			debug("*  fpga: subimage '%s' from FIT image ",
			      fit_uname);
			debug("at 0x%08lx\n", fit_addr);
		} else
#endif
		{
			fpga_data = (void *)simple_strtoul(argv[3], NULL, 16);
			debug("*  fpga: cmdline image address = 0x%08lx\n",
			      (ulong)fpga_data);
		}
		debug("%s: fpga_data = 0x%x\n", __func__, (uint)fpga_data);

	case 3:		/* fpga <op> <dev | data addr> */
		dev = (int)simple_strtoul(argv[2], NULL, 16);
		debug("%s: device = %d\n", __func__, dev);
		/* FIXME - this is a really weak test */
		if ((argc == 3) && (dev > fpga_count())) {
			/* must be buffer ptr */
			debug("%s: Assuming buffer pointer in arg 3\n",
			      __func__);

#if defined(CONFIG_FIT)
			if (fit_parse_subimage(argv[2], (ulong)fpga_data,
					       &fit_addr, &fit_uname)) {
				fpga_data = (void *)fit_addr;
				debug("*  fpga: subimage '%s' from FIT image ",
				      fit_uname);
				debug("at 0x%08lx\n", fit_addr);
			} else
#endif
			{
				fpga_data = (void *)dev;
				debug("*  fpga: cmdline image addr = 0x%08lx\n",
				      (ulong)fpga_data);
			}

			debug("%s: fpga_data = 0x%x\n",
			      __func__, (uint)fpga_data);
			dev = FPGA_INVALID_DEVICE;	/* reset device num */
		}

	case 2:		/* fpga <op> */
		op = (int)fpga_get_op(argv[1]);
		break;

	default:
		debug("%s: Too many or too few args (%d)\n", __func__, argc);
		op = FPGA_NONE;	/* force usage display */
		break;
	}

	if (dev == FPGA_INVALID_DEVICE) {
		puts("FPGA device not specified\n");
		op = FPGA_NONE;
	}

	switch (op) {
	case FPGA_NONE:
	case FPGA_INFO:
		break;
#if defined(CONFIG_CMD_FPGA_LOADFS)
	case FPGA_LOADFS:
		/* Blocksize can be zero */
		if (!fpga_fsinfo.interface || !fpga_fsinfo.dev_part ||
		    !fpga_fsinfo.filename)
			wrong_parms = 1;
#endif
	case FPGA_LOAD:
	case FPGA_LOADP:
	case FPGA_LOADB:
	case FPGA_LOADBP:
	case FPGA_DUMP:
		if (!fpga_data || !data_size)
			wrong_parms = 1;
		break;
#if defined(CONFIG_CMD_FPGA_LOADMK)
	case FPGA_LOADMK:
		if (!fpga_data)
			wrong_parms = 1;
		break;
#endif
	}

	if (wrong_parms) {
		puts("Wrong parameters for FPGA request\n");
		op = FPGA_NONE;
	}

	switch (op) {
	case FPGA_NONE:
		return CMD_RET_USAGE;

	case FPGA_INFO:
		rc = fpga_info(dev);
		break;

	case FPGA_LOAD:
		rc = fpga_load(dev, fpga_data, data_size, BIT_FULL);
		break;

#if defined(CONFIG_CMD_FPGA_LOADP)
	case FPGA_LOADP:
		rc = fpga_load(dev, fpga_data, data_size, BIT_PARTIAL);
		break;
#endif

	case FPGA_LOADB:
		rc = fpga_loadbitstream(dev, fpga_data, data_size, BIT_FULL);
		break;

#if defined(CONFIG_CMD_FPGA_LOADBP)
	case FPGA_LOADBP:
		rc = fpga_loadbitstream(dev, fpga_data, data_size, BIT_PARTIAL);
		break;
#endif

#if defined(CONFIG_CMD_FPGA_LOADFS)
	case FPGA_LOADFS:
		rc = fpga_fsload(dev, fpga_data, data_size, &fpga_fsinfo);
		break;
#endif

#if defined(CONFIG_CMD_FPGA_LOADMK)
	case FPGA_LOADMK:
		switch (genimg_get_format(fpga_data)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
		case IMAGE_FORMAT_LEGACY:
			{
				image_header_t *hdr =
						(image_header_t *)fpga_data;
				ulong data;
				uint8_t comp;

				comp = image_get_comp(hdr);
				if (comp == IH_COMP_GZIP) {
#if defined(CONFIG_GZIP)
					ulong image_buf = image_get_data(hdr);
					data = image_get_load(hdr);
					ulong image_size = ~0UL;

					if (gunzip((void *)data, ~0UL,
						   (void *)image_buf,
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
				rc = fpga_load(dev, (void *)data, data_size,
					       BIT_FULL);
			}
			break;
#endif
#if defined(CONFIG_FIT)
		case IMAGE_FORMAT_FIT:
			{
				const void *fit_hdr = (const void *)fpga_data;
				int noffset;
				const void *fit_data;

				if (fit_uname == NULL) {
					puts("No FIT subimage unit name\n");
					return 1;
				}

				if (!fit_check_format(fit_hdr)) {
					puts("Bad FIT image format\n");
					return 1;
				}

				/* get fpga component image node offset */
				noffset = fit_image_get_node(fit_hdr,
							     fit_uname);
				if (noffset < 0) {
					printf("Can't find '%s' FIT subimage\n",
					       fit_uname);
					return 1;
				}

				/* verify integrity */
				if (!fit_image_verify(fit_hdr, noffset)) {
					puts ("Bad Data Hash\n");
					return 1;
				}

				/* get fpga subimage data address and length */
				if (fit_image_get_data(fit_hdr, noffset,
						       &fit_data, &data_size)) {
					puts("Fpga subimage data not found\n");
					return 1;
				}

				rc = fpga_load(dev, fit_data, data_size,
					       BIT_FULL);
			}
			break;
#endif
		default:
			puts("** Unknown image type\n");
			rc = FPGA_FAIL;
			break;
		}
		break;
#endif

	case FPGA_DUMP:
		rc = fpga_dump(dev, fpga_data, data_size);
		break;

	default:
		printf("Unknown operation\n");
		return CMD_RET_USAGE;
	}
	return rc;
}

/*
 * Map op to supported operations.  We don't use a table since we
 * would just have to relocate it from flash anyway.
 */
static int fpga_get_op(char *opstr)
{
	int op = FPGA_NONE;

	if (!strcmp("info", opstr))
		op = FPGA_INFO;
	else if (!strcmp("loadb", opstr))
		op = FPGA_LOADB;
	else if (!strcmp("load", opstr))
		op = FPGA_LOAD;
#if defined(CONFIG_CMD_FPGA_LOADP)
	else if (!strcmp("loadp", opstr))
		op = FPGA_LOADP;
#endif
#if defined(CONFIG_CMD_FPGA_LOADBP)
	else if (!strcmp("loadbp", opstr))
		op = FPGA_LOADBP;
#endif
#if defined(CONFIG_CMD_FPGA_LOADFS)
	else if (!strcmp("loadfs", opstr))
		op = FPGA_LOADFS;
#endif
#if defined(CONFIG_CMD_FPGA_LOADMK)
	else if (!strcmp("loadmk", opstr))
		op = FPGA_LOADMK;
#endif
	else if (!strcmp("dump", opstr))
		op = FPGA_DUMP;

	if (op == FPGA_NONE)
		printf("Unknown fpga operation \"%s\"\n", opstr);

	return op;
}

#if defined(CONFIG_CMD_FPGA_LOADFS)
U_BOOT_CMD(fpga, 9, 1, do_fpga,
#else
U_BOOT_CMD(fpga, 6, 1, do_fpga,
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
);
