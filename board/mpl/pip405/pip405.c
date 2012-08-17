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

#include <common.h>
#include "pip405.h"
#include <asm/processor.h>
#include <i2c.h>
#include <stdio_dev.h>
#include "../common/isa.h"
#include "../common/common_util.h"

DECLARE_GLOBAL_DATA_PTR;

#undef SDRAM_DEBUG

#define FALSE           0
#define TRUE            1

/* stdlib.h causes some compatibility problems; should fixe these! -- wd */
#ifndef __ldiv_t_defined
typedef struct {
	long int quot;		/* Quotient */
	long int rem;		/* Remainder    */
} ldiv_t;
extern ldiv_t ldiv (long int __numer, long int __denom);

# define __ldiv_t_defined	1
#endif


typedef enum {
	SDRAM_NO_ERR,
	SDRAM_SPD_COMM_ERR,
	SDRAM_SPD_CHKSUM_ERR,
	SDRAM_UNSUPPORTED_ERR,
	SDRAM_UNKNOWN_ERR
} SDRAM_ERR;

typedef struct {
	const unsigned char mode;
	const unsigned char row;
	const unsigned char col;
	const unsigned char bank;
} SDRAM_SETUP;

static const SDRAM_SETUP sdram_setup_table[] = {
	{1, 11, 9, 2},
	{1, 11, 10, 2},
	{2, 12, 9, 4},
	{2, 12, 10, 4},
	{3, 13, 9, 4},
	{3, 13, 10, 4},
	{3, 13, 11, 4},
	{4, 12, 8, 2},
	{4, 12, 8, 4},
	{5, 11, 8, 2},
	{5, 11, 8, 4},
	{6, 13, 8, 2},
	{6, 13, 8, 4},
	{7, 13, 9, 2},
	{7, 13, 10, 2},
	{0, 0, 0, 0}
};

static const unsigned char cal_indextable[] = {
	9, 23, 25
};


/*
 * translate ns.ns/10 coding of SPD timing values
 * into 10 ps unit values
 */

unsigned short NS10to10PS (unsigned char spd_byte, unsigned char spd_version)
{
	unsigned short ns, ns10;

	/* isolate upper nibble */
	ns = (spd_byte >> 4) & 0x0F;
	/* isolate lower nibble */
	ns10 = (spd_byte & 0x0F);

	return (ns * 100 + ns10 * 10);
}

/*
 * translate ns.ns/4 coding of SPD timing values
 * into 10 ps unit values
 */

unsigned short NS4to10PS (unsigned char spd_byte, unsigned char spd_version)
{
	unsigned short ns, ns4;

	/* isolate upper 6 bits */
	ns = (spd_byte >> 2) & 0x3F;
	/* isloate lower 2 bits */
	ns4 = (spd_byte & 0x03);

	return (ns * 100 + ns4 * 25);
}

/*
 * translate ns coding of SPD timing values
 * into 10 ps unit values
 */

unsigned short NSto10PS (unsigned char spd_byte)
{
	return (spd_byte * 100);
}

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

