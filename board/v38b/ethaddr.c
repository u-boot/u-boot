/*
 *
 * (C) Copyright 2006
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
 */

#include <common.h>
#include <mpc5xxx.h>

#define GPIO_ENABLE	(MPC5XXX_WU_GPIO)

/* Open Drain Emulation Register */
#define GPIO_ODR	(MPC5XXX_WU_GPIO + 0x04)

/* Data Direction Register */
#define GPIO_DDR	(MPC5XXX_WU_GPIO + 0x08)

/* Data Value Out Register */
#define GPIO_DVOR	(MPC5XXX_WU_GPIO + 0x0C)

/* Interrupt Enable Register */
#define GPIO_IER	(MPC5XXX_WU_GPIO + 0x10)

/* Individual Interrupt Enable Register */
#define GPIO_IIER	(MPC5XXX_WU_GPIO + 0x14)

/* Interrupt Type Register */
#define GPIO_ITR	(MPC5XXX_WU_GPIO + 0x18)

/* Master Enable Register */
#define GPIO_MER	(MPC5XXX_WU_GPIO + 0x1C)

/* Data Input Value Register */
#define GPIO_DIVR	(MPC5XXX_WU_GPIO + 0x20)

/* Status Register */
#define GPIO_SR		(MPC5XXX_WU_GPIO + 0x24)

#define PSC6_0		0x10000000
#define WKUP_7		0x80000000

/* For NS4 A/B board define WKUP_7, for V38B board PSC_6 */
#define GPIO_PIN	PSC6_0

#define NO_ERROR	0
#define ERR_NO_NUMBER	1
#define ERR_BAD_NUMBER	2

typedef volatile unsigned long GPIO_REG;
typedef GPIO_REG *GPIO_REG_PTR;

static int is_high(void);
static int check_device(void);
static void io_out(int value);
static void io_input(void);
static void io_output(void);
static void init_gpio(void);
static void read_byte(unsigned char *data);
static void write_byte(unsigned char command);

void read_2501_memory(unsigned char *psernum, unsigned char *perr);
void board_get_enetaddr(uchar *enetaddr);

static int is_high()
{
	return (* ((vu_long *) GPIO_DIVR) & GPIO_PIN);
}

static void io_out(int value)
{
	if (value)
		*((vu_long *) GPIO_DVOR) |= GPIO_PIN;
	else
		*((vu_long *) GPIO_DVOR) &= ~GPIO_PIN;
}

static void io_input()
{
	*((vu_long *) GPIO_DDR) &= ~GPIO_PIN;
	udelay(3);	/* allow input to settle */
}

static void io_output()
{
	*((vu_long *) GPIO_DDR) |= GPIO_PIN;
}

static void init_gpio()
{
	*((vu_long *) GPIO_ENABLE) |= GPIO_PIN;	/* Enable appropriate pin */
}

void read_2501_memory(unsigned char *psernum, unsigned char *perr)
{
#define NBYTES 28
	unsigned char crcval, i;
	unsigned char buf[NBYTES];

	*perr = 0;
	crcval = 0;

	for (i=0; i<NBYTES; i++)


	if (!check_device())
		*perr = ERR_NO_NUMBER;
	else {
		*perr = NO_ERROR;
		write_byte(0xCC);		/* skip ROM (0xCC) */
		write_byte(0xF0);		/* Read memory command 0xF0 */
		write_byte(0x00);		/* Address TA1=0, TA2=0 */
		write_byte(0x00);
		read_byte(&crcval);		/* Read CRC of address and command */

		for (i=0; i<NBYTES; i++)
			read_byte( &buf[i] );
	}
	if (strncmp((const char*) &buf[11], "MAREL IEEE 802.3", 16)) {
		*perr = ERR_BAD_NUMBER;
		psernum[0] = 0x00;
		psernum[1] = 0xE0;
		psernum[2] = 0xEE;
		psernum[3] = 0xFF;
		psernum[4] = 0xFF;
		psernum[5] = 0xFF;
	}
	else {
		psernum[0] = 0x00;
		psernum[1] = 0xE0;
		psernum[2] = 0xEE;
		psernum[3] = buf[7];
		psernum[4] = buf[6];
		psernum[5] = buf[5];
	}
}

static int check_device()
{
	int found;

	io_output();
	io_out(0);
	udelay(500);  /* must be at least 480 us low pulse */

	io_input();
	udelay(60);

	found = (is_high() == 0) ? 1 : 0;
	udelay(500);  /* must be at least 480 us low pulse */

	return found;
}

static void write_byte(unsigned char command)
{
	char i;

	for (i=0; i<8; i++) {
		/* 1 us to 15 us low pulse starts bit slot */
		/* Start with high pulse for 3 us */
		io_input();

		udelay(3);

		io_out(0);
		io_output();

		udelay(3);

		if (command & 0x01) {
			/* 60 us high for 1-bit */
			io_input();
			udelay(60);
		}
		else {
			/* 60 us low for 0-bit */
			udelay(60);
		}
		/*  Leave pin as input */
		io_input();

		command = command >> 1;
	}
}

static void read_byte(unsigned char  *data)
{
	unsigned char i, rdat = 0;

	for (i=0; i<8; i++) {
		/* read one bit from one-wire device */

		/* 1 - 15 us low starts bit slot */
		io_out(0);
		io_output();
		udelay(0);

		/* allow line to be pulled high */
		io_input();

		/* delay 10 us */
		udelay(10);

		/* now sample input status */
		if (is_high())
			rdat = (rdat >> 1) | 0x80;
		else
			rdat = rdat >> 1;

		udelay(60);	/* at least 60 us */
	}
	/* copy the return value */
	*data = rdat;
}

void board_get_enetaddr(uchar *enetaddr)
{
	unsigned char sn[6], err=NO_ERROR;

	init_gpio();

	read_2501_memory(sn, &err);

	if (err == NO_ERROR) {
		sprintf(enetaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
				sn[0], sn[1], sn[2], sn[3], sn[4], sn[5]);
		printf("MAC address: %s\n", enetaddr);
		setenv("ethaddr", enetaddr);
	}
	else {
		sprintf(enetaddr, "00:01:02:03:04:05");
		printf("Error reading MAC address.\n");
		printf("Setting default to %s\n", enetaddr);
		setenv("ethaddr", enetaddr);
	}
}
