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
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	clk_power->LOCKTIME = 0xFFFFFF;

	/* configure MPLL */
	clk_power->MPLLCON = ((M_MDIV << 12) + (M_PDIV << 4) + M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (4000);

	/* configure UPLL */
	clk_power->UPLLCON = ((U_M_MDIV << 12) + (U_M_PDIV << 4) + U_M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (8000);

	/* set up the I/O ports */
	gpio->GPACON = 0x007FFFFF;
	gpio->GPBCON = 0x002AAAAA;
	gpio->GPBUP = 0x000002BF;
	gpio->GPCCON = 0xAAAAAAAA;
	gpio->GPCUP = 0x0000FFFF;
	gpio->GPDCON = 0xAAAAAAAA;
	gpio->GPDUP = 0x0000FFFF;
	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x000037F7;
	gpio->GPFCON = 0x00000000;
	gpio->GPFUP = 0x00000000;
	gpio->GPGCON = 0xFFEAFF5A;
	gpio->GPGUP = 0x0000F0DC;
	gpio->GPHCON = 0x0028AAAA;
	gpio->GPHUP = 0x00000656;

	/* setup correct IRQ modes for NIC */
	gpio->EXTINT2 = (gpio->EXTINT2 & ~(7<<8)) | (4<<8); /* rising edge mode */

	/* select USB port 2 to be host or device (fix to host for now) */
	gpio->MISCCR |= 0x08;

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
 * NAND flash initialization.
 */
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
extern void
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
#define TACLS   0
#define TWRPH0  3
#define TWRPH1  0
    NF_Conf((1<<15)|(0<<14)|(0<<13)|(1<<12)|(1<<11)|(TACLS<<8)|(TWRPH0<<4)|(TWRPH1<<0));
    //nand->NFCONF = (1<<15)|(1<<14)|(1<<13)|(1<<12)|(1<<11)|(TACLS<<8)|(TWRPH0<<4)|(TWRPH1<<0);
    // 1  1    1     1,   1      xxx,  r xxx,   r xxx
    // En 512B 4step ECCR nFCE=H tACLS   tWRPH0   tWRPH1

    NF_Reset();
}

void
nand_init(void)
{
	S3C2410_NAND * const nand = S3C2410_GetBase_NAND();

	NF_Init();
	printf("NAND flash probing at 0x%.8lX\n", (ulong)nand);
	nand_probe((ulong)nand);
}
#endif

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
			printf ("%s-%d PCB Rev %c SN: %s", b->serial_name, Get_Board_Config(),
					Get_Board_PCB(), &b->serial_name[6]);
		}
	} else {
		s[5] = 0;
		printf ("%s-%d PCB Rev %c SN: %s", s, Get_Board_Config(), Get_Board_PCB(),
				&s[6]);
	}
	printf("\n");
	return(0);
}



void print_vcma9_rev(void)
{
	printf("Board: VCMA9-%d PCB Rev: %c (PLD Ver: %d, Rev: %d)\n",
		Get_Board_Config(), Get_Board_PCB(),
		Get_PLD_Version(), Get_PLD_Revision());
}

extern void mem_test_reloc(void);

int last_stage_init(void)
{
	mem_test_reloc();
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
