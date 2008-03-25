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

	dspi->dctar0 = DSPI_DCTAR_TRSZ(7) | DSPI_DCTAR_CPOL | DSPI_DCTAR_CPHA |
	    DSPI_DCTAR_PCSSCK_1CLK | DSPI_DCTAR_PASC(0) |
	    DSPI_DCTAR_PDT(0) | DSPI_DCTAR_CSSCK(0) |
	    DSPI_DCTAR_ASC(0) | DSPI_DCTAR_PBR(0) |
	    DSPI_DCTAR_DT(1) | DSPI_DCTAR_BR(1);
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

#endif				/* CONFIG_HARD_SPI */
