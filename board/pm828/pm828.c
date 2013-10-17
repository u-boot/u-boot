/*
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ioports.h>
#include <mpc8260.h>
#include <pci.h>
#include <netdev.h>

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PA31 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 COL */
	/* PA30 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 CRS */
	/* PA29 */ {   1,   1,   1,   1,   0,   0   }, /* FCC1 TXER */
	/* PA28 */ {   1,   1,   1,   1,   0,   0   }, /* FCC1 TXEN */
	/* PA27 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 RXDV */
	/* PA26 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 RXER */
	/* PA25 */ {   0,   0,   0,   1,   0,   0   }, /* PA25 */
	/* PA24 */ {   0,   0,   0,   1,   0,   0   }, /* PA24 */
	/* PA23 */ {   0,   0,   0,   1,   0,   0   }, /* PA23 */
	/* PA22 */ {   0,   0,   0,   1,   0,   0   }, /* PA22 */
	/* PA21 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 TXD3 */
	/* PA20 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 TXD2 */
	/* PA19 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 TXD1 */
	/* PA18 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 TXD0 */
	/* PA17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 RXD0 */
	/* PA16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 RXD1*/
	/* PA15 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 RXD2 */
	/* PA14 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 RXD3 */
	/* PA13 */ {   0,   0,   0,   1,   0,   0   }, /* PA13 */
	/* PA12 */ {   0,   0,   0,   1,   0,   0   }, /* PA12 */
	/* PA11 */ {   0,   0,   0,   1,   0,   0   }, /* PA11 */
	/* PA10 */ {   0,   0,   0,   1,   0,   0   }, /* PA10 */
	/* PA9  */ {   0,   1,   0,   1,   0,   0   }, /* PA9 */
	/* PA8  */ {   0,   1,   0,   0,   0,   0   }, /* PA8 */
	/* PA7  */ {   0,   0,   0,   1,   0,   0   }, /* PA7 */
	/* PA6  */ {   0,   0,   0,   1,   0,   0   }, /* PA6 */
	/* PA5  */ {   0,   0,   0,   1,   0,   0   }, /* PA5 */
	/* PA4  */ {   0,   0,   0,   1,   0,   0   }, /* PA4 */
	/* PA3  */ {   0,   0,   0,   1,   0,   0   }, /* PA3 */
	/* PA2  */ {   0,   0,   0,   1,   0,   0   }, /* PA2 */
	/* PA1  */ {   0,   0,   0,   1,   0,   0   }, /* PA1 */
	/* PA0  */ {   0,   0,   0,   1,   0,   0   }  /* PA0 */
    },

    /* Port B configuration */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PB31 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 TX_ER */
	/* PB30 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 RX_DV  */
	/* PB29 */ {   1,   1,   1,   1,   0,   0   }, /* FCC2 TX_EN  */
#if defined(CONFIG_ETHER_ON_SCC) && (CONFIG_ETHER_INDEX == 1)
#ifdef CONFIG_ETHER_ON_FCC2
#error "SCC1 conflicts with FCC2"
#endif
	/* PB28 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 TXD */
#else
	/* PB28 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 RX_ER */
