/*
 * (C) Copyright 2008 Wolfgang Grandegger <wg@denx.de>
 *
 * (C) Copyright 2006
 * Thomas Waehner, TQ-Systems GmbH, thomas.waehner@tqs.de.
 *
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2002,2003, Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
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
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <asm/io.h>
#include <ioports.h>
#include <flash.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[];	/* FLASH chips info */

void local_bus_init (void);
ulong flash_get_size (ulong base, int banknum);

#ifdef CONFIG_PS2MULT
void ps2mult_early_init (void);
#endif

#ifdef CONFIG_CPM2
/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

	/* Port A: conf, ppar, psor, pdir, podr, pdat */
	{
	 {1, 1, 1, 0, 0, 0},	/* PA31: FCC1 MII COL */
	 {1, 1, 1, 0, 0, 0},	/* PA30: FCC1 MII CRS */
	 {1, 1, 1, 1, 0, 0},	/* PA29: FCC1 MII TX_ER */
	 {1, 1, 1, 1, 0, 0},	/* PA28: FCC1 MII TX_EN */
	 {1, 1, 1, 0, 0, 0},	/* PA27: FCC1 MII RX_DV */
	 {1, 1, 1, 0, 0, 0},	/* PA26: FCC1 MII RX_ER */
	 {0, 1, 0, 1, 0, 0},	/* PA25: FCC1 ATMTXD[0] */
	 {0, 1, 0, 1, 0, 0},	/* PA24: FCC1 ATMTXD[1] */
	 {0, 1, 0, 1, 0, 0},	/* PA23: FCC1 ATMTXD[2] */
	 {0, 1, 0, 1, 0, 0},	/* PA22: FCC1 ATMTXD[3] */
	 {1, 1, 0, 1, 0, 0},	/* PA21: FCC1 MII TxD[3] */
	 {1, 1, 0, 1, 0, 0},	/* PA20: FCC1 MII TxD[2] */
	 {1, 1, 0, 1, 0, 0},	/* PA19: FCC1 MII TxD[1] */
	 {1, 1, 0, 1, 0, 0},	/* PA18: FCC1 MII TxD[0] */
	 {1, 1, 0, 0, 0, 0},	/* PA17: FCC1 MII RxD[0] */
	 {1, 1, 0, 0, 0, 0},	/* PA16: FCC1 MII RxD[1] */
	 {1, 1, 0, 0, 0, 0},	/* PA15: FCC1 MII RxD[2] */
	 {1, 1, 0, 0, 0, 0},	/* PA14: FCC1 MII RxD[3] */
	 {0, 1, 0, 0, 0, 0},	/* PA13: FCC1 ATMRXD[3] */
	 {0, 1, 0, 0, 0, 0},	/* PA12: FCC1 ATMRXD[2] */
	 {0, 1, 0, 0, 0, 0},	/* PA11: FCC1 ATMRXD[1] */
	 {0, 1, 0, 0, 0, 0},	/* PA10: FCC1 ATMRXD[0] */
	 {0, 1, 1, 1, 0, 0},	/* PA9 : FCC1 L1TXD */
	 {0, 1, 1, 0, 0, 0},	/* PA8 : FCC1 L1RXD */
	 {0, 0, 0, 1, 0, 0},	/* PA7 : PA7 */
	 {0, 1, 1, 1, 0, 0},	/* PA6 : TDM A1 L1RSYNC */
	 {0, 0, 0, 1, 0, 0},	/* PA5 : PA5 */
	 {0, 0, 0, 1, 0, 0},	/* PA4 : PA4 */
	 {0, 0, 0, 1, 0, 0},	/* PA3 : PA3 */
	 {0, 0, 0, 1, 0, 0},	/* PA2 : PA2 */
	 {0, 0, 0, 0, 0, 0},	/* PA1 : FREERUN */
	 {0, 0, 0, 1, 0, 0}	/* PA0 : PA0 */
	 },

	/* Port B: conf, ppar, psor, pdir, podr, pdat */
	{
	 {1, 1, 0, 1, 0, 0},	/* PB31: FCC2 MII TX_ER */
	 {1, 1, 0, 0, 0, 0},	/* PB30: FCC2 MII RX_DV */
	 {1, 1, 1, 1, 0, 0},	/* PB29: FCC2 MII TX_EN */
	 {1, 1, 0, 0, 0, 0},	/* PB28: FCC2 MII RX_ER */
	 {1, 1, 0, 0, 0, 0},	/* PB27: FCC2 MII COL */
	 {1, 1, 0, 0, 0, 0},	/* PB26: FCC2 MII CRS */
	 {1, 1, 0, 1, 0, 0},	/* PB25: FCC2 MII TxD[3] */
	 {1, 1, 0, 1, 0, 0},	/* PB24: FCC2 MII TxD[2] */
	 {1, 1, 0, 1, 0, 0},	/* PB23: FCC2 MII TxD[1] */
	 {1, 1, 0, 1, 0, 0},	/* PB22: FCC2 MII TxD[0] */
	 {1, 1, 0, 0, 0, 0},	/* PB21: FCC2 MII RxD[0] */
	 {1, 1, 0, 0, 0, 0},	/* PB20: FCC2 MII RxD[1] */
	 {1, 1, 0, 0, 0, 0},	/* PB19: FCC2 MII RxD[2] */
	 {1, 1, 0, 0, 0, 0},	/* PB18: FCC2 MII RxD[3] */
	 {1, 1, 0, 0, 0, 0},	/* PB17: FCC3:RX_DIV */
	 {1, 1, 0, 0, 0, 0},	/* PB16: FCC3:RX_ERR */
	 {1, 1, 0, 1, 0, 0},	/* PB15: FCC3:TX_ERR */
	 {1, 1, 0, 1, 0, 0},	/* PB14: FCC3:TX_EN */
	 {1, 1, 0, 0, 0, 0},	/* PB13: FCC3:COL */
	 {1, 1, 0, 0, 0, 0},	/* PB12: FCC3:CRS */
	 {1, 1, 0, 0, 0, 0},	/* PB11: FCC3:RXD */
	 {1, 1, 0, 0, 0, 0},	/* PB10: FCC3:RXD */
	 {1, 1, 0, 0, 0, 0},	/* PB9 : FCC3:RXD */
	 {1, 1, 0, 0, 0, 0},	/* PB8 : FCC3:RXD */
	 {1, 1, 0, 1, 0, 0},	/* PB7 : FCC3:TXD */
	 {1, 1, 0, 1, 0, 0},	/* PB6 : FCC3:TXD */
	 {1, 1, 0, 1, 0, 0},	/* PB5 : FCC3:TXD */
	 {1, 1, 0, 1, 0, 0},	/* PB4 : FCC3:TXD */
	 {0, 0, 0, 0, 0, 0},	/* PB3 : pin doesn't exist */
	 {0, 0, 0, 0, 0, 0},	/* PB2 : pin doesn't exist */
	 {0, 0, 0, 0, 0, 0},	/* PB1 : pin doesn't exist */
	 {0, 0, 0, 0, 0, 0}	/* PB0 : pin doesn't exist */
	 },

	/* Port C: conf, ppar, psor, pdir, podr, pdat */
	{
	 {0, 0, 0, 1, 0, 0},	/* PC31: PC31 */
	 {0, 0, 0, 1, 0, 0},	/* PC30: PC30 */
	 {0, 1, 1, 0, 0, 0},	/* PC29: SCC1 EN *CLSN */
	 {0, 0, 0, 1, 0, 0},	/* PC28: PC28 */
	 {0, 0, 0, 1, 0, 0},	/* PC27: UART Clock in */
	 {0, 0, 0, 1, 0, 0},	/* PC26: PC26 */
	 {0, 0, 0, 1, 0, 0},	/* PC25: PC25 */
	 {0, 0, 0, 1, 0, 0},	/* PC24: PC24 */
	 {0, 1, 0, 1, 0, 0},	/* PC23: ATMTFCLK */
	 {0, 1, 0, 0, 0, 0},	/* PC22: ATMRFCLK */
	 {1, 1, 0, 0, 0, 0},	/* PC21: SCC1 EN RXCLK */
	 {1, 1, 0, 0, 0, 0},	/* PC20: SCC1 EN TXCLK */
	 {1, 1, 0, 0, 0, 0},	/* PC19: FCC2 MII RX_CLK CLK13 */
	 {1, 1, 0, 0, 0, 0},	/* PC18: FCC Tx Clock (CLK14) */
	 {1, 1, 0, 0, 0, 0},	/* PC17: PC17 */
	 {1, 1, 0, 0, 0, 0},	/* PC16: FCC Tx Clock (CLK16) */
	 {0, 1, 0, 0, 0, 0},	/* PC15: PC15 */
	 {0, 1, 0, 0, 0, 0},	/* PC14: SCC1 EN *CD */
	 {0, 1, 0, 0, 0, 0},	/* PC13: PC13 */
	 {0, 1, 0, 1, 0, 0},	/* PC12: PC12 */
	 {0, 0, 0, 1, 0, 0},	/* PC11: LXT971 transmit control */
	 {0, 0, 0, 1, 0, 0},	/* PC10: FETHMDC */
	 {0, 0, 0, 0, 0, 0},	/* PC9 : FETHMDIO */
	 {0, 0, 0, 1, 0, 0},	/* PC8 : PC8 */
	 {0, 0, 0, 1, 0, 0},	/* PC7 : PC7 */
	 {0, 0, 0, 1, 0, 0},	/* PC6 : PC6 */
	 {0, 0, 0, 1, 0, 0},	/* PC5 : PC5 */
	 {0, 0, 0, 1, 0, 0},	/* PC4 : PC4 */
	 {0, 0, 0, 1, 0, 0},	/* PC3 : PC3 */
	 {0, 0, 0, 1, 0, 1},	/* PC2 : ENET FDE */
	 {0, 0, 0, 1, 0, 0},	/* PC1 : ENET DSQE */
	 {0, 0, 0, 1, 0, 0},	/* PC0 : ENET LBK */
	 },

	/* Port D: conf, ppar, psor, pdir, podr, pdat */
	{
#ifdef CONFIG_TQM8560
	 {1, 1, 0, 0, 0, 0},	/* PD31: SCC1 EN RxD */
	 {1, 1, 1, 1, 0, 0},	/* PD30: SCC1 EN TxD */
	 {1, 1, 0, 1, 0, 0},	/* PD29: SCC1 EN TENA */
#else /* !CONFIG_TQM8560 */
	 {0, 0, 0, 0, 0, 0},	/* PD31: PD31 */
	 {0, 0, 0, 0, 0, 0},	/* PD30: PD30 */
	 {0, 0, 0, 0, 0, 0},	/* PD29: PD29 */
#endif /* CONFIG_TQM8560 */
	 {1, 1, 0, 0, 0, 0},	/* PD28: PD28 */
	 {1, 1, 0, 1, 0, 0},	/* PD27: PD27 */
	 {1, 1, 0, 1, 0, 0},	/* PD26: PD26 */
	 {0, 0, 0, 1, 0, 0},	/* PD25: PD25 */
	 {0, 0, 0, 1, 0, 0},	/* PD24: PD24 */
	 {0, 0, 0, 1, 0, 0},	/* PD23: PD23 */
	 {0, 0, 0, 1, 0, 0},	/* PD22: PD22 */
	 {0, 0, 0, 1, 0, 0},	/* PD21: PD21 */
	 {0, 0, 0, 1, 0, 0},	/* PD20: PD20 */
	 {0, 0, 0, 1, 0, 0},	/* PD19: PD19 */
	 {0, 0, 0, 1, 0, 0},	/* PD18: PD18 */
	 {0, 1, 0, 0, 0, 0},	/* PD17: FCC1 ATMRXPRTY */
	 {0, 1, 0, 1, 0, 0},	/* PD16: FCC1 ATMTXPRTY */
	 {0, 1, 1, 0, 1, 0},	/* PD15: I2C SDA */
	 {0, 0, 0, 1, 0, 0},	/* PD14: LED */
	 {0, 0, 0, 0, 0, 0},	/* PD13: PD13 */
	 {0, 0, 0, 0, 0, 0},	/* PD12: PD12 */
	 {0, 0, 0, 0, 0, 0},	/* PD11: PD11 */
	 {0, 0, 0, 0, 0, 0},	/* PD10: PD10 */
	 {0, 1, 0, 1, 0, 0},	/* PD9 : SMC1 TXD */
	 {0, 1, 0, 0, 0, 0},	/* PD8 : SMC1 RXD */
	 {0, 0, 0, 1, 0, 1},	/* PD7 : PD7 */
	 {0, 0, 0, 1, 0, 1},	/* PD6 : PD6 */
	 {0, 0, 0, 1, 0, 1},	/* PD5 : PD5 */
	 {0, 0, 0, 1, 0, 1},	/* PD4 : PD4 */
	 {0, 0, 0, 0, 0, 0},	/* PD3 : pin doesn't exist */
	 {0, 0, 0, 0, 0, 0},	/* PD2 : pin doesn't exist */
	 {0, 0, 0, 0, 0, 0},	/* PD1 : pin doesn't exist */
	 {0, 0, 0, 0, 0, 0}	/* PD0 : pin doesn't exist */
	 }
};
#endif /*  CONFIG_CPM2 */

