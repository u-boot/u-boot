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
 *
 * TODO: clean-up
 */

/*
 * How do I program the SDRAM Timing Register (SDRAM0_TR) for a specific SDRAM or DIMM?
 *
 * As an example, consider a case where PC133 memory with CAS Latency equal to 2 is being
 * used with a 200MHz 405GP. For a typical 128Mb, PC133 SDRAM, the relevant minimum
 * parameters from the datasheet are:
 * Tclk = 7.5ns (CL = 2)
 * Trp = 15ns
 * Trc = 60ns
 * Trcd = 15ns
 * Trfc = 66ns
 *
 * If we are operating the 405GP with the MemClk output frequency set to 100 MHZ, the clock
 * period is 10ns and the parameters needed for the Timing Register are:
 * CASL = CL = 2 clock cycles
 * PTA = Trp = 15ns / 10ns = 2 clock cycles
 * CTP = Trc - Trcd - Trp = (60ns - 15ns - 15ns) / 10ns= 3 clock cycles
 * LDF = 2 clock cycles (but can be extended to meet board-level timing)
 * RFTA = Trfc = 66ns / 10ns= 7 clock cycles
 * RCD = Trcd = 15ns / 10ns= 2 clock cycles
 *
 * The actual bit settings in the register would be:
 *
 * CASL = 0b01
 * PTA = 0b01
 * CTP = 0b10
 * LDF = 0b01
 * RFTA = 0b011
 * RCD = 0b01
 *
 * If Trfc is not specified in the datasheet for PC100 or PC133 memory, set RFTA = Trc
 * instead. Figure 24 in the PC SDRAM Specification Rev. 1.7 shows refresh to active delay
 * defined as Trc rather than Trfc.
 * When using DIMM modules, most but not all of the required timing parameters can be read
 * from the Serial Presence Detect (SPD) EEPROM on the module. Specifically, Trc and Trfc
 * are not available from the EEPROM
 */

#include <common.h>
#include "mip405.h"
#include <asm/processor.h>
#include <405gp_i2c.h>
#include <miiphy.h>
#include "../common/common_util.h"
#include <i2c.h>
extern block_dev_desc_t * scsi_get_dev(int dev);
extern block_dev_desc_t * ide_get_dev(int dev);

#undef SDRAM_DEBUG

#define FALSE           0
#define TRUE            1

/* stdlib.h causes some compatibility problems; should fixe these! -- wd */
#ifndef __ldiv_t_defined
typedef struct {
	long int quot;		/* Quotient	*/
	long int rem;		/* Remainder	*/
} ldiv_t;
extern ldiv_t ldiv (long int __numer, long int __denom);
# define __ldiv_t_defined	1
#endif


#define PLD_PART_REG 		PER_PLD_ADDR + 0
#define PLD_VERS_REG 		PER_PLD_ADDR + 1
#define PLD_BOARD_CFG_REG 	PER_PLD_ADDR + 2
#define PLD_IRQ_REG 		PER_PLD_ADDR + 3
#define PLD_COM_MODE_REG 	PER_PLD_ADDR + 4
#define PLD_EXT_CONF_REG 	PER_PLD_ADDR + 5

#define MEGA_BYTE (1024*1024)

typedef struct {
	unsigned char boardtype; /* Board revision and Population Options */
	unsigned char cal;		/* cas Latency (will be programmend as cal-1) */
	unsigned char trp;		/* datain27 in clocks */
	unsigned char trcd;		/* datain29 in clocks */
	unsigned char tras;		/* datain30 in clocks */
	unsigned char tctp;		/* tras - trcd in clocks */
	unsigned char am;		/* Address Mod (will be programmed as am-1) */
	unsigned char sz;		/* log binary => Size = (4MByte<<sz) 5 = 128, 4 = 64, 3 = 32, 2 = 16, 1=8 */
	unsigned char ecc;		/* if true, ecc is enabled */
} sdram_t;

