/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
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
 * hacked for evb440spe
 */

#include <common.h>
#include <command.h>
#include "yucca.h"
#include <i2c.h>
#include <asm/byteorder.h>

extern void print_evb440spe_info(void);
static int setBootStrapClock(cmd_tbl_t *cmdtp, int incrflag,
		int flag, int argc, char * const argv[]);

/* ------------------------------------------------------------------------- */
int do_evb440spe(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return setBootStrapClock (cmdtp, 1, flag, argc, argv);
}

/* ------------------------------------------------------------------------- */
/* Modify memory.
 *
 * Syntax:
 *	evb440spe wrclk prom0,prom1
 */
static int setBootStrapClock(cmd_tbl_t *cmdtp, int incrflag, int flag,
		int argc, char * const argv[])
{
	uchar	chip;
	ulong	data;
	int	nbytes;

	char sysClock[4];
	char cpuClock[4];
	char plbClock[4];
	char pcixClock[4];

	if (argc < 3)
		return cmd_usage(cmdtp);

	if (strcmp(argv[2], "prom0") == 0)
		chip = IIC0_BOOTPROM_ADDR;
	else
		chip = IIC0_ALT_BOOTPROM_ADDR;

	do {
		printf("enter sys clock frequency 33 or 66 MHz or quit to abort\n");
		nbytes = readline (" ? ");

		if (strcmp(console_buffer, "quit") == 0)
			return 0;

		if ((strcmp(console_buffer, "33") != 0) &
				(strcmp(console_buffer, "66") != 0))
			nbytes=0;

		strcpy(sysClock, console_buffer);

	} while (nbytes == 0);

	do {
		if (strcmp(sysClock, "66") == 0) {
			printf("enter cpu clock frequency 400, 533 MHz or quit to abort\n");
		} else {
#ifdef	CONFIG_STRESS
			printf("enter cpu clock frequency 400, 500, 533, 667 MHz or quit to abort\n");
#else
			printf("enter cpu clock frequency 400, 500, 533 MHz or quit to abort\n");
#endif
		}
		nbytes = readline (" ? ");

		if (strcmp(console_buffer, "quit") == 0)
			return 0;

		if (strcmp(sysClock, "66") == 0) {
			if ((strcmp(console_buffer, "400") != 0) &
					(strcmp(console_buffer, "533") != 0)
#ifdef	CONFIG_STRESS
					& (strcmp(console_buffer, "667") != 0)
#endif
			   ) {
				nbytes = 0;
			}
		} else {
			if ((strcmp(console_buffer, "400") != 0) &
					(strcmp(console_buffer, "500") != 0) &
					(strcmp(console_buffer, "533") != 0)
#ifdef	CONFIG_STRESS
					& (strcmp(console_buffer, "667") != 0)
#endif
			   ) {
				nbytes = 0;
			}
		}

		strcpy(cpuClock, console_buffer);

	} while (nbytes == 0);

	if (strcmp(cpuClock, "500") == 0){
		strcpy(plbClock, "166");
	} else if (strcmp(cpuClock, "533") == 0){
		strcpy(plbClock, "133");
	} else {
		do {
			if (strcmp(cpuClock, "400") == 0)
				printf("enter plb clock frequency 100, 133 MHz or quit to abort\n");

#ifdef	CONFIG_STRESS
			if (strcmp(cpuClock, "667") == 0)
				printf("enter plb clock frequency 133, 166 MHz or quit to abort\n");

#endif
			nbytes = readline (" ? ");

			if (strcmp(console_buffer, "quit") == 0)
				return 0;

			if (strcmp(cpuClock, "400") == 0) {
				if ((strcmp(console_buffer, "100") != 0) &
						(strcmp(console_buffer, "133") != 0))
					nbytes = 0;
			}
#ifdef	CONFIG_STRESS
			if (strcmp(cpuClock, "667") == 0) {
				if ((strcmp(console_buffer, "133") != 0) &
						(strcmp(console_buffer, "166") != 0))
					nbytes = 0;
			}
#endif
			strcpy(plbClock, console_buffer);

		} while (nbytes == 0);
	}

	do {
		printf("enter Pci-X clock frequency 33, 66, 100 or 133 MHz or quit to abort\n");
		nbytes = readline (" ? ");

		if (strcmp(console_buffer, "quit") == 0)
			return 0;

		if ((strcmp(console_buffer, "33") != 0) &
				(strcmp(console_buffer, "66") != 0) &
				(strcmp(console_buffer, "100") != 0) &
				(strcmp(console_buffer, "133") != 0)) {
			nbytes = 0;
		}
		strcpy(pcixClock, console_buffer);

	} while (nbytes == 0);

	printf("\nsys clk   = %s MHz\n", sysClock);
	printf("cpu clk   = %s MHz\n", cpuClock);
	printf("plb clk   = %s MHz\n", plbClock);
	printf("Pci-X clk = %s MHz\n", pcixClock);

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
		if ((strcmp(cpuClock, "400") == 0) &
				(strcmp(plbClock, "100") == 0))
			data = 0x8678c206;

		if ((strcmp(cpuClock, "400") == 0) &
				(strcmp(plbClock, "133") == 0))
			data = 0x8678c2c6;

		if ((strcmp(cpuClock, "500") == 0))
			data = 0x8778f2c6;

		if ((strcmp(cpuClock, "533") == 0))
			data = 0x87790252;

#ifdef	CONFIG_STRESS
		if ((strcmp(cpuClock, "667") == 0) &
				(strcmp(plbClock, "133") == 0))
			data = 0x87794256;

		if ((strcmp(cpuClock, "667") == 0) &
				(strcmp(plbClock, "166") == 0))
			data = 0x87794206;

#endif
	}
	if (strcmp(sysClock, "66") == 0) {
		if ((strcmp(cpuClock, "400") == 0) &
				(strcmp(plbClock, "100") == 0))
			data = 0x84706206;

		if ((strcmp(cpuClock, "400") == 0) &
				(strcmp(plbClock, "133") == 0))
			data = 0x847062c6;

		if ((strcmp(cpuClock, "533") == 0))
			data = 0x85708206;

#ifdef	CONFIG_STRESS
		if ((strcmp(cpuClock, "667") == 0) &
				(strcmp(plbClock, "133") == 0))
			data = 0x8570a256;

		if ((strcmp(cpuClock, "667") == 0) &
				(strcmp(plbClock, "166") == 0))
			data = 0x8570a206;

#endif
	}

#ifdef	DEBUG
	printf(" pin strap0 to write in i2c  = %x\n", data);
#endif	/* DEBUG */

	if (i2c_write(chip, 0, 1, (uchar *)&data, 4) != 0)
		printf("Error writing strap0 in %s\n", argv[2]);

	if (strcmp(pcixClock, "33") == 0)
		data = 0x00000701;

	if (strcmp(pcixClock, "66") == 0)
		data = 0x00000601;

	if (strcmp(pcixClock, "100") == 0)
		data = 0x00000501;

	if (strcmp(pcixClock, "133") == 0)
		data = 0x00000401;

	if (strcmp(plbClock, "166") == 0)
		data = data | 0x05950000;
	else
		data = data | 0x05A50000;

#ifdef	DEBUG
	printf(" pin strap1 to write in i2c  = %x\n", data);
#endif	/* DEBUG */

	udelay(1000);
	if (i2c_write(chip, 4, 1, (uchar *)&data, 4) != 0)
		printf("Error writing strap1 in %s\n", argv[2]);

	return 0;
}

U_BOOT_CMD(
	evb440spe,	3,	1,	do_evb440spe,
	"program the serial device strap",
	"wrclk [prom0|prom1] - program the serial device strap"
);
