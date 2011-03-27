/*
 * SH SPI driver
 *
 * Copyright (C) 2011 Renesas Solutions Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __SH_SPI_H__
#define __SH_SPI_H__

#include <spi.h>

struct sh_spi_regs {
	unsigned long tbr_rbr;
	unsigned long resv1;
	unsigned long cr1;
	unsigned long resv2;
	unsigned long cr2;
	unsigned long resv3;
	unsigned long cr3;
	unsigned long resv4;
	unsigned long cr4;
};

/* CR1 */
#define SH_SPI_TBE	0x80
#define SH_SPI_TBF	0x40
#define SH_SPI_RBE	0x20
#define SH_SPI_RBF	0x10
#define SH_SPI_PFONRD	0x08
#define SH_SPI_SSDB	0x04
#define SH_SPI_SSD	0x02
#define SH_SPI_SSA	0x01

/* CR2 */
#define SH_SPI_RSTF	0x80
#define SH_SPI_LOOPBK	0x40
#define SH_SPI_CPOL	0x20
#define SH_SPI_CPHA	0x10
#define SH_SPI_L1M0	0x08

/* CR3 */
#define SH_SPI_MAX_BYTE	0xFF

/* CR4 */
#define SH_SPI_TBEI	0x80
#define SH_SPI_TBFI	0x40
#define SH_SPI_RBEI	0x20
#define SH_SPI_RBFI	0x10
#define SH_SPI_WPABRT	0x04
#define SH_SPI_SSS	0x01

#define SH_SPI_FIFO_SIZE	32

struct sh_spi {
	struct spi_slave	slave;
	struct sh_spi_regs	*regs;
};

static inline struct sh_spi *to_sh_spi(struct spi_slave *slave)
{
	return container_of(slave, struct sh_spi, slave);
}

#endif