#endif
	/* PB27 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 COL */
	/* PB26 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 CRS */
	/* PB25 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 TxD[3] */
	/* PB24 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 TxD[2] */
	/* PB23 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 TxD[1] */
	/* PB22 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 TxD[0] */
	/* PB21 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 RxD[0] */
	/* PB20 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 RxD[1] */
	/* PB19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 RxD[2] */
	/* PB18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 RxD[3] */
	/* PB17 */ {   0,   0,   0,   0,   0,   0   }, /* PB17 */
	/* PB16 */ {   0,   0,   0,   0,   0,   0   }, /* PB16 */
	/* PB15 */ {   1,   1,   0,   0,   0,   0   }, /* SCC2 RXD */
	/* PB14 */ {   1,   1,   0,   0,   0,   0   }, /* SCC3 RXD */
	/* PB13 */ {   0,   0,   0,   0,   0,   0   }, /* PB13 */
	/* PB12 */ {   0,   0,   0,   0,   0,   0   }, /* PB12 */
	/* PB11 */ {   0,   0,   0,   0,   0,   0   }, /* PB11 */
	/* PB10 */ {   0,   0,   0,   0,   0,   0   }, /* PB10 */
	/* PB9  */ {   0,   0,   0,   0,   0,   0   }, /* PB9 */
	/* PB8  */ {   1,   1,   1,   1,   0,   0   }, /* SCC3 TXD */
	/* PB7  */ {   0,   0,   0,   0,   0,   0   }, /* PB7 */
	/* PB6  */ {   0,   0,   0,   0,   0,   0   }, /* PB6 */
	/* PB5  */ {   0,   0,   0,   0,   0,   0   }, /* PB5 */
	/* PB4  */ {   0,   0,   0,   0,   0,   0   }, /* PB4 */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   1,   0,   0   }, /* PC31 */
	/* PC30 */ {   0,   0,   0,   1,   0,   0   }, /* PC30 */
	/* PC29 */ {   0,   1,   1,   0,   0,   0   }, /* SCC1 CTS */
	/* PC28 */ {   0,   0,   0,   1,   0,   0   }, /* SCC2 CTS */
	/* PC27 */ {   0,   0,   0,   1,   0,   0   }, /* PC27 */
	/* PC26 */ {   0,   0,   0,   1,   0,   0   }, /* PC26 */
	/* PC25 */ {   0,   0,   0,   1,   0,   0   }, /* PC25 */
	/* PC24 */ {   0,   0,   0,   1,   0,   0   }, /* PC24 */
	/* PC23 */ {   0,   1,   0,   1,   0,   0   }, /* PC23 */
	/* PC22 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 TXCK */
	/* PC21 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 RXCK */
	/* PC20 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 TXCK(2) */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 RXCK */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 TXCK */
	/* PC17 */ {   0,   0,   0,   1,   0,   0   }, /* PC17 */
	/* PC16 */ {   0,   0,   0,   1,   0,   0   }, /* PC16 */
	/* PC15 */ {   1,   1,   0,   1,   0,   0   }, /* SMC2 TXD */
	/* PC14 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 DCD */
	/* PC13 */ {   0,   0,   0,   1,   0,   0   }, /* PC13 */
	/* PC12 */ {   0,   0,   0,   1,   0,   0   }, /* SCC2 DCD */
	/* PC11 */ {   0,   0,   0,   1,   0,   0   }, /* SCC3 CTS */
	/* PC10 */ {   0,   0,   0,   1,   0,   0   }, /* SCC3 DCD */
	/* PC9  */ {   0,   0,   0,   1,   0,   0   }, /* SCC4 CTS */
	/* PC8  */ {   0,   0,   0,   1,   0,   0   }, /* SCC4 DCD */
	/* PC7  */ {   0,   0,   0,   1,   0,   0   }, /* PC7 */
	/* PC6  */ {   0,   0,   0,   1,   0,   0   }, /* PC6 */
	/* PC5  */ {   0,   0,   0,   1,   0,   0   }, /* PC5 */
	/* PC4  */ {   0,   0,   0,   1,   0,   0   }, /* PC4 */
	/* PC3  */ {   0,   0,   0,   1,   0,   0   }, /* PC3 */
	/* PC2  */ {   0,   0,   0,   1,   0,   1   }, /* PC2 */
	/* PC1  */ {   0,   0,   0,   1,   0,   0   }, /* PC1 */
	/* PC0  */ {   0,   0,   0,   1,   0,   0   }, /* PC0 */
    },

    /* Port D */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 RXD */
	/* PD30 */ {   0,   1,   1,   1,   0,   0   }, /* PD30 */
	/* PD29 */ {   0,   1,   0,   1,   0,   0   }, /* SCC1 RTS */
	/* PD28 */ {   0,   0,   0,   1,   0,   0   }, /* PD28 */
	/* PD27 */ {   0,   1,   0,   1,   0,   0   }, /* SCC2 RTS */
	/* PD26 */ {   0,   0,   0,   1,   0,   0   }, /* PD26 */
	/* PD25 */ {   0,   0,   0,   1,   0,   0   }, /* PD25 */
	/* PD24 */ {   0,   0,   0,   1,   0,   0   }, /* PD24 */
	/* PD23 */ {   0,   0,   0,   1,   0,   0   }, /* SCC3 RTS */
	/* PD22 */ {   1,   1,   0,   0,   0,   0   }, /* SCC4 RXD */
	/* PD21 */ {   1,   1,   0,   1,   0,   0   }, /* SCC4 TXD */
	/* PD20 */ {   0,   0,   1,   1,   0,   0   }, /* SCC4 RTS */
	/* PD19 */ {   0,   0,   0,   1,   0,   0   }, /* PD19 */
	/* PD18 */ {   0,   0,   0,   1,   0,   0   }, /* PD18 */
	/* PD17 */ {   0,   1,   0,   0,   0,   0   }, /* PD17 */
	/* PD16 */ {   0,   1,   0,   1,   0,   0   }, /* PD16 */
