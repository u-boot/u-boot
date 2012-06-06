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
#include <netdev.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <stdio_dev.h>
#include <i2c.h>

#include "vcma9.h"
#include "../common/common_util.h"

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
	struct s3c24x0_clock_power * const clk_power =
					s3c24x0_get_base_clock_power();
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	clk_power->locktime = 0xFFFFFF;

	/* configure MPLL */
	clk_power->mpllcon = ((M_MDIV << 12) + (M_PDIV << 4) + M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (4000);

	/* configure UPLL */
	clk_power->upllcon = ((U_M_MDIV << 12) + (U_M_PDIV << 4) + U_M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (8000);

	/* set up the I/O ports */
	gpio->gpacon = 0x007FFFFF;
	gpio->gpbcon = 0x002AAAAA;
	gpio->gpbup = 0x000002BF;
	gpio->gpccon = 0xAAAAAAAA;
	gpio->gpcup = 0x0000FFFF;
	gpio->gpdcon = 0xAAAAAAAA;
	gpio->gpdup = 0x0000FFFF;
	gpio->gpecon = 0xAAAAAAAA;
	gpio->gpeup = 0x000037F7;
	gpio->gpfcon = 0x00000000;
	gpio->gpfup = 0x00000000;
	gpio->gpgcon = 0xFFEAFF5A;
	gpio->gpgup = 0x0000F0DC;
	gpio->gphcon = 0x0028AAAA;
	gpio->gphup = 0x00000656;

	/* setup correct IRQ modes for NIC */
	/* rising edge mode */
	gpio->extint2 = (gpio->extint2 & ~(7<<8)) | (4<<8);

	/* select USB port 2 to be host or device (fix to host for now) */
	gpio->misccr |= 0x08;

	/* init serial */
	gd->baudrate = CONFIG_BAUDRATE;
	gd->have_console = 1;
	serial_init();

	/* arch number of VCMA9-Board */
	gd->bd->bi_arch_number = MACH_TYPE_MPL_VCMA9;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	return 0;
}

/*
 * NAND flash initialization.
 */
#if defined(CONFIG_CMD_NAND)
extern ulong
nand_probe(ulong physadr);


static inline void NF_Reset(void)
{
    int i;

    NF_SetCE(NFCE_LOW);
    NF_Cmd(0xFF);		/* reset command */
    for(i = 0; i < 10; i++);	/* tWB = 100ns. */
    NF_WaitRB();		/* wait 200~500us; */
    NF_SetCE(NFCE_HIGH);
}


static inline void NF_Init(void)
{
#if 0 /* a little bit too optimistic */
#define TACLS   0
#define TWRPH0  3
#define TWRPH1  0
#else
#define TACLS   0
#define TWRPH0  4
#define TWRPH1  2
#endif

    NF_Conf((1<<15)|(0<<14)|(0<<13)|(1<<12)|(1<<11)|(TACLS<<8)|(TWRPH0<<4)|(TWRPH1<<0));
    /*nand->NFCONF = (1<<15)|(1<<14)|(1<<13)|(1<<12)|(1<<11)|(TACLS<<8)|(TWRPH0<<4)|(TWRPH1<<0); */
    /* 1  1    1     1,   1      xxx,  r xxx,   r xxx */
    /* En 512B 4step ECCR nFCE=H tACLS   tWRPH0   tWRPH1 */

    NF_Reset();
}

void
nand_init(void)
{
	struct s3c2410_nand * const nand = s3c2410_get_base_nand();

	NF_Init();
#ifdef DEBUG
	printf("NAND flash probing at 0x%.8lX\n", (ulong)nand);
#endif
	printf ("%4lu MB\n", nand_probe((ulong)nand) >> 20);
}
#endif

/*
 * Get some Board/PLD Info
 */

static u8 Get_PLD_ID(void)
{
	VCMA9_PLD * const pld = VCMA9_get_base_PLD();

	return(pld->ID);
}

static u8 Get_PLD_BOARD(void)
{
	VCMA9_PLD * const pld = VCMA9_get_base_PLD();

	return(pld->BOARD);
}

static u8 Get_PLD_SDRAM(void)
{
	VCMA9_PLD * const pld = VCMA9_get_base_PLD();

	return(pld->SDRAM);
}

static u8 Get_PLD_Version(void)
{
	return((Get_PLD_ID() >> 4) & 0x0F);
}

static u8 Get_PLD_Revision(void)
{
	return(Get_PLD_ID() & 0x0F);
}

#if 0	/* not used */
static int Get_Board_Config(void)
{
	u8 config = Get_PLD_BOARD() & 0x03;

	if (config == 3)
	    return 1;
	else
	    return 0;
}
#endif

static uchar Get_Board_PCB(void)
{
	return(((Get_PLD_BOARD() >> 4) & 0x03) + 'A');
}

static u8 Get_SDRAM_ChipNr(void)
{
	switch ((Get_PLD_SDRAM() >> 4) & 0x0F) {
		case 0: return 4;
		case 1: return 1;
		case 2: return 2;
		default: return 0;
	}
}

static ulong Get_SDRAM_ChipSize(void)
{
	switch (Get_PLD_SDRAM() & 0x0F) {
		case 0: return 16 * (1024*1024);
		case 1: return 32 * (1024*1024);
		case 2: return  8 * (1024*1024);
		case 3: return  8 * (1024*1024);
		default: return 0;
	}
}
static const char * Get_SDRAM_ChipGeom(void)
{
	switch (Get_PLD_SDRAM() & 0x0F) {
		case 0: return "4Mx8x4";
		case 1: return "8Mx8x4";
		case 2: return "2Mx8x4";
		case 3: return "4Mx8x2";
		default: return "unknown";
	}
}

static void Show_VCMA9_Info(char *board_name, char *serial)
{
	printf("Board: %s SN: %s  PCB Rev: %c PLD(%d,%d)\n",
		board_name, serial, Get_Board_PCB(), Get_PLD_Version(), Get_PLD_Revision());
	printf("SDRAM: %d chips %s\n", Get_SDRAM_ChipNr(), Get_SDRAM_ChipGeom());
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = Get_SDRAM_ChipSize() * Get_SDRAM_ChipNr();

	return 0;
}

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard(void)
{
	char s[50];
	int i;
	backup_t *b = (backup_t *) s;

	i = getenv_f("serial#", s, 32);
	if ((i < 0) || strncmp (s, "VCMA9", 5)) {
		get_backup_values (b);
		if (strncmp (b->signature, "MPL\0", 4) != 0) {
			puts ("### No HW ID - assuming VCMA9");
		} else {
			b->serial_name[5] = 0;
			Show_VCMA9_Info(b->serial_name, &b->serial_name[6]);
		}
	} else {
		s[5] = 0;
		Show_VCMA9_Info(s, &s[6]);
	}
	/*printf("\n");*/
	return(0);
}


int last_stage_init(void)
{
	checkboard();
	stdio_print_current_devices();
	check_env();
	return 0;
}

/***************************************************************************
 * some helping routines
 */
#if !CONFIG_USB_KEYBOARD
int overwrite_console(void)
{
	/* return TRUE if console should be overwritten */
	return 0;
}
#endif

/************************************************************************
* Print VCMA9 Info
************************************************************************/
void print_vcma9_info(void)
{
	char s[50];
	int i;

	if ((i = getenv_f("serial#", s, 32)) < 0) {
		puts ("### No HW ID - assuming VCMA9");
		printf("i %d", i*24);
	} else {
		s[5] = 0;
		Show_VCMA9_Info(s, &s[6]);
	}
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_CS8900
	rc = cs8900_initialize(0, CONFIG_CS8900_BASE);
#endif
	return rc;
}
#endif
