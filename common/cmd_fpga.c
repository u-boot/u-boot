/*
 * (C) Copyright 2000, 2001
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 *
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
 *
 */

/*
 *  FPGA support
 */
#include <common.h>
#include <command.h>
#if defined(CONFIG_CMD_NET)
#include <net.h>
#endif
#include <fpga.h>
#include <malloc.h>

/* Local functions */
static int fpga_get_op (char *opstr);

/* Local defines */
#define FPGA_NONE   -1
#define FPGA_INFO   0
#define FPGA_LOAD   1
#define FPGA_LOADB  2
#define FPGA_DUMP   3
#define FPGA_LOADMK 4

/* Convert bitstream data and load into the fpga */
int fpga_loadbitstream(unsigned long dev, char* fpgadata, size_t size)
{
#if defined(CONFIG_FPGA_XILINX)
	unsigned int length;
	unsigned int swapsize;
	char buffer[80];
	unsigned char *dataptr;
	unsigned int i;
	int rc;

	dataptr = (unsigned char *)fpgadata;

	/* skip the first bytes of the bitsteam, their meaning is unknown */
	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	dataptr+=length;

	/* get design name (identifier, length, string) */
	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	if (*dataptr++ != 0x61) {
		debug("%s: Design name identifier not recognized "
			"in bitstream\n",
			__func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	for(i=0;i<length;i++)
		buffer[i] = *dataptr++;

	printf("  design filename = \"%s\"\n", buffer);

	/* get part number (identifier, length, string) */
	if (*dataptr++ != 0x62) {
		printf("%s: Part number identifier not recognized "
			"in bitstream\n",
			__func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	for(i=0;i<length;i++)
		buffer[i] = *dataptr++;
	printf("  part number = \"%s\"\n", buffer);

	/* get date (identifier, length, string) */
	if (*dataptr++ != 0x63) {
		printf("%s: Date identifier not recognized in bitstream\n",
		       __func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	for(i=0;i<length;i++)
		buffer[i] = *dataptr++;
	printf("  date = \"%s\"\n", buffer);

	/* get time (identifier, length, string) */
	if (*dataptr++ != 0x64) {
		printf("%s: Time identifier not recognized in bitstream\n",
			__func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	for(i=0;i<length;i++)
		buffer[i] = *dataptr++;
	printf("  time = \"%s\"\n", buffer);

	/* get fpga data length (identifier, length) */
	if (*dataptr++ != 0x65) {
		printf("%s: Data length identifier not recognized in bitstream\n",
			__func__);
		return FPGA_FAIL;
	}
	swapsize = ((unsigned int) *dataptr     <<24) +
	           ((unsigned int) *(dataptr+1) <<16) +
	           ((unsigned int) *(dataptr+2) <<8 ) +
	           ((unsigned int) *(dataptr+3)     ) ;
	dataptr+=4;
	printf("  bytes in bitstream = %d\n", swapsize);

	rc = fpga_load(dev, dataptr, swapsize);
	return rc;
#else
	printf("Bitstream support only for Xilinx devices\n");
	return FPGA_FAIL;
#endif
}

/* ------------------------------------------------------------------------- */
/* command form:
 *   fpga <op> <device number> <data addr> <datasize>
 * where op is 'load', 'dump', or 'info'
 * If there is no device number field, the fpga environment variable is used.
 * If there is no data addr field, the fpgadata environment variable is used.
 * The info command requires no data address field.
 */
int do_fpga (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int op, dev = FPGA_INVALID_DEVICE;
	size_t data_size = 0;
	void *fpga_data = NULL;
	char *devstr = getenv ("fpga");
	char *datastr = getenv ("fpgadata");
	int rc = FPGA_FAIL;
	int wrong_parms = 0;
#if defined (CONFIG_FIT)
	const char *fit_uname = NULL;
	ulong fit_addr;
#endif

	if (devstr)
		dev = (int) simple_strtoul (devstr, NULL, 16);
	if (datastr)
		fpga_data = (void *) simple_strtoul (datastr, NULL, 16);

	switch (argc) {
	case 5:		/* fpga <op> <dev> <data> <datasize> */
		data_size = simple_strtoul (argv[4], NULL, 16);

	case 4:		/* fpga <op> <dev> <data> */
#if defined(CONFIG_FIT)
		if (fit_parse_subimage (argv[3], (ulong)fpga_data,
					&fit_addr, &fit_uname)) {
			fpga_data = (void *)fit_addr;
			debug("*  fpga: subimage '%s' from FIT image "
				"at 0x%08lx\n",
				fit_uname, fit_addr);
		} else
#endif
		{
			fpga_data = (void *) simple_strtoul (argv[3], NULL, 16);
			debug("*  fpga: cmdline image address = 0x%08lx\n",
				(ulong)fpga_data);
		}
		debug("%s: fpga_data = 0x%x\n", __func__, (uint) fpga_data);

	case 3:		/* fpga <op> <dev | data addr> */
		dev = (int) simple_strtoul (argv[2], NULL, 16);
		debug("%s: device = %d\n", __func__, dev);
		/* FIXME - this is a really weak test */
		if ((argc == 3) && (dev > fpga_count ())) {	/* must be buffer ptr */
			debug("%s: Assuming buffer pointer in arg 3\n",
				__func__);

#if defined(CONFIG_FIT)
			if (fit_parse_subimage (argv[2], (ulong)fpga_data,
						&fit_addr, &fit_uname)) {
				fpga_data = (void *)fit_addr;
				debug("*  fpga: subimage '%s' from FIT image "
					"at 0x%08lx\n",
					fit_uname, fit_addr);
			} else
#endif
			{
				fpga_data = (void *) dev;
				debug("*  fpga: cmdline image address = "
					"0x%08lx\n", (ulong)fpga_data);
			}

			debug("%s: fpga_data = 0x%x\n",
				__func__, (uint) fpga_data);
			dev = FPGA_INVALID_DEVICE;	/* reset device num */
		}

	case 2:		/* fpga <op> */
		op = (int) fpga_get_op (argv[1]);
		break;

	default:
		debug("%s: Too many or too few args (%d)\n",
			__func__, argc);
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
	case FPGA_LOAD:
	case FPGA_LOADB:
	case FPGA_DUMP:
		if (!fpga_data || !data_size)
			wrong_parms = 1;
		break;
	case FPGA_LOADMK:
		if (!fpga_data)
			wrong_parms = 1;
		break;
	}

	if (wrong_parms) {
		puts("Wrong parameters for FPGA request\n");
		op = FPGA_NONE;
	}

	switch (op) {
	case FPGA_NONE:
		return CMD_RET_USAGE;

	case FPGA_INFO:
		rc = fpga_info (dev);
		break;

	case FPGA_LOAD:
		rc = fpga_load (dev, fpga_data, data_size);
		break;

	case FPGA_LOADB:
		rc = fpga_loadbitstream(dev, fpga_data, data_size);
		break;

	case FPGA_LOADMK:
		switch (genimg_get_format (fpga_data)) {
		case IMAGE_FORMAT_LEGACY:
			{
				image_header_t *hdr = (image_header_t *)fpga_data;
				ulong	data;

				data = (ulong)image_get_data (hdr);
				data_size = image_get_data_size (hdr);
				rc = fpga_load (dev, (void *)data, data_size);
			}
			break;
#if defined(CONFIG_FIT)
		case IMAGE_FORMAT_FIT:
			{
				const void *fit_hdr = (const void *)fpga_data;
				int noffset;
				const void *fit_data;

				if (fit_uname == NULL) {
					puts ("No FIT subimage unit name\n");
					return 1;
				}

				if (!fit_check_format (fit_hdr)) {
					puts ("Bad FIT image format\n");
					return 1;
				}

				/* get fpga component image node offset */
				noffset = fit_image_get_node (fit_hdr, fit_uname);
				if (noffset < 0) {
					printf ("Can't find '%s' FIT subimage\n", fit_uname);
					return 1;
				}

				/* verify integrity */
				if (!fit_image_check_hashes (fit_hdr, noffset)) {
					puts ("Bad Data Hash\n");
					return 1;
				}

				/* get fpga subimage data address and length */
				if (fit_image_get_data (fit_hdr, noffset, &fit_data, &data_size)) {
					puts ("Could not find fpga subimage data\n");
					return 1;
				}

				rc = fpga_load (dev, fit_data, data_size);
			}
			break;
#endif
		default:
			puts ("** Unknown image type\n");
			rc = FPGA_FAIL;
			break;
		}
		break;

	case FPGA_DUMP:
		rc = fpga_dump (dev, fpga_data, data_size);
		break;

	default:
		printf ("Unknown operation\n");
		return CMD_RET_USAGE;
	}
	return (rc);
}

/*
 * Map op to supported operations.  We don't use a table since we
 * would just have to relocate it from flash anyway.
 */
static int fpga_get_op (char *opstr)
{
	int op = FPGA_NONE;

	if (!strcmp ("info", opstr)) {
		op = FPGA_INFO;
	} else if (!strcmp ("loadb", opstr)) {
		op = FPGA_LOADB;
	} else if (!strcmp ("load", opstr)) {
		op = FPGA_LOAD;
	} else if (!strcmp ("loadmk", opstr)) {
		op = FPGA_LOADMK;
	} else if (!strcmp ("dump", opstr)) {
		op = FPGA_DUMP;
	}

	if (op == FPGA_NONE) {
		printf ("Unknown fpga operation \"%s\"\n", opstr);
	}
	return op;
}

U_BOOT_CMD (fpga, 6, 1, do_fpga,
	"loadable FPGA image support",
	"[operation type] [device number] [image address] [image size]\n"
	"fpga operations:\n"
	"  dump\t[dev]\t\t\tLoad device to memory buffer\n"
	"  info\t[dev]\t\t\tlist known device information\n"
	"  load\t[dev] [address] [size]\tLoad device from memory buffer\n"
	"  loadb\t[dev] [address] [size]\t"
	"Load device from bitstream buffer (Xilinx only)\n"
	"  loadmk [dev] [address]\tLoad device generated with mkimage"
#if defined(CONFIG_FIT)
	"\n"
	"\tFor loadmk operating on FIT format uImage address must include\n"
	"\tsubimage unit name in the form of addr:<subimg_uname>"
#endif
);
