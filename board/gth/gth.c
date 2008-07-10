/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Adapted from FADS and other board config files to GTH by thomas@corelatus.com
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
#include <config.h>
#include <watchdog.h>
#include <mpc8xx.h>
#include "ee_access.h"
#include "ee_dev.h"

#ifdef CONFIG_BDM
#undef printf
#define printf(a,...)			/* nothing */
#endif


int checkboard (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	int Id = 0;
	int Rev = 0;
	u32 Pbdat;

	puts ("Board: ");

	/* Turn on leds and setup for reading rev and id */

#define PB_OUTS (PB_BLUE_LED|PB_ID_GND)
#define PB_INS  (PB_ID_0|PB_ID_1|PB_ID_2|PB_ID_3|PB_REV_1|PB_REV_0)

	immap->im_cpm.cp_pbpar &= ~(PB_OUTS | PB_INS);

	immap->im_cpm.cp_pbdir &= ~PB_INS;

	immap->im_cpm.cp_pbdir |= PB_OUTS;
	immap->im_cpm.cp_pbodr |= PB_OUTS;
	immap->im_cpm.cp_pbdat &= ~PB_OUTS;

	/* Hold 100 Mbit in reset until fpga is loaded */
	immap->im_ioport.iop_pcpar &= ~PC_ENET100_RESET;
	immap->im_ioport.iop_pcdir |= PC_ENET100_RESET;
	immap->im_ioport.iop_pcso &= ~PC_ENET100_RESET;
	immap->im_ioport.iop_pcdat &= ~PC_ENET100_RESET;

	/* Turn on front led to show that we are alive */
	immap->im_ioport.iop_papar &= ~PA_FRONT_LED;
	immap->im_ioport.iop_padir |= PA_FRONT_LED;
	immap->im_ioport.iop_paodr |= PA_FRONT_LED;
	immap->im_ioport.iop_padat &= ~PA_FRONT_LED;

	Pbdat = immap->im_cpm.cp_pbdat;

	if (!(Pbdat & PB_ID_0))
		Id += 1;
	if (!(Pbdat & PB_ID_1))
		Id += 2;
	if (!(Pbdat & PB_ID_2))
		Id += 4;
	if (!(Pbdat & PB_ID_3))
		Id += 8;

	if (Pbdat & PB_REV_0)
		Rev += 1;
	if (Pbdat & PB_REV_1)
		Rev += 2;

	/* Turn ID off since we dont need it anymore */
	immap->im_cpm.cp_pbdat |= PB_ID_GND;

	printf ("GTH board, rev %d, id=0x%01x\n", Rev, Id);
	return 0;
}

#define _NOT_USED_ 0xffffffff
const uint sdram_table[] = {
	/* Single read, offset 0 */
	0x0f3dfc04, 0x0eefbc04, 0x01bf7c04, 0x0feafc00,
	0x1fb5fc45, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* Burst read, Offset 0x8, 4 reads */
	0x0f3dfc04, 0x0eefbc04, 0x00bf7c04, 0x00ffec00,
	0x00fffc00, 0x01eafc00, 0x1fb5fc00, 0xfffffc45,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* Not used part of burst read is used for MRS, Offset 0x14 */
	0xefeabc34, 0x1fb57c34, 0xfffffc05, _NOT_USED_,
	/* _NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_, */

	/* Single write, Offset 0x18 */
	0x0f3dfc04, 0x0eebbc00, 0x01a27c04, 0x1fb5fc45,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* Burst write, Offset 0x20. 4 writes */
	0x0f3dfc04, 0x0eebbc00, 0x00b77c00, 0x00fffc00,
	0x00fffc00, 0x01eafc04, 0x1fb5fc45, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* Not used part of burst write is used for precharge, Offset 0x2C */
	0x0ff5fc04, 0xfffffc05, _NOT_USED_, _NOT_USED_,
	/* _NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_, */

	/* Period timer service. Offset 0x30. Refresh. Wait at least 70 ns after refresh command */
	0x1ffd7c04, 0xfffffc04, 0xfffffc04, 0xfffffc05,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* Exception, Offset 0x3C */
	0xfffffc04, 0xfffffc05, _NOT_USED_, _NOT_USED_
};

const uint fpga_table[] = {
	/* Single read, offset 0 */
	0x0cffec04, 0x00ffec04, 0x00ffec04, 0x00ffec04,
	0x00fffc04, 0x00fffc00, 0x00ffec04, 0xffffec05,

	/* Burst read, Offset 0x8 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* Single write, Offset 0x18 */
	0x0cffec04, 0x00ffec04, 0x00ffec04, 0x00ffec04,
	0x00fffc04, 0x00fffc00, 0x00ffec04, 0xffffec05,

	/* Burst write, Offset 0x20. */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* Period timer service. Offset 0x30. */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* Exception, Offset 0x3C */
	0xfffffc04, 0xfffffc05, _NOT_USED_, _NOT_USED_
};

