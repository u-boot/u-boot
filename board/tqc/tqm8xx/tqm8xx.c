/*
 * (C) Copyright 2000-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <hwconfig.h>
#include <mpc8xx.h>
#ifdef CONFIG_PS2MULT
#include <ps2mult.h>
#endif

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif

extern flash_info_t flash_info[];	/* FLASH chips info */

DECLARE_GLOBAL_DATA_PTR;

static long int dram_size (long int, long int *, long int);

#define	_NOT_USED_	0xFFFFFFFF

/* UPM initialization table for SDRAM: 40, 50, 66 MHz CLKOUT @ CAS latency 2, tWR=2 */
const uint sdram_table[] =
{
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x11AF7C04, 0xEFBAFC00,
	0x1FF5FC47, /* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
		    0x1FF5FC34, 0xEFEABC34, 0x1FB57C35, /* last */
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x10AF7C04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBAFC00, 0x1FF5FC47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEABBC00, 0x11B77C04, 0xEFFAFC44,
	0x1FF5FC47, /* last */
		    _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEABBC00, 0x10A77C00, 0xF0AFFC00,
	0xF0AFFC00, 0xF0AFFC04, 0xE1BAFC44, 0x1FF5FC47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1FFD7C84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, /* last */
				_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0xFFFFFC07, /* last */
		    _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Test TQ ID string (TQM8xx...)
 * If present, check for "L" type (no second DRAM bank),
 * otherwise "L" type is assumed as default.
 *
 * Set board_type to 'L' for "L" type, 'M' for "M" type, 0 else.
 */

int checkboard (void)
{
	char buf[64];
	int i;
	int l = getenv_f("serial#", buf, sizeof(buf));

	puts ("Board: ");

	if (l < 0 || strncmp(buf, "TQM8", 4)) {
		puts ("### No HW ID - assuming TQM8xxL\n");
		return (0);
	}

	if ((buf[6] == 'L')) {	/* a TQM8xxL type */
		gd->board_type = 'L';
	}

	if ((buf[6] == 'M')) {	/* a TQM8xxM type */
		gd->board_type = 'M';
	}

	if ((buf[6] == 'D')) {	/* a TQM885D type */
		gd->board_type = 'D';
	}

	for (i = 0; i < l; ++i) {
		if (buf[i] == ' ')
			break;
		putc (buf[i]);
	}

	putc ('\n');

	return (0);
}

/* ------------------------------------------------------------------------- */

int dram_init(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size8, size9, size10;
	long int size_b0 = 0;
	long int size_b1 = 0;
	int board_type = gd->board_type;

	upmconfig (UPMA, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_8K;

	/*
	 * The following value is used as an address (i.e. opcode) for
	 * the LOAD MODE REGISTER COMMAND during SDRAM initialisation. If
	 * the port size is 32bit the SDRAM does NOT "see" the lower two
	 * address lines, i.e. mar=0x00000088 -> opcode=0x00000022 for
	 * MICRON SDRAMs:
	 * ->    0 00 010 0 010
	 *       |  |   | |   +- Burst Length = 4
	 *       |  |   | +----- Burst Type   = Sequential
	 *       |  |   +------- CAS Latency  = 2
	 *       |  +----------- Operating Mode = Standard
	 *       +-------------- Write Burst Mode = Programmed Burst Length
	 */
	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller banks 2 and 3 to the SDRAM banks 2 and 3 at
	 * preliminary addresses - these have to be modified after the
	 * SDRAM size has been determined.
	 */
	memctl->memc_or2 = CONFIG_SYS_OR2_PRELIM;
	memctl->memc_br2 = CONFIG_SYS_BR2_PRELIM;

#ifndef	CONFIG_CAN_DRIVER
	if ((board_type != 'L') &&
	    (board_type != 'M') &&
	    (board_type != 'D') ) {	/* only one SDRAM bank on L, M and D modules */
		memctl->memc_or3 = CONFIG_SYS_OR3_PRELIM;
		memctl->memc_br3 = CONFIG_SYS_BR3_PRELIM;
	}
#endif							/* CONFIG_CAN_DRIVER */

	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80004105;	/* SDRAM bank 0 */
	udelay (1);
	memctl->memc_mcr = 0x80004230;	/* SDRAM bank 0 - execute twice */
	udelay (1);

#ifndef	CONFIG_CAN_DRIVER
	if ((board_type != 'L') &&
	    (board_type != 'M') &&
	    (board_type != 'D') ) {	/* only one SDRAM bank on L, M and D modules */
		memctl->memc_mcr = 0x80006105;	/* SDRAM bank 1 */
		udelay (1);
		memctl->memc_mcr = 0x80006230;	/* SDRAM bank 1 - execute twice */
		udelay (1);
	}
#endif							/* CONFIG_CAN_DRIVER */

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CONFIG_SYS_MAMR_8COL, SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);
	debug ("SDRAM Bank 0 in 8 column mode: %ld MB\n", size8 >> 20);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CONFIG_SYS_MAMR_9COL, SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);
	debug ("SDRAM Bank 0 in 9 column mode: %ld MB\n", size9 >> 20);

	udelay(1000);

#if defined(CONFIG_SYS_MAMR_10COL)
	/*
	 * try 10 column mode
	 */
	size10 = dram_size (CONFIG_SYS_MAMR_10COL, SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);
	debug ("SDRAM Bank 0 in 10 column mode: %ld MB\n", size10 >> 20);
#else
	size10 = 0;
#endif /* CONFIG_SYS_MAMR_10COL */

	if ((size8 < size10) && (size9 < size10)) {
		size_b0 = size10;
	} else if ((size8 < size9) && (size10 < size9)) {
		size_b0 = size9;
		memctl->memc_mamr = CONFIG_SYS_MAMR_9COL;
		udelay (500);
	} else {
		size_b0 = size8;
		memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;
		udelay (500);
	}
	debug ("SDRAM Bank 0: %ld MB\n", size_b0 >> 20);

#ifndef	CONFIG_CAN_DRIVER
	if ((board_type != 'L') &&
	    (board_type != 'M') &&
	    (board_type != 'D') ) {	/* only one SDRAM bank on L, M and D modules */
		/*
		 * Check Bank 1 Memory Size
		 * use current column settings
		 * [9 column SDRAM may also be used in 8 column mode,
		 *  but then only half the real size will be used.]
		 */
		size_b1 = dram_size (memctl->memc_mamr, (long int *)SDRAM_BASE3_PRELIM,
				     SDRAM_MAX_SIZE);
		debug ("SDRAM Bank 1: %ld MB\n", size_b1 >> 20);
	} else {
		size_b1 = 0;
	}
#endif	/* CONFIG_CAN_DRIVER */

	udelay (1000);

	/*
	 * Adjust refresh rate depending on SDRAM type, both banks
	 * For types > 128 MBit leave it at the current (fast) rate
	 */
	if ((size_b0 < 0x02000000) && (size_b1 < 0x02000000)) {
		/* reduce to 15.6 us (62.4 us / quad) */
		memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_4K;
		udelay (1000);
	}

	/*
	 * Final mapping: map bigger bank first
	 */
	if (size_b1 > size_b0) {	/* SDRAM Bank 1 is bigger - map first   */

		memctl->memc_or3 = ((-size_b1) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
		memctl->memc_br3 = (CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

		if (size_b0 > 0) {
			/*
			 * Position Bank 0 immediately above Bank 1
			 */
			memctl->memc_or2 = ((-size_b0) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
			memctl->memc_br2 = ((CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V)
					   + size_b1;
		} else {
			unsigned long reg;

			/*
			 * No bank 0
			 *
			 * invalidate bank
			 */
			memctl->memc_br2 = 0;

			/* adjust refresh rate depending on SDRAM type, one bank */
			reg = memctl->memc_mptpr;
			reg >>= 1;			/* reduce to CONFIG_SYS_MPTPR_1BK_8K / _4K */
			memctl->memc_mptpr = reg;
		}

	} else {					/* SDRAM Bank 0 is bigger - map first   */

		memctl->memc_or2 = ((-size_b0) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
		memctl->memc_br2 =
				(CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

		if (size_b1 > 0) {
			/*
			 * Position Bank 1 immediately above Bank 0
			 */
			memctl->memc_or3 =
					((-size_b1) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
			memctl->memc_br3 =
					((CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V)
					+ size_b0;
		} else {
			unsigned long reg;

#ifndef	CONFIG_CAN_DRIVER
			/*
			 * No bank 1
			 *
			 * invalidate bank
			 */
			memctl->memc_br3 = 0;
#endif							/* CONFIG_CAN_DRIVER */

			/* adjust refresh rate depending on SDRAM type, one bank */
			reg = memctl->memc_mptpr;
			reg >>= 1;			/* reduce to CONFIG_SYS_MPTPR_1BK_8K / _4K */
			memctl->memc_mptpr = reg;
		}
	}

	udelay (10000);

#ifdef	CONFIG_CAN_DRIVER
	/* UPM initialization for CAN @ CLKOUT <= 66 MHz */

	/* Initialize OR3 / BR3 */
	memctl->memc_or3 = CONFIG_SYS_OR3_CAN;
	memctl->memc_br3 = CONFIG_SYS_BR3_CAN;

	/* Initialize MBMR */
	memctl->memc_mbmr = MBMR_GPL_B4DIS;	/* GPL_B4 ouput line Disable */

	/* Initialize UPMB for CAN: single read */
	memctl->memc_mdr = 0xFFFFCC04;
	memctl->memc_mcr = 0x0100 | UPMB;

	memctl->memc_mdr = 0x0FFFD004;
	memctl->memc_mcr = 0x0101 | UPMB;

	memctl->memc_mdr = 0x0FFFC000;
	memctl->memc_mcr = 0x0102 | UPMB;

	memctl->memc_mdr = 0x3FFFC004;
	memctl->memc_mcr = 0x0103 | UPMB;

	memctl->memc_mdr = 0xFFFFDC07;
	memctl->memc_mcr = 0x0104 | UPMB;

	/* Initialize UPMB for CAN: single write */
	memctl->memc_mdr = 0xFFFCCC04;
	memctl->memc_mcr = 0x0118 | UPMB;

	memctl->memc_mdr = 0xCFFCDC04;
	memctl->memc_mcr = 0x0119 | UPMB;

	memctl->memc_mdr = 0x3FFCC000;
	memctl->memc_mcr = 0x011A | UPMB;

	memctl->memc_mdr = 0xFFFCC004;
	memctl->memc_mcr = 0x011B | UPMB;

	memctl->memc_mdr = 0xFFFDC405;
	memctl->memc_mcr = 0x011C | UPMB;
#endif							/* CONFIG_CAN_DRIVER */

#ifdef	CONFIG_ISP1362_USB
	/* Initialize OR5 / BR5 */
	memctl->memc_or5 = CONFIG_SYS_OR5_ISP1362;
	memctl->memc_br5 = CONFIG_SYS_BR5_ISP1362;
#endif							/* CONFIG_ISP1362_USB */
	gd->ram_size = size_b0 + size_b1;

	return 0;
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base, long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size(base, maxsize));
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_MISC_INIT_R
extern void load_sernum_ethaddr(void);
int misc_init_r (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	load_sernum_ethaddr();

#ifdef	CONFIG_SYS_OR_TIMING_FLASH_AT_50MHZ
	int scy, trlx, flash_or_timing, clk_diff;

	scy = (CONFIG_SYS_OR_TIMING_FLASH_AT_50MHZ & OR_SCY_MSK) >> 4;
	if (CONFIG_SYS_OR_TIMING_FLASH_AT_50MHZ & OR_TRLX) {
		trlx = OR_TRLX;
		scy *= 2;
	} else {
		trlx = 0;
	}

	/*
	 * We assume that each 10MHz of bus clock require 1-clk SCY
	 * adjustment.
	 */
	clk_diff = (gd->bus_clk / 1000000) - 50;

	/*
	 * We need proper rounding here. This is what the "+5" and "-5"
	 * are here for.
	 */
	if (clk_diff >= 0)
		scy += (clk_diff + 5) / 10;
	else
		scy += (clk_diff - 5) / 10;

	/*
	 * For bus frequencies above 50MHz, we want to use relaxed timing
	 * (OR_TRLX).
	 */
	if (gd->bus_clk >= 50000000)
		trlx = OR_TRLX;
	else
		trlx = 0;

	if (trlx)
		scy /= 2;

	if (scy > 0xf)
		scy = 0xf;
	if (scy < 1)
		scy = 1;

	flash_or_timing = (scy << 4) | trlx |
		(CONFIG_SYS_OR_TIMING_FLASH_AT_50MHZ & ~(OR_TRLX | OR_SCY_MSK));

	memctl->memc_or0 =
		flash_or_timing | (-flash_info[0].size & OR_AM_MSK);
#else
	memctl->memc_or0 =
		CONFIG_SYS_OR_TIMING_FLASH | (-flash_info[0].size & OR_AM_MSK);
#endif
	memctl->memc_br0 = (CONFIG_SYS_FLASH_BASE & BR_BA_MSK) | BR_MS_GPCM | BR_V;

	debug ("## BR0: 0x%08x    OR0: 0x%08x\n",
	       memctl->memc_br0, memctl->memc_or0);

	if (flash_info[1].size) {
#ifdef	CONFIG_SYS_OR_TIMING_FLASH_AT_50MHZ
		memctl->memc_or1 = flash_or_timing |
			(-flash_info[1].size & 0xFFFF8000);
#else
		memctl->memc_or1 = CONFIG_SYS_OR_TIMING_FLASH |
			(-flash_info[1].size & 0xFFFF8000);
#endif
		memctl->memc_br1 =
			((CONFIG_SYS_FLASH_BASE +
			  flash_info[0].
			  size) & BR_BA_MSK) | BR_MS_GPCM | BR_V;

		debug ("## BR1: 0x%08x    OR1: 0x%08x\n",
		       memctl->memc_br1, memctl->memc_or1);
	} else {
		memctl->memc_br1 = 0;	/* invalidate bank */

		debug ("## DISABLE BR1: 0x%08x    OR1: 0x%08x\n",
		       memctl->memc_br1, memctl->memc_or1);
	}

# ifdef CONFIG_IDE_LED
	/* Configure PA15 as output port */
	immap->im_ioport.iop_padir |= 0x0001;
	immap->im_ioport.iop_paodr |= 0x0001;
	immap->im_ioport.iop_papar &= ~0x0001;
	immap->im_ioport.iop_padat &= ~0x0001;	/* turn it off */
# endif

	return (0);
}
#endif	/* CONFIG_MISC_INIT_R */


# ifdef CONFIG_IDE_LED
void ide_led (uchar led, uchar status)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	/* We have one led for both pcmcia slots */
	if (status) {				/* led on */
		immap->im_ioport.iop_padat |= 0x0001;
	} else {
		immap->im_ioport.iop_padat &= ~0x0001;
	}
}
# endif

#ifdef CONFIG_LCD_INFO
#include <lcd.h>
#include <version.h>
#include <timestamp.h>

void lcd_show_board_info(void)
{
	char temp[32];

	lcd_printf ("%s (%s - %s)\n", U_BOOT_VERSION, U_BOOT_DATE, U_BOOT_TIME);
	lcd_printf ("(C) 2008 DENX Software Engineering GmbH\n");
	lcd_printf ("    Wolfgang DENK, wd@denx.de\n");
#ifdef CONFIG_LCD_INFO_BELOW_LOGO
	lcd_printf ("MPC823 CPU at %s MHz\n",
		strmhz(temp, gd->cpu_clk));
	lcd_printf ("  %ld MB RAM, %ld MB Flash\n",
		gd->ram_size >> 20,
		gd->bd->bi_flashsize >> 20 );
#else
	/* leave one blank line */
	lcd_printf ("\nMPC823 CPU at %s MHz, %ld MB RAM, %ld MB Flash\n",
		strmhz(temp, gd->cpu_clk),
		gd->ram_size >> 20,
		gd->bd->bi_flashsize >> 20 );
#endif /* CONFIG_LCD_INFO_BELOW_LOGO */
}
#endif /* CONFIG_LCD_INFO */

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int fdt_set_node_and_value (void *blob,
				char *nodename,
				char *regname,
				void *var,
				int size)
{
	int ret = 0;
	int nodeoffset = 0;

	nodeoffset = fdt_path_offset (blob, nodename);
	if (nodeoffset >= 0) {
		ret = fdt_setprop (blob, nodeoffset, regname, var,
					size);
		if (ret < 0) {
			printf("ft_blob_update(): "
				"cannot set %s/%s property; err: %s\n",
				nodename, regname, fdt_strerror (ret));
		}
	} else {
		printf("ft_blob_update(): "
			"cannot find %s node err:%s\n",
			nodename, fdt_strerror (nodeoffset));
	}
	return ret;
}

int fdt_del_node_name (void *blob, char *nodename)
{
	int ret = 0;
	int nodeoffset = 0;

	nodeoffset = fdt_path_offset (blob, nodename);
	if (nodeoffset >= 0) {
		ret = fdt_del_node (blob, nodeoffset);
		if (ret < 0) {
			printf("%s: cannot delete %s; err: %s\n",
				__func__, nodename, fdt_strerror (ret));
		}
	} else {
		printf("%s: cannot find %s node err:%s\n",
			__func__, nodename, fdt_strerror (nodeoffset));
	}
	return ret;
}

int fdt_del_prop_name (void *blob, char *nodename, char *propname)
{
	int ret = 0;
	int nodeoffset = 0;

	nodeoffset = fdt_path_offset (blob, nodename);
	if (nodeoffset >= 0) {
		ret = fdt_delprop (blob, nodeoffset, propname);
		if (ret < 0) {
			printf("%s: cannot delete %s %s; err: %s\n",
				__func__, nodename, propname,
				fdt_strerror (ret));
		}
	} else {
		printf("%s: cannot find %s node err:%s\n",
			__func__, nodename, fdt_strerror (nodeoffset));
	}
	return ret;
}

/*
 * update "brg" property in the blob
 */
void ft_blob_update (void *blob, bd_t *bd)
{
	uchar enetaddr[6];
	ulong brg_data = 0;

	/* BRG */
	brg_data = cpu_to_be32(bd->bi_busfreq);
	fdt_set_node_and_value(blob,
				"/soc/cpm", "brg-frequency",
				&brg_data, sizeof(brg_data));

	/* MAC addr */
	if (eth_getenv_enetaddr("ethaddr", enetaddr)) {
		fdt_set_node_and_value(blob,
					"ethernet0", "local-mac-address",
					enetaddr, sizeof(u8) * 6);
	}

	if (hwconfig_arg_cmp("fec", "off")) {
		/* no FEC on this plattform, delete DTS nodes */
		fdt_del_node_name (blob, "ethernet1");
		fdt_del_node_name (blob, "mdio1");
		/* also the aliases entries */
		fdt_del_prop_name (blob, "/aliases", "ethernet1");
		fdt_del_prop_name (blob, "/aliases", "mdio1");
	} else {
		/* adjust local-mac-address for FEC ethernet */
		if (eth_getenv_enetaddr("eth1addr", enetaddr)) {
			fdt_set_node_and_value(blob,
					"ethernet1", "local-mac-address",
					enetaddr, sizeof(u8) * 6);
		}
	}
}

int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	ft_blob_update(blob, bd);

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
