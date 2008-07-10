/*
 * (C) Copyright 2002 Wolfgang Grandegger <wg@denx.de>
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
#include <ioports.h>
#include <mpc8260.h>
#include <asm/io.h>
#include <asm/immap_8260.h>

int hwc_flash_size (void);
int hwc_local_sdram_size (void);
int hwc_main_sdram_size (void);
int hwc_serial_number (void);
int hwc_mac_address (char *str);
int hwc_manufact_date (char *str);
int seeprom_read (int addr, uchar * data, int size);

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 *
 * The port definitions are taken from the old firmware (see
 * also SYS/H/4539.H):
 *
 *        ppar      psor        pdir       podr       pdat
 * PA: 0x02ffffff 0x02c00000 0xfc403fe6 0x00000000 0x02403fc0
 * PB: 0x0fffdeb0 0x000000b0 0x0f032347 0x00000000 0x0f000290
 * PC: 0x030ffa55 0x030f0040 0xbcf005ea 0x00000000 0xc0c0ba7d
 * PD: 0x09c04e3c 0x01000e3c 0x0a7ff1c3 0x00000000 0x00ce0ae9
 */
const iop_conf_t iop_conf_tab[4][32] = {

	/* Port A configuration */
	{							/* conf ppar psor pdir podr pdat */
	 {0, 1, 0, 0, 0, 0},		/* PA31 FCC1_TXENB  SLAVE */
	 {0, 1, 0, 1, 0, 0},		/* PA30 FCC1_TXCLAV SLAVE */
	 {0, 1, 0, 1, 0, 0},		/* PA29 FCC1_TXSOC */
	 {0, 1, 0, 0, 0, 0},		/* PA28 FCC1_RXENB  SLAVE */
	 {0, 1, 0, 0, 0, 0},		/* PA27 FCC1_RXSOC */
	 {0, 1, 0, 1, 0, 0},		/* PA26 FCC1_RXCLAV SLAVE */
	 {0, 1, 0, 1, 0, 1},		/* PA25 FCC1_TXD0 */
	 {0, 1, 0, 1, 0, 1},		/* PA24 FCC1_TXD1 */
	 {0, 1, 0, 1, 0, 1},		/* PA23 FCC1_TXD2 */
	 {0, 1, 0, 1, 0, 1},		/* PA22 FCC1_TXD3 */
	 {0, 1, 0, 1, 0, 1},		/* PA21 FCC1_TXD4 */
	 {0, 1, 0, 1, 0, 1},		/* PA20 FCC1_TXD5 */
	 {0, 1, 0, 1, 0, 1},		/* PA19 FCC1_TXD6 */
	 {0, 1, 0, 1, 0, 1},		/* PA18 FCC1_TXD7 */
	 {0, 1, 0, 0, 0, 0},		/* PA17 FCC1_RXD7 */
	 {0, 1, 0, 0, 0, 0},		/* PA16 FCC1_RXD6 */
	 {0, 1, 0, 0, 0, 0},		/* PA15 FCC1_RXD5 */
	 {0, 1, 0, 0, 0, 0},		/* PA14 FCC1_RXD4 */
	 {0, 1, 0, 0, 0, 0},		/* PA13 FCC1_RXD3 */
	 {0, 1, 0, 0, 0, 0},		/* PA12 FCC1_RXD2 */
	 {0, 1, 0, 0, 0, 0},		/* PA11 FCC1_RXD1 */
	 {0, 1, 0, 0, 0, 0},		/* PA10 FCC1_RXD0 */
	 {0, 1, 1, 1, 0, 1},		/* PA9  TDMA1_L1TXD */
	 {0, 1, 1, 0, 0, 0},		/* PA8  TDMA1_L1RXD */
	 {0, 0, 0, 0, 0, 0},		/* PA7  CONFIG0 */
	 {0, 1, 1, 0, 0, 1},		/* PA6  TDMA1_L1RSYNC */
	 {0, 0, 0, 1, 0, 0},		/* PA5  FCC2:RxAddr[2] */
	 {0, 0, 0, 1, 0, 0},		/* PA4  FCC2:RxAddr[1] */
	 {0, 0, 0, 1, 0, 0},		/* PA3  FCC2:RxAddr[0] */
	 {0, 0, 0, 1, 0, 0},		/* PA2  FCC2:TxAddr[0] */
	 {0, 0, 0, 1, 0, 0},		/* PA1  FCC2:TxAddr[1] */
	 {0, 0, 0, 1, 0, 0}			/* PA0  FCC2:TxAddr[2] */
	 },
	/* Port B configuration */
	{							/* conf ppar psor pdir podr pdat */
	 {0, 0, 0, 1, 0, 0},		/* PB31 FCC2_RXSOC */
	 {0, 0, 0, 1, 0, 0},		/* PB30 FCC2_TXSOC */
	 {0, 0, 0, 1, 0, 0},		/* PB29 FCC2_RXCLAV */
	 {0, 0, 0, 0, 0, 0},		/* PB28 CONFIG2 */
	 {0, 1, 1, 0, 0, 1},		/* PB27 FCC2_TXD0 */
	 {0, 1, 1, 0, 0, 0},		/* PB26 FCC2_TXD1 */
	 {0, 0, 0, 1, 0, 0},		/* PB25 FCC2_TXD4 */
	 {0, 1, 1, 0, 0, 1},		/* PB24 FCC2_TXD5 */
	 {0, 0, 0, 1, 0, 0},		/* PB23 FCC2_TXD6 */
	 {0, 1, 0, 1, 0, 1},		/* PB22 FCC2_TXD7 */
	 {0, 1, 0, 0, 0, 0},		/* PB21 FCC2_RXD7 */
	 {0, 1, 0, 0, 0, 0},		/* PB20 FCC2_RXD6 */
	 {0, 1, 0, 0, 0, 0},		/* PB19 FCC2_RXD5 */
	 {0, 0, 0, 1, 0, 0},		/* PB18 FCC2_RXD4 */
	 {1, 1, 0, 0, 0, 0},		/* PB17 FCC3_RX_DV */
	 {1, 1, 0, 0, 0, 0},		/* PB16 FCC3_RX_ER */
	 {1, 1, 0, 1, 0, 0},		/* PB15 FCC3_TX_ER */
	 {1, 1, 0, 1, 0, 0},		/* PB14 FCC3_TX_EN */
	 {1, 1, 0, 0, 0, 0},		/* PB13 FCC3_COL */
	 {1, 1, 0, 0, 0, 0},		/* PB12 FCC3_CRS */
	 {1, 1, 0, 0, 0, 0},		/* PB11 FCC3_RXD3 */
	 {1, 1, 0, 0, 0, 0},		/* PB10 FCC3_RXD2 */
	 {1, 1, 0, 0, 0, 0},		/* PB9  FCC3_RXD1 */
	 {1, 1, 0, 0, 0, 0},		/* PB8  FCC3_RXD0 */
	 {1, 1, 0, 1, 0, 1},		/* PB7  FCC3_TXD0 */
	 {1, 1, 0, 1, 0, 1},		/* PB6  FCC3_TXD1 */
	 {1, 1, 0, 1, 0, 1},		/* PB5  FCC3_TXD2 */
	 {1, 1, 0, 1, 0, 1},		/* PB4  FCC3_TXD3 */
	 {0, 0, 0, 0, 0, 0},		/* PB3  */
	 {0, 0, 0, 0, 0, 0},		/* PB2  */
	 {0, 0, 0, 0, 0, 0},		/* PB1  */
	 {0, 0, 0, 0, 0, 0},		/* PB0  */
	 },
	/* Port C configuration */
	{							/* conf ppar psor pdir podr pdat */
	 {0, 1, 0, 0, 0, 1},		/* PC31 CLK1 */
	 {0, 0, 0, 1, 0, 0},		/* PC30 U1MASTER_N */
	 {0, 1, 0, 0, 0, 1},		/* PC29 CLK3 */
	 {0, 0, 0, 1, 0, 1},		/* PC28 -MT90220_RST */
	 {0, 1, 0, 0, 0, 1},		/* PC27 CLK5 */
	 {0, 0, 0, 1, 0, 1},		/* PC26 -QUADFALC_RST */
	 {0, 1, 1, 1, 0, 1},		/* PC25 BRG4 */
	 {1, 0, 0, 1, 0, 0},		/* PC24 MDIO */
	 {1, 0, 0, 1, 0, 0},		/* PC23 MDC */
	 {0, 1, 0, 0, 0, 1},		/* PC22 CLK10 */
	 {0, 0, 0, 1, 0, 0},		/* PC21  */
	 {0, 1, 0, 0, 0, 1},		/* PC20 CLK12 */
	 {0, 1, 0, 0, 0, 1},		/* PC19 CLK13 */
	 {1, 1, 0, 0, 0, 1},		/* PC18 CLK14 */
	 {0, 1, 0, 0, 0, 0},		/* PC17 CLK15 */
	 {1, 1, 0, 0, 0, 1},		/* PC16 CLK16 */
	 {0, 1, 1, 0, 0, 0},		/* PC15 FCC1_TXADDR0 SLAVE */
	 {0, 1, 1, 0, 0, 0},		/* PC14 FCC1_RXADDR0 SLAVE */
	 {0, 1, 1, 0, 0, 0},		/* PC13 FCC1_TXADDR1 SLAVE */
	 {0, 1, 1, 0, 0, 0},		/* PC12 FCC1_RXADDR1 SLAVE */
	 {0, 0, 0, 1, 0, 0},		/* PC11 FCC2_RXD2 */
	 {0, 0, 0, 1, 0, 0},		/* PC10 FCC2_RXD3 */
	 {0, 0, 0, 1, 0, 1},		/* PC9  LTMODE */
	 {0, 0, 0, 1, 0, 1},		/* PC8  SELSYNC */
	 {0, 1, 1, 0, 0, 0},		/* PC7  FCC1_TXADDR2 SLAVE  */
	 {0, 1, 1, 0, 0, 0},		/* PC6  FCC1_RXADDR2 SLAVE */
	 {0, 0, 0, 1, 0, 0},		/* PC5  FCC2_TXCLAV MASTER */
	 {0, 0, 0, 1, 0, 0},		/* PC4  FCC2_RXENB MASTER */
	 {0, 0, 0, 1, 0, 0},		/* PC3  FCC2_TXD2 */
	 {0, 0, 0, 1, 0, 0},		/* PC2  FCC2_TXD3 */
	 {0, 0, 0, 0, 0, 1},		/* PC1  PTMC -PTEENB */
	 {0, 0, 0, 1, 0, 1},		/* PC0  COMCLK_N */
	 },
	/* Port D configuration */
	{							/* conf ppar psor pdir podr pdat */
	 {0, 0, 0, 1, 0, 1},		/* PD31 -CAM_RST */
	 {0, 0, 0, 1, 0, 0},		/* PD30 FCC2_TXENB */
	 {0, 1, 1, 0, 0, 0},		/* PD29 FCC1_RXADDR3 SLAVE */
	 {0, 1, 1, 0, 0, 1},		/* PD28 TDMC1_L1TXD */
	 {0, 1, 1, 0, 0, 0},		/* PD27 TDMC1_L1RXD */
	 {0, 1, 1, 0, 0, 1},		/* PD26 TDMC1_L1RSYNC */
	 {0, 0, 0, 1, 0, 1},		/* PD25 LED0 -OFF */
	 {0, 0, 0, 1, 0, 1},		/* PD24 LED5 -OFF */
	 {1, 0, 0, 1, 0, 1},		/* PD23 -LXT971_RST */
	 {0, 1, 1, 0, 0, 1},		/* PD22 TDMA2_L1TXD */
	 {0, 1, 1, 0, 0, 0},		/* PD21 TDMA2_L1RXD */
	 {0, 1, 1, 0, 0, 1},		/* PD20 TDMA2_L1RSYNC */
	 {0, 0, 0, 1, 0, 0},		/* PD19 FCC2_TXADDR3 */
	 {0, 0, 0, 1, 0, 0},		/* PD18 FCC2_RXADDR3 */
	 {0, 1, 0, 1, 0, 0},		/* PD17 BRG2 */
	 {0, 0, 0, 1, 0, 0},		/* PD16  */
	 {0, 0, 0, 1, 0, 0},		/* PD15 PT2TO1 */
	 {0, 0, 0, 1, 0, 1},		/* PD14 PT4TO3 */
	 {0, 0, 0, 1, 0, 1},		/* PD13 -SWMODE */
	 {0, 0, 0, 1, 0, 1},		/* PD12 -PTMODE */
	 {0, 0, 0, 1, 0, 0},		/* PD11 FCC2_RXD0 */
	 {0, 0, 0, 1, 0, 0},		/* PD10 FCC2_RXD1 */
	 {1, 1, 0, 1, 0, 1},		/* PD9  SMC1_SMTXD */
	 {1, 1, 0, 0, 0, 1},		/* PD8  SMC1_SMRXD */
	 {0, 1, 1, 0, 0, 0},		/* PD7  FCC1_TXADDR3 SLAVE */
	 {0, 0, 0, 1, 0, 0},		/* PD6  IMAMODE */
	 {0, 0, 0, 0, 0, 0},		/* PD5  CONFIG2 */
	 {0, 1, 0, 1, 0, 0},		/* PD4  BRG8 */
	 {0, 0, 0, 0, 0, 0},		/* PD3  */
	 {0, 0, 0, 0, 0, 0},		/* PD2  */
	 {0, 0, 0, 0, 0, 0},		/* PD1  */
	 {0, 0, 0, 0, 0, 0},		/* PD0  */
	 }
};

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar *base;
	ulong maxsize;
	int i;

	memctl->memc_psrt = CFG_PSRT;
	memctl->memc_mptpr = CFG_MPTPR;