#define CASL_STRING1	"casl=xx"
#define CASL_STRING2	"casl="

static const int casl_table[] = { 20, 25, 30 };
#define	N_CASL (sizeof(casl_table) / sizeof(casl_table[0]))

int cas_latency (void)
{
	char *s = getenv ("serial#");
	int casl;
	int val;
	int i;

	casl = CONFIG_DDR_DEFAULT_CL;

	if (s != NULL) {
		if (strncmp(s + strlen (s) - strlen (CASL_STRING1),
			    CASL_STRING2, strlen (CASL_STRING2)) == 0) {
			val = simple_strtoul (s + strlen (s) - 2, NULL, 10);

			for (i = 0; i < N_CASL; ++i) {
				if (val == casl_table[i]) {
					return val;
				}
			}
		}
	}

	return casl;
}

int checkboard (void)
{
	char *s = getenv ("serial#");

	printf ("Board: %s", CONFIG_BOARDNAME);
	if (s != NULL) {
		puts (", serial# ");
		puts (s);
	}
	putc ('\n');

	/*
	 * Initialize local bus.
	 */
	local_bus_init ();

	return 0;
}

int misc_init_r (void)
{
	/*
	 * Adjust flash start and offset to detected values
	 */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Recalculate CS configuration if second FLASH bank is available
	 */
	if (flash_info[0].size > 0) {
		set_lbc_or(1, ((-flash_info[0].size) & 0xffff8000) |
			   (CONFIG_SYS_OR1_PRELIM & 0x00007fff));
		set_lbc_br(1, gd->bd->bi_flashstart |
			   (CONFIG_SYS_BR1_PRELIM & 0x00007fff));
		/*
		 * Re-check to get correct base address for bank 1
		 */
		flash_get_size (gd->bd->bi_flashstart, 0);
	} else {
		set_lbc_or(1, 0);
		set_lbc_br(1, 0);
	}

	/*
	 *  If bank 1 is equipped, bank 0 is mapped after bank 1
	 */
	set_lbc_or(0, ((-flash_info[1].size) & 0xffff8000) |
		   (CONFIG_SYS_OR0_PRELIM & 0x00007fff));
	set_lbc_br(0, gd->bd->bi_flashstart |
		   (CONFIG_SYS_BR0_PRELIM & 0x00007fff));

	/*
	 * Re-check to get correct base address for bank 0
	 */
	flash_get_size (gd->bd->bi_flashstart + flash_info[0].size, 1);

	/*
	 * Re-do flash protection upon new addresses
	 */
	flash_protect (FLAG_PROTECT_CLEAR,
		       gd->bd->bi_flashstart, 0xffffffff,
		       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

	/* Monitor protection ON by default */
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_SYS_MONITOR_BASE, 0xffffffff,
		       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

	/* Environment protection ON by default */
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_ENV_ADDR,
		       CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
		       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

#ifdef CONFIG_ENV_ADDR_REDUND
	/* Redundant environment protection ON by default */
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_ENV_ADDR_REDUND,
		       CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
		       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);
#endif

	return 0;
}

