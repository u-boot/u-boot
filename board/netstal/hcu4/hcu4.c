/*
 *(C) Copyright 2005-2007 Netstal Maschinen AG
 *    Niklaus Giger (Niklaus.Giger@netstal.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include  <common.h>
#include  <ppc4xx.h>
#include  <asm/processor.h>
#include  <asm/io.h>
#include  <asm-ppc/u-boot.h>
#include  "../common/nm_bsp.c"

DECLARE_GLOBAL_DATA_PTR;

#define HCU_MACH_VERSIONS_REGISTER	(0x7C000000 + 0xF00000)

#define mtsdram(reg, data)  { mtdcr(memcfga,reg);mtdcr(memcfgd,data); }
#define mfsdram(value, reg) { mtdcr(memcfga,reg); value = mfdcr(memcfgd); }

#define SDRAM_LEN 32*1024*1024 /* 32 MB -RAM */

#define DO_UGLY_SDRAM_WORKAROUND

enum {
	/* HW_GENERATION_HCU wird nicht mehr unterstuetzt */
	HW_GENERATION_HCU2  = 0x10,
	HW_GENERATION_HCU3  = 0x10,
	HW_GENERATION_HCU4  = 0x20,
	HW_GENERATION_MCU   = 0x08,
	HW_GENERATION_MCU20 = 0x0a,
	HW_GENERATION_MCU25 = 0x09,
};

void sysLedSet(u32 value);
long int spd_sdram(int(read_spd)(uint addr));

#ifdef CONFIG_SPD_EEPROM
#define DEBUG
#endif

#if defined(DEBUG)
void show_sdram_registers(void);
#endif

/*
 * This function is run very early, out of flash, and before devices are
 * initialized. It is called by lib_ppc/board.c:board_init_f by virtue
 * of being in the init_sequence array.
 *
 * The SDRAM has been initialized already -- start.S:start called
 * init.S:init_sdram early on -- but it is not yet being used for
 * anything, not even stack. So be careful.
 */

#define CPC0_CR0	0xb1	/* Chip control register 0 */
#define CPC0_CR1        0xb2	/* Chip control register 1 */
/* Attention: If you want 1 microsecs times from the external oscillator
 * use  0x00804051. But this causes problems with u-boot and linux!
 */
#define CPC0_CR1_VALUE	0x00004051
#define CPC0_ECR	0xaa	/* Edge condition register */
#define EBC0_CFG	0x23	/* External Peripheral Control Register */
#define CPC0_EIRR	0xb6	/* External Interrupt Register */


int board_early_init_f (void)
{
	/*-------------------------------------------------------------------+
	| Interrupt controller setup for the HCU4 board.
	| Note: IRQ 0-15  405GP internally generated; high; level sensitive
	|       IRQ 16    405GP internally generated; low; level sensitive
	|       IRQ 17-24 RESERVED/UNUSED
	|       IRQ 31 (EXT IRQ 6) (unused)
	+-------------------------------------------------------------------*/
	mtdcr (uicsr, 0xFFFFFFFF); /* clear all ints */
	mtdcr (uicer, 0x00000000); /* disable all ints */
	mtdcr (uiccr, 0x00000000); /* set all to be non-critical */
	mtdcr (uicpr, 0xFFFFFF87); /* set int polarities */
	mtdcr (uictr, 0x10000000); /* set int trigger levels */
	mtdcr (uicsr, 0xFFFFFFFF); /* clear all ints */

	mtdcr(CPC0_CR1,  CPC0_CR1_VALUE);
	mtdcr(CPC0_ECR,  0x60606000);
	mtdcr(CPC0_EIRR, 0x7c000000);

	return 0;
}

#ifdef CONFIG_BOARD_PRE_INIT
int board_pre_init (void)
{
	return board_early_init_f ();
}
#endif

int checkboard (void)
{
	unsigned int j;
	u16 *boardVersReg = (u16 *) HCU_MACH_VERSIONS_REGISTER;
	u16 generation = *boardVersReg & 0xf0;
	u16 index      = *boardVersReg & 0x0f;

	/* Force /RTS to active. The board it not wired quite
	   correctly to use cts/rtc flow control, so just force the
	   /RST active and forget about it. */
	writeb (readb (0xef600404) | 0x03, 0xef600404);
	printf ("\nNetstal Maschinen AG ");
	if (generation == HW_GENERATION_HCU3)
		printf ("HCU3: index %d\n\n", index);
	else if (generation == HW_GENERATION_HCU4)
		printf ("HCU4: index %d\n\n", index);
	/* GPIO here noch nicht richtig initialisert !!! */
	sysLedSet(0);
	for (j = 0; j < 7; j++) {
		sysLedSet(1 << j);
		udelay(50 * 1000);
	}

	return 0;
}

u32 sysLedGet(void)
{
	return (~((*(u32 *)GPIO0_OR)) >> 23) & 0xff;
}

void sysLedSet(u32 value /* value to place in LEDs */)
{
	u32   tmp = ~value;
	u32   *ledReg;

	tmp = (tmp << 23) | 0x7FFFFF;
	ledReg = (u32 *)GPIO0_OR;
	*ledReg = tmp;
}