const sdram_t sdram_table[] = {
	{ 0x0f,	/* Rev A, 128MByte -1 Board */
		3,	/* Case Latenty = 3 */
		3,	/* trp 20ns / 7.5 ns datain[27] */
  		3, 	/* trcd 20ns /7.5 ns (datain[29]) */
  		6,  /* tras 44ns /7.5 ns  (datain[30]) */
		4,	/* tcpt 44 - 20ns = 24ns */
  		3,	/* Address Mode = 3 */
		5,	/* size value */
		1},	/* ECC enabled */
	{ 0x07,	/* Rev A, 64MByte -2 Board */
		3,	/* Case Latenty = 3 */
		3,	/* trp 20ns / 7.5 ns datain[27] */
  		3, 	/* trcd 20ns /7.5 ns (datain[29]) */
  		6,  /* tras 44ns /7.5 ns  (datain[30]) */
		4,	/* tcpt 44 - 20ns = 24ns */
  		2,	/* Address Mode = 2 */
		4,	/* size value */
		1},	/* ECC enabled */
	{ 0xff, /* terminator */
	  0xff,
	  0xff,
	  0xff,
	  0xff,
	  0xff,
	  0xff,
	  0xff }
};

void SDRAM_err (const char *s)
{
#ifndef SDRAM_DEBUG
	DECLARE_GLOBAL_DATA_PTR;

	(void) get_clocks ();
	gd->baudrate = 9600;
	serial_init ();
#endif
	serial_puts ("\n");
	serial_puts (s);
	serial_puts ("\n enable SDRAM_DEBUG for more info\n");
	for (;;);
}


unsigned char get_board_revcfg (void)
{
	out8 (PER_BOARD_ADDR, 0);
	return (in8 (PER_BOARD_ADDR));
}


#ifdef SDRAM_DEBUG

void write_hex (unsigned char i)
{
	char cc;

	cc = i >> 4;
	cc &= 0xf;
	if (cc > 9)
		serial_putc (cc + 55);
	else
		serial_putc (cc + 48);
	cc = i & 0xf;
	if (cc > 9)
		serial_putc (cc + 55);
	else
		serial_putc (cc + 48);
}

void write_4hex (unsigned long val)
{
	write_hex ((unsigned char) (val >> 24));
	write_hex ((unsigned char) (val >> 16));
	write_hex ((unsigned char) (val >> 8));
	write_hex ((unsigned char) val);
}

#endif


int init_sdram (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned long	tmp, baseaddr;
	unsigned short	i;
	unsigned char	trp_clocks,
			trcd_clocks,
			tras_clocks,
			trc_clocks,
			tctp_clocks;
	unsigned char	cal_val;
	unsigned char	bc;
	unsigned long	pbcr, sdram_tim, sdram_bank;
	unsigned long	*p;

	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);
	(void) get_clocks ();
	gd->baudrate = 9600;
	serial_init ();
	serial_puts ("\nInitializing SDRAM, Please stand by");
	mtdcr (ebccfga, pb0cr);		/* get cs0 config reg */
	pbcr = mfdcr (ebccfgd);
	if ((pbcr & 0x00002000) == 0) {
		/* MPS Boot, set up the flash */
		mtdcr (ebccfga, pb1ap);
		mtdcr (ebccfgd, FLASH_AP);
		mtdcr (ebccfga, pb1cr);
		mtdcr (ebccfgd, FLASH_CR);
	} else {
		/* Flash boot, set up the MPS */
		mtdcr (ebccfga, pb1ap);
		mtdcr (ebccfgd, MPS_AP);
		mtdcr (ebccfga, pb1cr);
		mtdcr (ebccfgd, MPS_CR);
	}
	/* set up UART0 (CS2) and UART1 (CS3) */
	mtdcr (ebccfga, pb2ap);
	mtdcr (ebccfgd, UART0_AP);
	mtdcr (ebccfga, pb2cr);
	mtdcr (ebccfgd, UART0_CR);
	mtdcr (ebccfga, pb3ap);
	mtdcr (ebccfgd, UART1_AP);
	mtdcr (ebccfga, pb3cr);
	mtdcr (ebccfgd, UART1_CR);

	/* set up the pld */
	mtdcr (ebccfga, pb7ap);
	mtdcr (ebccfgd, PLD_AP);
	mtdcr (ebccfga, pb7cr);
	mtdcr (ebccfgd, PLD_CR);
	/* set up the board rev reg */
	mtdcr (ebccfga, pb5ap);
	mtdcr (ebccfgd, BOARD_AP);
	mtdcr (ebccfga, pb5cr);
	mtdcr (ebccfgd, BOARD_CR);