#ifdef CONFIG_CAN_DRIVER
/*
 * Initialize UPMC RAM
 */
static void upmc_write (u_char addr, uint val)
{
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;

	out_be32 (&lbc->mdr, val);

	clrsetbits_be32(&lbc->mcmr, MxMR_MAD_MSK,
			MxMR_OP_WARR | (addr & MxMR_MAD_MSK));

	/* dummy access to perform write */
	out_8 ((void __iomem *)CONFIG_SYS_CAN_BASE, 0);

	/* normal operation */
	clrbits_be32(&lbc->mcmr, MxMR_OP_WARR);
}
#endif /* CONFIG_CAN_DRIVER */

uint get_lbc_clock (void)
{
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	sys_info_t sys_info;
	ulong clkdiv = lbc->lcrr & LCRR_CLKDIV;

	get_sys_info (&sys_info);

	if (clkdiv == 2 || clkdiv == 4 || clkdiv == 8) {
#ifdef CONFIG_MPC8548
		/*
		 * Yes, the entire PQ38 family use the same
		 * bit-representation for twice the clock divider value.
		 */
		clkdiv *= 2;
#endif
		return sys_info.freqSystemBus / clkdiv;
	}

	puts("Invalid clock divider value in CONFIG_SYS_LBC_LCRR\n");

	return 0;
}