#ifndef CFG_RAMBOOT
	immap->im_siu_conf.sc_ppc_acr = 0x00000026;
	immap->im_siu_conf.sc_ppc_alrh = 0x01276345;
	immap->im_siu_conf.sc_ppc_alrl = 0x89ABCDEF;
	immap->im_siu_conf.sc_lcl_acr = 0x00000000;
	immap->im_siu_conf.sc_lcl_alrh = 0x01234567;
	immap->im_siu_conf.sc_lcl_alrl = 0x89ABCDEF;
	immap->im_siu_conf.sc_tescr1 = 0x00004000;
	immap->im_siu_conf.sc_ltescr1 = 0x00004000;

	/* Init Main SDRAM */
#define OP_VALUE   0x404A241A
#define OP_VALUE_M (OP_VALUE & 0x87FFFFFF);
	base = (uchar *) CFG_SDRAM_BASE;
	memctl->memc_psdmr = 0x28000000 | OP_VALUE_M;
	*base = 0xFF;
	memctl->memc_psdmr = 0x08000000 | OP_VALUE_M;
	for (i = 0; i < 8; i++)
		*base = 0xFF;
	memctl->memc_psdmr = 0x18000000 | OP_VALUE_M;
	*(base + 0x110) = 0xFF;
	memctl->memc_psdmr = OP_VALUE;
	memctl->memc_lsdmr = 0x4086A522;
	*base = 0xFF;

	/* We must be able to test a location outsize the maximum legal size
	 * to find out THAT we are outside; but this address still has to be
	 * mapped by the controller. That means, that the initial mapping has
	 * to be (at least) twice as large as the maximum expected size.
	 */
	maxsize = (1 + (~memctl->memc_or1 | 0x7fff)) / 2;

	maxsize = get_ram_size((long *)base, maxsize);

	memctl->memc_or1 |= ~(maxsize - 1);

	if (maxsize != hwc_main_sdram_size ())
		printf ("Oops: memory test has not found all memory!\n");
