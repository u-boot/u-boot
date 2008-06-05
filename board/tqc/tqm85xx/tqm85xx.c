/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the
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
#include <ioports.h>
#include <flash.h>

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

#ifdef CONFIG_PCI
	printf ("PCI1:  32 bit, %d MHz (compiled)\n",
		CONFIG_SYS_CLK_FREQ / 1000000);
#else
	printf ("PCI1:  disabled\n");
#endif

	/*
	 * Initialize local bus.
	 */
	local_bus_init ();

	return 0;
}

int misc_init_r (void)
{
	volatile ccsr_lbc_t *memctl = (void *)(CFG_MPC85xx_LBC_ADDR);

	/*
	 * Adjust flash start and offset to detected values
	 */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Check if boot FLASH isn't max size
	 */
	if (gd->bd->bi_flashsize < (0 - CFG_FLASH0)) {
		memctl->or0 =
		    gd->bd->bi_flashstart | (CFG_OR0_PRELIM & 0x00007fff);
		memctl->br0 =
		    gd->bd->bi_flashstart | (CFG_BR0_PRELIM & 0x00007fff);

		/*
		 * Re-check to get correct base address
		 */
		flash_get_size (gd->bd->bi_flashstart, CFG_MAX_FLASH_BANKS - 1);
	}

	/*
	 * Check if only one FLASH bank is available
	 */
	if (gd->bd->bi_flashsize != CFG_MAX_FLASH_BANKS * (0 - CFG_FLASH0)) {
		memctl->or1 = 0;
		memctl->br1 = 0;

		/*
		 * Re-do flash protection upon new addresses
		 */
		flash_protect (FLAG_PROTECT_CLEAR,
			       gd->bd->bi_flashstart, 0xffffffff,
			       &flash_info[CFG_MAX_FLASH_BANKS - 1]);

		/* Monitor protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CFG_MONITOR_BASE,
			       CFG_MONITOR_BASE + monitor_flash_len - 1,
			       &flash_info[CFG_MAX_FLASH_BANKS - 1]);

		/* Environment protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CFG_ENV_ADDR,
			       CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
			       &flash_info[CFG_MAX_FLASH_BANKS - 1]);

		/* Redundant environment protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CFG_ENV_ADDR_REDUND,
			       CFG_ENV_ADDR_REDUND + CFG_ENV_SIZE_REDUND - 1,
			       &flash_info[CFG_MAX_FLASH_BANKS - 1]);
	}

	return 0;
}

/*
 * Initialize Local Bus
 */
void local_bus_init (void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);

	uint clkdiv;
	uint lbc_hz;
	sys_info_t sysinfo;

	/*
	 * Errata LBC11.
	 * Fix Local Bus clock glitch when DLL is enabled.
	 *
	 * If localbus freq is < 66Mhz, DLL bypass mode must be used.
	 * If localbus freq is > 133Mhz, DLL can be safely enabled.
	 * Between 66 and 133, the DLL is enabled with an override workaround.
	 */

	get_sys_info (&sysinfo);
	clkdiv = lbc->lcrr & 0x0f;
	lbc_hz = sysinfo.freqSystemBus / 1000000 / clkdiv;

	if (lbc_hz < 66) {
		lbc->lcrr = CFG_LBC_LCRR | 0x80000000;	/* DLL Bypass */
		lbc->ltedr = 0xa4c80000;	/* DK: !!! */

	} else if (lbc_hz >= 133) {
		lbc->lcrr = CFG_LBC_LCRR & (~0x80000000);	/* DLL Enabled */

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

		lbc->lcrr = CFG_LBC_LCRR & (~0x80000000);	/* DLL Enabled */
		udelay (200);

		/*
		 * Sample LBC DLL ctrl reg, upshift it to set the
		 * override bits.
		 */
		temp_lbcdll = gur->lbcdllcr;
		gur->lbcdllcr = (((temp_lbcdll & 0xff) << 16) | 0x80000000);
		asm ("sync;isync;msync");
	}
}

#if defined(CONFIG_PCI)
/*
 * Initialize PCI Devices, report devices found.
 */

#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_mpc85xxads_config_table[] = {
	{PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 PCI_IDSEL_NUMBER, PCI_ANY_ID,
	 pci_cfgfunc_config_device, {PCI_ENET0_IOADDR,
				     PCI_ENET0_MEMADDR,
				     PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER}},
	{}
};
#endif

static struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
      config_table:pci_mpc85xxads_config_table,
#endif
};

#endif /* CONFIG_PCI */

void pci_init_board (void)
{
#ifdef CONFIG_PCI
	pci_mpc85xx_init (&hose);
#endif /* CONFIG_PCI */
}

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r (void)
{
#ifdef CONFIG_PS2MULT
	ps2mult_early_init ();
#endif /* CONFIG_PS2MULT */
	return (0);
}
#endif /* CONFIG_BOARD_EARLY_INIT_R */