#ifdef SDRAM_DEBUG
	out8 (PER_BOARD_ADDR, 0);
	bc = in8 (PER_BOARD_ADDR);
	serial_puts ("\nBoard Rev: ");
	write_hex (bc);
	serial_puts (" (PLD=");
	bc = in8 (PLD_BOARD_CFG_REG);
	write_hex (bc);
	serial_puts (")\n");
#endif
	bc = get_board_revcfg ();
#ifdef SDRAM_DEBUG
	serial_puts ("\nstart SDRAM Setup\n");
	serial_puts ("\nBoard Rev: ");
	write_hex (bc);
	serial_puts ("\n");
#endif
	i = 0;
	baseaddr = CFG_SDRAM_BASE;
	while (sdram_table[i].sz != 0xff) {
		if (sdram_table[i].boardtype == bc)
			break;
		i++;
	}
	if (sdram_table[i].boardtype != bc)
		SDRAM_err ("No SDRAM table found for this board!!!\n");
#ifdef SDRAM_DEBUG
	serial_puts (" found table ");
	write_hex (i);
	serial_puts (" \n");
#endif
	cal_val = sdram_table[i].cal - 1;	/* Cas Latency */
	trp_clocks = sdram_table[i].trp;	/* 20ns / 7.5 ns datain[27] */
	trcd_clocks = sdram_table[i].trcd;	/* 20ns /7.5 ns (datain[29]) */
	tras_clocks = sdram_table[i].tras;	/* 44ns /7.5 ns  (datain[30]) */
	/* ctp = ((trp + tras) - trp - trcd) => tras - trcd */
	tctp_clocks = sdram_table[i].tctp;	/* 44 - 20ns = 24ns */
	/* trc_clocks is sum of trp_clocks + tras_clocks */
	trc_clocks = trp_clocks + tras_clocks;
	/* get SDRAM timing register */
	mtdcr (memcfga, mem_sdtr1);
	sdram_tim = mfdcr (memcfgd) & ~0x018FC01F;
	/* insert CASL value */
	sdram_tim |= ((unsigned long) (cal_val)) << 23;
	/* insert PTA value */
	sdram_tim |= ((unsigned long) (trp_clocks - 1)) << 18;
	/* insert CTP value */
	sdram_tim |=
			((unsigned long) (trc_clocks - trp_clocks -
							  trcd_clocks)) << 16;
	/* insert LDF (always 01) */
	sdram_tim |= ((unsigned long) 0x01) << 14;
	/* insert RFTA value */
	sdram_tim |= ((unsigned long) (trc_clocks - 4)) << 2;
	/* insert RCD value */
	sdram_tim |= ((unsigned long) (trcd_clocks - 1)) << 0;

	tmp = ((unsigned long) (sdram_table[i].am - 1) << 13);	/* AM = 3 */
	/* insert SZ value; */
	tmp |= ((unsigned long) sdram_table[i].sz << 17);
	/* get SDRAM bank 0 register */
	mtdcr (memcfga, mem_mb0cf);
	sdram_bank = mfdcr (memcfgd) & ~0xFFCEE001;
	sdram_bank |= (baseaddr | tmp | 0x01);

#ifdef SDRAM_DEBUG
	serial_puts ("sdtr: ");
	write_4hex (sdram_tim);
	serial_puts ("\n");
#endif

	/* write SDRAM timing register */
	mtdcr (memcfga, mem_sdtr1);
	mtdcr (memcfgd, sdram_tim);

#ifdef SDRAM_DEBUG
	serial_puts ("mb0cf: ");
	write_4hex (sdram_bank);
	serial_puts ("\n");
