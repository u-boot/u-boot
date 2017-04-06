/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#include <asm/ppc4xx.h>
#include <asm/ppc4xx-i2c.h>
#include <miiphy.h>
#include "../common/common_util.h"
#include <stdio_dev.h>
#include <i2c.h>
#include <rtc.h>

DECLARE_GLOBAL_DATA_PTR;

#undef SDRAM_DEBUG
#define ENABLE_ECC /* for ecc boards */

/* stdlib.h causes some compatibility problems; should fixe these! -- wd */
#ifndef __ldiv_t_defined
typedef struct {
	long int quot;		/* Quotient	*/
	long int rem;		/* Remainder	*/
} ldiv_t;
extern ldiv_t ldiv (long int __numer, long int __denom);
# define __ldiv_t_defined	1
#endif


#define PLD_PART_REG		PER_PLD_ADDR + 0
#define PLD_VERS_REG		PER_PLD_ADDR + 1
#define PLD_BOARD_CFG_REG	PER_PLD_ADDR + 2
#define PLD_IRQ_REG		PER_PLD_ADDR + 3
#define PLD_COM_MODE_REG	PER_PLD_ADDR + 4
#define PLD_EXT_CONF_REG	PER_PLD_ADDR + 5

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
#if defined(CONFIG_TARGET_MIP405T)
const sdram_t sdram_table[] = {
	{ 0x0F,	/* MIP405T Rev A, 64MByte -1 Board */
		3,	/* Case Latenty = 3 */
		3,	/* trp 20ns / 7.5 ns datain[27] */
		3,	/* trcd 20ns /7.5 ns (datain[29]) */
		6,	/* tras 44ns /7.5 ns  (datain[30]) */
		4,	/* tcpt 44 - 20ns = 24ns */
		2,	/* Address Mode = 2 (12x9x4) */
		3,	/* size value (32MByte) */
		0},	/* ECC disabled */
	{ 0xff, /* terminator */
	  0xff,
	  0xff,
	  0xff,
	  0xff,
	  0xff,
	  0xff,
	  0xff }
};
#else
const sdram_t sdram_table[] = {
	{ 0x0f,	/* Rev A, 128MByte -1 Board */
		3,	/* Case Latenty = 3 */
		3,	/* trp 20ns / 7.5 ns datain[27] */
		3,	/* trcd 20ns /7.5 ns (datain[29]) */
		6,	/* tras 44ns /7.5 ns  (datain[30]) */
		4,	/* tcpt 44 - 20ns = 24ns */
		3,	/* Address Mode = 3 */
		5,	/* size value */
		1},	/* ECC enabled */
	{ 0x07,	/* Rev A, 64MByte -2 Board */
		3,	/* Case Latenty = 3 */
		3,	/* trp 20ns / 7.5 ns datain[27] */
		3,	/* trcd 20ns /7.5 ns (datain[29]) */
		6,	/* tras 44ns /7.5 ns  (datain[30]) */
		4,	/* tcpt 44 - 20ns = 24ns */
		2,	/* Address Mode = 2 */
		4,	/* size value */
		1},	/* ECC enabled */
	{ 0x03,	/* Rev A, 128MByte -4 Board */
		3,	/* Case Latenty = 3 */
		3,	/* trp 20ns / 7.5 ns datain[27] */
		3,	/* trcd 20ns /7.5 ns (datain[29]) */
		6,	/* tras 44ns /7.5 ns  (datain[30]) */
		4,	/* tcpt 44 - 20ns = 24ns */
		3,	/* Address Mode = 3 */
		5,	/* size value */
		1},	/* ECC enabled */
	{ 0x1f,	/* Rev B, 128MByte -3 Board */
		3,	/* Case Latenty = 3 */
		3,	/* trp 20ns / 7.5 ns datain[27] */
		3,	/* trcd 20ns /7.5 ns (datain[29]) */
		6,	/* tras 44ns /7.5 ns  (datain[30]) */
		4,	/* tcpt 44 - 20ns = 24ns */
		3,	/* Address Mode = 3 */
		5,	/* size value */
		1},	/* ECC enabled */
	{ 0x2f,	/* Rev C, 128MByte -3 Board */
		3,	/* Case Latenty = 3 */
		3,	/* trp 20ns / 7.5 ns datain[27] */
		3,	/* trcd 20ns /7.5 ns (datain[29]) */
		6,	/* tras 44ns /7.5 ns  (datain[30]) */
		4,	/* tcpt 44 - 20ns = 24ns */
		3,	/* Address Mode = 3 */
		5,	/* size value */
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
#endif /*CONFIG_TARGET_MIP405T */
void SDRAM_err (const char *s)
{
#ifndef SDRAM_DEBUG
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
	unsigned long	tmp, baseaddr;
	unsigned short	i;
	unsigned char	trp_clocks,
			trcd_clocks,
			tras_clocks,
			trc_clocks;
	unsigned char	cal_val;
	unsigned char	bc;
	unsigned long	sdram_tim, sdram_bank;

	/*i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);*/
	(void) get_clocks ();
	gd->baudrate = 9600;
	serial_init ();
	/* set up the pld */
	mtdcr (EBC0_CFGADDR, PB7AP);
	mtdcr (EBC0_CFGDATA, PLD_AP);
	mtdcr (EBC0_CFGADDR, PB7CR);
	mtdcr (EBC0_CFGDATA, PLD_CR);
	/* THIS IS OBSOLETE */
	/* set up the board rev reg*/
	mtdcr (EBC0_CFGADDR, PB5AP);
	mtdcr (EBC0_CFGDATA, BOARD_AP);
	mtdcr (EBC0_CFGADDR, PB5CR);
	mtdcr (EBC0_CFGDATA, BOARD_CR);
#ifdef SDRAM_DEBUG
	/* get all informations from PLD */
	serial_puts ("\nPLD Part  0x");
	bc = in8 (PLD_PART_REG);
	write_hex (bc);
	serial_puts ("\nPLD Vers  0x");
	bc = in8 (PLD_VERS_REG);
	write_hex (bc);
	serial_puts ("\nBoard Rev 0x");
	bc = in8 (PLD_BOARD_CFG_REG);
	write_hex (bc);
	serial_puts ("\n");
#endif
	/* check board */
	bc = in8 (PLD_PART_REG);
#if defined(CONFIG_TARGET_MIP405T)
	if((bc & 0x80)==0)
		SDRAM_err ("U-Boot configured for a MIP405T not for a MIP405!!!\n");
#else
	if((bc & 0x80)==0x80)
		SDRAM_err ("U-Boot configured for a MIP405 not for a MIP405T!!!\n");
#endif
	/* set-up the chipselect machine */
	mtdcr (EBC0_CFGADDR, PB0CR);		/* get cs0 config reg */
	tmp = mfdcr (EBC0_CFGDATA);
	if ((tmp & 0x00002000) == 0) {
		/* MPS Boot, set up the flash */
		mtdcr (EBC0_CFGADDR, PB1AP);
		mtdcr (EBC0_CFGDATA, FLASH_AP);
		mtdcr (EBC0_CFGADDR, PB1CR);
		mtdcr (EBC0_CFGDATA, FLASH_CR);
	} else {
		/* Flash boot, set up the MPS */
		mtdcr (EBC0_CFGADDR, PB1AP);
		mtdcr (EBC0_CFGDATA, MPS_AP);
		mtdcr (EBC0_CFGADDR, PB1CR);
		mtdcr (EBC0_CFGDATA, MPS_CR);
	}
	/* set up UART0 (CS2) and UART1 (CS3) */
	mtdcr (EBC0_CFGADDR, PB2AP);
	mtdcr (EBC0_CFGDATA, UART0_AP);
	mtdcr (EBC0_CFGADDR, PB2CR);
	mtdcr (EBC0_CFGDATA, UART0_CR);
	mtdcr (EBC0_CFGADDR, PB3AP);
	mtdcr (EBC0_CFGDATA, UART1_AP);
	mtdcr (EBC0_CFGADDR, PB3CR);
	mtdcr (EBC0_CFGDATA, UART1_CR);
	bc = in8 (PLD_BOARD_CFG_REG);
#ifdef SDRAM_DEBUG
	serial_puts ("\nstart SDRAM Setup\n");
	serial_puts ("\nBoard Rev: ");
	write_hex (bc);
	serial_puts ("\n");
#endif
	i = 0;
	baseaddr = CONFIG_SYS_SDRAM_BASE;
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
	/* since the ECC initialisation needs some time,
	 * we show that we're alive
	 */
	if (sdram_table[i].ecc)
		serial_puts ("\nInitializing SDRAM, Please stand by");
	cal_val = sdram_table[i].cal - 1;	/* Cas Latency */
	trp_clocks = sdram_table[i].trp;	/* 20ns / 7.5 ns datain[27] */
	trcd_clocks = sdram_table[i].trcd;	/* 20ns /7.5 ns (datain[29]) */
	tras_clocks = sdram_table[i].tras;	/* 44ns /7.5 ns  (datain[30]) */
	/* ctp = ((trp + tras) - trp - trcd) => tras - trcd */
	/* trc_clocks is sum of trp_clocks + tras_clocks */
	trc_clocks = trp_clocks + tras_clocks;
	/* get SDRAM timing register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_TR);
	sdram_tim = mfdcr (SDRAM0_CFGDATA) & ~0x018FC01F;
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
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B0CR);
	sdram_bank = mfdcr (SDRAM0_CFGDATA) & ~0xFFCEE001;
	sdram_bank |= (baseaddr | tmp | 0x01);

#ifdef SDRAM_DEBUG
	serial_puts ("sdtr: ");
	write_4hex (sdram_tim);
	serial_puts ("\n");
#endif

	/* write SDRAM timing register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_TR);
	mtdcr (SDRAM0_CFGDATA, sdram_tim);

#ifdef SDRAM_DEBUG
	serial_puts ("mb0cf: ");
	write_4hex (sdram_bank);
	serial_puts ("\n");
#endif

	/* write SDRAM bank 0 register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B0CR);
	mtdcr (SDRAM0_CFGDATA, sdram_bank);

	if (get_bus_freq (tmp) > 110000000) {	/* > 110MHz */
		/* get SDRAM refresh interval register */
		mtdcr (SDRAM0_CFGADDR, SDRAM0_RTR);
		tmp = mfdcr (SDRAM0_CFGDATA) & ~0x3FF80000;
		tmp |= 0x07F00000;
	} else {
		/* get SDRAM refresh interval register */
		mtdcr (SDRAM0_CFGADDR, SDRAM0_RTR);
		tmp = mfdcr (SDRAM0_CFGDATA) & ~0x3FF80000;
		tmp |= 0x05F00000;
	}
	/* write SDRAM refresh interval register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_RTR);
	mtdcr (SDRAM0_CFGDATA, tmp);
	/* enable ECC if used */
#if defined(ENABLE_ECC) && !defined(CONFIG_BOOT_PCI)
	if (sdram_table[i].ecc) {
		/* disable checking for all banks */
		unsigned long	*p;
#ifdef SDRAM_DEBUG
		serial_puts ("disable ECC.. ");
#endif
		mtdcr (SDRAM0_CFGADDR, SDRAM0_ECCCFG);
		tmp = mfdcr (SDRAM0_CFGDATA);
		tmp &= 0xff0fffff;		/* disable all banks */
		mtdcr (SDRAM0_CFGADDR, SDRAM0_ECCCFG);
		/* set up SDRAM Controller with ECC enabled */
#ifdef SDRAM_DEBUG
		serial_puts ("setup SDRAM Controller.. ");
#endif
		mtdcr (SDRAM0_CFGDATA, tmp);
		mtdcr (SDRAM0_CFGADDR, SDRAM0_CFG);
		tmp = (mfdcr (SDRAM0_CFGDATA) & ~0xFFE00000) | 0x90800000;
		mtdcr (SDRAM0_CFGADDR, SDRAM0_CFG);
		mtdcr (SDRAM0_CFGDATA, tmp);
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
		mtdcr (SDRAM0_CFGADDR, SDRAM0_ECCCFG);
		tmp = mfdcr (SDRAM0_CFGDATA);
		tmp |= 0x00800000;		/* enable bank 0 */
		mtdcr (SDRAM0_CFGDATA, tmp);
		udelay (400);
	} else
#endif
	{
		/* enable SDRAM controller with no ECC, 32-bit SDRAM width, 16 byte burst */
		mtdcr (SDRAM0_CFGADDR, SDRAM0_CFG);
		tmp = (mfdcr (SDRAM0_CFGDATA) & ~0xFFE00000) | 0x80C00000;
		mtdcr (SDRAM0_CFGADDR, SDRAM0_CFG);
		mtdcr (SDRAM0_CFGDATA, tmp);
		udelay (400);
	}
	serial_puts ("\n");
	return (0);
}

int board_early_init_f (void)
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
	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (UIC0ER, 0x00000000);	/* disable all ints */
	mtdcr (UIC0CR, 0x00000000);	/* set all to be non-critical (for now) */
	mtdcr (UIC0PR, 0xFFFFFF80);	/* set int polarities */
	mtdcr (UIC0TR, 0x10000000);	/* set int trigger levels */
	mtdcr (UIC0VCR, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	return 0;
}

int board_early_init_r(void)
{
	int mode;

	/*
	 * since we are relocated, we can finally enable i-cache
	 * and set up the flash CS correctly
	 */
	icache_enable();
	setup_cs_reloc();
	/* get and display boot mode */
	mode = get_boot_mode();
	if (mode & BOOT_PCI)
		printf("PCI Boot %s Map\n", (mode & BOOT_MPS) ?
			"MPS" : "Flash");
	else
		printf("%s Boot\n", (mode & BOOT_MPS) ?
			"MPS" : "Flash");

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

void get_pcbrev_var(unsigned char *pcbrev, unsigned char *var)
{
#if !defined(CONFIG_TARGET_MIP405T)
	unsigned char bc,rc,tmp;
	int i;

	bc = in8 (PLD_BOARD_CFG_REG);
	tmp = ~bc;
	tmp &= 0xf;
	rc = 0;
	for (i = 0; i < 4; i++) {
		rc <<= 1;
		rc += (tmp & 0x1);
		tmp >>= 1;
	}
	rc++;
	if((  (((bc>>4) & 0xf)==0x2) /* Rev C PCB or */
	   || (((bc>>4) & 0xf)==0x1)) /* Rev B PCB with */
		&& (rc==0x1))     /* Population Option 1 is a -3 */
		rc=3;
	*pcbrev=(bc >> 4) & 0xf;
	*var=rc;
#else
	unsigned char bc;
	bc = in8 (PLD_BOARD_CFG_REG);
	*pcbrev=(bc >> 4) & 0xf;
	*var=16-(bc & 0xf);
#endif
}

/*
 * Check Board Identity:
 */
/* serial String: "MIP405_1000" OR "MIP405T_1000" */
#if !defined(CONFIG_TARGET_MIP405T)
#define BOARD_NAME	"MIP405"
#else
#define BOARD_NAME	"MIP405T"
#endif

int checkboard (void)
{
	char s[50];
	unsigned char bc, var;
	int i;
	backup_t *b = (backup_t *) s;

	puts ("Board: ");
	get_pcbrev_var(&bc,&var);
	i = getenv_f("serial#", (char *)s, 32);
	if ((i == 0) || strncmp ((char *)s, BOARD_NAME,sizeof(BOARD_NAME))) {
		get_backup_values (b);
		if (strncmp (b->signature, "MPL\0", 4) != 0) {
			puts ("### No HW ID - assuming " BOARD_NAME);
			printf ("-%d Rev %c", var, 'A' + bc);
		} else {
			b->serial_name[sizeof(BOARD_NAME)-1] = 0;
			printf ("%s-%d Rev %c SN: %s", b->serial_name, var,
					'A' + bc, &b->serial_name[sizeof(BOARD_NAME)]);
		}
	} else {
		s[sizeof(BOARD_NAME)-1] = 0;
		printf ("%s-%d Rev %c SN: %s", s, var,'A' + bc,
				&s[sizeof(BOARD_NAME)]);
	}
	bc = in8 (PLD_EXT_CONF_REG);
	printf (" Boot Config: 0x%x\n", bc);
	return (0);
}


/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/*
  dram_init() reads EEPROM via I2c. EEPROM contains all of
  the necessary info for SDRAM controller configuration
*/
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
static int test_dram (unsigned long ramsize);

int dram_init(void)
{

	unsigned long bank_reg[4], tmp, bank_size;
	int i;
	unsigned long TotalSize;

	/* since the DRAM controller is allready set up, calculate the size with the
	   bank registers    */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B0CR);
	bank_reg[0] = mfdcr (SDRAM0_CFGDATA);
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B1CR);
	bank_reg[1] = mfdcr (SDRAM0_CFGDATA);
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B2CR);
	bank_reg[2] = mfdcr (SDRAM0_CFGDATA);
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B3CR);
	bank_reg[3] = mfdcr (SDRAM0_CFGDATA);
	TotalSize = 0;
	for (i = 0; i < 4; i++) {
		if ((bank_reg[i] & 0x1) == 0x1) {
			tmp = (bank_reg[i] >> 17) & 0x7;
			bank_size = 4 << tmp;
			TotalSize += bank_size;
		}
	}
	mtdcr (SDRAM0_CFGADDR, SDRAM0_ECCCFG);
	tmp = mfdcr (SDRAM0_CFGDATA);

	if (!tmp)
		printf ("No ");
	printf ("ECC ");

	test_dram (TotalSize * MEGA_BYTE);
	gd->ram_size = TotalSize * MEGA_BYTE;

	return 0;
}

