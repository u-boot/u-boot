/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Microblaze FSL support
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <asm/asm.h>

int do_frd (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned int fslnum;
	unsigned int num;
	unsigned int blocking;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	fslnum = (unsigned int)simple_strtoul (argv[1], NULL, 16);
	blocking = (unsigned int)simple_strtoul (argv[2], NULL, 16);
	if (fslnum < 0 || fslnum >= XILINX_FSL_NUMBER) {
		puts ("Bad number of FSL\n");
		cmd_usage(cmdtp);
		return 1;
	}

	switch (fslnum) {
#if (XILINX_FSL_NUMBER > 0)
	case 0:
		switch (blocking) {
		case 0:	NGET (num, 0);
			break;
		case 1:	NCGET (num, 0);
			break;
		case 2:	GET (num, 0);
			break;
		case 3:	CGET (num, 0);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 1)
	case 1:
		switch (blocking) {
		case 0:	NGET (num, 1);
			break;
		case 1:	NCGET (num, 1);
			break;
		case 2:	GET (num, 1);
			break;
		case 3:	CGET (num, 1);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 2)
	case 2:
		switch (blocking) {
		case 0:	NGET (num, 2);
			break;
		case 1:	NCGET (num, 2);
			break;
		case 2:	GET (num, 2);
			break;
		case 3:	CGET (num, 2);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 3)
	case 3:
		switch (blocking) {
		case 0:	NGET (num, 3);
			break;
		case 1:	NCGET (num, 3);
			break;
		case 2:	GET (num, 3);
			break;
		case 3:	CGET (num, 3);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 4)
	case 4:
		switch (blocking) {
		case 0:	NGET (num, 4);
			break;
		case 1:	NCGET (num, 4);
			break;
		case 2:	GET (num, 4);
			break;
		case 3:	CGET (num, 4);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 5)
	case 5:
		switch (blocking) {
		case 0:	NGET (num, 5);
			break;
		case 1:	NCGET (num, 5);
			break;
		case 2:	GET (num, 5);
			break;
		case 3:	CGET (num, 5);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 6)
	case 6:
		switch (blocking) {
		case 0:	NGET (num, 6);
			break;
		case 1:	NCGET (num, 6);
			break;
		case 2:	GET (num, 6);
			break;
		case 3:	CGET (num, 6);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 7)
	case 7:
		switch (blocking) {
		case 0:	NGET (num, 7);
			break;
		case 1:	NCGET (num, 7);
			break;
		case 2:	GET (num, 7);
			break;
		case 3:	CGET (num, 7);
			break;
		default:
			return 2;
		}
		break;
#endif
	default:
		return 1;
	}

	printf ("%01x: 0x%08x - %s %s read\n", fslnum, num,
		blocking < 2  ? "non blocking" : "blocking",
		((blocking == 1) || (blocking == 3)) ? "control" : "data" );
	return 0;
}

int do_fwr (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned int fslnum;
	unsigned int num;
	unsigned int blocking;

	if (argc < 3) {
		cmd_usage(cmdtp);
		return 1;
	}

	fslnum = (unsigned int)simple_strtoul (argv[1], NULL, 16);
	num = (unsigned int)simple_strtoul (argv[2], NULL, 16);
	blocking = (unsigned int)simple_strtoul (argv[3], NULL, 16);
	if (fslnum < 0 || fslnum >= XILINX_FSL_NUMBER) {
		cmd_usage(cmdtp);
		return 1;
	}

	switch (fslnum) {
#if (XILINX_FSL_NUMBER > 0)
	case 0:
		switch (blocking) {
		case 0:	NPUT (num, 0);
			break;
		case 1:	NCPUT (num, 0);
			break;
		case 2:	PUT (num, 0);
			break;
		case 3:	CPUT (num, 0);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 1)
	case 1:
		switch (blocking) {
		case 0:	NPUT (num, 1);
			break;
		case 1:	NCPUT (num, 1);
			break;
		case 2:	PUT (num, 1);
			break;
		case 3:	CPUT (num, 1);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 2)
	case 2:
		switch (blocking) {
		case 0:	NPUT (num, 2);
			break;
		case 1:	NCPUT (num, 2);
			break;
		case 2:	PUT (num, 2);
			break;
		case 3:	CPUT (num, 2);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 3)
	case 3:
		switch (blocking) {
		case 0:	NPUT (num, 3);
			break;
		case 1:	NCPUT (num, 3);
			break;
		case 2:	PUT (num, 3);
			break;
		case 3:	CPUT (num, 3);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 4)
	case 4:
		switch (blocking) {
		case 0:	NPUT (num, 4);
			break;
		case 1:	NCPUT (num, 4);
			break;
		case 2:	PUT (num, 4);
			break;
		case 3:	CPUT (num, 4);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 5)
	case 5:
		switch (blocking) {
		case 0:	NPUT (num, 5);
			break;
		case 1:	NCPUT (num, 5);
			break;
		case 2:	PUT (num, 5);
			break;
		case 3:	CPUT (num, 5);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 6)
	case 6:
		switch (blocking) {
		case 0:	NPUT (num, 6);
			break;
		case 1:	NCPUT (num, 6);
			break;
		case 2:	PUT (num, 6);
			break;
		case 3:	CPUT (num, 6);
			break;
		default:
			return 2;
		}
		break;
#endif
#if (XILINX_FSL_NUMBER > 7)
	case 7:
		switch (blocking) {
		case 0:	NPUT (num, 7);
			break;
		case 1:	NCPUT (num, 7);
			break;
		case 2:	PUT (num, 7);
			break;
		case 3:	CPUT (num, 7);
			break;
		default:
			return 2;
		}
		break;
#endif
	default:
		return 1;
	}

	printf ("%01x: 0x%08x - %s %s write\n", fslnum, num,
		blocking < 2  ? "non blocking" : "blocking",
		((blocking == 1) || (blocking == 3)) ? "control" : "data" );
	return 0;

}

int do_rspr (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned int reg = 0;
	unsigned int val = 0;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}
	reg = (unsigned int)simple_strtoul (argv[1], NULL, 16);
	val = (unsigned int)simple_strtoul (argv[2], NULL, 16);
	switch (reg) {
	case 0x1:
		if (argc > 2) {
			MTS (val, rmsr);
			NOP;
			MFS (val, rmsr);
		} else {
			MFS (val, rmsr);
		}
		puts ("MSR");
		break;
	case 0x3:
		MFS (val, rear);
		puts ("EAR");
		break;
	case 0x5:
		MFS (val, resr);
		puts ("ESR");
		break;
	default:
		puts ("Unsupported register\n");
		return 1;
	}
	printf (": 0x%08x\n", val);
	return 0;
}

/***************************************************/

U_BOOT_CMD (frd, 3, 1, do_frd,
		"read data from FSL",
		"- [fslnum [0|1|2|3]]\n"
		" 0 - non blocking data read\n"
		" 1 - non blocking control read\n"
		" 2 - blocking data read\n"
		" 3 - blocking control read");

U_BOOT_CMD (fwr, 4, 1, do_fwr,
		"write data to FSL",
		"- [fslnum [0|1|2|3]]\n"
		" 0 - non blocking data write\n"
		" 1 - non blocking control write\n"
		" 2 - blocking data write\n"
		" 3 - blocking control write");

U_BOOT_CMD (rspr, 3, 1, do_rspr,
		"read/write special purpose register",
		"- reg_num [write value] read/write special purpose register\n"
		" 1 - MSR - Machine status register\n"
		" 3 - EAR - Exception address register\n"
		" 5 - ESR - Exception status register");