int _initsdram (uint base, uint * noMbytes)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *mc = &immap->im_memctl;
	volatile u32 *memptr;

	mc->memc_mptpr = MPTPR_PTP_DIV16;	/* (16-17) */

	/*  SDRAM in UPMA

	   GPL_0 is connected instead of A19 to SDRAM.
	   According to table 16-17, AMx should be 001, i.e. type 1
	   and GPL_0 should hold address A10 when multiplexing */

	mc->memc_mamr = (0x2E << MAMR_PTA_SHIFT) | MAMR_PTAE | MAMR_AMA_TYPE_1 | MAMR_G0CLA_A10 | MAMR_RLFA_1X | MAMR_WLFA_1X | MAMR_TLFA_1X;	/* (16-13) */

	upmconfig (UPMA, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/* Perform init of sdram ( Datasheet Page 9 )
	   Precharge */
	mc->memc_mcr = 0x8000212C;	/* run upm a at 0x2C (16-15) */

	/* Run 2 refresh cycles */
	mc->memc_mcr = 0x80002130;	/* run upm a at 0x30 (16-15) */
	mc->memc_mcr = 0x80002130;	/* run upm a at 0x30 (16-15) */

	/* Set Mode register */
	mc->memc_mar = 0x00000088;	/* set mode register (address) to 0x022 (16-17) */
	/* Lower 2 bits are not connected to chip */
	mc->memc_mcr = 0x80002114;	/* run upm a at 0x14 (16-15) */

	/* CS1, base 0x0000000 - 64 Mbyte, use UPM A */
	mc->memc_or1 = 0xfc000000 | OR_CSNT_SAM;
	mc->memc_br1 = BR_MS_UPMA | BR_V;	/* SDRAM base always 0 */

	/* Test if we really have 64 MB SDRAM */
	memptr = (u32 *) 0;
	*memptr = 0;

	memptr = (u32 *) 0x2000000;	/* First u32 in upper 32 MB */
	*memptr = 0x12345678;

	memptr = (u32 *) 0;
	if (*memptr == 0x12345678) {
		/* Wrapped, only have 32 MB */
		mc->memc_or1 = 0xfe000000 | OR_CSNT_SAM;
		*noMbytes = 32;
	} else {
		/* 64 MB */
		*noMbytes = 64;
	}

	/* Setup FPGA in UPMB */
	upmconfig (UPMB, (uint *) fpga_table,
			   sizeof (fpga_table) / sizeof (uint));

	/* Enable UPWAITB */
	mc->memc_mbmr = MBMR_GPL_B4DIS;	/* (16-13) */

	/* CS2, base FPGA_2_BASE - 4 MByte, use UPM B 32 Bit */
	mc->memc_or2 = 0xffc00000 | OR_BI;
	mc->memc_br2 = FPGA_2_BASE | BR_MS_UPMB | BR_V;

	/* CS3, base FPGA_3_BASE - 4 MByte, use UPM B 16 bit */
	mc->memc_or3 = 0xffc00000 | OR_BI;
	mc->memc_br3 = FPGA_3_BASE | BR_MS_UPMB | BR_V | BR_PS_16;

	return 0;
}

/* ------------------------------------------------------------------------- */

void _sdramdisable (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_br1 = 0x00000000;

	/* maybe we should turn off upmb here or something */
}

/* ------------------------------------------------------------------------- */

int initsdram (uint base, uint * noMbytes)
{
	*noMbytes = 32;

#ifdef CONFIG_START_IN_RAM
	/* SDRAM is already setup. Dont touch it */
	return 0;
#else

	if (!_initsdram (base, noMbytes)) {

		return 0;
	} else {
		_sdramdisable ();

		return -1;
	}
#endif
}