#endif

	icache_enable ();
	/* return total ram size of SDRAM */
	return (maxsize);
}

int checkboard (void)
{
	char string[32];

	hwc_manufact_date (string);

	printf ("Board: Interphase 4539 (#%d %s)\n",
		hwc_serial_number (),
		string);

#ifdef DEBUG
	printf ("Manufacturing date: %s\n", string);
	printf ("Serial number     : %d\n", hwc_serial_number ());
	printf ("FLASH size        : %d MB\n", hwc_flash_size () >> 20);
	printf ("Main SDRAM size   : %d MB\n", hwc_main_sdram_size () >> 20);
	printf ("Local SDRAM size  : %d MB\n", hwc_local_sdram_size () >> 20);
	hwc_mac_address (string);
	printf ("MAC address       : %s\n", string);
#endif

	return 0;
}

int misc_init_r (void)
{
	char *s, str[32];
	int num;

	if ((s = getenv ("serial#")) == NULL &&
		(num = hwc_serial_number ()) != -1) {
		sprintf (str, "%06d", num);
		setenv ("serial#", str);
	}
	if ((s = getenv ("ethaddr")) == NULL && hwc_mac_address (str) == 0) {
		setenv ("ethaddr", str);
	}
	return (0);
}

/***************************************************************
 * We take some basic Hardware Configuration Parameter from the
 * Serial EEPROM conected to the PSpan bridge. We keep it as
 * simple as possible.
 */
