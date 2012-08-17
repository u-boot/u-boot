/*
 *  Copyright (C) 2002 Scott McNutt <smcnutt@artesyncp.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>
#include <spd_sdram.h>

#define BOOT_SMALL_FLASH	32	/* 00100000 */
#define FLASH_ONBD_N		2	/* 00000010 */
#define FLASH_SRAM_SEL		1	/* 00000001 */

DECLARE_GLOBAL_DATA_PTR;

long int fixed_sdram(void);

int board_early_init_f(void)
{
	uint reg;
	unsigned char *fpga_base = (unsigned char *)CONFIG_SYS_FPGA_BASE;
	unsigned char status;

	/*--------------------------------------------------------------------
	 * Setup the external bus controller/chip selects
	 *-------------------------------------------------------------------*/
	mtdcr(EBC0_CFGADDR, EBC0_CFG);
	reg = mfdcr(EBC0_CFGDATA);
	mtdcr(EBC0_CFGDATA, reg | 0x04000000);	/* Set ATC */

	mtebc(PB1AP, 0x02815480);	/* NVRAM/RTC */
	mtebc(PB1CR, 0x48018000);	/* BA=0x480 1MB R/W 8-bit */
	mtebc(PB7AP, 0x01015280);	/* FPGA registers */
	mtebc(PB7CR, 0x48318000);	/* BA=0x483 1MB R/W 8-bit */

	/* read FPGA_REG0  and set the bus controller */
	status = *fpga_base;
	if ((status & BOOT_SMALL_FLASH) && !(status & FLASH_ONBD_N)) {
		mtebc(PB0AP, 0x9b015480);	/* FLASH/SRAM */
		mtebc(PB0CR, 0xfff18000);	/* BAS=0xfff 1MB R/W 8-bit */
		mtebc(PB2AP, 0x9b015480);	/* 4MB FLASH */
		mtebc(PB2CR, 0xff858000);	/* BAS=0xff8 4MB R/W 8-bit */
	} else {
		mtebc(PB0AP, 0x9b015480);	/* 4MB FLASH */
		mtebc(PB0CR, 0xffc58000);	/* BAS=0xffc 4MB R/W 8-bit */

		/* set CS2 if FLASH_ONBD_N == 0 */
		if (!(status & FLASH_ONBD_N)) {
			mtebc(PB2AP, 0x9b015480);	/* FLASH/SRAM */
			mtebc(PB2CR, 0xff818000);	/* BAS=0xff8 4MB R/W 8-bit */
		}
	}

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */
	mtdcr(UIC0ER, 0x00000000);	/* disable all */
	mtdcr(UIC0CR, 0x00000009);	/* SMI & UIC1 crit are critical */
	mtdcr(UIC0PR, 0xfffffe13);	/* per ref-board manual */
	mtdcr(UIC0TR, 0x01c00008);	/* per ref-board manual */
	mtdcr(UIC0VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */

	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xffffe0ff);	/* per ref-board manual */
	mtdcr(UIC1TR, 0x00ffc000);	/* per ref-board manual */
	mtdcr(UIC1VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	return 0;
}

int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: Ebony - AMCC PPC440GP Evaluation Board");
	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return (0);
}

phys_size_t initdram(int board_type)
{
	long dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram();
#else
	dram_size = fixed_sdram();
#endif
	return dram_size;
}

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 *
 *  Assumes:    128 MB, non-ECC, non-registered
 *              PLB @ 133 MHz
 *
 ************************************************************************/
long int fixed_sdram(void)
{
	uint reg;

	/*--------------------------------------------------------------------
	 * Setup some default
	 *------------------------------------------------------------------*/
	mtsdram(SDRAM0_UABBA, 0x00000000);	/* ubba=0 (default)             */
	mtsdram(SDRAM0_SLIO, 0x00000000);	/* rdre=0 wrre=0 rarw=0         */
	mtsdram(SDRAM0_DEVOPT, 0x00000000);	/* dll=0 ds=0 (normal)          */
	mtsdram(SDRAM0_WDDCTR, 0x00000000);	/* wrcp=0 dcd=0                 */
	mtsdram(SDRAM0_CLKTR, 0x40000000);	/* clkp=1 (90 deg wr) dcdt=0    */

	/*--------------------------------------------------------------------
	 * Setup for board-specific specific mem
	 *------------------------------------------------------------------*/
	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram(SDRAM0_B0CR, 0x000a4001);	/* SDBA=0x000 128MB, Mode 3, enabled */
	mtsdram(SDRAM0_TR0, 0x410a4012);	/* WR=2  WD=1 CL=2.5 PA=3 CP=4 LD=2 */
	/* RA=10 RD=3                       */
	mtsdram(SDRAM0_TR1, 0x8080082f);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x02f   */
	mtsdram(SDRAM0_RTR, 0x08200000);	/* Rate 15.625 ns @ 133 MHz PLB     */
	mtsdram(SDRAM0_CFG1, 0x00000000);	/* Self-refresh exit, disable PM    */
	udelay(400);		/* Delay 200 usecs (min)            */

	/*--------------------------------------------------------------------
	 * Enable the controller, then wait for DCEN to complete
	 *------------------------------------------------------------------*/
	mtsdram(SDRAM0_CFG0, 0x86000000);	/* DCEN=1, PMUD=1, 64-bit           */
	for (;;) {
		mfsdram(SDRAM0_MCSTS, reg);
		if (reg & 0x80000000)
			break;
	}

	return (128 * 1024 * 1024);	/* 128 MB                           */
}
#endif				/* !defined(CONFIG_SPD_EEPROM) */