/*
 * Initialize Local Bus
 */
void local_bus_init (void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	uint lbc_mhz = get_lbc_clock ()  / 1000000;

#ifdef CONFIG_MPC8548
	uint svr = get_svr ();
	uint lcrr;

	/*
	 * MPC revision < 2.0
	 * According to MPC8548E_Device_Errata Rev. L, Erratum LBIU1:
	 * Modify engineering use only register at address 0xE_0F20.
	 * "1. Read register at offset 0xE_0F20
	 * 2. And value with 0x0000_FFFF
	 * 3. OR result with 0x0000_0004
	 * 4. Write result back to offset 0xE_0F20."
	 *
	 * According to MPC8548E_Device_Errata Rev. L, Erratum LBIU2:
	 * Modify engineering use only register at address 0xE_0F20.
	 * "1. Read register at offset 0xE_0F20
	 * 2. And value with 0xFFFF_FFDF
	 * 3. Write result back to offset 0xE_0F20."
	 *
	 * Since it is the same register, we do the modification in one step.
	 */
	if (SVR_MAJ (svr) < 2) {
		uint dummy = gur->lbiuiplldcr1;
		dummy &= 0x0000FFDF;
		dummy |= 0x00000004;
		gur->lbiuiplldcr1 = dummy;
	}

	lcrr = CONFIG_SYS_LBC_LCRR;

	/*
	 * Local Bus Clock > 83.3 MHz. According to timing
	 * specifications set LCRR[EADC] to 2 delay cycles.
	 */
	if (lbc_mhz > 83) {
		lcrr &= ~LCRR_EADC;
		lcrr |= LCRR_EADC_2;
	}

	/*
	 * According to MPC8548ERMAD Rev. 1.3, 13.3.1.16, 13-30
	 * disable PLL bypass for Local Bus Clock > 83 MHz.
	 */
	if (lbc_mhz >= 66)
		lcrr &= (~LCRR_DBYP);	/* DLL Enabled */

	else
		lcrr |= LCRR_DBYP;	/* DLL Bypass */

	lbc->lcrr = lcrr;
	asm ("sync;isync;msync");

	/*
	 * According to MPC8548ERMAD Rev.1.3 read back LCRR
	 * and terminate with isync
	 */
	lcrr = lbc->lcrr;
	asm ("isync;");

	/* let DLL stabilize */
	udelay (500);

#else /* !CONFIG_MPC8548 */

	/*
	 * Errata LBC11.
	 * Fix Local Bus clock glitch when DLL is enabled.
	 *
	 * If localbus freq is < 66MHz, DLL bypass mode must be used.
	 * If localbus freq is > 133MHz, DLL can be safely enabled.
	 * Between 66 and 133, the DLL is enabled with an override workaround.
	 */

	if (lbc_mhz < 66) {
		lbc->lcrr = CONFIG_SYS_LBC_LCRR | LCRR_DBYP;	/* DLL Bypass */
		lbc->ltedr = LTEDR_BMD | LTEDR_PARD | LTEDR_WPD | LTEDR_WARA |
			     LTEDR_RAWA | LTEDR_CSD;	/* Disable all error checking */

	} else if (lbc_mhz >= 133) {
		lbc->lcrr = CONFIG_SYS_LBC_LCRR & (~LCRR_DBYP);	/* DLL Enabled */

	} else {
		/*
		 * On REV1 boards, need to change CLKDIV before enable DLL.
		 * Default CLKDIV is 8, change it to 4 temporarily.
		 */
		uint pvr = get_pvr ();
		uint temp_lbcdll = 0;

		if (pvr == PVR_85xx_REV1) {
			/* FIXME: Justify the high bit here. */
			lbc->lcrr = 0x10000004;
		}

		lbc->lcrr = CONFIG_SYS_LBC_LCRR & (~LCRR_DBYP);	/* DLL Enabled */
		udelay (200);

		/*
		 * Sample LBC DLL ctrl reg, upshift it to set the
		 * override bits.
		 */
		temp_lbcdll = gur->lbcdllcr;
		gur->lbcdllcr = (((temp_lbcdll & 0xff) << 16) | 0x80000000);
		asm ("sync;isync;msync");
	}
#endif /* !CONFIG_MPC8548 */

#ifdef	CONFIG_CAN_DRIVER
	/*
	 * According to timing specifications EAD must be
	 * set if Local Bus Clock is > 83 MHz.
	 */
	if (lbc_mhz > 83)
		set_lbc_or(2, CONFIG_SYS_OR2_CAN | OR_UPM_EAD);
	else
		set_lbc_or(2, CONFIG_SYS_OR2_CAN);
	set_lbc_br(2, CONFIG_SYS_BR2_CAN);

	/* LGPL4 is UPWAIT */
	out_be32(&lbc->mcmr, MxMR_DSx_3_CYCL | MxMR_GPL_x4DIS | MxMR_WLFx_3X);

	/* Initialize UPMC for CAN: single read */
	upmc_write (0x00, 0xFFFFED00);
	upmc_write (0x01, 0xCCFFCC00);
	upmc_write (0x02, 0x00FFCF00);
	upmc_write (0x03, 0x00FFCF00);
	upmc_write (0x04, 0x00FFDC00);
	upmc_write (0x05, 0x00FFCF00);
	upmc_write (0x06, 0x00FFED00);
	upmc_write (0x07, 0x3FFFCC07);

	/* Initialize UPMC for CAN: single write */
	upmc_write (0x18, 0xFFFFED00);
	upmc_write (0x19, 0xCCFFEC00);
	upmc_write (0x1A, 0x00FFED80);
	upmc_write (0x1B, 0x00FFED80);
	upmc_write (0x1C, 0x00FFFC00);
	upmc_write (0x1D, 0x0FFFEC00);
	upmc_write (0x1E, 0x0FFFEF00);
	upmc_write (0x1F, 0x3FFFEC05);
#endif /* CONFIG_CAN_DRIVER */
}