#endif

	/* write SDRAM bank 0 register */
	mtdcr (memcfga, mem_mb0cf);
	mtdcr (memcfgd, sdram_bank);

	if (get_bus_freq (tmp) > 110000000) {	/* > 110MHz */
		/* get SDRAM refresh interval register */
		mtdcr (memcfga, mem_rtr);
		tmp = mfdcr (memcfgd) & ~0x3FF80000;
		tmp |= 0x07F00000;
	} else {
		/* get SDRAM refresh interval register */
		mtdcr (memcfga, mem_rtr);
		tmp = mfdcr (memcfgd) & ~0x3FF80000;
		tmp |= 0x05F00000;
	}
	/* write SDRAM refresh interval register */
	mtdcr (memcfga, mem_rtr);
	mtdcr (memcfgd, tmp);
	/* enable ECC if used */
#if 1
	if (sdram_table[i].ecc) {
		/* disable checking for all banks */
#ifdef SDRAM_DEBUG
		serial_puts ("disable ECC.. ");
#endif
		mtdcr (memcfga, mem_ecccf);
		tmp = mfdcr (memcfgd);
		tmp &= 0xff0fffff;		/* disable all banks */
		mtdcr (memcfga, mem_ecccf);
		/* set up SDRAM Controller with ECC enabled */
#ifdef SDRAM_DEBUG
		serial_puts ("setup SDRAM Controller.. ");
#endif
		mtdcr (memcfgd, tmp);
		mtdcr (memcfga, mem_mcopt1);
		tmp = (mfdcr (memcfgd) & ~0xFFE00000) | 0x90800000;
		mtdcr (memcfga, mem_mcopt1);
		mtdcr (memcfgd, tmp);
		udelay (600);
#ifdef SDRAM_DEBUG
		serial_puts ("fill the memory..\n");
#endif
		serial_puts (".");
		/* now, fill all the memory */
		tmp = ((4 * MEGA_BYTE) << sdram_table[i].sz);
		p = (unsigned long) 0;
		while ((unsigned long) p < tmp) {
			*p++ = 0L;
			if (!((unsigned long) p % 0x00800000))	/* every 8MByte */
				serial_puts (".");


		}
		/* enable bank 0 */
		serial_puts (".");
#ifdef SDRAM_DEBUG
		serial_puts ("enable ECC\n");
#endif
		udelay (400);
		mtdcr (memcfga, mem_ecccf);
		tmp = mfdcr (memcfgd);
		tmp |= 0x00800000;		/* enable bank 0 */
		mtdcr (memcfgd, tmp);
		udelay (400);
	} else
#endif
	{
		/* enable SDRAM controller with no ECC, 32-bit SDRAM width, 16 byte burst */
		mtdcr (memcfga, mem_mcopt1);
		tmp = (mfdcr (memcfgd) & ~0xFFE00000) | 0x80C00000;
		mtdcr (memcfga, mem_mcopt1);
		mtdcr (memcfgd, tmp);
		udelay (400);
	}
	serial_puts ("\n");
	return (0);
}

