/*
 * Copyright (C) 2006 Embedded Planet, LLC.
 *
 * Support for Embedded Planet EP82xxM boards.
 * Tested on EP82xxM (MPC8270).
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8260.h>
#include <ioports.h>
#include <asm/m8260_pci.h>
#ifdef CONFIG_PCI
#include <pci.h>
#endif
#include <miiphy.h>
#include <linux/compiler.h>

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

#define CONFIG_SYS_FCC2 1
#define CONFIG_SYS_FCC3 1

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A */
    {	/*	     conf      ppar psor pdir podr pdat */
	/* PA31 */ { 0,		 0,   0,   0,	0,   1 }, /* PA31	     */
	/* PA30 */ { 0,		 0,   0,   0,	0,   1 }, /* PA30	     */
	/* PA29 */ { 0,		 0,   0,   0,	0,   1 }, /* PA29	     */
	/* PA28 */ { 0,		 0,   0,   0,	0,   1 }, /* PA28	     */
	/* PA27 */ { 0,		 0,   0,   0,	0,   1 }, /* PA27	     */
	/* PA26 */ { 0,		 0,   0,   0,	0,   1 }, /* PA26	     */
	/* PA25 */ { 0,		 0,   0,   0,	0,   1 }, /* PA25	     */
	/* PA24 */ { 0,		 0,   0,   0,	0,   1 }, /* PA24	     */
	/* PA23 */ { 0,		 0,   0,   0,	0,   1 }, /* PA23	     */
	/* PA22 */ { 0,		 0,   0,   0,	0,   0 }, /* PA22	     */
	/* PA21 */ { 0,		 0,   0,   0,	0,   1 }, /* PA21	     */
	/* PA20 */ { 0,		 0,   0,   0,	0,   1 }, /* PA20	     */
	/* PA19 */ { 0,		 0,   0,   0,	0,   1 }, /* PA19	     */
	/* PA18 */ { 0,		 0,   0,   0,	0,   1 }, /* PA18	     */
	/* PA17 */ { 0,		 0,   0,   0,	0,   1 }, /* PA17	     */
	/* PA16 */ { 0,		 0,   0,   0,	0,   1 }, /* PA16	     */
	/* PA15 */ { 0,		 0,   0,   0,	0,   1 }, /* PA15	     */
	/* PA14 */ { 0,		 0,   0,   0,	0,   1 }, /* PA14	     */
	/* PA13 */ { 0,		 0,   0,   0,	0,   1 }, /* PA13	     */
	/* PA12 */ { 0,		 0,   0,   0,	0,   1 }, /* PA12	     */
	/* PA11 */ { 0,		 0,   0,   0,	0,   1 }, /* PA11	     */
	/* PA10 */ { 0,		 0,   0,   0,	0,   1 }, /* PA10	     */
	/* PA9	*/ { 1,		 1,   0,   1,	0,   1 }, /* SMC2 TxD	     */
	/* PA8	*/ { 1,		 1,   0,   0,	0,   1 }, /* SMC2 RxD	     */
	/* PA7	*/ { 0,		 0,   0,   0,	0,   1 }, /* PA7	     */
	/* PA6	*/ { 0,		 0,   0,   0,	0,   1 }, /* PA6	     */
	/* PA5	*/ { 0,		 0,   0,   0,	0,   1 }, /* PA5	     */
	/* PA4	*/ { 0,		 0,   0,   0,	0,   1 }, /* PA4	     */
	/* PA3	*/ { 0,		 0,   0,   0,	0,   1 }, /* PA3	     */
	/* PA2	*/ { 0,		 0,   0,   0,	0,   1 }, /* PA2	     */
	/* PA1	*/ { 0,		 0,   0,   0,	0,   1 }, /* PA1	     */
	/* PA0	*/ { 0,		 0,   0,   0,	0,   1 }  /* PA0	     */
    },

    /* Port B */
    {	/*	     conf	ppar psor pdir podr pdat */
	/* PB31 */ { CONFIG_SYS_FCC2,	 1,   0,   1,	0,   0 }, /* FCC2 MII TX_ER  */
	/* PB30 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* FCC2 MII RX_DV  */
	/* PB29 */ { CONFIG_SYS_FCC2,	 1,   1,   1,	0,   0 }, /* FCC2 MII TX_EN  */
	/* PB28 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* FCC2 MII RX_ER  */
	/* PB27 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* FCC2 MII COL    */
	/* PB26 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* FCC2 MII CRS    */
	/* PB25 */ { CONFIG_SYS_FCC2,	 1,   0,   1,	0,   0 }, /* FCC2 MII TxD[3] */
	/* PB24 */ { CONFIG_SYS_FCC2,	 1,   0,   1,	0,   0 }, /* FCC2 MII TxD[2] */
	/* PB23 */ { CONFIG_SYS_FCC2,	 1,   0,   1,	0,   0 }, /* FCC2 MII TxD[1] */
	/* PB22 */ { CONFIG_SYS_FCC2,	 1,   0,   1,	0,   0 }, /* FCC2 MII TxD[0] */
	/* PB21 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* FCC2 MII RxD[0] */
	/* PB20 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* FCC2 MII RxD[1] */
	/* PB19 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* FCC2 MII RxD[2] */
	/* PB18 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* FCC2 MII RxD[3] */
	/* PB17 */ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* FCC3:RX_DIV     */
	/* PB16 */ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* FCC3:RX_ERR     */
	/* PB15 */ { CONFIG_SYS_FCC3,	 1,   0,   1,	0,   0 }, /* FCC3:TX_ERR     */
	/* PB14 */ { CONFIG_SYS_FCC3,	 1,   0,   1,	0,   0 }, /* FCC3:TX_EN      */
	/* PB13 */ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* FCC3:COL	     */
	/* PB12 */ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* FCC3:CRS	     */
	/* PB11 */ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* FCC3:RXD	     */
	/* PB10 */ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* FCC3:RXD	     */
	/* PB9	*/ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* FCC3:RXD	     */
	/* PB8	*/ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* FCC3:RXD	     */
	/* PB7	*/ { 0,		 0,   0,   0,	0,   0 }, /* PB7	     */
	/* PB6	*/ { CONFIG_SYS_FCC3,	 1,   0,   1,	0,   0 }, /* FCC3:TXD	     */
	/* PB5	*/ { CONFIG_SYS_FCC3,	 1,   0,   1,	0,   0 }, /* FCC3:TXD	     */
	/* PB4	*/ { CONFIG_SYS_FCC3,	 1,   0,   1,	0,   0 }, /* FCC3:TXD	     */
	/* PB3	*/ { 0,		 0,   0,   0,	0,   0 }, /* non-existent    */
	/* PB2	*/ { 0,		 0,   0,   0,	0,   0 }, /* non-existent    */
	/* PB1	*/ { 0,		 0,   0,   0,	0,   0 }, /* non-existent    */
	/* PB0	*/ { 0,		 0,   0,   0,	0,   0 }  /* non-existent    */
    },

    /* Port C */
    {	/*	     conf	ppar psor pdir podr pdat */
	/* PC31 */ { 0,		 0,   0,   0,	0,   0 }, /* PC31	     */
	/* PC30 */ { 0,		 0,   0,   0,	0,   0 }, /* PC30	     */
	/* PC29 */ { 1,		 1,   1,   0,	0,   0 }, /* SCC1 CTS#	     */
	/* PC28 */ { 0,		 0,   0,   0,	0,   0 }, /* PC28	     */
	/* PC27 */ { CONFIG_SYS_FCC3,	 1,   0,   1,	0,   0 }, /* FCC3: TXD[0]    */
	/* PC26 */ { 0,		 0,   0,   0,	0,   0 }, /* PC26	     */
	/* PC25 */ { 0,		 0,   0,   0,	0,   0 }, /* PC25	     */
	/* PC24 */ { 0,		 0,   0,   0,	0,   0 }, /* PC24	     */
	/* PC23 */ { 0,		 0,   0,   0,	0,   0 }, /* PC23	     */
	/* PC22 */ { 0,		 0,   0,   0,	0,   0 }, /* PC22	     */
	/* PC21 */ { 0,		 0,   0,   0,	0,   0 }, /* PC21	     */
	/* PC20 */ { 0,		 0,   0,   0,	0,   0 }, /* PC20	     */
	/* PC19 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* RxClk (CLK13)   */
	/* PC18 */ { CONFIG_SYS_FCC2,	 1,   0,   0,	0,   0 }, /* TxClk (CLK14)   */
	/* PC17 */ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* RxClk (CLK15)   */
	/* PC16 */ { CONFIG_SYS_FCC3,	 1,   0,   0,	0,   0 }, /* TxClk (CLK16)   */
	/* PC15 */ { 0,		 0,   0,   0,	0,   0 }, /* PC15	     */
	/* PC14 */ { 1,		 1,   0,   0,	0,   0 }, /* SCC1 CD#	     */
	/* PC13 */ { 1,		 1,   0,   0,	0,   0 }, /* SCC2 CTS#	     */
	/* PC12 */ { 1,		 1,   0,   0,	0,   0 }, /* SCC2 CD#	     */
	/* PC11 */ { 0,		 0,   0,   0,	0,   0 }, /* PC11	     */
	/* PC10 */ { 1,		 1,   0,   0,	0,   0 }, /* SCC3 CD#	     */
	/* PC9	*/ { 0,		 0,   0,   0,	0,   0 }, /* PC9	     */
	/* PC8	*/ { 1,		 1,   1,   0,	0,   0 }, /* SCC3 CTS#	     */
	/* PC7	*/ { 0,		 0,   0,   0,	0,   0 }, /* PC7	     */
	/* PC6	*/ { 0,		 0,   0,   0,	0,   0 }, /* PC6	     */
	/* PC5	*/ { 0,		 0,   0,   0,	0,   0 }, /* PC5	     */
	/* PC4	*/ { 0,		 0,   0,   0,	0,   0 }, /* PC4	     */
	/* PC3	*/ { 0,		 0,   0,   0,	0,   0 }, /* PC3	     */
	/* PC2	*/ { 0,		 0,   0,   0,	0,   0 }, /* PC2	     */
	/* PC1	*/ { 0,		 0,   0,   0,	0,   0 }, /* PC1	     */
	/* PC0	*/ { 0,		 0,   0,   0,	0,   0 }, /* PC0	     */
    },

    /* Port D */
    {	/*	     conf      ppar psor pdir podr pdat */
	/* PD31 */ { 1,		 1,   0,   0,	0,   0 }, /* SCC1 RXD	     */
	/* PD30 */ { 1,		 1,   1,   1,	0,   1 }, /* SCC1 TXD	     */
	/* PD29 */ { 1,		 1,   0,   1,	0,   0 }, /* SCC1 RTS#	     */
	/* PD28 */ { 1,		 1,   0,   0,	0,   0 }, /* SCC2 RXD	     */
	/* PD27 */ { 1,		 1,   0,   1,	0,   0 }, /* SCC2 TXD	     */
	/* PD26 */ { 1,		 1,   0,   1,	0,   0 }, /* SCC2 RTS#	     */
	/* PD25 */ { 1,		 1,   0,   0,	0,   0 }, /* SCC3 RXD	     */
	/* PD24 */ { 1,		 1,   0,   1,	0,   0 }, /* SCC3 TXD	     */
	/* PD23 */ { 1,		 1,   0,   1,	0,   0 }, /* SCC3 RTS#	     */
	/* PD22 */ { 0,		 0,   0,   0,	0,   1 }, /* PD22	     */
	/* PD21 */ { 0,		 0,   0,   0,	0,   1 }, /* PD21	     */
	/* PD20 */ { 0,		 0,   0,   0,	0,   1 }, /* PD20	     */
	/* PD19 */ { 0,		 0,   0,   0,	0,   1 }, /* PD19	     */
	/* PD18 */ { 0,		 0,   0,   0,	0,   1 }, /* PD18	     */
	/* PD17 */ { 0,		 0,   0,   0,	0,   1 }, /* PD17	     */
	/* PD16 */ { 0,		 0,   0,   0,	0,   1 }, /* PD16	     */
	/* PD15 */ { 1,		 1,   1,   0,	1,   1 }, /* I2C SDA	     */
	/* PD14 */ { 1,		 1,   1,   0,	1,   1 }, /* I2C SCL	     */
	/* PD13 */ { 0,		 0,   0,   0,	0,   1 }, /* PD13	     */
	/* PD12 */ { 0,		 0,   0,   0,	0,   1 }, /* PD12	     */
	/* PD11 */ { 0,		 0,   0,   0,	0,   1 }, /* PD11	     */
	/* PD10 */ { 0,		 0,   0,   0,	0,   1 }, /* PD10	     */
	/* PD9	*/ { 1,		 1,   0,   1,	0,   1 }, /* SMC1 TxD	     */
	/* PD8	*/ { 1,		 1,   0,   0,	0,   1 }, /* SMC1 RxD	     */
	/* PD7	*/ { 1,		 1,   0,   0,	0,   1 }, /* SMC1 SMSYN      */
	/* PD6	*/ { 0,		 0,   0,   0,	0,   1 }, /* PD6	     */
	/* PD5	*/ { 0,		 0,   0,   0,	0,   1 }, /* PD5	     */
	/* PD4	*/ { 0,		 0,   0,   0,	0,   1 }, /* PD4	     */
	/* PD3	*/ { 0,		 0,   0,   0,	0,   0 }, /* non-existent    */
	/* PD2	*/ { 0,		 0,   0,   0,	0,   0 }, /* non-existent    */
	/* PD1	*/ { 0,		 0,   0,   0,	0,   0 }, /* non-existent    */
	/* PD0	*/ { 0,		 0,   0,   0,	0,   0 }  /* non-existent    */
    }
};

