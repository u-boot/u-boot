/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * hacked for Hymod FPGA support by Murray.Jensen@csiro.au, 29-Jan-01
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/iopin_8260.h>

DECLARE_GLOBAL_DATA_PTR;

/*-----------------------------------------------------------------------
 * Board Special Commands: FPGA load/store, EEPROM erase
 */

#if defined(CONFIG_CMD_BSP)

#define LOAD_SUCCESS		0
#define LOAD_FAIL_NOCONF	1
#define LOAD_FAIL_NOINIT	2
#define LOAD_FAIL_NODONE	3

#define STORE_SUCCESS		0

/*
 * Programming the Hymod FPGAs
 *
 * The 8260 io port config table is set up so that the INIT pin is
 * held Low (Open Drain output 0) - this will delay the automatic
 * Power-On config until INIT is released (by making it an input).
 *
 * If the FPGA has been programmed before, then the assertion of PROGRAM
 * will initiate configuration (i.e. it begins clearing the RAM).
 *
 * When the FPGA is ready to receive configuration data (either after
 * releasing INIT after Power-On, or after asserting PROGRAM), it will
 * pull INIT high.
 *
 * Notes from Paul Dunn:
 *
 *  1. program pin should be forced low for >= 300ns
 *     (about 20 bus clock cycles minimum).
 *
 *  2. then wait for init to go high, which signals
 *     that the FPGA has cleared its internal memory
 *     and is ready to load
 *
 *  3. perform load writes of entire config file
 *
 *  4. wait for done to go high, which should be
 *     within a few bus clock cycles. If done has not
 *     gone high after reasonable period, then load
 *     has not worked (wait several ms?)
 */

int
fpga_load (int mezz, uchar *addr, ulong size)
{
	hymod_conf_t *cp = &gd->bd->bi_hymod_conf;
	xlx_info_t *fp;
	xlx_iopins_t *fpgaio;
	volatile uchar *fpgabase;
	volatile uint cnt;
	uchar *eaddr = addr + size;
	int result;

	if (mezz)
		fp = &cp->mezz.xlx[0];
	else
		fp = &cp->main.xlx[0];

	if (!fp->mmap.prog.exists)
		return (LOAD_FAIL_NOCONF);

	fpgabase = (uchar *)fp->mmap.prog.base;
	fpgaio = &fp->iopins;

	/* set enable HIGH if required */
	if (fpgaio->enable_pin.flag)
		iopin_set_high (&fpgaio->enable_pin);

	/* ensure INIT is released (set it to be an input) */
	iopin_set_in (&fpgaio->init_pin);

	/* toggle PROG Low then High (will already be Low after Power-On) */
	iopin_set_low (&fpgaio->prog_pin);
	udelay (1);	/* minimum 300ns - 1usec should do it */
	iopin_set_high (&fpgaio->prog_pin);

	/* wait for INIT High */
	cnt = 0;
	while (!iopin_is_high (&fpgaio->init_pin))
		if (++cnt == 10000000) {
			result = LOAD_FAIL_NOINIT;
			goto done;
		}

	/* write configuration data */
	while (addr < eaddr)
		*fpgabase = *addr++;

	/* wait for DONE High */
	cnt = 0;
	while (!iopin_is_high (&fpgaio->done_pin))
		if (++cnt == 100000000) {
			result = LOAD_FAIL_NODONE;
			goto done;
		}

	/* success */
	result = LOAD_SUCCESS;

  done:

	if (fpgaio->enable_pin.flag)
		iopin_set_low (&fpgaio->enable_pin);

	return (result);
}