int board_early_init_f (void)
{
	unsigned char datain[128];
	unsigned long sdram_size = 0;
	SDRAM_SETUP *t = (SDRAM_SETUP *) sdram_setup_table;
	unsigned long memclk;
	unsigned long tmemclk = 0;
	unsigned long tmp, bank, baseaddr, bank_size;
	unsigned short i;
	unsigned char rows, cols, banks, sdram_banks, density;
	unsigned char supported_cal, trp_clocks, trcd_clocks, tras_clocks,
		trc_clocks;
	unsigned char cal_index, cal_val, spd_version, spd_chksum;
	unsigned char buf[8];
#ifdef SDRAM_DEBUG
	unsigned char tctp_clocks;
#endif

	/* set up the config port */
	mtdcr (EBC0_CFGADDR, PB7AP);
	mtdcr (EBC0_CFGDATA, CONFIG_PORT_AP);
	mtdcr (EBC0_CFGADDR, PB7CR);
	mtdcr (EBC0_CFGDATA, CONFIG_PORT_CR);

	memclk = get_bus_freq (tmemclk);
	tmemclk = 1000000000 / (memclk / 100);	/* in 10 ps units */

#ifdef SDRAM_DEBUG
	(void) get_clocks ();
	gd->baudrate = 9600;
	serial_init ();
	serial_puts ("\nstart SDRAM Setup\n");
#endif

	/* Read Serial Presence Detect Information */
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	for (i = 0; i < 128; i++)
		datain[i] = 127;
	i2c_read(SPD_EEPROM_ADDRESS,0,1,datain,128);
#ifdef SDRAM_DEBUG
	serial_puts ("\ni2c_read returns ");
	write_hex (i);
	serial_puts ("\n");
#endif

#ifdef SDRAM_DEBUG
	for (i = 0; i < 128; i++) {
		write_hex (datain[i]);
		serial_puts (" ");
		if (((i + 1) % 16) == 0)
			serial_puts ("\n");
	}
	serial_puts ("\n");
#endif
	spd_chksum = 0;
	for (i = 0; i < 63; i++) {
		spd_chksum += datain[i];
	}							/* endfor */
	if (datain[63] != spd_chksum) {
#ifdef SDRAM_DEBUG
		serial_puts ("SPD chksum: 0x");
		write_hex (datain[63]);
		serial_puts (" != calc. chksum: 0x");
		write_hex (spd_chksum);
		serial_puts ("\n");
#endif
		SDRAM_err ("SPD checksum Error");
	}
	/* SPD seems to be ok, use it */

	/* get SPD version */
	spd_version = datain[62];

	/* do some sanity checks on the kind of RAM */
	if ((datain[0] < 0x80) ||	/* less than 128 valid bytes in SPD */
		(datain[2] != 0x04) ||	/* if not SDRAM */
		(!((datain[6] == 0x40) || (datain[6] == 0x48))) ||	/* or not (64 Bit or 72 Bit)  */
		(datain[7] != 0x00) || (datain[8] != 0x01) ||	/* or not LVTTL signal levels */
		(datain[126] == 0x66))	/* or a 66MHz modules */
		SDRAM_err ("unsupported SDRAM");
#ifdef SDRAM_DEBUG
	serial_puts ("SDRAM sanity ok\n");
#endif

	/* get number of rows/cols/banks out of byte 3+4+5 */
	rows = datain[3];
	cols = datain[4];
	banks = datain[5];

	/* get number of SDRAM banks out of byte 17 and
	   supported CAS latencies out of byte 18 */
	sdram_banks = datain[17];
	supported_cal = datain[18] & ~0x81;

	while (t->mode != 0) {
		if ((t->row == rows) && (t->col == cols)
			&& (t->bank == sdram_banks))
			break;
		t++;
	}							/* endwhile */

#ifdef SDRAM_DEBUG
	serial_puts ("rows: ");
	write_hex (rows);
	serial_puts (" cols: ");
	write_hex (cols);
	serial_puts (" banks: ");
	write_hex (banks);
	serial_puts (" mode: ");
	write_hex (t->mode);
	serial_puts ("\n");
#endif
	if (t->mode == 0)
		SDRAM_err ("unsupported SDRAM");
	/* get tRP, tRCD, tRAS and density from byte 27+29+30+31 */
#ifdef SDRAM_DEBUG
	serial_puts ("tRP: ");
	write_hex (datain[27]);
	serial_puts ("\ntRCD: ");
	write_hex (datain[29]);
	serial_puts ("\ntRAS: ");
	write_hex (datain[30]);
	serial_puts ("\n");
#endif

	trp_clocks = (NSto10PS (datain[27]) + (tmemclk - 1)) / tmemclk;
	trcd_clocks = (NSto10PS (datain[29]) + (tmemclk - 1)) / tmemclk;
	tras_clocks = (NSto10PS (datain[30]) + (tmemclk - 1)) / tmemclk;
	density = datain[31];

	/* trc_clocks is sum of trp_clocks + tras_clocks */
	trc_clocks = trp_clocks + tras_clocks;

#ifdef SDRAM_DEBUG
	/* ctp = ((trp + tras) - trp - trcd) => tras - trcd */
	tctp_clocks =
			((NSto10PS (datain[30]) - NSto10PS (datain[29])) +
			 (tmemclk - 1)) / tmemclk;

	serial_puts ("c_RP: ");
	write_hex (trp_clocks);
	serial_puts ("\nc_RCD: ");
	write_hex (trcd_clocks);
	serial_puts ("\nc_RAS: ");
	write_hex (tras_clocks);
	serial_puts ("\nc_RC: (RP+RAS): ");
	write_hex (trc_clocks);
	serial_puts ("\nc_CTP: ((RP+RAS)-RP-RCD): ");
	write_hex (tctp_clocks);
	serial_puts ("\nt_CTP: RAS - RCD: ");
	write_hex ((unsigned
				char) ((NSto10PS (datain[30]) -
						NSto10PS (datain[29])) >> 8));
	write_hex ((unsigned char) (NSto10PS (datain[30]) - NSto10PS (datain[29])));
	serial_puts ("\ntmemclk: ");
	write_hex ((unsigned char) (tmemclk >> 8));
	write_hex ((unsigned char) (tmemclk));
	serial_puts ("\n");
#endif


	cal_val = 255;
	for (i = 6, cal_index = 0; (i > 0) && (cal_index < 3); i--) {
		/* is this CAS latency supported ? */
		if ((supported_cal >> i) & 0x01) {
			buf[0] = datain[cal_indextable[cal_index]];
			if (cal_index < 2) {
				if (NS10to10PS (buf[0], spd_version) <= tmemclk)
					cal_val = i;
			} else {
				/* SPD bytes 25+26 have another format */
				if (NS4to10PS (buf[0], spd_version) <= tmemclk)
					cal_val = i;
			}	/* endif */
			cal_index++;
		}	/* endif */
	}	/* endfor */
#ifdef SDRAM_DEBUG
	serial_puts ("CAL: ");
	write_hex (cal_val + 1);
	serial_puts ("\n");
#endif

	if (cal_val == 255)
		SDRAM_err ("unsupported SDRAM");

	/* get SDRAM timing register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_TR);
	tmp = mfdcr (SDRAM0_CFGDATA) & ~0x018FC01F;
	/* insert CASL value */
/*  tmp |= ((unsigned long)cal_val) << 23; */
	tmp |= ((unsigned long) cal_val) << 23;
	/* insert PTA value */
	tmp |= ((unsigned long) (trp_clocks - 1)) << 18;
	/* insert CTP value */
/*  tmp |= ((unsigned long)(trc_clocks - trp_clocks - trcd_clocks - 1)) << 16; */
	tmp |= ((unsigned long) (trc_clocks - trp_clocks - trcd_clocks)) << 16;
	/* insert LDF (always 01) */
	tmp |= ((unsigned long) 0x01) << 14;
	/* insert RFTA value */
	tmp |= ((unsigned long) (trc_clocks - 4)) << 2;
	/* insert RCD value */
	tmp |= ((unsigned long) (trcd_clocks - 1)) << 0;

#ifdef SDRAM_DEBUG
	serial_puts ("sdtr: ");
	write_4hex (tmp);
	serial_puts ("\n");
#endif

	/* write SDRAM timing register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_TR);
	mtdcr (SDRAM0_CFGDATA, tmp);
	baseaddr = CONFIG_SYS_SDRAM_BASE;
	bank_size = (((unsigned long) density) << 22) / 2;
	/* insert AM value */
	tmp = ((unsigned long) t->mode - 1) << 13;
	/* insert SZ value; */
	switch (bank_size) {
	case 0x00400000:
		tmp |= ((unsigned long) 0x00) << 17;
		break;
	case 0x00800000:
		tmp |= ((unsigned long) 0x01) << 17;
		break;
	case 0x01000000:
		tmp |= ((unsigned long) 0x02) << 17;
		break;
	case 0x02000000:
		tmp |= ((unsigned long) 0x03) << 17;
		break;
	case 0x04000000:
		tmp |= ((unsigned long) 0x04) << 17;
		break;
	case 0x08000000:
		tmp |= ((unsigned long) 0x05) << 17;
		break;
	case 0x10000000:
		tmp |= ((unsigned long) 0x06) << 17;
		break;
	default:
		SDRAM_err ("unsupported SDRAM");
	}	/* endswitch */
	/* get SDRAM bank 0 register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B0CR);
	bank = mfdcr (SDRAM0_CFGDATA) & ~0xFFCEE001;
	bank |= (baseaddr | tmp | 0x01);
#ifdef SDRAM_DEBUG
	serial_puts ("bank0: baseaddr: ");
	write_4hex (baseaddr);
	serial_puts (" banksize: ");
	write_4hex (bank_size);
	serial_puts (" mb0cf: ");
	write_4hex (bank);
	serial_puts ("\n");
#endif
	baseaddr += bank_size;
	sdram_size += bank_size;

	/* write SDRAM bank 0 register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B0CR);
	mtdcr (SDRAM0_CFGDATA, bank);

	/* get SDRAM bank 1 register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B1CR);
	bank = mfdcr (SDRAM0_CFGDATA) & ~0xFFCEE001;
	sdram_size = 0;

#ifdef SDRAM_DEBUG
	serial_puts ("bank1: baseaddr: ");
	write_4hex (baseaddr);
	serial_puts (" banksize: ");
	write_4hex (bank_size);
#endif
	if (banks == 2) {
		bank |= (baseaddr | tmp | 0x01);
		baseaddr += bank_size;
		sdram_size += bank_size;
	}	/* endif */
#ifdef SDRAM_DEBUG
	serial_puts (" mb1cf: ");
	write_4hex (bank);
	serial_puts ("\n");
#endif
	/* write SDRAM bank 1 register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B1CR);
	mtdcr (SDRAM0_CFGDATA, bank);

	/* get SDRAM bank 2 register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B2CR);
	bank = mfdcr (SDRAM0_CFGDATA) & ~0xFFCEE001;

	bank |= (baseaddr | tmp | 0x01);

#ifdef SDRAM_DEBUG
	serial_puts ("bank2: baseaddr: ");
	write_4hex (baseaddr);
	serial_puts (" banksize: ");
	write_4hex (bank_size);
	serial_puts (" mb2cf: ");
	write_4hex (bank);
	serial_puts ("\n");
#endif

	baseaddr += bank_size;
	sdram_size += bank_size;

	/* write SDRAM bank 2 register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B2CR);
	mtdcr (SDRAM0_CFGDATA, bank);

	/* get SDRAM bank 3 register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B3CR);
	bank = mfdcr (SDRAM0_CFGDATA) & ~0xFFCEE001;

#ifdef SDRAM_DEBUG
	serial_puts ("bank3: baseaddr: ");
	write_4hex (baseaddr);
	serial_puts (" banksize: ");
	write_4hex (bank_size);
#endif

	if (banks == 2) {
		bank |= (baseaddr | tmp | 0x01);
		baseaddr += bank_size;
		sdram_size += bank_size;
	}
	/* endif */
#ifdef SDRAM_DEBUG
	serial_puts (" mb3cf: ");
	write_4hex (bank);
	serial_puts ("\n");
#endif

	/* write SDRAM bank 3 register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B3CR);
	mtdcr (SDRAM0_CFGDATA, bank);


	/* get SDRAM refresh interval register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_RTR);
	tmp = mfdcr (SDRAM0_CFGDATA) & ~0x3FF80000;

	if (tmemclk < NSto10PS (16))
		tmp |= 0x05F00000;
	else
		tmp |= 0x03F80000;

	/* write SDRAM refresh interval register */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_RTR);
	mtdcr (SDRAM0_CFGDATA, tmp);

	/* enable SDRAM controller with no ECC, 32-bit SDRAM width, 16 byte burst */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_CFG);
	tmp = (mfdcr (SDRAM0_CFGDATA) & ~0xFFE00000) | 0x80E00000;
	mtdcr (SDRAM0_CFGADDR, SDRAM0_CFG);
	mtdcr (SDRAM0_CFGDATA, tmp);


   /*-------------------------------------------------------------------------+
   | Interrupt controller setup for the PIP405 board.
   | Note: IRQ 0-15  405GP internally generated; active high; level sensitive
   |       IRQ 16    405GP internally generated; active low; level sensitive
   |       IRQ 17-24 RESERVED
   |       IRQ 25 (EXT IRQ 0) SouthBridg; active low; level sensitive
   |       IRQ 26 (EXT IRQ 1) NMI: active low; level sensitive
   |       IRQ 27 (EXT IRQ 2) SMI: active Low; level sensitive
   |       IRQ 28 (EXT IRQ 3) PCI SLOT 3; active low; level sensitive
   |       IRQ 29 (EXT IRQ 4) PCI SLOT 2; active low; level sensitive
   |       IRQ 30 (EXT IRQ 5) PCI SLOT 1; active low; level sensitive
   |       IRQ 31 (EXT IRQ 6) PCI SLOT 0; active low; level sensitive
   | Note for PIP405 board:
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
/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	char s[50];
	unsigned char bc;
	int i;
	backup_t *b = (backup_t *) s;

	puts ("Board: ");

	i = getenv_f("serial#", (char *)s, 32);
	if ((i == 0) || strncmp ((char *)s, "PIP405", 6)) {
		get_backup_values (b);
		if (strncmp (b->signature, "MPL\0", 4) != 0) {
			puts ("### No HW ID - assuming PIP405");
		} else {
			b->serial_name[6] = 0;
			printf ("%s SN: %s", b->serial_name,
				&b->serial_name[7]);
		}
	} else {
		s[6] = 0;
		printf ("%s SN: %s", s, &s[7]);
	}
	bc = in8 (CONFIG_PORT_ADDR);
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

phys_size_t initdram (int board_type)
{
	unsigned long bank_reg[4], tmp, bank_size;
	int i, ds;
	unsigned long TotalSize;

	ds = 0;
	/* since the DRAM controller is allready set up,
	 * calculate the size with the bank registers
	 */
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
		} else
			ds = 1;
	}
	if (ds == 1)
		printf ("single-sided DIMM ");
	else
		printf ("double-sided DIMM ");
	test_dram (TotalSize * 1024 * 1024);
	/* bank 2 (SDRAM Clock 2) is not usable if 133MHz SDRAM IF */
	(void) get_clocks();
	if (gd->cpu_clk > 220000000)
		TotalSize /= 2;
	return (TotalSize * 1024 * 1024);
}