#ifdef CONFIG_PCI
typedef struct pci_ic_s {
	unsigned long pci_int_stat;
	unsigned long pci_int_mask;
}pci_ic_t;
#endif

int board_early_init_f (void)
{
	vu_char *bcsr = (vu_char *)CONFIG_SYS_BCSR;

	bcsr[4] |= 0x30; /* Turn the LEDs off */

#if defined(CONFIG_CONS_ON_SMC) || defined(CONFIG_KGDB_ON_SMC)
	bcsr[6] |= 0x10;
#endif
#if defined(CONFIG_CONS_ON_SCC) || defined(CONFIG_KGDB_ON_SCC)
	bcsr[7] |= 0x10;
#endif

#if CONFIG_SYS_FCC3
	bcsr[8] |= 0xC0;
#endif /* CONFIG_SYS_FCC3 */
#if CONFIG_SYS_FCC2
	bcsr[8] |= 0x30;
#endif /* CONFIG_SYS_FCC2 */

	return 0;
}

phys_size_t initdram(int board_type)
{
	/* Size in MB of SDRAM populated on board*/
	long int msize = 256;

#ifndef CONFIG_SYS_RAMBOOT
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	uint psdmr = CONFIG_SYS_PSDMR;
	int i;

	unsigned char	*ramptr1 = (unsigned char *)0x00000110;
	__maybe_unused unsigned char	ramtmp;

	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

udelay(400);

	/* Initialise 60x bus SDRAM */
	memctl->memc_psrt = CONFIG_SYS_PSRT;
	memctl->memc_or1  = CONFIG_SYS_SDRAM_OR;
	memctl->memc_br1  = CONFIG_SYS_SDRAM_BR;
	memctl->memc_psdmr = psdmr;

udelay(400);

	memctl->memc_psdmr = psdmr | PSDMR_OP_PREA; /* Precharge all banks */
	ramtmp = *ramptr1;
	memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR; /* CBR refresh */
	for (i = 0; i < 8; i++) {
		memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR; /* CBR refresh */
	}
	ramtmp = *ramptr1;
	memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;  /* Mode Register write */
	*ramptr1  = 0xFF;
	memctl->memc_psdmr = psdmr | PSDMR_RFEN;    /* Refresh enable */
#endif /* !CONFIG_SYS_RAMBOOT */

	/* Return total 60x bus SDRAM size */
	return msize * 1024 * 1024;
}

int checkboard(void)
{
	vu_char *bcsr = (vu_char *)CONFIG_SYS_BCSR;

	puts("Board: ");
	switch (bcsr[0]) {
	case 0x0A:
		printf("EP82xxM 1.0 CPLD revision %d\n", bcsr[1]);
		break;
	default:
		printf("unknown: ID=%02X\n", bcsr[0]);
	}

	return 0;
}

#ifdef CONFIG_PCI
struct pci_controller hose;

extern void pci_mpc8250_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc8250_init(&hose);
}
#endif
