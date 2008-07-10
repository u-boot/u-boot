/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/byteorder.h>

#define	CONFIG_STRESS		/* enable 667 MHz CPU freq selection */
#define DEBUG

static int do_bootstrap(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uchar	chip;
	ulong	data;
	int	nbytes;
	extern char console_buffer[];

	char sysClock[4];
	char cpuClock[4];
	char plbClock[4];
	char pcixClock[4];

	if (argc < 3) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if (strcmp(argv[2], "prom0") == 0)
		chip = IIC0_BOOTPROM_ADDR;
	else
		chip = IIC0_ALT_BOOTPROM_ADDR;

	/* on Katmai SysClk is always 33MHz */
	strcpy(sysClock, "33");

	do {
#ifdef	CONFIG_STRESS
		printf("enter cpu clock frequency 400, 500, 533, 667 Mhz or quit to abort\n");
#else
		printf("enter cpu clock frequency 400, 500, 533 Mhz or quit to abort\n");
#endif
		nbytes = readline (" ? ");

		if (strcmp(console_buffer, "quit") == 0)
			return 0;

		if ((strcmp(console_buffer, "400") != 0) &&
		    (strcmp(console_buffer, "500") != 0) &&
		    (strcmp(console_buffer, "533") != 0)
#ifdef	CONFIG_STRESS
		    && (strcmp(console_buffer, "667") != 0)
#endif
			) {
			nbytes = 0;
		}

		strcpy(cpuClock, console_buffer);

	} while (nbytes == 0);

	if (strcmp(cpuClock, "500") == 0)
		strcpy(plbClock, "166");
	else if (strcmp(cpuClock, "533") == 0)
		strcpy(plbClock, "133");
	else {
		do {
			if (strcmp(cpuClock, "400") == 0)
				printf("enter plb clock frequency 100, 133 Mhz or quit to abort\n");

#ifdef	CONFIG_STRESS
			if (strcmp(cpuClock, "667") == 0)
				printf("enter plb clock frequency 133, 166 Mhz or quit to abort\n");

#endif
			nbytes = readline (" ? ");

			if (strcmp(console_buffer, "quit") == 0)
				return 0;

			if (strcmp(cpuClock, "400") == 0) {
				if ((strcmp(console_buffer, "100") != 0) &&
				    (strcmp(console_buffer, "133") != 0))
					nbytes = 0;
			}
#ifdef	CONFIG_STRESS
			if (strcmp(cpuClock, "667") == 0) {
				if ((strcmp(console_buffer, "133") != 0) &&
				    (strcmp(console_buffer, "166") != 0))
					nbytes = 0;
			}
#endif
			strcpy(plbClock, console_buffer);

		} while (nbytes == 0);
	}

	do {
		printf("enter Pci-X clock frequency 33, 66, 100 or 133 Mhz or quit to abort\n");
		nbytes = readline (" ? ");

		if (strcmp(console_buffer, "quit") == 0)
			return 0;

		if ((strcmp(console_buffer, "33") != 0) &&
		    (strcmp(console_buffer, "66") != 0) &&
		    (strcmp(console_buffer, "100") != 0) &&
		    (strcmp(console_buffer, "133") != 0)) {
			nbytes = 0;
		}
		strcpy(pcixClock, console_buffer);

	} while (nbytes == 0);

	printf("\nsys clk   = %sMhz\n", sysClock);
	printf("cpu clk   = %sMhz\n", cpuClock);
	printf("plb clk   = %sMhz\n", plbClock);
	printf("Pci-X clk = %sMhz\n", pcixClock);

	do {
		printf("\npress [y] to write I2C bootstrap \n");
		printf("or [n] to abort.  \n");
		printf("Don't forget to set board switches \n");
		printf("according to your choice before re-starting \n");
		printf("(refer to 440spe_uboot_kit_um_1_01.pdf) \n");

		nbytes = readline (" ? ");
		if (strcmp(console_buffer, "n") == 0)
			return 0;

	} while (nbytes == 0);

	if (strcmp(sysClock, "33") == 0) {
		if ((strcmp(cpuClock, "400") == 0) &&
		    (strcmp(plbClock, "100") == 0))
			data = 0x8678c206;

		if ((strcmp(cpuClock, "400") == 0) &&
		    (strcmp(plbClock, "133") == 0))
			data = 0x8678c2c6;

		if ((strcmp(cpuClock, "500") == 0))
			data = 0x8778f2c6;

		if ((strcmp(cpuClock, "533") == 0))
			data = 0x87790252;
#ifdef	CONFIG_STRESS
		if ((strcmp(cpuClock, "667") == 0) &&
		    (strcmp(plbClock, "133") == 0))
			data = 0x87794256;

		if ((strcmp(cpuClock, "667") == 0) &&
		    (strcmp(plbClock, "166") == 0))
			data = 0x87794206;
#endif
	}
#ifdef	DEBUG
	printf(" pin strap0 to write in i2c  = %lx\n", data);
#endif	/* DEBUG */

	if (i2c_write(chip, 0, 1, (uchar *)&data, 4) != 0)
		printf("Error writing strap0 in %s\n", argv[2]);

	if (strcmp(pcixClock, "33") == 0)
		data = 0x000007E1;

	if (strcmp(pcixClock, "66") == 0)
		data = 0x000006E1;

	if (strcmp(pcixClock, "100") == 0)
		data = 0x000005E1;

	if (strcmp(pcixClock, "133") == 0)
		data = 0x000004E1;

	if (strcmp(plbClock, "166") == 0)
/*		data |= 0x05950000; */	/* this set's DDR2 clock == PLB clock */
		data |= 0x05A50000;	/* this set's DDR2 clock == 2 * PLB clock */
	else
		data |= 0x05A50000;

#ifdef	DEBUG
	printf(" pin strap1 to write in i2c  = %lx\n", data);
#endif	/* DEBUG */

	udelay(1000);
	if (i2c_write(chip, 4, 1, (uchar *)&data, 4) != 0)
		printf("Error writing strap1 in %s\n", argv[2]);

	return 0;
}

U_BOOT_CMD(
	bootstrap,	3,	1,	do_bootstrap,
	"bootstrap - program the serial device strap\n",
	"wrclk [prom0|prom1] - program the serial device strap\n"
	);