int board_pre_init (void)
{
	init_sdram ();

   /*-------------------------------------------------------------------------+
   | Interrupt controller setup for the PIP405 board.
   | Note: IRQ 0-15  405GP internally generated; active high; level sensitive
   |       IRQ 16    405GP internally generated; active low; level sensitive
   |       IRQ 17-24 RESERVED
   |       IRQ 25 (EXT IRQ 0) SouthBridge; active low; level sensitive
   |       IRQ 26 (EXT IRQ 1) NMI: active low; level sensitive
   |       IRQ 27 (EXT IRQ 2) SMI: active Low; level sensitive
   |       IRQ 28 (EXT IRQ 3) PCI SLOT 3; active low; level sensitive
   |       IRQ 29 (EXT IRQ 4) PCI SLOT 2; active low; level sensitive
   |       IRQ 30 (EXT IRQ 5) PCI SLOT 1; active low; level sensitive
   |       IRQ 31 (EXT IRQ 6) PCI SLOT 0; active low; level sensitive
   | Note for MIP405 board:
   |       An interrupt taken for the SouthBridge (IRQ 25) indicates that
   |       the Interrupt Controller in the South Bridge has caused the
   |       interrupt. The IC must be read to determine which device
   |       caused the interrupt.
   |
   +-------------------------------------------------------------------------*/
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (uicer, 0x00000000);	/* disable all ints */
	mtdcr (uiccr, 0x00000000);	/* set all to be non-critical (for now) */
	mtdcr (uicpr, 0xFFFFFF80);	/* set int polarities */
	mtdcr (uictr, 0x10000000);	/* set int trigger levels */
	mtdcr (uicvcr, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */
	return 0;
}


/*
 * Get some PLD Registers
 */

unsigned short get_pld_parvers (void)
{
	unsigned short result;
	unsigned char rc;

	rc = in8 (PLD_PART_REG);
	result = (unsigned short) rc << 8;
	rc = in8 (PLD_VERS_REG);
	result |= rc;
	return result;
}



void user_led0 (unsigned char on)
{
	if (on)
		out8 (PLD_COM_MODE_REG, (in8 (PLD_COM_MODE_REG) | 0x4));
	else
		out8 (PLD_COM_MODE_REG, (in8 (PLD_COM_MODE_REG) & 0xfb));
}


void ide_set_reset (int idereset)
{
	/* if reset = 1 IDE reset will be asserted */
	if (idereset)
		out8 (PLD_COM_MODE_REG, (in8 (PLD_COM_MODE_REG) | 0x1));
	else {
		udelay (10000);
		out8 (PLD_COM_MODE_REG, (in8 (PLD_COM_MODE_REG) & 0xfe));
	}
}


/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	unsigned char s[50];
	unsigned char bc, var, rc;
	int i;
	backup_t *b = (backup_t *) s;

	puts ("Board: ");

	bc = get_board_revcfg ();
	var = ~bc;
	var &= 0xf;
	rc = 0;
	for (i = 0; i < 4; i++) {
		rc <<= 1;
		rc += (var & 0x1);
		var >>= 1;
	}
	rc++;
	i = getenv_r ("serial#", s, 32);
	if ((i == 0) || strncmp (s, "MIP405", 6)) {
		get_backup_values (b);
		if (strncmp (b->signature, "MPL\0", 4) != 0) {
			puts ("### No HW ID - assuming MIP405");
			printf ("-%d Rev %c", rc, 'A' + ((bc >> 4) & 0xf));
		} else {
			b->serial_name[6] = 0;
			printf ("%s-%d Rev %c SN: %s", b->serial_name, rc,
					'A' + ((bc >> 4) & 0xf), &b->serial_name[7]);
		}
	} else {
		s[6] = 0;
		printf ("%s-%d Rev %c SN: %s", s, rc, 'A' + ((bc >> 4) & 0xf),
				&s[7]);
	}
	bc = in8 (PLD_EXT_CONF_REG);
	printf (" Boot Config: 0x%x\n", bc);
	return (0);
}


/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/*
  initdram(int board_type) reads EEPROM via I2c. EEPROM contains all of
  the necessary info for SDRAM controller configuration
*/
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
static int test_dram (unsigned long ramsize);

long int initdram (int board_type)
{

	unsigned long bank_reg[4], tmp, bank_size;
	int i, ds;
	unsigned long TotalSize;

	ds = 0;
	/* since the DRAM controller is allready set up, calculate the size with the
	   bank registers    */
	mtdcr (memcfga, mem_mb0cf);
	bank_reg[0] = mfdcr (memcfgd);
	mtdcr (memcfga, mem_mb1cf);
	bank_reg[1] = mfdcr (memcfgd);
	mtdcr (memcfga, mem_mb2cf);
	bank_reg[2] = mfdcr (memcfgd);
	mtdcr (memcfga, mem_mb3cf);
	bank_reg[3] = mfdcr (memcfgd);
	TotalSize = 0;
	for (i = 0; i < 4; i++) {
		if ((bank_reg[i] & 0x1) == 0x1) {
			tmp = (bank_reg[i] >> 17) & 0x7;
			bank_size = 4 << tmp;
			TotalSize += bank_size;
		} else
			ds = 1;
	}
	mtdcr (memcfga, mem_ecccf);
	tmp = mfdcr (memcfgd);

	if (!tmp)
		printf ("No ");
	printf ("ECC ");

	test_dram (TotalSize * MEGA_BYTE);
	return (TotalSize * MEGA_BYTE);
}

