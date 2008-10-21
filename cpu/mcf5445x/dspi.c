/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
#include <spi.h>
#include <malloc.h>

#if defined(CONFIG_CF_DSPI)
#include <asm/immap.h>

void dspi_init(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;

	gpio->par_dspi = GPIO_PAR_DSPI_PCS5_PCS5 | GPIO_PAR_DSPI_PCS2_PCS2 |
	    GPIO_PAR_DSPI_PCS1_PCS1 | GPIO_PAR_DSPI_PCS0_PCS0 |
	    GPIO_PAR_DSPI_SIN_SIN | GPIO_PAR_DSPI_SOUT_SOUT |
	    GPIO_PAR_DSPI_SCK_SCK;

	dspi->dmcr = DSPI_DMCR_MSTR | DSPI_DMCR_CSIS7 | DSPI_DMCR_CSIS6 |
	    DSPI_DMCR_CSIS5 | DSPI_DMCR_CSIS4 | DSPI_DMCR_CSIS3 |
	    DSPI_DMCR_CSIS2 | DSPI_DMCR_CSIS1 | DSPI_DMCR_CSIS0 |
	    DSPI_DMCR_CRXF | DSPI_DMCR_CTXF;

#ifdef CONFIG_SYS_DSPI_DCTAR0
	dspi->dctar0 = CONFIG_SYS_DSPI_DCTAR0;
#endif
#ifdef CONFIG_SYS_DSPI_DCTAR1
	dspi->dctar1 = CONFIG_SYS_DSPI_DCTAR1;
#endif
#ifdef CONFIG_SYS_DSPI_DCTAR2
	dspi->dctar2 = CONFIG_SYS_DSPI_DCTAR2;
#endif
#ifdef CONFIG_SYS_DSPI_DCTAR3
	dspi->dctar3 = CONFIG_SYS_DSPI_DCTAR3;
#endif
#ifdef CONFIG_SYS_DSPI_DCTAR4
	dspi->dctar4 = CONFIG_SYS_DSPI_DCTAR4;
#endif
#ifdef CONFIG_SYS_DSPI_DCTAR5
	dspi->dctar5 = CONFIG_SYS_DSPI_DCTAR5;
#endif
#ifdef CONFIG_SYS_DSPI_DCTAR6
	dspi->dctar6 = CONFIG_SYS_DSPI_DCTAR6;
#endif
#ifdef CONFIG_SYS_DSPI_DCTAR7
	dspi->dctar7 = CONFIG_SYS_DSPI_DCTAR7;
#endif
}

void dspi_tx(int chipsel, u8 attrib, u16 data)
{
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;

	while ((dspi->dsr & 0x0000F000) >= 4) ;

	dspi->dtfr = (attrib << 24) | ((1 << chipsel) << 16) | data;
}

u16 dspi_rx(void)
{
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;

	while ((dspi->dsr & 0x000000F0) == 0) ;

	return (dspi->drfr & 0xFFFF);
}

#if defined(CONFIG_CMD_SPI)
void spi_init_f(void)
{
}

void spi_init_r(void)
{
}

void spi_init(void)
{
	dspi_init();
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct spi_slave *slave;

	slave = malloc(sizeof(struct spi_slave));
	if (!slave)
		return NULL;

	slave->bus = bus;
	slave->cs = cs;

	return slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	free(slave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	static int bWrite = 0;
	u8 *spi_rd, *spi_wr;
	int len = bitlen >> 3;

	spi_rd = (u8 *) din;
	spi_wr = (u8 *) dout;

	/* command handling */
	if (((len == 4) || (len == 1) || (len == 5)) && (dout != NULL)) {
		switch (*spi_wr) {
		case 0x02:	/* Page Prog */
			bWrite = 1;
			dspi_tx(slave->cs, 0x80, spi_wr[0]);
			dspi_rx();
			dspi_tx(slave->cs, 0x80, spi_wr[1]);
			dspi_rx();
			dspi_tx(slave->cs, 0x80, spi_wr[2]);
			dspi_rx();
			dspi_tx(slave->cs, 0x80, spi_wr[3]);
			dspi_rx();
			return 0;
		case 0x05:	/* Read Status */
			if (len == 4)
				if ((spi_wr[1] == 0xFF) && (spi_wr[2] == 0xFF)
				    && (spi_wr[3] == 0xFF)) {
					dspi_tx(slave->cs, 0x80, *spi_wr);
					dspi_rx();
				}
			return 0;
		case 0x06:	/* WREN */
			dspi_tx(slave->cs, 0x00, *spi_wr);
			dspi_rx();
			return 0;
		case 0x0B:	/* Fast read */
			if ((len == 5) && (spi_wr[4] == 0)) {
				dspi_tx(slave->cs, 0x80, spi_wr[0]);
				dspi_rx();
				dspi_tx(slave->cs, 0x80, spi_wr[1]);
				dspi_rx();
				dspi_tx(slave->cs, 0x80, spi_wr[2]);
				dspi_rx();
				dspi_tx(slave->cs, 0x80, spi_wr[3]);
				dspi_rx();
				dspi_tx(slave->cs, 0x80, spi_wr[4]);
				dspi_rx();
			}
			return 0;
		case 0x9F:	/* RDID */
			dspi_tx(slave->cs, 0x80, *spi_wr);
			dspi_rx();
			return 0;
		case 0xD8:	/* Sector erase */
			if (len == 4)
				if ((spi_wr[2] == 0) && (spi_wr[3] == 0)) {
					dspi_tx(slave->cs, 0x80, spi_wr[0]);
					dspi_rx();
					dspi_tx(slave->cs, 0x80, spi_wr[1]);
					dspi_rx();
					dspi_tx(slave->cs, 0x80, spi_wr[2]);
					dspi_rx();
					dspi_tx(slave->cs, 0x00, spi_wr[3]);
					dspi_rx();
				}
			return 0;
		}
	}

	if (bWrite)
		len--;

	while (len--) {
		if (dout != NULL) {
			dspi_tx(slave->cs, 0x80, *spi_wr);
			dspi_rx();
			spi_wr++;
		}

		if (din != NULL) {
			dspi_tx(slave->cs, 0x80, 0);
			*spi_rd = dspi_rx();
			spi_rd++;
		}
	}

	if (flags == SPI_XFER_END) {
		if (bWrite) {
			dspi_tx(slave->cs, 0x00, *spi_wr);
			dspi_rx();
			bWrite = 0;
		} else {
			dspi_tx(slave->cs, 0x00, 0);
			dspi_rx();
		}
	}

	return 0;
}
#endif				/* CONFIG_CMD_SPI */

#endif				/* CONFIG_CF_DSPI */
