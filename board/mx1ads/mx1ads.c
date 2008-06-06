/*
 * board/mx1ads/mx1ads.c
 *
 * (c) Copyright 2004
 * Techware Information Technology, Inc.
 * http://www.techware.com.tw/
 *
 * Ming-Len Wu <minglen_wu@techware.com.tw>
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
/*#include <mc9328.h>*/
#include <asm/arch/imx-regs.h>

DECLARE_GLOBAL_DATA_PTR;

#define FCLK_SPEED 1

#if FCLK_SPEED==0		/* Fout = 203MHz, Fin = 12MHz for Audio */
#define M_MDIV	0xC3
#define M_PDIV	0x4
#define M_SDIV	0x1
#elif FCLK_SPEED==1		/* Fout = 202.8MHz */
#define M_MDIV	0xA1
#define M_PDIV	0x3
#define M_SDIV	0x1
#endif

#define USB_CLOCK 1

#if USB_CLOCK==0
#define U_M_MDIV	0xA1
#define U_M_PDIV	0x3
#define U_M_SDIV	0x1
#elif USB_CLOCK==1
#define U_M_MDIV	0x48
#define U_M_PDIV	0x3
#define U_M_SDIV	0x2
#endif

#if 0

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
			  "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0" (loops));
}

#endif

/*
 * Miscellaneous platform dependent initialisations
 */

void SetAsynchMode (void)
{
	__asm__ ("mrc p15,0,r0,c1,c0,0 \n"
		 "mov r2, #0xC0000000 \n"
		 "orr r0,r2,r0 \n" "mcr p15,0,r0,c1,c0,0 \n");
}

static u32 mc9328sid;

int board_init (void)
{
	volatile unsigned int tmp;

	mc9328sid = SIDR;

	GPCR = 0x000003AB;	/* I/O pad driving strength     */

	/*	MX1_CS1U	= 0x00000A00;	*/ /* SRAM initialization          */
/*	MX1_CS1L	= 0x11110601;	*/

	MPCTL0 = 0x04632410;	/* setting for 150 MHz MCU PLL CLK      */

/* set FCLK divider 1 (i.e. FCLK to MCU PLL CLK) and
 * BCLK divider to 2 (i.e. BCLK to 48 MHz)
 */
	CSCR = 0xAF000403;

	CSCR |= 0x00200000;	/* Trigger the restart bit(bit 21)      */
	CSCR &= 0xFFFF7FFF;	/* Program PRESC bit(bit 15) to 0 to divide-by-1 */

/* setup cs4 for cs8900 ethernet */

	CS4U = 0x00000F00;	/* Initialize CS4 for CS8900 ethernet   */
	CS4L = 0x00001501;

	GIUS (0) &= 0xFF3FFFFF;
	GPR (0) &= 0xFF3FFFFF;

	tmp = *(unsigned int *) (0x1500000C);
	tmp = *(unsigned int *) (0x1500000C);

	SetAsynchMode ();

	gd->bd->bi_arch_number = MACH_TYPE_MX1ADS;

	gd->bd->bi_boot_params = 0x08000100;	/* adress of boot parameters    */

	icache_enable ();
	dcache_enable ();

/* set PERCLKs				*/
	PCDR = 0x00000055;	/* set PERCLKS                          */

/* PERCLK3 is only used by SSI so the SSI driver can set it any value it likes
 * PERCLK1 and PERCLK2 are shared so DO NOT change it in any other place
 * all sources selected as normal interrupt
 */

/*	MX1_INTTYPEH = 0;
	MX1_INTTYPEL = 0;
*/
	return 0;
}

int board_late_init (void)
{

	setenv ("stdout", "serial");
	setenv ("stderr", "serial");

	switch (mc9328sid) {
	case 0x0005901d:
		printf ("MX1ADS board with MC9328 MX1 (0L44N), Silicon ID 0x%08x \n\n",
			mc9328sid);
		break;
	case 0x04d4c01d:
		printf ("MX1ADS board with MC9328 MXL (1L45N), Silicon ID 0x%08x \n\n",
			mc9328sid);
		break;
	case 0x00d4c01d:
		printf ("MX1ADS board with MC9328 MXL (2L45N), Silicon ID 0x%08x \n\n",
			mc9328sid);
		break;

	default:
		printf ("MX1ADS board with UNKNOWN MC9328 cpu, Silicon ID 0x%08x \n",
			mc9328sid);
		break;
	}
	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}