#if defined(CONFIG_SYS_I2C_SOFT)
	/* PD15 */ {   1,   0,   0,   1,   1,   1   }, /* I2C SDA */
	/* PD14 */ {   1,   0,   0,   1,   1,   1   }, /* I2C SCL */
#else
#if defined(CONFIG_HARD_I2C)
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
#else /* normal I/O port pins */
	/* PD15 */ {   0,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   0,   1,   1,   0,   1,   0   }, /* I2C SCL */
#endif
#endif
	/* PD13 */ {   0,   0,   0,   0,   0,   0   }, /* PD13 */
	/* PD12 */ {   0,   0,   0,   0,   0,   0   }, /* PD12 */
	/* PD11 */ {   0,   0,   0,   0,   0,   0   }, /* PD11 */
	/* PD10 */ {   0,   0,   0,   0,   0,   0   }, /* PD10 */
	/* PD9  */ {   0,   1,   0,   1,   0,   0   }, /* PD9 */
	/* PD8  */ {   0,   1,   0,   0,   0,   0   }, /* PD8 */
	/* PD7  */ {   0,   0,   0,   1,   0,   1   }, /* PD7 */
	/* PD6  */ {   0,   0,   0,   1,   0,   1   }, /* PD6 */
	/* PD5  */ {   0,   0,   0,   1,   0,   1   }, /* PD5 */
	/* PD4  */ {   1,   1,   1,   0,   0,   0   }, /* SMC2 RXD */
	/* PD3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    }
};

/* ------------------------------------------------------------------------- */

/* Check Board Identity:
 */
int checkboard (void)
{
	puts ("Board: PM828\n");
	return 0;
}

/* ------------------------------------------------------------------------- */


/* Try SDRAM initialization with P/LSDMR=sdmr and ORx=orx
 *
 * This routine performs standard 8260 initialization sequence
 * and calculates the available memory size. It may be called
 * several times to try different SDRAM configurations on both
 * 60x and local buses.
 */
