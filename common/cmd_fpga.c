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
#if (CONFIG_COMMANDS & CFG_CMD_NET)
#include <net.h>
#endif
#include <fpga.h>

#if 0
#define	FPGA_DEBUG
#endif

#ifdef	FPGA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#if defined (CONFIG_FPGA) && ( CONFIG_COMMANDS & CFG_CMD_FPGA )

/* Local functions */
static void fpga_usage (cmd_tbl_t * cmdtp);
static int fpga_get_op (char *opstr);

/* Local defines */
#define FPGA_NONE   -1
#define FPGA_INFO   0
#define FPGA_LOAD   1
#define FPGA_DUMP   3

/* ------------------------------------------------------------------------- */
/* command form:
 *   fpga <op> <device number> <data addr> <datasize>
 * where op is 'load', 'dump', or 'info'
 * If there is no device number field, the fpga environment variable is used.
 * If there is no data addr field, the fpgadata environment variable is used.
 * The info command requires no data address field.
 */
int do_fpga (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int op, dev = FPGA_INVALID_DEVICE;
	size_t data_size = 0;
	void *fpga_data = NULL;
	char *devstr = getenv ("fpga");
	char *datastr = getenv ("fpgadata");
	int rc = FPGA_FAIL;

	if (devstr)
		dev = (int) simple_strtoul (devstr, NULL, 16);
	if (datastr)
		fpga_data = (void *) simple_strtoul (datastr, NULL, 16);

	switch (argc) {
	case 5:		/* fpga <op> <dev> <data> <datasize> */
		data_size = simple_strtoul (argv[4], NULL, 16);
	case 4:		/* fpga <op> <dev> <data> */
		fpga_data = (void *) simple_strtoul (argv[3], NULL, 16);
		PRINTF (__FUNCTION__ ": fpga_data = 0x%x\n",
			(uint) fpga_data);
	case 3:		/* fpga <op> <dev | data addr> */
		dev = (int) simple_strtoul (argv[2], NULL, 16);
		PRINTF (__FUNCTION__ ": device = %d\n", dev);
		/* FIXME - this is a really weak test */
		if ((argc == 3) && (dev > fpga_count ())) {	/* must be buffer ptr */
			PRINTF (__FUNCTION__
				": Assuming buffer pointer in arg 3\n");
			fpga_data = (void *) dev;
			PRINTF (__FUNCTION__ ": fpga_data = 0x%x\n",
				(uint) fpga_data);
			dev = FPGA_INVALID_DEVICE;	/* reset device num */
		}
	case 2:		/* fpga <op> */
		op = (int) fpga_get_op (argv[1]);
		break;
	default:
		PRINTF (__FUNCTION__ ": Too many or too few args (%d)\n",
			argc);
		op = FPGA_NONE;	/* force usage display */
		break;
	}

	switch (op) {
	case FPGA_NONE:
		fpga_usage (cmdtp);
		break;

	case FPGA_INFO:
		rc = fpga_info (dev);
		break;

	case FPGA_LOAD:
		rc = fpga_load (dev, fpga_data, data_size);
		break;

	case FPGA_DUMP:
		rc = fpga_dump (dev, fpga_data, data_size);
		break;

	default:
		printf ("Unknown operation.\n");
		fpga_usage (cmdtp);
		break;
	}
	return (rc);
}

static void fpga_usage (cmd_tbl_t * cmdtp)
{
	printf ("Usage:\n%s\n", cmdtp->usage);
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
	} else if (!strcmp ("load", opstr)) {
		op = FPGA_LOAD;
	} else if (!strcmp ("dump", opstr)) {
		op = FPGA_DUMP;
	}

	if (op == FPGA_NONE) {
		printf ("Unknown fpga operation \"%s\"\n", opstr);
	}
	return op;
}

U_BOOT_CMD (fpga, 6, 1, do_fpga,
	    "fpga    - loadable FPGA image support\n",
	    "fpga [operation type] [device number] [image address] [image size]\n"
	    "fpga operations:\n"
	    "\tinfo\tlist known device information.\n"
	    "\tload\tLoad device from memory buffer.\n"
	    "\tdump\tLoad device to memory buffer.\n");
#endif /* CONFIG_FPGA && CONFIG_COMMANDS & CFG_CMD_FPGA */