/* ------------------------------------------------------------------------- */


static int test_dram (unsigned long ramsize)
{
#ifdef SDRAM_DEBUG
	mem_test (0L, ramsize, 1);
#endif
	/* not yet implemented */
	return (1);
}

/* used to check if the time in RTC is valid */
static unsigned long start;
static struct rtc_time tm;

int misc_init_r (void)
{
	/* adjust flash start and size as well as the offset */
	gd->bd->bi_flashstart=0-flash_info[0].size;
	gd->bd->bi_flashsize=flash_info[0].size-CONFIG_SYS_MONITOR_LEN;
	gd->bd->bi_flashoffset=0;

	/* check, if RTC is running */
	rtc_get (&tm);
	start=get_timer(0);
	/* if MIP405 has booted from PCI, reset CCR0[24] as described in errata PCI_18 */
	if (mfdcr(CPC0_PSR) & PSR_ROM_LOC)
	       mtspr(SPRN_CCR0, (mfspr(SPRN_CCR0) & ~0x80));

	return (0);
}


void print_mip405_rev (void)
{
	unsigned char part, vers, pcbrev, var;

	get_pcbrev_var(&pcbrev,&var);
	part = in8 (PLD_PART_REG);
	vers = in8 (PLD_VERS_REG);
	printf ("Rev:   " BOARD_NAME "-%d Rev %c PLD %d Vers %d\n",
			var, pcbrev + 'A', part & 0x7F, vers);
}