/*
 * Initialize PCI Devices, report devices found.
 */
static int first_free_busno;

#ifdef CONFIG_PCI1
static struct pci_controller pci1_hose;
#endif /* CONFIG_PCI1 */

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif /* CONFIG_PCIE1 */

static inline void init_pci1(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#ifdef CONFIG_PCI1
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *)CONFIG_SYS_PCI1_ADDR;
	struct pci_controller *hose = &pci1_hose;
	struct pci_region *r = hose->regions;

	/* PORDEVSR[15] */
	uint pci_32 = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_PCI32;
	/* PORDEVSR[14] */
	uint pci_arb = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;
	/* PORPLLSR[16] */
	uint pci_clk_sel = gur->porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;

	int pci_agent = fsl_setup_hose(hose, CONFIG_SYS_PCI1_ADDR);

	uint pci_speed = CONFIG_SYS_CLK_FREQ;	/* PCI PSPEED in [4:5] */

	if (!(gur->devdisr & MPC85xx_DEVDISR_PCI1)) {
		printf ("PCI1:  %d bit, %s MHz, %s, %s, %s\n",
			(pci_32) ? 32 : 64,
			(pci_speed == 33333333) ? "33" :
			(pci_speed == 66666666) ? "66" : "unknown",
			pci_clk_sel ? "sync" : "async",
			pci_agent ? "agent" : "host",
			pci_arb ? "arbiter" : "external-arbiter");

		/* outbound memory */
		pci_set_region (r++,
				CONFIG_SYS_PCI1_MEM_BASE,
				CONFIG_SYS_PCI1_MEM_PHYS,
				CONFIG_SYS_PCI1_MEM_SIZE,
				PCI_REGION_MEM);

		/* outbound io */
		pci_set_region (r++,
				CONFIG_SYS_PCI1_IO_BASE,
				CONFIG_SYS_PCI1_IO_PHYS,
				CONFIG_SYS_PCI1_IO_SIZE,
				PCI_REGION_IO);

		hose->region_count = r - hose->regions;

		hose->first_busno = first_free_busno;

		fsl_pci_init(hose, (u32)&pci->cfg_addr, (u32)&pci->cfg_data);

		printf ("       PCI on bus %02x..%02x\n",
			hose->first_busno, hose->last_busno);

		first_free_busno = hose->last_busno + 1;
#ifdef CONFIG_PCIX_CHECK
		if (!(gur->pordevsr & MPC85xx_PORDEVSR_PCI1)) {
			ushort reg16 =
				PCI_X_CMD_MAX_SPLIT | PCI_X_CMD_MAX_READ |
				PCI_X_CMD_ERO | PCI_X_CMD_DPERR_E;
			uint dev = PCI_BDF(hose->first_busno, 0, 0);

			/* PCI-X init */
			if (CONFIG_SYS_CLK_FREQ < 66000000)
				puts ("PCI-X will only work at 66 MHz\n");

			pci_hose_write_config_word (hose, dev, PCIX_COMMAND,
						    reg16);
		}
#endif
	} else {
		puts ("PCI1:  disabled\n");
	}
