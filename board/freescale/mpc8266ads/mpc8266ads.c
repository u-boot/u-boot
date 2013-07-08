/*
 * (C) Copyright 2001-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Modified during 2001 by
 * Advanced Communications Technologies (Australia) Pty. Ltd.
 * Howard Walker, Tuong Vu-Dinh
 *
 * (C) Copyright 2001, Stuart Hughes, Lineo Inc, stuarth@lineo.com
 * Added support for the 16M dram simm on the 8260ads boards
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ioports.h>
#include <i2c.h>
#include <mpc8260.h>
#include <pci.h>

/*
 * PBI Page Based Interleaving
 *   PSDMR_PBI page based interleaving
 *   0         bank based interleaving
 * External Address Multiplexing (EAMUX) adds a clock to address cycles
 *   (this can help with marginal board layouts)
 *   PSDMR_EAMUX  adds a clock
 *   0            no extra clock
 * Buffer Command (BUFCMD) adds a clock to command cycles.
 *   PSDMR_BUFCMD adds a clock
 *   0            no extra clock
 */
#define CONFIG_PBI		0
#define PESSIMISTIC_SDRAM	0
#define EAMUX			0	/* EST requires EAMUX */
#define BUFCMD			0


/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

	/* Port A configuration */
	{	/*  conf ppar psor pdir podr pdat */
	/* PA31 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 TxENB */
	/* PA30 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 TxClav   */
	/* PA29 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 TxSOC  */
	/* PA28 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 RxENB */
	/* PA27 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 RxSOC */
	/* PA26 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 RxClav */
	/* PA25 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[0] */
	/* PA24 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[1] */
	/* PA23 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[2] */
	/* PA22 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[3] */
	/* PA21 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[4] */
	/* PA20 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[5] */
	/* PA19 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[6] */
	/* PA18 */ {   0,   1,   0,   1,   0,   0   }, /* FCC1 ATMTXD[7] */
	/* PA17 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[7] */
	/* PA16 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[6] */
	/* PA15 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[5] */
	/* PA14 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[4] */
	/* PA13 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[3] */
	/* PA12 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[2] */
	/* PA11 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[1] */
	/* PA10 */ {   0,   1,   0,   0,   0,   0   }, /* FCC1 ATMRXD[0] */
	/* PA9  */ {   0,   1,   1,   1,   0,   0   }, /* FCC1 L1TXD */
	/* PA8  */ {   0,   1,   1,   0,   0,   0   }, /* FCC1 L1RXD */
	/* PA7  */ {   0,   0,   0,   1,   0,   0   }, /* PA7 */
	/* PA6  */ {   1,   1,   1,   1,   0,   0   }, /* TDM A1 L1RSYNC */
	/* PA5  */ {   0,   0,   0,   1,   0,   0   }, /* PA5 */
	/* PA4  */ {   0,   0,   0,   1,   0,   0   }, /* PA4 */
	/* PA3  */ {   0,   0,   0,   1,   0,   0   }, /* PA3 */
	/* PA2  */ {   0,   0,   0,   1,   0,   0   }, /* PA2 */
	/* PA1  */ {   1,   0,   0,   0,   0,   0   }, /* FREERUN */
	/* PA0  */ {   0,   0,   0,   1,   0,   0   }  /* PA0 */
	},

	/* Port B configuration */
	{	/*  conf ppar psor pdir podr pdat */
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
	/* PB17 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RX_DIV */
	/* PB16 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RX_ERR */
	/* PB15 */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TX_ERR */
	/* PB14 */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TX_EN */
	/* PB13 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:COL */
	/* PB12 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:CRS */
	/* PB11 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB10 */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB9  */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB8  */ {   0,   1,   0,   0,   0,   0   }, /* FCC3:RXD */
	/* PB7  */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB6  */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB5  */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB4  */ {   0,   1,   0,   1,   0,   0   }, /* FCC3:TXD */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
	},

	/* Port C */
	{	/*  conf ppar psor pdir podr pdat */
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
	/* PC21 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN RXCLK */
	/* PC20 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN TXCLK */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_CLK CLK13 */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC Tx Clock (CLK14) */
	/* PC17 */ {   0,   0,   0,   1,   0,   0   }, /* PC17 */
	/* PC16 */ {   0,   1,   0,   0,   0,   0   }, /* FCC Tx Clock (CLK16) */
	/* PC15 */ {   0,   0,   0,   1,   0,   0   }, /* PC15 */
	/* PC14 */ {   0,   1,   0,   0,   0,   0   }, /* SCC1 EN *CD */
	/* PC13 */ {   0,   0,   0,   1,   0,   0   }, /* PC13 */
	/* PC12 */ {   0,   1,   0,   1,   0,   0   }, /* PC12 */
	/* PC11 */ {   0,   0,   0,   1,   0,   0   }, /* LXT971 transmit control */
	/* PC10 */ {   1,   0,   0,   1,   0,   0   }, /* LXT970 FETHMDC */
	/* PC9  */ {   1,   0,   0,   0,   0,   0   }, /* LXT970 FETHMDIO */
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
	{	/*  conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 EN RxD */
	/* PD30 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 EN TxD */
	/* PD29 */ {   0,   1,   0,   1,   0,   0   }, /* SCC1 EN TENA */
	/* PD28 */ {   0,   1,   0,   0,   0,   0   }, /* PD28 */
	/* PD27 */ {   0,   1,   1,   1,   0,   0   }, /* PD27 */
	/* PD26 */ {   0,   0,   0,   1,   0,   0   }, /* PD26 */
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
	/* PD15 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   1,   0   }, /* I2C SCL */
	/* PD13 */ {   0,   0,   0,   0,   0,   0   }, /* PD13 */
	/* PD12 */ {   0,   0,   0,   0,   0,   0   }, /* PD12 */
	/* PD11 */ {   0,   0,   0,   0,   0,   0   }, /* PD11 */
	/* PD10 */ {   0,   0,   0,   0,   0,   0   }, /* PD10 */
	/* PD9  */ {   1,   1,   0,   1,   0,   0   }, /* SMC1 TXD */
	/* PD8  */ {   1,   1,   0,   0,   0,   0   }, /* SMC1 RXD */
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

typedef struct bscr_ {
	unsigned long bcsr0;
	unsigned long bcsr1;
	unsigned long bcsr2;
	unsigned long bcsr3;
	unsigned long bcsr4;
	unsigned long bcsr5;
	unsigned long bcsr6;
	unsigned long bcsr7;
} bcsr_t;

typedef struct pci_ic_s {
	unsigned long pci_int_stat;
	unsigned long pci_int_mask;
} pci_ic_t;

void reset_phy(void)
{
	volatile bcsr_t *bcsr = (bcsr_t *)CONFIG_SYS_BCSR;

	/* reset the FEC port */
	bcsr->bcsr1 &= ~FETH_RST;
	bcsr->bcsr1 |= FETH_RST;
}


int board_early_init_f(void)
{
	volatile bcsr_t *bcsr = (bcsr_t *)CONFIG_SYS_BCSR;
	volatile pci_ic_t *pci_ic = (pci_ic_t *)CONFIG_SYS_PCI_INT;

	bcsr->bcsr1 = ~FETHIEN & ~RS232EN_1 & ~RS232EN_2;

	/* mask all PCI interrupts */
	pci_ic->pci_int_mask |= 0xfff00000;

	return 0;
}

int checkboard(void)
{
	puts("Board: Motorola MPC8266ADS\n");
	return 0;
}

phys_size_t initdram(int board_type)
{
	/* Autoinit part stolen from board/sacsng/sacsng.c */
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar c = 0xff;
	volatile uchar *ramaddr = (uchar *) (CONFIG_SYS_SDRAM_BASE + 0x8);
	uint psdmr = CONFIG_SYS_PSDMR;
	int i;

	uint psrt = 0x21;	/* for no SPD */
	uint chipselects = 1;	/* for no SPD */
	uint sdram_size = CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;	/* for no SPD */
	uint or = CONFIG_SYS_OR2_PRELIM;	/* for no SPD */
	uint data_width;
	uint rows;
	uint banks;
	uint cols;
	uint caslatency;
	uint width;
	uint rowst;
	uint sdam;
	uint bsma;
	uint sda10;
	u_char data;
	u_char cksum;
	int j;

	/*
	 * Keep the compiler from complaining about
	 * potentially uninitialized vars
	 */
	data_width = rows = banks = cols = caslatency = 0;

	/*
	 * Read the SDRAM SPD EEPROM via I2C.
	 */
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	i2c_read(SDRAM_SPD_ADDR, 0, 1, &data, 1);
	cksum = data;
	for (j = 1; j < 64; j++) {	/* read only the checksummed bytes */
		/* note: the I2C address autoincrements when alen == 0 */
		i2c_read(SDRAM_SPD_ADDR, 0, 0, &data, 1);
		/*printf("addr %d = 0x%02x\n", j, data); */
		if (j == 5)
			chipselects = data & 0x0F;
		else if (j == 6)
			data_width = data;
		else if (j == 7)
			data_width |= data << 8;
		else if (j == 3)
			rows = data & 0x0F;
		else if (j == 4)
			cols = data & 0x0F;
		else if (j == 12) {
			/*
			 * Refresh rate: this assumes the prescaler is set to
			 * approximately 0.39uSec per tick and the target
			 * refresh period is about 85% of maximum.
			 */
			switch (data & 0x7F) {
			default:
			case 0:
				psrt = 0x21;	/*  15.625uS */
				break;
			case 1:
				psrt = 0x07;	/*   3.9uS   */
				break;
			case 2:
				psrt = 0x0F;	/*   7.8uS   */
				break;
			case 3:
				psrt = 0x43;	/*  31.3uS   */
				break;
			case 4:
				psrt = 0x87;	/*  62.5uS   */
				break;
			case 5:
				psrt = 0xFF;	/* 125uS     */
				break;
			}
		} else if (j == 17)
			banks = data;
		else if (j == 18) {
			caslatency = 3;	/* default CL */
#if (PESSIMISTIC_SDRAM)
			if ((data & 0x04) != 0)
				caslatency = 3;
			else if ((data & 0x02) != 0)
				caslatency = 2;
			else if ((data & 0x01) != 0)
				caslatency = 1;
#else
			if ((data & 0x01) != 0)
				caslatency = 1;
			else if ((data & 0x02) != 0)
				caslatency = 2;
			else if ((data & 0x04) != 0)
				caslatency = 3;
#endif
			else {
				printf("WARNING: Unknown CAS latency 0x%02X, using 3\n",
					data);
			}
		} else if (j == 63) {
			if (data != cksum) {
				printf("WARNING: Configuration data checksum failure:"
					" is 0x%02x, calculated 0x%02x\n",
					data, cksum);
			}
		}
		cksum += data;
	}

	/* We don't trust CL less than 2 (only saw it on an old 16MByte DIMM) */
	if (caslatency < 2) {
		printf("CL was %d, forcing to 2\n", caslatency);
		caslatency = 2;
	}
	if (rows > 14) {
		printf("This doesn't look good, rows = %d, should be <= 14\n",
		       rows);
		rows = 14;
	}
	if (cols > 11) {
		printf("This doesn't look good, columns = %d, should be <= 11\n",
			cols);
		cols = 11;
	}

	if ((data_width != 64) && (data_width != 72)) {
		printf("WARNING: SDRAM width unsupported, is %d, expected 64 or 72.\n",
			data_width);
	}
	width = 3;		/* 2^3 = 8 bytes = 64 bits wide */
	/*
	 * Convert banks into log2(banks)
	 */
	if (banks == 2)
		banks = 1;
	else if (banks == 4)
		banks = 2;
	else if (banks == 8)
		banks = 3;


	sdram_size = 1 << (rows + cols + banks + width);
	/* hack for high density memory (512MB per CS) */
	/* !!!!! Will ONLY work with Page Based Interleave !!!!!
	   ( PSDMR[PBI] = 1 )
	 */
	/*
	 * memory actually has 11 column addresses, but the memory
	 * controller doesn't really care.
	 *
	 * the calculations that follow will however move the rows so
	 * that they are muxed one bit off if you use 11 bit columns.
	 *
	 * The solution is to tell the memory controller the correct
	 * size of the memory but change the number of columns to 10
	 * afterwards.
	 *
	 * The 11th column addre will still be mucxed correctly onto
	 * the bus.
	 *
	 * Also be aware that the MPC8266ADS board Rev B has not
	 * connected Row address 13 to anything.
	 *
	 * The fix is to connect ADD16 (from U37-47) to SADDR12 (U28-126)
	 */
	if (cols > 10)
		cols = 10;

#if (CONFIG_PBI == 0)		/* bank-based interleaving */
	rowst = ((32 - 6) - (rows + cols + width)) * 2;
#else
	rowst = 32 - (rows + banks + cols + width);
#endif

	or = ~(sdram_size - 1) |	/* SDAM address mask    */
		((banks - 1) << 13) |	/* banks per device     */
		(rowst << 9) |		/* rowst                */
		((rows - 9) << 6);	/* numr                 */


	/*printf("memctl->memc_or2 = 0x%08x\n", or); */

	/*
	 * SDAM specifies the number of columns that are multiplexed
	 * (reference AN2165/D), defined to be (columns - 6) for page
	 * interleave, (columns - 8) for bank interleave.
	 *
	 * BSMA is 14 - max(rows, cols).  The bank select lines come
	 * into play above the highest "address" line going into the
	 * the SDRAM.
	 */
#if (CONFIG_PBI == 0)		/* bank-based interleaving */
	sdam = cols - 8;
	bsma = ((31 - width) - 14) - ((rows > cols) ? rows : cols);
	sda10 = sdam + 2;
#else
	sdam = cols + banks - 8;
	bsma = ((31 - width) - 14) - ((rows > cols) ? rows : cols);
	sda10 = sdam;
#endif
#if (PESSIMISTIC_SDRAM)
	psdmr = (CONFIG_PBI | PSDMR_RFEN | PSDMR_RFRC_16_CLK |
		PSDMR_PRETOACT_8W | PSDMR_ACTTORW_8W | PSDMR_WRC_4C |
		PSDMR_EAMUX | PSDMR_BUFCMD) | caslatency |
		((caslatency - 1) << 6) |	/* LDOTOPRE is CL - 1 */
		(sdam << 24) | (bsma << 21) | (sda10 << 18);
#else
	psdmr = (CONFIG_PBI | PSDMR_RFEN | PSDMR_RFRC_7_CLK |
		PSDMR_PRETOACT_3W |	/* 1 for 7E parts (fast PC-133) */
		PSDMR_ACTTORW_2W |	/* 1 for 7E parts (fast PC-133) */
		PSDMR_WRC_1C |	/* 1 clock + 7nSec */
		EAMUX | BUFCMD) | caslatency |
		((caslatency - 1) << 6) |	/* LDOTOPRE is CL - 1 */
		(sdam << 24) | (bsma << 21) | (sda10 << 18);
#endif
	/*printf("psdmr = 0x%08x\n", psdmr); */

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
	 * Quote from Micron MT48LC8M16A2 data sheet:
	 *
	 *  "...the SDRAM requires a 100uS delay prior to issuing any
	 *  command other than a COMMAND INHIBIT or NOP.  Starting at some
	 *  point during this 100uS period and continuing at least through
	 *  the end of this period, COMMAND INHIBIT or NOP commands should
	 *  be applied."
	 *
	 *  "Once the 100uS delay has been satisfied with at least one COMMAND
	 *  INHIBIT or NOP command having been applied, a /PRECHARGE command/
	 *  should be applied.  All banks must then be precharged, thereby
	 *  placing the device in the all banks idle state."
	 *
	 *  "Once in the idle state, /two/ AUTO REFRESH cycles must be
	 *  performed.  After the AUTO REFRESH cycles are complete, the
	 *  SDRAM is ready for mode register programming."
	 *
	 *  (/emphasis/ mine, gvb)
	 *
	 *  The way I interpret this, Micron start up sequence is:
	 *  1. Issue a PRECHARGE-BANK command (initial precharge)
	 *  2. Issue a PRECHARGE-ALL-BANKS command ("all banks ... precharged")
	 *  3. Issue two (presumably, doing eight is OK) CBR REFRESH commands
	 *  4. Issue a MODE-SET command to initialize the mode register
	 *
	 *  --------
	 *
	 *  The initial commands are executed by setting P/LSDMR[OP] and
	 *  accessing the SDRAM with a single-byte transaction."
	 *
	 * The appropriate BRx/ORx registers have already been set
	 * when we get here. The SDRAM can be accessed at the address
	 * CONFIG_SYS_SDRAM_BASE.
	 */

	memctl->memc_mptpr = CONFIG_SYS_MPTPR;
	memctl->memc_psrt = psrt;

	memctl->memc_br2 = CONFIG_SYS_BR2_PRELIM;
	memctl->memc_or2 = or;

	memctl->memc_psdmr = psdmr | PSDMR_OP_PREA;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
		*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*ramaddr = c;

	/*
	 * Do it a second time for the second set of chips if the DIMM has
	 * two chip selects (double sided).
	 */
	if (chipselects > 1) {
		ramaddr += sdram_size;

		memctl->memc_br3 = CONFIG_SYS_BR3_PRELIM + sdram_size;
		memctl->memc_or3 = or;

		memctl->memc_psdmr = psdmr | PSDMR_OP_PREA;
		*ramaddr = c;

		memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR;
		for (i = 0; i < 8; i++)
			*ramaddr = c;

		memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;
		*ramaddr = c;

		memctl->memc_psdmr = psdmr | PSDMR_OP_NORM | PSDMR_RFEN;
		*ramaddr = c;
	}

	/* print info */
	printf("SDRAM configuration read from SPD\n");
	printf("\tSize per side = %dMB\n", sdram_size >> 20);
	printf("\tOrganization: %d sides, %d banks, %d Columns, %d Rows, Data width = %d bits\n",
		chipselects, 1 << (banks), cols, rows, data_width);
	printf("\tRefresh rate = %d, CAS latency = %d", psrt, caslatency);
#if (CONFIG_PBI == 0)		/* bank-based interleaving */
	printf(", Using Bank Based Interleave\n");
#else
	printf(", Using Page Based Interleave\n");
#endif
	printf("\tTotal size: ");

	/* this delay only needed for original 16MB DIMM...
	 * Not needed for any other memory configuration */
	if ((sdram_size * chipselects) == (16 * 1024 * 1024))
		udelay(250000);

	return sdram_size * chipselects;
}

#ifdef	CONFIG_PCI
struct pci_controller hose;

extern void pci_mpc8250_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc8250_init(&hose);
}
#endif
