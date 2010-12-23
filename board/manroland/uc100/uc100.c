/*
 * (C) Copyright 2000-2004
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

#if 0
#define DEBUG
#endif

#include <common.h>
#include <mpc8xx.h>
#include <i2c.h>
#include <miiphy.h>

int fec8xx_miiphy_write(char *devname, unsigned char  addr,
		unsigned char  reg, unsigned short value);

/*********************************************************************/
/* UPMA Pre Initilization Table by WV (Miron MT48LC16M16A2-7E B)     */
/*********************************************************************/
const uint sdram_init_upm_table[] = {
	/* SDRAM Initialisation Sequence (offset 0 in UPMA RAM) WV */
	/* NOP    - Precharge - AutoRefr  - NOP       - NOP        */
	/* NOP    - AutoRefr  - NOP                                */
	/* NOP    - NOP       - LoadModeR - NOP       - Active     */
	/* Position of Single Read                                 */
	0x0ffffc04, 0x0ff77c04, 0x0ff5fc04, 0x0ffffc04, 0x0ffffc04,
	0x0ffffc04, 0x0ff5fc04, 0x0ffffc04,

	/* Burst Read. (offset 8 in UPMA RAM)     */
	/* Cycle lent for Initialisation WV */
	0x0ffffc04, 0x0ffffc34, 0x0f057c34, 0x0ffffc30, 0x1ff7fc05,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Single Write. (offset 18 in UPMA RAM) */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Burst Write. (offset 20 in UPMA RAM) */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Refresh  (offset 30 in UPMA RAM) */
	0x0FF77C04, 0x0FFFFC04, 0x0FF5FC84, 0x0FFFFC04, 0x0FFFFC04,
	0x0FFFFC84, 0x1FFFFC05, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,

	/* Exception. (offset 3c in UPMA RAM) */
	0x7FFFFC05, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};

/*********************************************************************/
/* UPMA initilization table.                                         */
/*********************************************************************/
const uint sdram_upm_table[] = {
	/* single read. (offset 0 in UPMA RAM) */
	0x0F07FC04, 0x0FFFFC04, 0x00BDFC04, 0x0FF77C00, 0x1FFFFC05,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,     /* 0x05-0x07 new WV */

	/* Burst Read. (offset 8 in UPMA RAM) */
	0x0F07FC04, 0x0FFFFC04, 0x00BDFC04, 0x00FFFC00, 0x00FFFC00,
	0x00FFFC00, 0x0FF77C00, 0x1FFFFC05, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Single Write. (offset 18 in UPMA RAM) */
	0x0F07FC04, 0x0FFFFC00, 0x00BD7C04, 0x0FFFFC04, 0x0FF77C04,
	0x1FFFFC05, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Burst Write. (offset 20 in UPMA RAM) */
	0x0F07FC04, 0x0FFFFC00, 0x00BD7C00, 0x00FFFC00, 0x00FFFC00,
	0x00FFFC04, 0x0FFFFC04, 0x0FF77C04, 0x1FFFFC05, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Refresh  (offset 30 in UPMA RAM) */
	0x0FF77C04, 0x0FFFFC04, 0x0FF5FC84, 0x0FFFFC04, 0x0FFFFC04,
	0x0FFFFC84, 0x1FFFFC05, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,

	/* Exception. (offset 3c in UPMA RAM) */
	0x7FFFFC05, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, /* 0x3C new WV */
};

/*********************************************************************/
/* UPMB initilization table.                                         */
/*********************************************************************/
const uint mpm_upm_table[] = {
	/*  single read. (offset 0 in upm RAM) */
	0x8FF00004, 0x0FF00004, 0x0FF81004, 0x1FF00001,
	0x1FF00001, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* burst read. (Offset 8 in upm RAM)   */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* single write. (Offset 0x18 in upm RAM) */
	0x8FF00004, 0x0FF00004, 0x0FF81004, 0x0FF00004,
	0x0FF00004, 0x1FF00001, 0xFFFFFFFF, 0xFFFFFFFF,

	/*  burst write. (Offset 0x20 in upm RAM) */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Refresh cycle, offset 0x30 */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* Exception, 0ffset 0x3C */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};