extern int mk_date (char *, struct rtc_time *);

int last_stage_init (void)
{
	unsigned long stop;
	struct rtc_time newtm;
	char *s;

	/* write correct LED configuration */
	if (miiphy_write("ppc_4xx_eth0", 0x1, 0x14, 0x2402) != 0) {
		printf ("Error writing to the PHY\n");
	}
	/* since LED/CFG2 is not connected on the -2,
	 * write to correct capability information */
	if (miiphy_write("ppc_4xx_eth0", 0x1, 0x4, 0x01E1) != 0) {
		printf ("Error writing to the PHY\n");
	}
	print_mip405_rev ();
	stdio_print_current_devices ();
	check_env ();
	/* check if RTC time is valid */
	stop=get_timer(start);
	while(stop<1200) {   /* we wait 1.2 sec to check if the RTC is running */
		udelay(1000);
		stop=get_timer(start);
	}
	rtc_get (&newtm);
	if(tm.tm_sec==newtm.tm_sec) {
		s=getenv("defaultdate");
		if(!s)
			mk_date ("010112001970", &newtm);
		else
			if(mk_date (s, &newtm)!=0) {
				printf("RTC: Bad date format in defaultdate\n");
				return 0;
			}
		rtc_reset ();
		rtc_set(&newtm);
	}
	return 0;
}