int hwc_flash_size (void)
{
	uchar byte;

	if (!seeprom_read (0x40, &byte, sizeof (byte))) {
		switch ((byte >> 2) & 0x3) {
		case 0x1:
			return 0x0400000;
			break;
		case 0x2:
			return 0x0800000;
			break;
		case 0x3:
			return 0x1000000;
		default:
			return 0x0100000;
		}
	}
	return -1;
}
int hwc_local_sdram_size (void)
{
	uchar byte;

	if (!seeprom_read (0x40, &byte, sizeof (byte))) {
		switch ((byte & 0x03)) {
		case 0x1:
			return 0x0800000;
		case 0x2:
			return 0x1000000;
		default:
			return 0;			/* not present */
		}
	}
	return -1;
}
int hwc_main_sdram_size (void)
{
	uchar byte;

	if (!seeprom_read (0x41, &byte, sizeof (byte))) {
		return 0x1000000 << ((byte >> 5) & 0x7);
	}
	return -1;
}
int hwc_serial_number (void)
{
	int sn = -1;

	if (!seeprom_read (0xa0, (uchar *) &sn, sizeof (sn))) {
		sn = cpu_to_le32 (sn);
	}
	return sn;
}
int hwc_mac_address (char *str)
{
	char mac[6];

	if (!seeprom_read (0xb0, (uchar *)mac, sizeof (mac))) {
		sprintf (str, "%02x:%02x:%02x:%02x:%02x:%02x\n",
				 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	} else {
		strcpy (str, "ERROR");
		return -1;
	}
	return 0;
}
int hwc_manufact_date (char *str)
{
	uchar byte;
	int value;

	if (seeprom_read (0x92, &byte, sizeof (byte)))
		goto out;
	value = byte;
	if (seeprom_read (0x93, &byte, sizeof (byte)))
		goto out;
	value += byte << 8;
	sprintf (str, "%02d/%02d/%04d",
			 value & 0x1F, (value >> 5) & 0xF,
			 1980 + ((value >> 9) & 0x1FF));
	return 0;

  out:
	strcpy (str, "ERROR");
	return -1;
}

#define PSPAN_ADDR      0xF0020000
#define EEPROM_REG      0x408
#define EEPROM_READ_CMD 0xA000
#define PSPAN_WRITE(a,v) \
    *((volatile unsigned long *)(PSPAN_ADDR+(a))) = v; eieio()
#define PSPAN_READ(a) \
    *((volatile unsigned long *)(PSPAN_ADDR+(a)))

int seeprom_read (int addr, uchar * data, int size)
{
	ulong val, cmd;
	int i;

	for (i = 0; i < size; i++) {

		cmd = EEPROM_READ_CMD;
		cmd |= ((addr + i) << 24) & 0xff000000;

		/* Wait for ACT to authorize write */
		while ((val = PSPAN_READ (EEPROM_REG)) & 0x80)
			eieio ();

		/* Write command */
		PSPAN_WRITE (EEPROM_REG, cmd);

		/* Wait for data to be valid */
		while ((val = PSPAN_READ (EEPROM_REG)) & 0x80)
			eieio ();
		/* Do it twice, first read might be erratic */
		while ((val = PSPAN_READ (EEPROM_REG)) & 0x80)
			eieio ();

		/* Read error */
		if (val & 0x00000040) {
			return -1;
		} else {
			data[i] = (val >> 16) & 0xff;
		}
	}
	return 0;
}