phys_size_t initdram (int board_type)
{
	u32 *i;
	u32 j;
	u32 k;

	/* GTH only have SDRAM */
	uint sdramsz;

	if (!initsdram (0x00000000, &sdramsz)) {
		printf ("(%u MB SDRAM) ", sdramsz);
	} else {
	/********************************
     *SDRAM ERROR, HALT PROCESSOR
     *********************************/
		printf ("SDRAM ERROR\n");
		while (1);
	}

#ifndef CONFIG_START_IN_RAM

#define U32_S ((sdramsz<<18)-1)

#if 1
	/* Do a simple memory test */
	for (i = (u32 *) 0, j = 0; (u32) i < U32_S; i += 2, j += 2) {
		*i = j + (j << 17);
		*(i + 1) = ~(j + (j << 18));
	}

	WATCHDOG_RESET ();

	printf (".");

	for (i = (u32 *) 0, j = 0; (u32) i < U32_S; i += 2, j += 2) {
		k = *i;
		if (k != (j + (j << 17))) {
			printf ("Mem test error, i=0x%x, 0x%x\n, 0x%x", (u32) i, j, k);
			while (1);
		}
		k = *(i + 1);
		if (k != ~(j + (j << 18))) {
			printf ("Mem test error(+1), i=0x%x, 0x%x\n, 0x%x",
					(u32) i + 1, j, k);
			while (1);
		}
	}
#endif

	WATCHDOG_RESET ();

	/* Clear memory */
	for (i = (u32 *) 0; (u32) i < U32_S; i++) {
		*i = 0;
	}
#endif /* !start in ram */

	WATCHDOG_RESET ();

	return (sdramsz << 20);
}

#define POWER_OFFSET    0xF0000
#define SW_WATCHDOG_REASON 13

#define BOOTDATA_OFFSET 0xF8000
#define MAX_ATTEMPTS 5

#define FAILSAFE_BOOT 1
#define SYSTEM_BOOT   2

#define WRITE_FLASH16(a, d)      \
do                              \
{                               \
  *((volatile u16 *) (a)) = (d);\
 } while(0)

static void write_bootdata (volatile u16 * addr, u8 System, u8 Count)
{
	u16 data;
	volatile u16 *flash = (u16 *) (CFG_FLASH_BASE);

	if ((System != FAILSAFE_BOOT) & (System != SYSTEM_BOOT)) {
		printf ("Invalid system data %u, setting failsafe\n", System);
		System = FAILSAFE_BOOT;
	}

	if ((Count < 1) | (Count > MAX_ATTEMPTS)) {
		printf ("Invalid boot count %u, setting 1\n", Count);
		Count = 1;
	}

	if (System == FAILSAFE_BOOT) {
		printf ("Setting failsafe boot in flash\n");
	} else {
		printf ("Setting system boot in flash\n");
	}
	printf ("Boot attempt %d\n", Count);

	data = (System << 8) | Count;
	/* AMD 16 bit */
	WRITE_FLASH16 (&flash[0x555], 0xAAAA);
	WRITE_FLASH16 (&flash[0x2AA], 0x5555);
	WRITE_FLASH16 (&flash[0x555], 0xA0A0);

	WRITE_FLASH16 (addr, data);
}

static void maybe_update_restart_reason (volatile u32 * addr32)
{
	/* Update addr if sw wd restart */
	volatile u16 *flash = (u16 *) (CFG_FLASH_BASE);
	volatile u16 *addr_16 = (u16 *) addr32;
	u32 rsr;

	/* Dont reset register now */
	rsr = ((volatile immap_t *) CFG_IMMR)->im_clkrst.car_rsr;

	rsr >>= 24;

	if (rsr & 0x10) {
		/* Was really a sw wd restart, update reason */

		printf ("Last restart by software watchdog\n");

		/* AMD 16 bit */
		WRITE_FLASH16 (&flash[0x555], 0xAAAA);
		WRITE_FLASH16 (&flash[0x2AA], 0x5555);
		WRITE_FLASH16 (&flash[0x555], 0xA0A0);

		WRITE_FLASH16 (addr_16, 0);

		udelay (1000);

		WATCHDOG_RESET ();

		/* AMD 16 bit */
		WRITE_FLASH16 (&flash[0x555], 0xAAAA);
		WRITE_FLASH16 (&flash[0x2AA], 0x5555);
		WRITE_FLASH16 (&flash[0x555], 0xA0A0);

		WRITE_FLASH16 (addr_16 + 1, SW_WATCHDOG_REASON);

	}
}

static void check_restart_reason (void)
{
	/* Update restart reason if sw watchdog was
	   triggered */

	int i;
	volatile u32 *raddr;

	raddr = (u32 *) (CFG_FLASH_BASE + POWER_OFFSET);

	if (*raddr == 0xFFFFFFFF) {
		/* Nothing written */
		maybe_update_restart_reason (raddr);
	} else {
		/* Search for latest written reason */
		i = 0;
		while ((*(raddr + 2) != 0xFFFFFFFF) & (i < 2000)) {
			raddr += 2;
			i++;
		}
		if (i >= 2000) {
			/* Whoa, dont write any more */
			printf ("*** No free restart reason found ***\n");
		} else {
			/* Check if written */
			if (*raddr == 0) {
				/* Erased by kernel, no new reason written */
				maybe_update_restart_reason (raddr + 2);
			}
		}
	}
}

