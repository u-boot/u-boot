/*
 * Copyright (C) 2004 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Support for Interphase iSPAN Communications Controllers
 * (453x and others). Tested on 4532.
 *
 * Derived from iSPAN 4539 port (iphase4539) by
 * Wolfgang Grandegger <wg@denx.de>
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

/*
 * I/O Ports configuration table
 *
 * If conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

#define CFG_FCC1 (CONFIG_ETHER_INDEX == 1)
#define CFG_FCC2 (CONFIG_ETHER_INDEX == 2)
#define CFG_FCC3 (CONFIG_ETHER_INDEX == 3)

const iop_conf_t iop_conf_tab[4][32] = {
    /* Port A */
    {	/*	      conf      ppar psor pdir podr pdat */
	/* PA31 */ { CFG_FCC1,   1,   1,   0,   0,   0 }, /* FCC1 MII COL   */
	/* PA30 */ { CFG_FCC1,   1,   1,   0,   0,   0 }, /* FCC1 MII CRS   */
	/* PA29 */ { CFG_FCC1,   1,   1,   1,   0,   0 }, /* FCC1 MII TX_ER */
	/* PA28 */ { CFG_FCC1,   1,   1,   1,   0,   0 }, /* FCC1 MII TX_EN */
	/* PA27 */ { CFG_FCC1,   1,   1,   0,   0,   0 }, /* FCC1 MII RX_DV */
	/* PA26 */ { CFG_FCC1,   1,   1,   0,   0,   0 }, /* FCC1 MII RX_ER */
	/* PA25 */ { 0,          0,   0,   0,   0,   0 }, /* PA25 */
	/* PA24 */ { 0,          0,   0,   0,   0,   0 }, /* PA24 */
	/* PA23 */ { 0,          0,   0,   0,   0,   0 }, /* PA23 */
	/* PA22 */ { 0,          0,   0,   0,   0,   0 }, /* PA22 */
	/* PA21 */ { CFG_FCC1,   1,   0,   1,   0,   0 }, /* FCC1 MII TxD[3] */
	/* PA20 */ { CFG_FCC1,   1,   0,   1,   0,   0 }, /* FCC1 MII TxD[2] */
	/* PA19 */ { CFG_FCC1,   1,   0,   1,   0,   0 }, /* FCC1 MII TxD[1] */
	/* PA18 */ { CFG_FCC1,   1,   0,   1,   0,   0 }, /* FCC1 MII TxD[0] */
	/* PA17 */ { CFG_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII RxD[0] */
	/* PA16 */ { CFG_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII RxD[1] */
	/* PA15 */ { CFG_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII RxD[2] */
	/* PA14 */ { CFG_FCC1,   1,   0,   0,   0,   0 }, /* FCC1 MII RxD[3] */
	/* PA13 */ { 0,          0,   0,   0,   0,   0 }, /* PA13 */
	/* PA12 */ { 0,          0,   0,   0,   0,   0 }, /* PA12 */
	/* PA11 */ { 0,          0,   0,   0,   0,   0 }, /* PA11 */
	/* PA10 */ { 0,          0,   0,   0,   0,   0 }, /* PA10 */
	/* PA9  */ { 0,          1,   0,   1,   0,   0 }, /* SMC2 SMTXD */
	/* PA8  */ { 0,          1,   0,   0,   0,   0 }, /* SMC2 SMRXD */
	/* PA7  */ { 0,          0,   0,   0,   0,   0 }, /* PA7 */
	/* PA6  */ { 0,          0,   0,   0,   0,   0 }, /* PA6 */
	/* PA5  */ { 0,          0,   0,   0,   0,   0 }, /* PA5 */
	/* PA4  */ { 0,          0,   0,   0,   0,   0 }, /* PA4 */
	/* PA3  */ { 0,          0,   0,   0,   0,   0 }, /* PA3 */
	/* PA2  */ { 0,          0,   0,   0,   0,   0 }, /* PA2 */
	/* PA1  */ { 0,          0,   0,   0,   0,   0 }, /* PA1 */
	/* PA0  */ { 0,          0,   0,   0,   0,   0 }  /* PA0 */
    },

    /* Port B */
    {   /*	      conf      ppar psor pdir podr pdat */
	/* PB31 */ { CFG_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TX_ER  */
	/* PB30 */ { CFG_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RX_DV  */
	/* PB29 */ { CFG_FCC2,   1,   1,   1,   0,   0 }, /* FCC2 MII TX_EN  */
	/* PB28 */ { CFG_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RX_ER  */
	/* PB27 */ { CFG_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII COL    */
	/* PB26 */ { CFG_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII CRS    */
	/* PB25 */ { CFG_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TxD[3] */
	/* PB24 */ { CFG_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TxD[2] */
	/* PB23 */ { CFG_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TxD[1] */
	/* PB22 */ { CFG_FCC2,   1,   0,   1,   0,   0 }, /* FCC2 MII TxD[0] */
	/* PB21 */ { CFG_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RxD[0] */
	/* PB20 */ { CFG_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RxD[1] */
	/* PB19 */ { CFG_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RxD[2] */
	/* PB18 */ { CFG_FCC2,   1,   0,   0,   0,   0 }, /* FCC2 MII RxD[3] */
	/* PB17 */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII RX_DV  */
	/* PB16 */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII RX_ER  */
	/* PB15 */ { CFG_FCC3,   1,   0,   1,   0,   0 }, /* FCC3 MII TX_ER  */
	/* PB14 */ { CFG_FCC3,   1,   0,   1,   0,   0 }, /* FCC3 MII TX_EN  */
	/* PB13 */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII COL    */
	/* PB12 */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII CRS    */
	/* PB11 */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII RxD[3] */
	/* PB10 */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII RxD[2] */
	/* PB9  */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII RxD[1] */
	/* PB8  */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII RxD[0] */
	/* PB7  */ { CFG_FCC3,   1,   0,   1,   0,   0 }, /* FCC3 MII TxD[0] */
	/* PB6  */ { CFG_FCC3,   1,   0,   1,   0,   0 }, /* FCC3 MII TxD[1] */
	/* PB5  */ { CFG_FCC3,   1,   0,   1,   0,   0 }, /* FCC3 MII TxD[2] */
	/* PB4  */ { CFG_FCC3,   1,   0,   1,   0,   0 }, /* FCC3 MII TxD[3] */
	/* PB3  */ { 0,          0,   0,   0,   0,   0 }, /* pin doesn't exist */
	/* PB2  */ { 0,          0,   0,   0,   0,   0 }, /* pin doesn't exist */
	/* PB1  */ { 0,          0,   0,   0,   0,   0 }, /* pin doesn't exist */
	/* PB0  */ { 0,          0,   0,   0,   0,   0 }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*	      conf      ppar psor pdir podr pdat */
	/* PC31 */ { 0,          0,   0,   0,   0,   0 }, /* PC31 */
	/* PC30 */ { 0,          0,   0,   0,   0,   0 }, /* PC30 */
	/* PC29 */ { 0,          0,   0,   0,   0,   0 }, /* PC29 */
	/* PC28 */ { 0,          0,   0,   0,   0,   0 }, /* PC28 */
	/* PC27 */ { 0,          0,   0,   0,   0,   0 }, /* PC27 */
	/* PC26 */ { 0,          0,   0,   0,   0,   0 }, /* PC26 */
	/* PC25 */ { 0,          0,   0,   0,   0,   0 }, /* PC25 */
	/* PC24 */ { 0,          0,   0,   0,   0,   0 }, /* PC24 */
	/* PC23 */ { 0,          0,   0,   0,   0,   0 }, /* PC23 */
	/* PC22 */ { 0,          0,   0,   0,   0,   0 }, /* PC22 */
	/* PC21 */ { 0,          0,   0,   0,   0,   0 }, /* PC21 */
	/* PC20 */ { 0,          0,   0,   0,   0,   0 }, /* PC20 */
	/* PC19 */ { 0,          0,   0,   0,   0,   0 }, /* PC19 */
	/* PC18 */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII Rx Clock (CLK14) */
	/* PC17 */ { 0,          0,   0,   0,   0,   0 }, /* PC17 */
	/* PC16 */ { CFG_FCC3,   1,   0,   0,   0,   0 }, /* FCC3 MII Tx Clock (CLK16) */
	/* PC15 */ { 0,          0,   0,   0,   0,   0 }, /* PC15 */
	/* PC14 */ { 0,          0,   0,   0,   0,   0 }, /* PC14 */
	/* PC13 */ { 0,          0,   0,   0,   0,   0 }, /* PC13 */
	/* PC12 */ { 0,          0,   0,   0,   0,   0 }, /* PC12 */
	/* PC11 */ { 0,          0,   0,   0,   0,   0 }, /* PC11 */
	/* PC10 */ { 0,          0,   0,   0,   0,   0 }, /* PC10 */
	/* PC9  */ { 0,          0,   0,   0,   0,   0 }, /* PC9  */
	/* PC8  */ { 0,          0,   0,   0,   0,   0 }, /* PC8  */
	/* PC7  */ { 0,          0,   0,   0,   0,   0 }, /* PC7  */
	/* PC6  */ { 0,          0,   0,   0,   0,   0 }, /* PC6  */
	/* PC5  */ { 0,          0,   0,   0,   0,   0 }, /* PC5  */
	/* PC4  */ { 0,          0,   0,   0,   0,   0 }, /* PC4  */
	/* PC3  */ { 0,          0,   0,   0,   0,   0 }, /* PC3  */
	/* PC2  */ { 0,          0,   0,   0,   0,   0 }, /* PC2  */
	/* PC1  */ { 0,          0,   0,   0,   0,   0 }, /* PC1  */
	/* PC0  */ { 0,          0,   0,   0,   0,   0 }  /* PC0  */
    },

    /* Port D */
    {   /*	      conf      ppar psor pdir podr pdat */
	/* PD31 */ { 0,          0,   0,   0,   0,   0 }, /* PD31 */
	/* PD30 */ { 0,          0,   0,   0,   0,   0 }, /* PD30 */
	/* PD29 */ { 0,          0,   0,   0,   0,   0 }, /* PD29 */
	/* PD28 */ { 0,          0,   0,   0,   0,   0 }, /* PD28 */
	/* PD27 */ { 0,          0,   0,   0,   0,   0 }, /* PD27 */
	/* PD26 */ { 0,          0,   0,   0,   0,   0 }, /* PD26 */
	/* PD25 */ { 0,          0,   0,   0,   0,   0 }, /* PD25 */
	/* PD24 */ { 0,          0,   0,   0,   0,   0 }, /* PD24 */
	/* PD23 */ { 0,          0,   0,   0,   0,   0 }, /* PD23 */
	/* PD22 */ { 0,          0,   0,   0,   0,   0 }, /* PD22 */
	/* PD21 */ { 0,          0,   0,   0,   0,   0 }, /* PD21 */
	/* PD20 */ { 0,          0,   0,   0,   0,   0 }, /* PD20 */
	/* PD19 */ { 0,          0,   0,   0,   0,   0 }, /* PD19 */
	/* PD18 */ { 0,          1,   1,   0,   0,   0 }, /* SPICLK  */
	/* PD17 */ { 0,          1,   1,   0,   0,   0 }, /* SPIMOSI */
	/* PD16 */ { 0,          1,   1,   0,   0,   0 }, /* SPIMISO */
	/* PD15 */ { 0,          1,   1,   0,   1,   0 }, /* I2C SDA */
	/* PD14 */ { 0,          1,   1,   0,   1,   0 }, /* I2C SCL */
	/* PD13 */ { 1,          0,   0,   0,   0,   0 }, /* MII MDIO */
	/* PD12 */ { 1,          0,   0,   1,   0,   0 }, /* MII MDC  */
	/* PD11 */ { 0,          0,   0,   0,   0,   0 }, /* PD11 */
	/* PD10 */ { 0,          0,   0,   0,   0,   0 }, /* PD10 */
	/* PD9  */ { 1,          1,   0,   1,   0,   0 }, /* SMC1 SMTXD */
	/* PD8  */ { 1,          1,   0,   0,   0,   0 }, /* SMC1 SMRXD */
	/* PD7  */ { 0,          0,   0,   0,   0,   0 }, /* PD7 */
	/* PD6  */ { CFG_FCC3,   0,   0,   1,   0,   1 }, /* MII PHY Reset  */
	/* PD5  */ { CFG_FCC3,   0,   0,   1,   0,   0 }, /* MII PHY Enable */
	/* PD4  */ { 0,          0,   0,   0,   0,   0 }, /* PD4 */
	/* PD3  */ { 0,          0,   0,   0,   0,   0 }, /* pin doesn't exist */
	/* PD2  */ { 0,          0,   0,   0,   0,   0 }, /* pin doesn't exist */
	/* PD1  */ { 0,          0,   0,   0,   0,   0 }, /* pin doesn't exist */
	/* PD0  */ { 0,          0,   0,   0,   0,   0 }  /* pin doesn't exist */
    }
};

#define PSPAN_ADDR      0xF0020000
#define EEPROM_REG      0x408
#define EEPROM_READ_CMD 0xA000
#define PSPAN_WRITE(a,v) \
    *((volatile unsigned long *)(PSPAN_ADDR+(a))) = v; eieio()
#define PSPAN_READ(a) \
    *((volatile unsigned long *)(PSPAN_ADDR+(a)))

static int seeprom_read (int addr, uchar * data, int size)
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

/***************************************************************
 * We take some basic Hardware Configuration Parameter from the
 * Serial EEPROM conected to the PSpan bridge. We keep it as
 * simple as possible.
 */
#ifdef DEBUG
static int hwc_flash_size (void)
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

static int hwc_local_sdram_size (void)
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
#endif	/* DEBUG */

static int hwc_main_sdram_size (void)
{
	uchar byte;

	if (!seeprom_read (0x41, &byte, sizeof (byte))) {
		return 0x1000000 << ((byte >> 5) & 0x7);
	}
	return -1;
}

static int hwc_serial_number (void)
{
	int sn = -1;

	if (!seeprom_read (0xa0, (uchar *) &sn, sizeof (sn))) {
		sn = cpu_to_le32 (sn);
	}
	return sn;
}

static int hwc_mac_address (char *str)
{
	char mac[6];

	if (!seeprom_read (0xb0, (uchar *)mac, sizeof (mac))) {
		sprintf (str, "%02X:%02X:%02X:%02X:%02X:%02X",
				 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	} else {
		strcpy (str, "ERROR");
		return -1;
	}
	return 0;
}

static int hwc_manufact_date (char *str)
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

static int hwc_board_type (char **str)
{
	ushort id = 0;

	if (seeprom_read (7, (uchar *) & id, sizeof (id)) == 0) {
		switch (id) {
		case 0x9080:
			*str = "4532-002";
			break;
		case 0x9081:
			*str = "4532-001";
			break;
		case 0x9082:
			*str = "4532-000";
			break;
		default:
			*str = "Unknown";
		}
	} else {
		*str = "Unknown";
	}

	return id;
}

long int initdram (int board_type)
{
	long maxsize = hwc_main_sdram_size();

#if !defined(CFG_RAMBOOT) && !defined(CFG_USE_FIRMWARE)
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8260_t *memctl = &immap->im_memctl;
	volatile uchar *base;
	int i;

	immap->im_siu_conf.sc_ppc_acr  = 0x00000026;
	immap->im_siu_conf.sc_ppc_alrh = 0x01276345;
	immap->im_siu_conf.sc_ppc_alrl = 0x89ABCDEF;
	immap->im_siu_conf.sc_lcl_acr  = 0x00000000;
	immap->im_siu_conf.sc_lcl_alrh = 0x01234567;
	immap->im_siu_conf.sc_lcl_alrl = 0x89ABCDEF;
	immap->im_siu_conf.sc_tescr1   = 0x00004000;
	immap->im_siu_conf.sc_ltescr1  = 0x00004000;

	memctl->memc_mptpr = CFG_MPTPR;

	/* Initialise 60x bus SDRAM */
	base = (uchar *)(CFG_SDRAM_BASE | 0x110);
	memctl->memc_psrt  = CFG_PSRT;
	memctl->memc_or1   = CFG_60x_OR;
	memctl->memc_br1   = CFG_SDRAM_BASE | CFG_60x_BR;

	memctl->memc_psdmr = CFG_PSDMR | 0x28000000;
	*base = 0xFF;
	memctl->memc_psdmr = CFG_PSDMR | 0x08000000;
	for (i = 0; i < 8; i++)
		*base = 0xFF;
	memctl->memc_psdmr = CFG_PSDMR | 0x18000000;
	*base = 0xFF;
	memctl->memc_psdmr = CFG_PSDMR | 0x40000000;

	/* Initialise local bus SDRAM */
	base = (uchar *)CFG_LSDRAM_BASE;
	memctl->memc_lsrt  = CFG_LSRT;
	memctl->memc_or2   = CFG_LOC_OR;
	memctl->memc_br2   = CFG_LSDRAM_BASE | CFG_LOC_BR;

	memctl->memc_lsdmr = CFG_LSDMR | 0x28000000;
	*base = 0xFF;
	memctl->memc_lsdmr = CFG_LSDMR | 0x08000000;
	for (i = 0; i < 8; i++)
		*base = 0xFF;
	memctl->memc_lsdmr = CFG_LSDMR | 0x18000000;
	*base = 0xFF;
	memctl->memc_lsdmr = CFG_LSDMR | 0x40000000;

	/* We must be able to test a location outsize the maximum legal size
	 * to find out THAT we are outside; but this address still has to be
	 * mapped by the controller. That means, that the initial mapping has
	 * to be (at least) twice as large as the maximum expected size.
	 */
	maxsize = (~(memctl->memc_or1 & BRx_BA_MSK) + 1) / 2;

	maxsize = get_ram_size((long *)(memctl->memc_br1 & BRx_BA_MSK), maxsize);

	memctl->memc_or1 |= ~(maxsize - 1);

	if (maxsize != hwc_main_sdram_size())
		puts("Oops: memory test has not found all memory!\n");
#endif /* !CFG_RAMBOOT && !CFG_USE_FIRMWARE */

	/* Return total RAM size (size of 60x SDRAM) */
	return maxsize;
}

int checkboard(void)
{
	char string[32], *id;

	hwc_manufact_date(string);
	hwc_board_type(&id);
	printf("Board: Interphase iSPAN %s (#%d %s)\n",
	       id, hwc_serial_number(), string);
#ifdef DEBUG
	printf("Manufacturing date: %s\n", string);
	printf("Serial number     : %d\n", hwc_serial_number());
	printf("FLASH size        : %d MB\n", hwc_flash_size() >> 20);
	printf("Main SDRAM size   : %d MB\n", hwc_main_sdram_size() >> 20);
	printf("Local SDRAM size  : %d MB\n", hwc_local_sdram_size() >> 20);
	hwc_mac_address(string);
	printf("MAC address       : %s\n", string);
#endif
	return 0;
}

int misc_init_r(void)
{
	char *s, str[32];
	int num;

	if ((s = getenv("serial#")) == NULL &&
	    (num = hwc_serial_number()) != -1) {
		sprintf(str, "%06d", num);
		setenv("serial#", str);
	}
	if ((s = getenv("ethaddr")) == NULL && hwc_mac_address(str) == 0) {
		setenv("ethaddr", str);
	}

	return 0;
}