int board_switch(void)
{
	volatile pcmconf8xx_t	*pcmp;

	pcmp  = (pcmconf8xx_t *)(&(((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia));

	return ((pcmp->pcmc_pipr >> 24) & 0xf);
}


/*
 * Check Board Identity:
 */
int checkboard (void)
{
	char str[64];
	int i = getenv_f("serial#", str, sizeof(str));

	puts ("Board: ");

	if (i == -1) {
		puts ("### No HW ID - assuming UC100");
	} else {
		puts(str);
	}

	printf (" (SWITCH=%1X)\n", board_switch());

	return 0;
}


/*
 * Initialize SDRAM
 */
phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	/*---------------------------------------------------------------------*/
	/* Initialize the UPMA/UPMB registers with the appropriate table.      */
	/*---------------------------------------------------------------------*/
	upmconfig (UPMA, (uint *) sdram_init_upm_table,
		   sizeof (sdram_init_upm_table) / sizeof (uint));
	upmconfig (UPMB, (uint *) mpm_upm_table,
		   sizeof (mpm_upm_table) / sizeof (uint));

	/*---------------------------------------------------------------------*/
	/* Memory Periodic Timer Prescaler: divide by 16                       */
	/*---------------------------------------------------------------------*/
	memctl->memc_mptpr = 0x0200; /* Divide by 32 WV */

	memctl->memc_mamr = CONFIG_SYS_MAMR_VAL & 0xFF7FFFFF; /* Bit 8 := "0" Kein Refresh WV */
	memctl->memc_mbmr = CONFIG_SYS_MBMR_VAL;

	/*---------------------------------------------------------------------*/
	/* Initialize the Memory Controller registers, MPTPR, Chip Select 1    */
	/* for SDRAM                                                           */
	/*                                                                     */
	/* NOTE: The refresh rate in MAMR reg is set according to the lowest   */
	/*       clock rate (16.67MHz) to allow proper operation for all ADS   */
	/*       clock frequencies.                                            */
	/*---------------------------------------------------------------------*/
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;

	/*-------------------------------------------------------------------*/
	/* Wait at least 200 usec for DRAM to stabilize, this magic number   */
	/* obtained from the init code.                                      */
	/*-------------------------------------------------------------------*/
	udelay(200);

	memctl->memc_mamr = (memctl->memc_mamr | 0x04) & ~0x08;

	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;

	/*---------------------------------------------------------------------*/
	/* run MRS command in location 5-8 of UPMB.                            */
	/*---------------------------------------------------------------------*/
	memctl->memc_mar = 0x88;
	/* RUN UPMA on CS1 1-time from UPMA addr 0x05 */

	memctl->memc_mcr = 0x80002100;
	/* RUN UPMA on CS1 1-time from UPMA addr 0x00 WV */

	udelay(200);

	/*---------------------------------------------------------------------*/
	/* Initialisation for normal access WV                                 */
	/*---------------------------------------------------------------------*/

	/*---------------------------------------------------------------------*/
	/* Initialize the UPMA register with the appropriate table.            */
	/*---------------------------------------------------------------------*/
	upmconfig (UPMA, (uint *) sdram_upm_table,
		   sizeof (sdram_upm_table) / sizeof (uint));

	/*---------------------------------------------------------------------*/
	/* rerstore MBMR value (4-beat refresh burst.)                         */
	/*---------------------------------------------------------------------*/
	memctl->memc_mamr = CONFIG_SYS_MAMR_VAL | 0x00800000; /* Bit 8 := "1" Refresh Enable WV */

	udelay(200);

	return (64 * 1024 * 1024); /* fixed setup for 64MBytes! */
}


int misc_init_r (void)
{
	uchar val;

	/*
	 * Make sure that RTC has clock output enabled (triggers watchdog!)
	 */
	val = i2c_reg_read (CONFIG_SYS_I2C_RTC_ADDR, 0x0D);
	val |= 0x80;
	i2c_reg_write (CONFIG_SYS_I2C_RTC_ADDR, 0x0D, val);

	/*
	 * Configure PHY to setup LED's correctly and use 100MBit, FD
	 */
	mii_init();

	/* disable auto-negotiation, 100mbit, full-duplex */
	fec8xx_miiphy_write(NULL, 0, MII_BMCR, 0x2100);

	/* set LED's to Link, Transmit, Receive           */
	fec8xx_miiphy_write(NULL,  0, MII_NWAYTEST, 0x4122);

	return 0;
}
