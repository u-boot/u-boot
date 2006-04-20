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
#include <spd.h>
#include <flash.h>

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[];	/* FLASH chips info */

void local_bus_init (void);
long int fixed_sdram (void);
ulong flash_get_size (ulong base, int banknum);

#ifdef CONFIG_CPM2
/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {   /*            conf ppar psor pdir podr pdat */
	/* PA31 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII COL */
	/* PA30 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII CRS */
	/* PA29 */ {   1,   1,   1,   1,   0,   0   }, /* FCC1 MII TX_ER */
	/* PA28 */ {   1,   1,   1,   1,   0,   0   }, /* FCC1 MII TX_EN */
	/* PA27 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII RX_DV */
	/* PA26 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII RX_ER */
	/* PA25 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[0] */
	/* PA24 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[1] */
	/* PA23 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[2] */
	/* PA22 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[3] */
	/* PA21 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[3] */
	/* PA20 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[2] */
	/* PA19 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[1] */
	/* PA18 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[0] */
	/* PA17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[0] */
	/* PA16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[1] */
	/* PA15 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[2] */
	/* PA14 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[3] */
	/* PA13 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[3] */
	/* PA12 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[2] */
	/* PA11 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[1] */
	/* PA10 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[0] */
	/* PA9  */ {   0,   1,   1,   1,   0,   0   }, /* FCC1 L1TXD */
	/* PA8  */ {   0,   1,   1,   0,   0,   0   }, /* FCC1 L1RXD */
	/* PA7  */ {   0,   0,   0,   1,   0,   0   }, /* PA7 */
	/* PA6  */ {   0,   1,   1,   1,   0,   0   }, /* TDM A1 L1RSYNC */
	/* PA5  */ {   0,   0,   0,   1,   0,   0   }, /* PA5 */
	/* PA4  */ {   0,   0,   0,   1,   0,   0   }, /* PA4 */
	/* PA3  */ {   0,   0,   0,   1,   0,   0   }, /* PA3 */
	/* PA2  */ {   0,   0,   0,   1,   0,   0   }, /* PA2 */
	/* PA1  */ {   0,   0,   0,   0,   0,   0   }, /* FREERUN */
	/* PA0  */ {   0,   0,   0,   1,   0,   0   }  /* PA0 */
    },

    /* Port B configuration */
    {   /*            conf ppar psor pdir podr pdat */
	/* PB31 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TX_ER */
	/* PB30 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_DV */
	/* PB29 */ {   1,   1,   1,   1,   0,   0   }, /* FCC2 MII TX_EN */
	/* PB28 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_ER */
	/* PB27 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII COL */
	/* PB26 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII CRS */
	/* PB25 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[3] */
	/* PB24 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[2] */
	/* PB23 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[1] */
	/* PB22 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[0] */
	/* PB21 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[0] */
	/* PB20 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[1] */
	/* PB19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[2] */
	/* PB18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[3] */
	/* PB17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3:RX_DIV */
	/* PB16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3:RX_ERR */
	/* PB15 */ {   1,   1,   0,   1,   0,   0   }, /* FCC3:TX_ERR */
	/* PB14 */ {   1,   1,   0,   1,   0,   0   }, /* FCC3:TX_EN */
	/* PB13 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3:COL */
	/* PB12 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3:CRS */
	/* PB11 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB10 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB9  */ {   1,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB8  */ {   1,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB7  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB6  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB5  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB4  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*            conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   1,   0,   0   }, /* PC31 */
	/* PC30 */ {   0,   0,   0,   1,   0,   0   }, /* PC30 */
	/* PC29 */ {   0,   1,   1,   0,   0,   0   }, /* SCC1 EN *CLSN */
	/* PC28 */ {   0,   0,   0,   1,   0,   0   }, /* PC28 */
	/* PC27 */ {   0,   0,   0,   1,   0,   0   }, /* UART Clock in */
	/* PC26 */ {   0,   0,   0,   1,   0,   0   }, /* PC26 */
	/* PC25 */ {   0,   0,   0,   1,   0,   0   }, /* PC25 */
	/* PC24 */ {   0,   0,   0,   1,   0,   0   }, /* PC24 */
	/* PC23 */ {   0,   1,   0,   1,   0,   0   }, /* ATMTFCLK */
	/* PC22 */ {   0,   1,   0,   0,   0,   0   }, /* ATMRFCLK */
	/* PC21 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN RXCLK */
	/* PC20 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN TXCLK */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_CLK CLK13 */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC Tx Clock (CLK14) */
	/* PC17 */ {   1,   1,   0,   0,   0,   0   }, /* PC17 */
	/* PC16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC Tx Clock (CLK16) */
	/* PC15 */ {   0,   1,   0,   0,   0,   0   }, /* PC15 */
	/* PC14 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN *CD */
	/* PC13 */ {   0,   1,   0,   0,   0,   0   }, /* PC13 */
	/* PC12 */ {   0,   1,   0,   1,   0,   0   }, /* PC12 */
	/* PC11 */ {   0,   0,   0,   1,   0,   0   }, /* LXT971 transmit control */
	/* PC10 */ {   0,   0,   0,   1,   0,   0   }, /* FETHMDC */
	/* PC9  */ {   0,   0,   0,   0,   0,   0   }, /* FETHMDIO */
	/* PC8  */ {   0,   0,   0,   1,   0,   0   }, /* PC8 */
	/* PC7  */ {   0,   0,   0,   1,   0,   0   }, /* PC7 */
	/* PC6  */ {   0,   0,   0,   1,   0,   0   }, /* PC6 */
	/* PC5  */ {   0,   0,   0,   1,   0,   0   }, /* PC5 */
	/* PC4  */ {   0,   0,   0,   1,   0,   0   }, /* PC4 */
	/* PC3  */ {   0,   0,   0,   1,   0,   0   }, /* PC3 */
	/* PC2  */ {   0,   0,   0,   1,   0,   1   }, /* ENET FDE */
	/* PC1  */ {   0,   0,   0,   1,   0,   0   }, /* ENET DSQE */
	/* PC0  */ {   0,   0,   0,   1,   0,   0   }, /* ENET LBK */
    },

    /* Port D */
    {   /*            conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN RxD */
	/* PD30 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 EN TxD */
	/* PD29 */ {   1,   1,   0,   1,   0,   0   }, /* SCC1 EN TENA */
	/* PD28 */ {   1,   1,   0,   0,   0,   0   }, /* PD28 */
	/* PD27 */ {   1,   1,   0,   1,   0,   0   }, /* PD27 */
	/* PD26 */ {   1,   1,   0,   1,   0,   0   }, /* PD26 */
	/* PD25 */ {   0,   0,   0,   1,   0,   0   }, /* PD25 */
	/* PD24 */ {   0,   0,   0,   1,   0,   0   }, /* PD24 */
	/* PD23 */ {   0,   0,   0,   1,   0,   0   }, /* PD23 */
	/* PD22 */ {   0,   0,   0,   1,   0,   0   }, /* PD22 */
	/* PD21 */ {   0,   0,   0,   1,   0,   0   }, /* PD21 */
	/* PD20 */ {   0,   0,   0,   1,   0,   0   }, /* PD20 */
	/* PD19 */ {   0,   0,   0,   1,   0,   0   }, /* PD19 */
	/* PD18 */ {   0,   0,   0,   1,   0,   0   }, /* PD18 */
	/* PD17 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXPRTY */
	/* PD16 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXPRTY */
	/* PD15 */ {   0,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   0,   0,   0,   1,   0,   0   }, /* LED */
	/* PD13 */ {   0,   0,   0,   0,   0,   0   }, /* PD13 */
	/* PD12 */ {   0,   0,   0,   0,   0,   0   }, /* PD12 */
	/* PD11 */ {   0,   0,   0,   0,   0,   0   }, /* PD11 */
	/* PD10 */ {   0,   0,   0,   0,   0,   0   }, /* PD10 */
	/* PD9  */ {   0,   1,   0,   1,   0,   0   }, /* SMC1 TXD */
	/* PD8  */ {   0,   1,   0,   0,   0,   0   }, /* SMC1 RXD */
	/* PD7  */ {   0,   0,   0,   1,   0,   1   }, /* PD7 */
	/* PD6  */ {   0,   0,   0,   1,   0,   1   }, /* PD6 */
	/* PD5  */ {   0,   0,   0,   1,   0,   1   }, /* PD5 */
	/* PD4  */ {   0,   0,   0,   1,   0,   1   }, /* PD4 */
	/* PD3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    }
};
#endif /*  CONFIG_CPM2 */

#define CASL_STRING1	"casl=xx"
#define CASL_STRING2	"casl="

static const int casl_table[] = { 20, 25, 30 };
#define	N_CASL (sizeof(casl_table) / sizeof(casl_table[0]))

int cas_latency(void)
{
	char *s = getenv("serial#");
	int casl;
	int val;
	int i;

	casl = CONFIG_DDR_DEFAULT_CL;

	if (s != NULL) {
		if (strncmp(s + strlen(s) - strlen(CASL_STRING1), CASL_STRING2,
			    strlen(CASL_STRING2)) == 0) {
			val = simple_strtoul(s + strlen(s) - 2, NULL, 10);

			for (i=0; i<N_CASL; ++i) {
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
	char *s = getenv("serial#");

	printf("Board: %s", CONFIG_BOARDNAME);
	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

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
	volatile immap_t    *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_lbc_t *memctl = &immap->im_lbc;

	/*
	 * Adjust flash start and offset to detected values
	 */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Check if boot FLASH isn't max size
	 */
	if (gd->bd->bi_flashsize < (0 - CFG_FLASH0)) {
		memctl->or0 = gd->bd->bi_flashstart | (CFG_OR0_PRELIM & 0x00007fff);
		memctl->br0 = gd->bd->bi_flashstart | (CFG_BR0_PRELIM & 0x00007fff);

		/*
		 * Re-check to get correct base address
		 */
		flash_get_size(gd->bd->bi_flashstart, CFG_MAX_FLASH_BANKS - 1);
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
			       CFG_MONITOR_BASE, CFG_MONITOR_BASE + monitor_flash_len - 1,
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
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	volatile ccsr_lbc_t *lbc = &immap->im_lbc;

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
				     PCI_COMMAND_MEMORY |
				     PCI_COMMAND_MASTER}},
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
	extern void pci_mpc85xx_init (struct pci_controller *hose);

	pci_mpc85xx_init (&hose);
#endif /* CONFIG_PCI */
}