/*
 * sdram_init - Dummy implementation for start.S, spd_sdram  or initdram
 *		used for HCUx
 */
void sdram_init(void)
{
	return;
}

#if defined(DEBUG)
void show_sdram_registers(void)
{
	u32 value;

	printf ("SDRAM Controller Registers --\n");
	mfsdram(value, mem_mcopt1);
	printf ("    SDRAM0_CFG   : 0x%08x\n", value);
	mfsdram(value, mem_status);
	printf ("    SDRAM0_STATUS: 0x%08x\n", value);
	mfsdram(value, mem_mb0cf);
	printf ("    SDRAM0_B0CR  : 0x%08x\n", value);
	mfsdram(value, mem_mb1cf);
	printf ("    SDRAM0_B1CR  : 0x%08x\n", value);
	mfsdram(value, mem_sdtr1);
	printf ("    SDRAM0_TR    : 0x%08x\n", value);
	mfsdram(value, mem_rtr);
	printf ("    SDRAM0_RTR   : 0x%08x\n", value);
}
#endif

/*
 * this is even after checkboard. It returns the size of the SDRAM
 * that we have installed. This function is called by board_init_f
 * in lib_ppc/board.c to initialize the memory and return what I
 * found. These are default value, which will be overridden later.
 */

long int fixed_hcu4_sdram (int board_type)
{
#ifdef DEBUG
	printf (__FUNCTION__);
#endif
	/* disable memory controller */
	mtdcr (memcfga, mem_mcopt1);
	mtdcr (memcfgd, 0x00000000);

	udelay (500);

	/* Clear SDRAM0_BESR0 (Bus Error Syndrome Register) */
	mtdcr (memcfga, mem_besra);
	mtdcr (memcfgd, 0xffffffff);

	/* Clear SDRAM0_BESR1 (Bus Error Syndrome Register) */
	mtdcr (memcfga, mem_besrb);
	mtdcr (memcfgd, 0xffffffff);

	/* Clear SDRAM0_ECCCFG (disable ECC) */
	mtdcr (memcfga, mem_ecccf);
	mtdcr (memcfgd, 0x00000000);

	/* Clear SDRAM0_ECCESR (ECC Error Syndrome Register) */
	mtdcr (memcfga, mem_eccerr);
	mtdcr (memcfgd, 0xffffffff);

	/* Timing register: CASL=2, PTA=2, CTP=2, LDF=1, RFTA=5, RCD=2
	 * TODO ngngng
	 */
	mtdcr (memcfga, mem_sdtr1);
	mtdcr (memcfgd, 0x008a4015);

	/* Memory Bank 0 Config == BA=0x00000000, SZ=64M, AM=3, BE=1
	 * TODO ngngng
	 */
	mtdcr (memcfga, mem_mb0cf);
	mtdcr (memcfgd, 0x00062001);

	/* refresh timer = 0x400  */
	mtdcr (memcfga, mem_rtr);
	mtdcr (memcfgd, 0x04000000);

	/* Power management idle timer set to the default. */
	mtdcr (memcfga, mem_pmit);
	mtdcr (memcfgd, 0x07c00000);

	udelay (500);

	/* Enable banks (DCE=1, BPRF=1, ECCDD=1, EMDUL=1) TODO */
	mtdcr (memcfga, mem_mcopt1);
	mtdcr (memcfgd, 0x90800000);

#ifdef DEBUG
	printf ("%s: done\n", __FUNCTION__);
#endif
	return SDRAM_LEN;
}

/*---------------------------------------------------------------------------+
 * getSerialNr
 *---------------------------------------------------------------------------*/
static u32 getSerialNr(void)
{
	u32 *serial = (u32 *)CFG_FLASH_BASE;

	if (*serial == 0xffffffff)
		return get_ticks();

	return *serial;
}


/*---------------------------------------------------------------------------+
 * misc_init_r.
 *---------------------------------------------------------------------------*/

int misc_init_r(void)
{
	char *s = getenv("ethaddr");
	char *e;
	int i;
	u32 serial = getSerialNr();

	for (i = 0; i < 6; ++i) {
		gd->bd->bi_enetaddr[i] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}

	if (gd->bd->bi_enetaddr[3] == 0 &&
	    gd->bd->bi_enetaddr[4] == 0 &&
	    gd->bd->bi_enetaddr[5] == 0) {
		char ethaddr[22];
		/* [0..3] Must be in sync with CONFIG_ETHADDR */
		gd->bd->bi_enetaddr[0] = 0x00;
		gd->bd->bi_enetaddr[1] = 0x60;
		gd->bd->bi_enetaddr[2] = 0x13;
		gd->bd->bi_enetaddr[3] = (serial          >> 16) & 0xff;
		gd->bd->bi_enetaddr[4] = (serial          >>  8) & 0xff;
		gd->bd->bi_enetaddr[5] = (serial          >>  0) & 0xff;
		sprintf (ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X\0",
			 gd->bd->bi_enetaddr[0], gd->bd->bi_enetaddr[1],
			 gd->bd->bi_enetaddr[2], gd->bd->bi_enetaddr[3],
			 gd->bd->bi_enetaddr[4], gd->bd->bi_enetaddr[5]) ;
		printf("%s: Setting eth %s serial 0x%x\n",  __FUNCTION__,
		       ethaddr, serial);
		setenv ("ethaddr", ethaddr);
	}
	return 0;
}

