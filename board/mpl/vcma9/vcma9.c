/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
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
#include <s3c2410.h>
#include <i2c.h>

#include "vcma9.h"
#include "../common/common_util.h"

/* ------------------------------------------------------------------------- */

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

static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	rLOCKTIME = 0xFFFFFF;

	/* configure MPLL */
	rMPLLCON = ((M_MDIV << 12) + (M_PDIV << 4) + M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (4000);

	/* configure UPLL */
	rUPLLCON = ((U_M_MDIV << 12) + (U_M_PDIV << 4) + U_M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (8000);

	/* set up the I/O ports */
	rGPACON = 0x007FFFFF;
	rGPBCON = 0x002AAAAA;
	rGPBUP = 0x000002BF;
	rGPCCON = 0xAAAAAAAA;
	rGPCUP = 0x0000FFFF;
	rGPDCON = 0xAAAAAAAA;
	rGPDUP = 0x0000FFFF;
	rGPECON = 0xAAAAAAAA;
	rGPEUP = 0x000037F7;
	rGPFCON = 0x00000000;
	rGPFUP = 0x00000000;
	rGPGCON = 0xFFEAFF5A;
	rGPGUP = 0x0000F0DC;
	rGPHCON = 0x0028AAAA;
	rGPHUP = 0x00000656;

	/* setup correct IRQ modes for NIC */
	rEXTINT2 = (rEXTINT2 & ~(7<<8)) | (4<<8); /* rising edge mode */

	/* init serial */
	gd->baudrate = CONFIG_BAUDRATE;
	gd->have_console = 1;
	serial_init();

	/* arch number of VCMA9-Board */
	gd->bd->bi_arch_number = 227;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

/*
 * Get some Board/PLD Info
 */

static uchar Get_PLD_ID(void)
{
	return(*(volatile uchar *)PLD_ID_REG);
}

static uchar Get_PLD_BOARD(void)
{
	return(*(volatile uchar *)PLD_BOARD_REG);
}

static uchar Get_PLD_Version(void)
{
	return((Get_PLD_ID() >> 4) & 0x0F);
}

static uchar Get_PLD_Revision(void)
{
	return(Get_PLD_ID() & 0x0F);
}

static int Get_Board_Config(void)
{
	uchar config = Get_PLD_BOARD() & 0x03;

	if (config == 3)
	    return 1;
	else
	    return 0;
}

static uchar Get_Board_PCB(void)
{
	return(((Get_PLD_BOARD() >> 4) & 0x03) + 'A');
}

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard(void)
{
	unsigned char s[50];
	int i;
	backup_t *b = (backup_t *) s;

	puts("Board: ");

	i = getenv_r("serial#", s, 32);
	if ((i < 0) || strncmp (s, "VCMA9", 5)) {
		get_backup_values (b);
		if (strncmp (b->signature, "MPL\0", 4) != 0) {
			puts ("### No HW ID - assuming VCMA9");
		} else {
			b->serial_name[5] = 0;
			printf ("%s-%d Rev %c SN: %s", b->serial_name, Get_Board_Config(),
					Get_Board_PCB(), &b->serial_name[6]);
		}
	} else {
		s[5] = 0;
		printf ("%s-%d Rev %c SN: %s", s, Get_Board_Config(), Get_Board_PCB(),
				&s[6]);
	}
	printf("\n");
	return(0);
}



void print_vcma9_rev(void)
{
	printf("Board: VCMA9-%d Rev: %c (PLD Ver: %d, Rev: %d)\n",
		Get_Board_Config(), Get_Board_PCB(),
		Get_PLD_Version(), Get_PLD_Revision());
}


int last_stage_init(void)
{
	print_vcma9_rev();
	show_stdio_dev();
	check_env();
	return 0;
}

/***************************************************************************
 * some helping routines
 */

int overwrite_console(void)
{
	/* return TRUE if console should be overwritten */
	return 0;
}


/************************************************************************
* Print VCMA9 Info
************************************************************************/
void print_vcma9_info(void)
{
    print_vcma9_rev();
}