/* ------------------------------------------------------------------------- */

extern int mem_test (unsigned long start, unsigned long ramsize,
					 int quiet);

static int test_dram (unsigned long ramsize)
{
#ifdef SDRAM_DEBUG
	mem_test (0L, ramsize, 1);
#endif
	/* not yet implemented */
	return (1);
}

int misc_init_r (void)
{
	return (0);
}


void print_mip405_rev (void)
{
	unsigned char part, vers, cfg, rev;

	cfg = get_board_revcfg ();
	vers = cfg;
	vers &= 0xf;
	rev = (((vers & 0x1) ? 0x8 : 0) |
		   ((vers & 0x2) ? 0x4 : 0) |
		   ((vers & 0x4) ? 0x2 : 0) | ((vers & 0x8) ? 0x1 : 0));

	part = in8 (PLD_PART_REG);
	vers = in8 (PLD_VERS_REG);
	printf ("Rev:   MIP405-%d Rev %c PLD%d Vers %d\n",
			(16 - rev), ((cfg >> 4) & 0xf) + 'A', part, vers);
}


int last_stage_init (void)
{
	if (miiphy_write (0x1, 0x14, 0x2402) != 0) {
		printf ("Error writing to the PHY\n");
	}
	print_mip405_rev ();
	show_stdio_dev ();
	check_env ();
	return 0;
}

/***************************************************************************
 * some helping routines
 */

int overwrite_console (void)
{
	return ((in8 (PLD_EXT_CONF_REG) & 0x1)==0);	/* return TRUE if console should be overwritten */
}


/************************************************************************
* Print MIP405 Info
************************************************************************/
void print_mip405_info (void)
{
	unsigned char part, vers, cfg, irq_reg, com_mode, ext;

	part = in8 (PLD_PART_REG);
	vers = in8 (PLD_VERS_REG);
	cfg = in8 (PLD_BOARD_CFG_REG);
	irq_reg = in8 (PLD_IRQ_REG);
	com_mode = in8 (PLD_COM_MODE_REG);
	ext = in8 (PLD_EXT_CONF_REG);

	printf ("PLD Part %d version %d\n", part, vers);
	printf ("Board Revision %c\n", ((cfg >> 4) & 0xf) + 'A');
	printf ("Population Options %d %d %d %d\n", (cfg) & 0x1,
			(cfg >> 1) & 0x1, (cfg >> 2) & 0x1, (cfg >> 3) & 0x1);
	printf ("User LED %s\n", (com_mode & 0x4) ? "on" : "off");
	printf ("UART Clocks %d\n", (com_mode >> 4) & 0x3);
	printf ("Test ist %x\n", com_mode);
	printf ("User Config Switch %d %d %d %d %d %d %d %d\n",
			(ext) & 0x1, (ext >> 1) & 0x1, (ext >> 2) & 0x1,
			(ext >> 3) & 0x1, (ext >> 4) & 0x1, (ext >> 5) & 0x1,
			(ext >> 6) & 0x1, (ext >> 7) & 0x1);
	printf ("SER1 uses handshakes %s\n",
			(ext & 0x80) ? "DTR/DSR" : "RTS/CTS");
	printf ("IDE Reset %s\n", (ext & 0x01) ? "asserted" : "not asserted");
	printf ("IRQs:\n");
	printf ("  PIIX INTR: %s\n", (irq_reg & 0x80) ? "inactive" : "active");
	printf ("  UART0 IRQ: %s\n", (irq_reg & 0x40) ? "inactive" : "active");
	printf ("  UART1 IRQ: %s\n", (irq_reg & 0x20) ? "inactive" : "active");
	printf ("  PIIX SMI:  %s\n", (irq_reg & 0x10) ? "inactive" : "active");
	printf ("  PIIX INIT: %s\n", (irq_reg & 0x8) ? "inactive" : "active");
	printf ("  PIIX NMI:  %s\n", (irq_reg & 0x4) ? "inactive" : "active");
}