#ifdef  DO_UGLY_SDRAM_WORKAROUND
#include "i2c.h"

void set_spd_default_value(unsigned int spd_addr,uchar def_val)
{
	uchar value;
	int res = i2c_read(SPD_EEPROM_ADDRESS, spd_addr, 1, &value, 1) ;

	if (res == 0 && value == 0xff) {
		res = i2c_write(SPD_EEPROM_ADDRESS,
				spd_addr, 1, &def_val, 1) ;
#ifdef DEBUG
		printf("%s: Setting spd offset %3d to %3d res %d\n",
		       __FUNCTION__, spd_addr,  def_val, res);
#endif
	}
}
#endif

long int initdram(int board_type)
{
	long dram_size = 0;

#if !defined(CONFIG_SPD_EEPROM)
	dram_size = fixed_hcu4_sdram();
#else
#ifdef  DO_UGLY_SDRAM_WORKAROUND
	/* Workaround if you have no working I2C-EEPROM-SPD-configuration */
	i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);
	set_spd_default_value(2,  4); /* SDRAM Type */
	set_spd_default_value(7,  0); /* module width, high byte */
	set_spd_default_value(12, 1); /* Refresh or 0x81 */

	/* Only correct for HCU3 with 32 MB RAM*/
	/* Number of bytes used by module manufacturer */
	set_spd_default_value( 0, 128);
	set_spd_default_value( 1, 11 ); /* Total SPD memory size */
	set_spd_default_value( 2, 4  ); /* Memory type */
	set_spd_default_value( 3, 12 ); /* Number of row address bits */
	set_spd_default_value( 4, 9  ); /* Number of column address bits */
	set_spd_default_value( 5, 1  ); /* Number of module rows */
	set_spd_default_value( 6, 32 ); /* Module data width, LSB */
	set_spd_default_value( 7, 0  ); /* Module data width, MSB */
	set_spd_default_value( 8, 1  ); /* Module interface signal levels */
	/* SDRAM cycle time for highest CL (Tclk) */
	set_spd_default_value( 9, 112);
	/* SDRAM access time from clock for highest CL (Tac) */
	set_spd_default_value(10, 84 );
	set_spd_default_value(11, 2  ); /* Module configuration type */
	set_spd_default_value(12, 128); /* Refresh rate/type */
	set_spd_default_value(13, 16 ); /* Primary SDRAM width */
	set_spd_default_value(14, 8  ); /* Error Checking SDRAM width */
	/* SDRAM device attributes, min clock delay for back to back */
	/*random column addresses (Tccd) */
	set_spd_default_value(15, 1  );
	/* SDRAM device attributes, burst lengths supported */
	set_spd_default_value(16, 143);
	/* SDRAM device attributes, number of banks on SDRAM device */
	set_spd_default_value(17, 4  );
	/* SDRAM device attributes, CAS latency */
	set_spd_default_value(18, 6  );
	/* SDRAM device attributes, CS latency */
	set_spd_default_value(19, 1  );
	/* SDRAM device attributes, WE latency */
	set_spd_default_value(20, 1  );
	set_spd_default_value(21, 0  ); /* SDRAM module attributes */
	/* SDRAM device attributes, general */
	set_spd_default_value(22, 14 );
	/* SDRAM cycle time for 2nd highest CL (Tclk) */
	set_spd_default_value(23, 117);
	/* SDRAM access time from clock for2nd highest CL (Tac) */
	set_spd_default_value(24, 84 );
	/* SDRAM cycle time for 3rd highest CL (Tclk) */
	set_spd_default_value(25, 0  );
	/* SDRAM access time from clock for3rd highest CL (Tac) */
	set_spd_default_value(26, 0  );
	set_spd_default_value(27, 15 ); /* Minimum row precharge time (Trp) */
	/* Minimum row active to row active delay (Trrd) */
	set_spd_default_value(28, 14 );
	set_spd_default_value(29, 15 ); /* Minimum CAS to RAS delay (Trcd) */
	set_spd_default_value(30, 37 ); /* Minimum RAS pulse width (Tras) */
	set_spd_default_value(31, 8  ); /* Module bank density */
	/* Command and Address signal input setup time */
	set_spd_default_value(32, 21 );
	/* Command and Address signal input hold time */
	set_spd_default_value(33, 8  );
	set_spd_default_value(34, 21 ); /* Data signal input setup time */
	set_spd_default_value(35, 8  ); /* Data signal input hold time */
#endif  /* DO_UGLY_SDRAM_WORKAROUND */
	dram_size = spd_sdram(0);
#endif

#ifdef DEBUG
	show_sdram_registers();
#endif

#if defined(CFG_DRAM_TEST)
	bcu4_testdram(dram_size);
	printf("%s %d MB of SDRAM\n", __FUNCTION__, dram_size/(1024*1024));
#endif

	return dram_size;
}