/* ------------------------------------------------------------------------- */
int
do_fpga (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	uchar *addr, *save_addr;
	ulong size;
	int mezz, arg, result;

	switch (argc) {

	case 0:
	case 1:
		break;

	case 2:
		if (strcmp (argv[1], "info") == 0) {
			printf ("\nHymod FPGA Info...\n");
			printf ("\t\t\t\tAddress\t\tSize\n");
			printf ("\tMain Configuration:\t0x%08x\t%d\n",
				FPGA_MAIN_CFG_BASE, FPGA_MAIN_CFG_SIZE);
			printf ("\tMain Register:\t\t0x%08x\t%d\n",
				FPGA_MAIN_REG_BASE, FPGA_MAIN_REG_SIZE);
			printf ("\tMain Port:\t\t0x%08x\t%d\n",
				FPGA_MAIN_PORT_BASE, FPGA_MAIN_PORT_SIZE);
			printf ("\tMezz Configuration:\t0x%08x\t%d\n",
				FPGA_MEZZ_CFG_BASE, FPGA_MEZZ_CFG_SIZE);
			return 0;
		}
		break;

	case 3:
		if (strcmp (argv[1], "store") == 0) {
			addr = (uchar *) simple_strtoul (argv[2], NULL, 16);

			save_addr = addr;
#if 0
			/* fpga readback unimplemented */
			while (more readback data)
				*addr++ = *fpga;
			result = error ? STORE_FAIL_XXX : STORE_SUCCESS;
#else
			result = STORE_SUCCESS;
#endif

			if (result == STORE_SUCCESS) {
				printf ("SUCCEEDED (%d bytes)\n",
					addr - save_addr);
				return 0;
			} else
				printf ("FAILED (%d bytes)\n",
					addr - save_addr);
			return 1;
		}
		break;

	case 4:
		if (strcmp (argv[1], "tftp") == 0) {
			copy_filename (BootFile, argv[2], sizeof (BootFile));
			load_addr = simple_strtoul (argv[3], NULL, 16);
			NetBootFileXferSize = 0;

			if (NetLoop (TFTP) <= 0) {
				printf ("tftp transfer failed - aborting "
					"fgpa load\n");
				return 1;
			}

			if (NetBootFileXferSize == 0) {
				printf ("can't determine file size - "
					"aborting fpga load\n");
				return 1;
			}

			printf ("File transfer succeeded - "
				"beginning fpga load...");

			result = fpga_load (0, (uchar *) load_addr,
				NetBootFileXferSize);

			if (result == LOAD_SUCCESS) {
				printf ("SUCCEEDED\n");
				return 0;
			} else if (result == LOAD_FAIL_NOCONF)
				printf ("FAILED (no CONF)\n");
			else if (result == LOAD_FAIL_NOINIT)
				printf ("FAILED (no INIT)\n");
			else
				printf ("FAILED (no DONE)\n");
			return 1;

		}
		/* fall through ... */

	case 5:
		if (strcmp (argv[1], "load") == 0) {
			if (argc == 5) {
				if (strcmp (argv[2], "main") == 0)
					mezz = 0;
				else if (strcmp (argv[2], "mezz") == 0)
					mezz = 1;
				else {
					printf ("FPGA type must be either "
						"`main' or `mezz'\n");
					return 1;
				}
				arg = 3;
			} else {
				mezz = 0;
				arg = 2;
			}

			addr = (uchar *) simple_strtoul (argv[arg++], NULL, 16);
			size = (ulong) simple_strtoul (argv[arg], NULL, 16);

			result = fpga_load (mezz, addr, size);

			if (result == LOAD_SUCCESS) {
				printf ("SUCCEEDED\n");
				return 0;
			} else if (result == LOAD_FAIL_NOCONF)
				printf ("FAILED (no CONF)\n");
			else if (result == LOAD_FAIL_NOINIT)
				printf ("FAILED (no INIT)\n");
			else
				printf ("FAILED (no DONE)\n");
			return 1;
		}
		break;

	default:
		break;
	}

	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
}
U_BOOT_CMD(
	fpga,	6,	1,	do_fpga,
	"fpga    - FPGA sub-system\n",
	"load [type] addr size\n"
	"  - write the configuration data at memory address `addr',\n"
	"    size `size' bytes, into the FPGA of type `type' (either\n"
	"    `main' or `mezz', default `main'). e.g.\n"
	"        `fpga load 100000 7d8f'\n"
	"    loads the main FPGA with config data at address 100000\n"
	"    HEX, size 7d8f HEX (32143 DEC) bytes\n"
	"fpga tftp file addr\n"
	"  - transfers `file' from the tftp server into memory at\n"
	"    address `addr', then writes the entire file contents\n"
	"    into the main FPGA\n"
	"fpga store addr\n"
	"  - read configuration data from the main FPGA (the mezz\n"
	"    FPGA is write-only), into address `addr'. There must be\n"
	"    enough memory available at `addr' to hold all the config\n"
	"    data - the size of which is determined by VC:???\n"
	"fpga info\n"
	"  - print information about the Hymod FPGA, namely the\n"
	"    memory addresses at which the four FPGA local bus\n"
	"    address spaces appear in the physical address space\n"
);
/* ------------------------------------------------------------------------- */
int
do_eecl (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	uchar data[HYMOD_EEPROM_SIZE];
	uint addr = CFG_I2C_EEPROM_ADDR;

	switch (argc) {

	case 1:
		addr |= HYMOD_EEOFF_MAIN;
		break;

	case 2:
		if (strcmp (argv[1], "main") == 0) {
			addr |= HYMOD_EEOFF_MAIN;
			break;
		}
		if (strcmp (argv[1], "mezz") == 0) {
			addr |= HYMOD_EEOFF_MEZZ;
			break;
		}
		/* fall through ... */

	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	memset (data, 0, HYMOD_EEPROM_SIZE);

	eeprom_write (addr, 0, data, HYMOD_EEPROM_SIZE);

	return 0;
}
U_BOOT_CMD(
	eeclear,	1,	0,	do_eecl,
	"eeclear - Clear the eeprom on a Hymod board \n",
	"[type]\n"
	"  - write zeroes into the EEPROM on the board of type `type'\n"
	"    (`type' is either `main' or `mezz' - default `main')\n"
	"    Note: the EEPROM write enable jumper must be installed\n"
);

/* ------------------------------------------------------------------------- */

int
do_htest (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if 0
	int rc;
#endif
#ifdef CONFIG_ETHER_LOOPBACK_TEST
	extern void eth_loopback_test (void);
#endif /* CONFIG_ETHER_LOOPBACK_TEST */

	printf ("HYMOD tests - ensure loopbacks etc. are connected\n\n");

#if 0
	/* Load FPGA with test program */

	printf ("Loading test FPGA program ...");

	rc = fpga_load (0, test_bitfile, sizeof (test_bitfile));

	switch (rc) {

	case LOAD_SUCCESS:
		printf (" SUCCEEDED\n");
		break;

	case LOAD_FAIL_NOCONF:
		printf (" FAILED (no configuration space defined)\n");
		return 1;

	case LOAD_FAIL_NOINIT:
		printf (" FAILED (timeout - no INIT signal seen)\n");
		return 1;

	case LOAD_FAIL_NODONE:
		printf (" FAILED (timeout - no DONE signal seen)\n");
		return 1;

	default:
		printf (" FAILED (unknown return code from fpga_load\n");
		return 1;
	}

	/* run Local Bus <=> Xilinx tests */

	/* tell Xilinx to run ZBT Ram, High Speed serial and Mezzanine tests */

	/* run SDRAM test */
#endif

#ifdef CONFIG_ETHER_LOOPBACK_TEST
	/* run Ethernet test */
	eth_loopback_test ();
#endif /* CONFIG_ETHER_LOOPBACK_TEST */

	return 0;
}

#endif