/* ------------------------------------------------------------------------- */


static int test_dram (unsigned long ramsize)
{
	/* not yet implemented */
	return (1);
}

int misc_init_r (void)
{
	/* adjust flash start and size as well as the offset */
	gd->bd->bi_flashstart=0-flash_info[0].size;
	gd->bd->bi_flashsize=flash_info[0].size-CONFIG_SYS_MONITOR_LEN;
	gd->bd->bi_flashoffset=0;

	/* if PIP405 has booted from PCI, reset CCR0[24] as described in errata PCI_18 */
	if (mfdcr(CPC0_PSR) & PSR_ROM_LOC)
	       mtspr(SPRN_CCR0, (mfspr(SPRN_CCR0) & ~0x80));

	return (0);
}

/***************************************************************************
 * some helping routines
 */

int overwrite_console (void)
{
	return (in8 (CONFIG_PORT_ADDR) & 0x1);	/* return TRUE if console should be overwritten */
}


extern int isa_init (void);


void print_pip405_rev (void)
{
	unsigned char part, vers, cfg;

	part = in8 (PLD_PART_REG);
	vers = in8 (PLD_VERS_REG);
	cfg = in8 (PLD_BOARD_CFG_REG);
	printf ("Rev:   PIP405-%d Rev %c PLD%d %d PLD%d %d\n",
			16 - ((cfg >> 4) & 0xf), (cfg & 0xf) + 'A', part & 0xf,
			vers & 0xf, (part >> 4) & 0xf, (vers >> 4) & 0xf);
}