/***************************************************************************
 * some helping routines
 */

int overwrite_console (void)
{
	/* return true if console should be overwritten */
	return ((in8(PLD_EXT_CONF_REG) & 0x1) == 0);
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

	printf ("PLD Part %d version %d\n", part & 0x7F, vers);
	printf ("Board Revision %c\n", ((cfg >> 4) & 0xf) + 'A');
	printf ("Population Options %d %d %d %d\n", (cfg) & 0x1,
			(cfg >> 1) & 0x1, (cfg >> 2) & 0x1, (cfg >> 3) & 0x1);
	printf ("User LED %s\n", (com_mode & 0x4) ? "on" : "off");
	printf ("UART Clocks %d\n", (com_mode >> 4) & 0x3);
#if !defined(CONFIG_TARGET_MIP405T)
	printf ("User Config Switch %d %d %d %d %d %d %d %d\n",
			(ext) & 0x1, (ext >> 1) & 0x1, (ext >> 2) & 0x1,
			(ext >> 3) & 0x1, (ext >> 4) & 0x1, (ext >> 5) & 0x1,
			(ext >> 6) & 0x1, (ext >> 7) & 0x1);
	printf ("SER1 uses handshakes %s\n",
			(ext & 0x80) ? "DTR/DSR" : "RTS/CTS");
#else
	printf ("User Config Switch %d %d %d %d %d %d %d %d\n",
			(ext) & 0x1, (ext >> 1) & 0x1, (ext >> 2) & 0x1,
			(ext >> 3) & 0x1, (ext >> 4) & 0x1, (ext >> 5) & 0x1,
			(ext >> 6) & 0x1,(ext >> 7) & 0x1);
#endif
	printf ("IDE Reset %s\n", (ext & 0x01) ? "asserted" : "not asserted");
	printf ("IRQs:\n");
	printf ("  PIIX INTR: %s\n", (irq_reg & 0x80) ? "inactive" : "active");
#if !defined(CONFIG_TARGET_MIP405T)
	printf ("  UART0 IRQ: %s\n", (irq_reg & 0x40) ? "inactive" : "active");
	printf ("  UART1 IRQ: %s\n", (irq_reg & 0x20) ? "inactive" : "active");
#endif
	printf ("  PIIX SMI:  %s\n", (irq_reg & 0x10) ? "inactive" : "active");
	printf ("  PIIX INIT: %s\n", (irq_reg & 0x8) ? "inactive" : "active");
	printf ("  PIIX NMI:  %s\n", (irq_reg & 0x4) ? "inactive" : "active");
}