static void check_boot_tries (void)
{
	/* Count the number of boot attemps
	   switch system if too many */

	int i;
	volatile u16 *addr;
	volatile u16 data;
	int failsafe = 1;
	u8 system;
	u8 count;

	addr = (u16 *) (CFG_FLASH_BASE + BOOTDATA_OFFSET);

	if (*addr == 0xFFFF) {
		printf ("*** No bootdata exists. ***\n");
		write_bootdata (addr, FAILSAFE_BOOT, 1);
	} else {
		/* Search for latest written bootdata */
		i = 0;
		while ((*(addr + 1) != 0xFFFF) & (i < 8000)) {
			addr++;
			i++;
		}
		if (i >= 8000) {
			/* Whoa, dont write any more */
			printf ("*** No bootdata found. Not updating flash***\n");
		} else {
			/* See how many times we have tried to boot real system */
			data = *addr;
			system = data >> 8;
			count = data & 0xFF;
			if ((system != SYSTEM_BOOT) & (system != FAILSAFE_BOOT)) {
				printf ("*** Wrong system %d\n", system);
				system = FAILSAFE_BOOT;
				count = 1;
			} else {
				switch (count) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
					/* Try same system again if needed */
					count++;
					break;

				case 5:
					/* Switch system and reset tries */
					count = 1;
					system = 3 - system;
					printf ("***Too many boot attempts, switching system***\n");
					break;
				default:
					/* Switch system, start over and hope it works */
					printf ("***Unexpected data on addr 0x%x, %u***\n",
							(u32) addr, data);
					count = 1;
					system = 3 - system;
				}
			}
			write_bootdata (addr + 1, system, count);
			if (system == SYSTEM_BOOT) {
				failsafe = 0;
			}
		}
	}
	if (failsafe) {
		printf ("Booting failsafe system\n");
		setenv ("bootargs", "panic=1 root=/dev/hda7");
		setenv ("bootcmd", "disk 100000 0:5;bootm 100000");
	} else {
		printf ("Using normal system\n");
		setenv ("bootargs", "panic=1 root=/dev/hda4");
		setenv ("bootcmd", "disk 100000 0:2;bootm 100000");
	}
}

int misc_init_r (void)
{
	u8 Rx[80];
	u8 Tx[5];
	int page;
	int read = 0;
	volatile immap_t *immap = (immap_t *) CFG_IMMR;

	/* Kill fpga */
	immap->im_ioport.iop_papar &= ~(PA_FL_CONFIG | PA_FL_CE);
	immap->im_ioport.iop_padir |= (PA_FL_CONFIG | PA_FL_CE);
	immap->im_ioport.iop_paodr &= ~(PA_FL_CONFIG | PA_FL_CE);

	/* Enable fpga, active low */
	immap->im_ioport.iop_padat &= ~PA_FL_CE;

	/* Start configuration */
	immap->im_ioport.iop_padat &= ~PA_FL_CONFIG;
	udelay (2);

	immap->im_ioport.iop_padat |= (PA_FL_CONFIG | PA_FL_CE);

	/* Check if we need to boot failsafe system */
	check_boot_tries ();

	/* Check if we need to update restart reason */
	check_restart_reason ();

	if (ee_init_data ()) {
		printf ("EEPROM init failed\n");
		return (0);
	}

	/* Read the pages where ethernet address is stored */

	for (page = EE_USER_PAGE_0; page <= EE_USER_PAGE_0 + 2; page++) {
		/* Copy from nvram to scratchpad */
		Tx[0] = RECALL_MEMORY;
		Tx[1] = page;
		if (ee_do_command (Tx, 2, NULL, 0, TRUE)) {
			printf ("EE user page %d recall failed\n", page);
			return (0);
		}

		Tx[0] = READ_SCRATCHPAD;
		if (ee_do_command (Tx, 2, Rx + read, 9, TRUE)) {
			printf ("EE user page %d read failed\n", page);
			return (0);
		}
		/* Crc in 9:th byte */
		if (!ee_crc_ok (Rx + read, 8, *(Rx + read + 8))) {
			printf ("EE read failed, page %d. CRC error\n", page);
			return (0);
		}
		read += 8;
	}

	/* Add eos after eth addr */
	Rx[17] = 0;

	printf ("Ethernet addr read from eeprom: %s\n\n", Rx);

	if ((Rx[2] != ':') |
		(Rx[5] != ':') |
		(Rx[8] != ':') | (Rx[11] != ':') | (Rx[14] != ':')) {
		printf ("*** ethernet addr invalid, using default ***\n");
	} else {
		setenv ("ethaddr", (char *)Rx);
	}
	return (0);
}