static long int try_init (volatile memctl8260_t * memctl, ulong sdmr,
						  ulong orx, volatile uchar * base)
{
	volatile uchar c = 0xff;
	volatile ulong cnt, val;
	volatile ulong *addr;
	volatile uint *sdmr_ptr;
	volatile uint *orx_ptr;
	int i;
	ulong save[32];				/* to make test non-destructive */
	ulong maxsize;

	/* We must be able to test a location outsize the maximum legal size
	 * to find out THAT we are outside; but this address still has to be
	 * mapped by the controller. That means, that the initial mapping has
	 * to be (at least) twice as large as the maximum expected size.
	 */
	maxsize = (1 + (~orx | 0x7fff)) / 2;

	sdmr_ptr = &memctl->memc_psdmr;
	orx_ptr = &memctl->memc_or2;

	*orx_ptr = orx;

	/*
	 * Quote from 8260 UM (10.4.2 SDRAM Power-On Initialization, 10-35):
	 *
	 * "At system reset, initialization software must set up the
	 *  programmable parameters in the memory controller banks registers
	 *  (ORx, BRx, P/LSDMR). After all memory parameters are configured,
	 *  system software should execute the following initialization sequence
	 *  for each SDRAM device.
	 *
	 *  1. Issue a PRECHARGE-ALL-BANKS command
	 *  2. Issue eight CBR REFRESH commands
	 *  3. Issue a MODE-SET command to initialize the mode register
	 *
	 *  The initial commands are executed by setting P/LSDMR[OP] and
	 *  accessing the SDRAM with a single-byte transaction."
	 *
	 * The appropriate BRx/ORx registers have already been set when we
	 * get here. The SDRAM can be accessed at the address CONFIG_SYS_SDRAM_BASE.
	 */

	*sdmr_ptr = sdmr | PSDMR_OP_PREA;
	*base = c;

	*sdmr_ptr = sdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
		*base = c;

	*sdmr_ptr = sdmr | PSDMR_OP_MRW;
	*(base + CONFIG_SYS_MRS_OFFS) = c;	/* setting MR on address lines */

	*sdmr_ptr = sdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*base = c;

	/*
	 * Check memory range for valid RAM. A simple memory test determines
	 * the actually available RAM size between addresses `base' and
	 * `base + maxsize'. Some (not all) hardware errors are detected:
	 * - short between address lines
	 * - short between data lines
	 */
	i = 0;
	for (cnt = maxsize / sizeof (long); cnt > 0; cnt >>= 1) {
		addr = (volatile ulong *) base + cnt;	/* pointer arith! */
		save[i++] = *addr;
		*addr = ~cnt;
	}

	addr = (volatile ulong *) base;
	save[i] = *addr;
	*addr = 0;

	if ((val = *addr) != 0) {
		*addr = save[i];
		return (0);
	}

	for (cnt = 1; cnt <= maxsize / sizeof (long); cnt <<= 1) {
		addr = (volatile ulong *) base + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		if (val != ~cnt) {
			/* Write the actual size to ORx
			 */
			*orx_ptr = orx | ~(cnt * sizeof (long) - 1);
			return (cnt * sizeof (long));
		}
	}
	return (maxsize);
}


phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;

#ifndef CONFIG_SYS_RAMBOOT
	ulong size8, size9;
#endif
	ulong psize = 32 * 1024 * 1024;

	memctl->memc_psrt = CONFIG_SYS_PSRT;
	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

#ifndef CONFIG_SYS_RAMBOOT
	size8 = try_init (memctl, CONFIG_SYS_PSDMR_8COL, CONFIG_SYS_OR2_8COL,
					  (uchar *) CONFIG_SYS_SDRAM_BASE);
	size9 = try_init (memctl, CONFIG_SYS_PSDMR_9COL, CONFIG_SYS_OR2_9COL,
					  (uchar *) CONFIG_SYS_SDRAM_BASE);

	if (size8 < size9) {
		psize = size9;
		printf ("(60x:9COL) ");
	} else {
		psize = try_init (memctl, CONFIG_SYS_PSDMR_8COL, CONFIG_SYS_OR2_8COL,
						  (uchar *) CONFIG_SYS_SDRAM_BASE);
		printf ("(60x:8COL) ");
	}
#endif
	return (psize);
}

#if defined(CONFIG_CMD_DOC)
void doc_init (void)
{
	doc_probe (CONFIG_SYS_DOC_BASE);
}
#endif

#ifdef	CONFIG_PCI
struct pci_controller hose;

extern void pci_mpc8250_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc8250_init(&hose);
}
#endif

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