#else /* !CONFIG_PCI1 */
	gur->devdisr |= MPC85xx_DEVDISR_PCI1; /* disable */
#endif /* CONFIG_PCI1 */
}

static inline void init_pcie1(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#ifdef CONFIG_PCIE1
	uint io_sel = (gur->pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *)CONFIG_SYS_PCIE1_ADDR;
	struct pci_controller *hose = &pcie1_hose;
	int pcie_ep;
	struct pci_region *r = hose->regions;

	int pcie_configured = is_fsl_pci_cfg(LAW_TRGT_IF_PCIE_1, io_sel);

	pcie_ep = fsl_setup_hose(hose, CONFIG_SYS_PCIE1_ADDR);

	if (pcie_configured && !(gur->devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("PCIe:  %s, base address %x",
			pcie_ep ? "Endpoint" : "Root complex", (uint)pci);

		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (", with errors. Clearing. Now 0x%08x",
			       pci->pme_msg_det);
		}
		puts ("\n");

		/* outbound memory */
		pci_set_region (r++,
				CONFIG_SYS_PCIE1_MEM_BASE,
				CONFIG_SYS_PCIE1_MEM_PHYS,
				CONFIG_SYS_PCIE1_MEM_SIZE,
				PCI_REGION_MEM);

		/* outbound io */
		pci_set_region (r++,
				CONFIG_SYS_PCIE1_IO_BASE,
				CONFIG_SYS_PCIE1_IO_PHYS,
				CONFIG_SYS_PCIE1_IO_SIZE,
				PCI_REGION_IO);

		hose->region_count = r - hose->regions;

		hose->first_busno = first_free_busno;

		fsl_pci_init(hose, (u32)&pci->cfg_addr, (u32)&pci->cfg_data);
		printf ("       PCIe on bus %02x..%02x\n",
			hose->first_busno, hose->last_busno);

		first_free_busno = hose->last_busno + 1;

	} else {
		printf ("PCIe:  disabled\n");
	}
#else /* !CONFIG_PCIE1 */
	gur->devdisr |= MPC85xx_DEVDISR_PCIE; /* disable */
#endif /* CONFIG_PCIE1 */
}

void pci_init_board (void)
{
	init_pci1();
	init_pcie1();
}

#ifdef CONFIG_OF_BOARD_SETUP
void ft_board_setup (void *blob, bd_t *bd)
{
	ft_cpu_setup (blob, bd);

	FT_FSL_PCI_SETUP;
}
#endif /* CONFIG_OF_BOARD_SETUP */

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r (void)
{
#ifdef CONFIG_PS2MULT
	ps2mult_early_init ();
#endif /* CONFIG_PS2MULT */
	return (0);
}
#endif /* CONFIG_BOARD_EARLY_INIT_R */

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis);	/* Intialize TSECs first */
	return pci_eth_init(bis);
}