extern void check_env(void);


int last_stage_init (void)
{
	print_pip405_rev ();
	isa_init ();
	stdio_print_current_devices ();
	check_env();
	return 0;
}

/************************************************************************
* Print PIP405 Info
************************************************************************/
void print_pip405_info (void)
{
	unsigned char part, vers, cfg, ledu, sysman, flashcom, can, serpwr,
			compwr, nicvga, scsirst;

	part = in8 (PLD_PART_REG);
	vers = in8 (PLD_VERS_REG);
	cfg = in8 (PLD_BOARD_CFG_REG);
	ledu = in8 (PLD_LED_USER_REG);
	sysman = in8 (PLD_SYS_MAN_REG);
	flashcom = in8 (PLD_FLASH_COM_REG);
	can = in8 (PLD_CAN_REG);
	serpwr = in8 (PLD_SER_PWR_REG);
	compwr = in8 (PLD_COM_PWR_REG);
	nicvga = in8 (PLD_NIC_VGA_REG);
	scsirst = in8 (PLD_SCSI_RST_REG);
	printf ("PLD Part %d version %d\n",
		part & 0xf, vers & 0xf);
	printf ("PLD Part %d version %d\n",
		(part >> 4) & 0xf, (vers >> 4) & 0xf);
	printf ("Board Revision %c\n", (cfg & 0xf) + 'A');
	printf ("Population Options %d %d %d %d\n",
		(cfg >> 4) & 0x1, (cfg >> 5) & 0x1,
		(cfg >> 6) & 0x1, (cfg >> 7) & 0x1);
	printf ("User LED0 %s User LED1 %s\n",
		((ledu & 0x1) == 0x1) ? "on" : "off",
		((ledu & 0x2) == 0x2) ? "on" : "off");
	printf ("Additionally Options %d %d\n",
		(ledu >> 2) & 0x1, (ledu >> 3) & 0x1);
	printf ("User Config Switch %d %d %d %d\n",
		(ledu >> 4) & 0x1, (ledu >> 5) & 0x1,
		(ledu >> 6) & 0x1, (ledu >> 7) & 0x1);
	switch (sysman & 0x3) {
	case 0:
		printf ("PCI Clocks are running\n");
		break;
	case 1:
		printf ("PCI Clocks are stopped in POS State\n");
		break;
	case 2:
		printf ("PCI Clocks are stopped when PCI_STP# is asserted\n");
		break;
	case 3:
		printf ("PCI Clocks are stopped\n");
		break;
	}
	switch ((sysman >> 2) & 0x3) {
	case 0:
		printf ("Main Clocks are running\n");
		break;
	case 1:
		printf ("Main Clocks are stopped in POS State\n");
		break;
	case 2:
	case 3:
		printf ("PCI Clocks are stopped\n");
		break;
	}
	printf ("INIT asserts %sINT2# (SMI)\n",
			((sysman & 0x10) == 0x10) ? "" : "not ");
	printf ("INIT asserts %sINT1# (NMI)\n",
			((sysman & 0x20) == 0x20) ? "" : "not ");
	printf ("INIT occured %d\n", (sysman >> 6) & 0x1);
	printf ("SER1 is routed to %s\n",
			((flashcom & 0x1) == 0x1) ? "RS485" : "RS232");
	printf ("COM2 is routed to %s\n",
			((flashcom & 0x2) == 0x2) ? "RS485" : "RS232");
	printf ("RS485 is configured as %s duplex\n",
			((flashcom & 0x4) == 0x4) ? "full" : "half");
	printf ("RS485 is connected to %s\n",
			((flashcom & 0x8) == 0x8) ? "COM1" : "COM2");
	printf ("SER1 uses handshakes %s\n",
			((flashcom & 0x10) == 0x10) ? "DTR/DSR" : "RTS/CTS");
	printf ("Bootflash is %swriteprotected\n",
			((flashcom & 0x20) == 0x20) ? "not " : "");
	printf ("Bootflash VPP is %s\n",
			((flashcom & 0x40) == 0x40) ? "on" : "off");
	printf ("Bootsector is %swriteprotected\n",
			((flashcom & 0x80) == 0x80) ? "not " : "");
	switch ((can) & 0x3) {
	case 0:
		printf ("CAN Controller is on address 0x1000..0x10FF\n");
		break;
	case 1:
		printf ("CAN Controller is on address 0x8000..0x80FF\n");
		break;
	case 2:
		printf ("CAN Controller is on address 0xE000..0xE0FF\n");
		break;
	case 3:
		printf ("CAN Controller is disabled\n");
		break;
	}
	switch ((can >> 2) & 0x3) {
	case 0:
		printf ("CAN Controller Reset is ISA Reset\n");
		break;
	case 1:
		printf ("CAN Controller Reset is ISA Reset and POS State\n");
		break;
	case 2:
	case 3:
		printf ("CAN Controller is in reset\n");
		break;
	}
	if (((can >> 4) < 3) || ((can >> 4) == 8) || ((can >> 4) == 13))
		printf ("CAN Interrupt is disabled\n");
	else
		printf ("CAN Interrupt is ISA INT%d\n", (can >> 4) & 0xf);
	switch (serpwr & 0x3) {
	case 0:
		printf ("SER0 Drivers are enabled\n");
		break;
	case 1:
		printf ("SER0 Drivers are disabled in the POS state\n");
		break;
	case 2:
	case 3:
		printf ("SER0 Drivers are disabled\n");
		break;
	}
	switch ((serpwr >> 2) & 0x3) {
	case 0:
		printf ("SER1 Drivers are enabled\n");
		break;
	case 1:
		printf ("SER1 Drivers are disabled in the POS state\n");
		break;
	case 2:
	case 3:
		printf ("SER1 Drivers are disabled\n");
		break;
	}
	switch (compwr & 0x3) {
	case 0:
		printf ("COM1 Drivers are enabled\n");
		break;
	case 1:
		printf ("COM1 Drivers are disabled in the POS state\n");
		break;
	case 2:
	case 3:
		printf ("COM1 Drivers are disabled\n");
		break;
	}
	switch ((compwr >> 2) & 0x3) {
	case 0:
		printf ("COM2 Drivers are enabled\n");
		break;
	case 1:
		printf ("COM2 Drivers are disabled in the POS state\n");
		break;
	case 2:
	case 3:
		printf ("COM2 Drivers are disabled\n");
		break;
	}
	switch ((nicvga) & 0x3) {
	case 0:
		printf ("PHY is running\n");
		break;
	case 1:
		printf ("PHY is in Power save mode in POS state\n");
		break;
	case 2:
	case 3:
		printf ("PHY is in Power save mode\n");
		break;
	}
	switch ((nicvga >> 2) & 0x3) {
	case 0:
		printf ("VGA is running\n");
		break;
	case 1:
		printf ("VGA is in Power save mode in POS state\n");
		break;
	case 2:
	case 3:
		printf ("VGA is in Power save mode\n");
		break;
	}
	printf ("PHY is %sreseted\n", ((nicvga & 0x10) == 0x10) ? "" : "not ");
	printf ("VGA is %sreseted\n", ((nicvga & 0x20) == 0x20) ? "" : "not ");
	printf ("Reserved Configuration is %d %d\n", (nicvga >> 6) & 0x1,
			(nicvga >> 7) & 0x1);
	switch ((scsirst) & 0x3) {
	case 0:
		printf ("SCSI Controller is running\n");
		break;
	case 1:
		printf ("SCSI Controller is in Power save mode in POS state\n");
		break;
	case 2:
	case 3:
		printf ("SCSI Controller is in Power save mode\n");
		break;
	}
	printf ("SCSI termination is %s\n",
			((scsirst & 0x4) == 0x4) ? "disabled" : "enabled");
	printf ("SCSI Controller is %sreseted\n",
			((scsirst & 0x10) == 0x10) ? "" : "not ");
	printf ("IDE disks are %sreseted\n",
			((scsirst & 0x20) == 0x20) ? "" : "not ");
	printf ("ISA Bus is %sreseted\n",
			((scsirst & 0x40) == 0x40) ? "" : "not ");
	printf ("Super IO is %sreseted\n",
			((scsirst & 0x80) == 0x80) ? "" : "not ");
}

void user_led0 (unsigned char on)
{
	if (on == TRUE)
		out8 (PLD_LED_USER_REG, (in8 (PLD_LED_USER_REG) | 0x1));
	else
		out8 (PLD_LED_USER_REG, (in8 (PLD_LED_USER_REG) & 0xfe));
}

void user_led1 (unsigned char on)
{
	if (on == TRUE)
		out8 (PLD_LED_USER_REG, (in8 (PLD_LED_USER_REG) | 0x2));
	else
		out8 (PLD_LED_USER_REG, (in8 (PLD_LED_USER_REG) & 0xfd));
}

void ide_set_reset (int idereset)
{
	/* if reset = 1 IDE reset will be asserted */
	unsigned char resreg;

	resreg = in8 (PLD_SCSI_RST_REG);
	if (idereset == 1)
		resreg |= 0x20;
	else {
		udelay(10000);
		resreg &= 0xdf;
	}
	out8 (PLD_SCSI_RST_REG, resreg);
}
