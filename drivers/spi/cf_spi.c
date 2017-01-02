/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2009 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/immap.h>

struct cf_spi_slave {
	struct spi_slave slave;
	uint baudrate;
	int charbit;
};

extern void cfspi_port_conf(void);
extern int cfspi_claim_bus(uint bus, uint cs);
extern void cfspi_release_bus(uint bus, uint cs);

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SPI_IDLE_VAL
#if defined(CONFIG_SPI_MMC)
#define CONFIG_SPI_IDLE_VAL	0xFFFF
#else
#define CONFIG_SPI_IDLE_VAL	0x0
#endif
#endif

#if defined(CONFIG_CF_DSPI)
/* DSPI specific mode */
#define SPI_MODE_MOD	0x00200000
#define SPI_DBLRATE	0x00100000

static inline struct cf_spi_slave *to_cf_spi_slave(struct spi_slave *slave)
{
	return container_of(slave, struct cf_spi_slave, slave);
}

static void cfspi_init(void)
{
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;

	cfspi_port_conf();	/* port configuration */

	dspi->mcr = DSPI_MCR_MSTR | DSPI_MCR_CSIS7 | DSPI_MCR_CSIS6 |
	    DSPI_MCR_CSIS5 | DSPI_MCR_CSIS4 | DSPI_MCR_CSIS3 |
	    DSPI_MCR_CSIS2 | DSPI_MCR_CSIS1 | DSPI_MCR_CSIS0 |
	    DSPI_MCR_CRXF | DSPI_MCR_CTXF;

	/* Default setting in platform configuration */
#ifdef CONFIG_SYS_DSPI_CTAR0
	dspi->ctar[0] = CONFIG_SYS_DSPI_CTAR0;
#endif
#ifdef CONFIG_SYS_DSPI_CTAR1
	dspi->ctar[1] = CONFIG_SYS_DSPI_CTAR1;
#endif
#ifdef CONFIG_SYS_DSPI_CTAR2
	dspi->ctar[2] = CONFIG_SYS_DSPI_CTAR2;
#endif
#ifdef CONFIG_SYS_DSPI_CTAR3
	dspi->ctar[3] = CONFIG_SYS_DSPI_CTAR3;
#endif
#ifdef CONFIG_SYS_DSPI_CTAR4
	dspi->ctar[4] = CONFIG_SYS_DSPI_CTAR4;
#endif
#ifdef CONFIG_SYS_DSPI_CTAR5
	dspi->ctar[5] = CONFIG_SYS_DSPI_CTAR5;
#endif
#ifdef CONFIG_SYS_DSPI_CTAR6
	dspi->ctar[6] = CONFIG_SYS_DSPI_CTAR6;
#endif
#ifdef CONFIG_SYS_DSPI_CTAR7
	dspi->ctar[7] = CONFIG_SYS_DSPI_CTAR7;
#endif
}

static void cfspi_tx(u32 ctrl, u16 data)
{
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;

	while ((dspi->sr & 0x0000F000) >= 4) ;

	dspi->tfr = (ctrl | data);
}

static u16 cfspi_rx(void)
{
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;

	while ((dspi->sr & 0x000000F0) == 0) ;

	return (dspi->rfr & 0xFFFF);
}

static int cfspi_xfer(struct spi_slave *slave, uint bitlen, const void *dout,
		      void *din, ulong flags)
{
	struct cf_spi_slave *cfslave = to_cf_spi_slave(slave);
	u16 *spi_rd16 = NULL, *spi_wr16 = NULL;
	u8 *spi_rd = NULL, *spi_wr = NULL;
	static u32 ctrl = 0;
	uint len = bitlen >> 3;

	if (cfslave->charbit == 16) {
		bitlen >>= 1;
		spi_wr16 = (u16 *) dout;
		spi_rd16 = (u16 *) din;
	} else {
		spi_wr = (u8 *) dout;
		spi_rd = (u8 *) din;
	}

	if ((flags & SPI_XFER_BEGIN) == SPI_XFER_BEGIN)
		ctrl |= DSPI_TFR_CONT;

	ctrl = (ctrl & 0xFF000000) | ((1 << slave->cs) << 16);

	if (len > 1) {
		int tmp_len = len - 1;
		while (tmp_len--) {
			if (dout != NULL) {
				if (cfslave->charbit == 16)
					cfspi_tx(ctrl, *spi_wr16++);
				else
					cfspi_tx(ctrl, *spi_wr++);
				cfspi_rx();
			}

			if (din != NULL) {
				cfspi_tx(ctrl, CONFIG_SPI_IDLE_VAL);
				if (cfslave->charbit == 16)
					*spi_rd16++ = cfspi_rx();
				else
					*spi_rd++ = cfspi_rx();
			}
		}

		len = 1;	/* remaining byte */
	}

	if ((flags & SPI_XFER_END) == SPI_XFER_END)
		ctrl &= ~DSPI_TFR_CONT;

	if (len) {
		if (dout != NULL) {
			if (cfslave->charbit == 16)
				cfspi_tx(ctrl, *spi_wr16);
			else
				cfspi_tx(ctrl, *spi_wr);
			cfspi_rx();
		}

		if (din != NULL) {
			cfspi_tx(ctrl, CONFIG_SPI_IDLE_VAL);
			if (cfslave->charbit == 16)
				*spi_rd16 = cfspi_rx();
			else
				*spi_rd = cfspi_rx();
		}
	} else {
		/* dummy read */
		cfspi_tx(ctrl, CONFIG_SPI_IDLE_VAL);
		cfspi_rx();
	}

	return 0;
}

static struct spi_slave *cfspi_setup_slave(struct cf_spi_slave *cfslave,
					   uint mode)
{
	/*
	 * bit definition for mode:
	 * bit 31 - 28: Transfer size 3 to 16 bits
	 *     27 - 26: PCS to SCK delay prescaler
	 *     25 - 24: After SCK delay prescaler
	 *     23 - 22: Delay after transfer prescaler
	 *     21     : Allow overwrite for bit 31-22 and bit 20-8
	 *     20     : Double baud rate
	 *     19 - 16: PCS to SCK delay scaler
	 *     15 - 12: After SCK delay scaler
	 *     11 -  8: Delay after transfer scaler
	 *      7 -  0: SPI_CPHA, SPI_CPOL, SPI_LSB_FIRST
	 */
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;
	int prescaler[] = { 2, 3, 5, 7 };
	int scaler[] = {
		2, 4, 6, 8,
		16, 32, 64, 128,
		256, 512, 1024, 2048,
		4096, 8192, 16384, 32768
	};
	int i, j, pbrcnt, brcnt, diff, tmp, dbr = 0;
	int best_i, best_j, bestmatch = 0x7FFFFFFF, baud_speed;
	u32 bus_setup = 0;

	tmp = (prescaler[3] * scaler[15]);
	/* Maximum and minimum baudrate it can handle */
	if ((cfslave->baudrate > (gd->bus_clk >> 1)) ||
	    (cfslave->baudrate < (gd->bus_clk / tmp))) {
		printf("Exceed baudrate limitation: Max %d - Min %d\n",
		       (int)(gd->bus_clk >> 1), (int)(gd->bus_clk / tmp));
		return NULL;
	}

	/* Activate Double Baud when it exceed 1/4 the bus clk */
	if ((CONFIG_SYS_DSPI_CTAR0 & DSPI_CTAR_DBR) ||
	    (cfslave->baudrate > (gd->bus_clk / (prescaler[0] * scaler[0])))) {
		bus_setup |= DSPI_CTAR_DBR;
		dbr = 1;
	}

	if (mode & SPI_CPOL)
		bus_setup |= DSPI_CTAR_CPOL;
	if (mode & SPI_CPHA)
		bus_setup |= DSPI_CTAR_CPHA;
	if (mode & SPI_LSB_FIRST)
		bus_setup |= DSPI_CTAR_LSBFE;

	/* Overwrite default value set in platform configuration file */
	if (mode & SPI_MODE_MOD) {

		if ((mode & 0xF0000000) == 0)
			bus_setup |=
			    dspi->ctar[cfslave->slave.bus] & 0x78000000;
		else
			bus_setup |= ((mode & 0xF0000000) >> 1);

		/*
		 * Check to see if it is enabled by default in platform
		 * config, or manual setting passed by mode parameter
		 */
		if (mode & SPI_DBLRATE) {
			bus_setup |= DSPI_CTAR_DBR;
			dbr = 1;
		}
		bus_setup |= (mode & 0x0FC00000) >> 4;	/* PSCSCK, PASC, PDT */
		bus_setup |= (mode & 0x000FFF00) >> 4;	/* CSSCK, ASC, DT */
	} else
		bus_setup |= (dspi->ctar[cfslave->slave.bus] & 0x78FCFFF0);

	cfslave->charbit =
	    ((dspi->ctar[cfslave->slave.bus] & 0x78000000) ==
	     0x78000000) ? 16 : 8;

	pbrcnt = sizeof(prescaler) / sizeof(int);
	brcnt = sizeof(scaler) / sizeof(int);

	/* baudrate calculation - to closer value, may not be exact match */
	for (best_i = 0, best_j = 0, i = 0; i < pbrcnt; i++) {
		baud_speed = gd->bus_clk / prescaler[i];
		for (j = 0; j < brcnt; j++) {
			tmp = (baud_speed / scaler[j]) * (1 + dbr);

			if (tmp > cfslave->baudrate)
				diff = tmp - cfslave->baudrate;
			else
				diff = cfslave->baudrate - tmp;

			if (diff < bestmatch) {
				bestmatch = diff;
				best_i = i;
				best_j = j;
			}
		}
	}
	bus_setup |= (DSPI_CTAR_PBR(best_i) | DSPI_CTAR_BR(best_j));
	dspi->ctar[cfslave->slave.bus] = bus_setup;

	return &cfslave->slave;
}
#endif				/* CONFIG_CF_DSPI */

#ifdef CONFIG_CMD_SPI
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	if (((cs >= 0) && (cs < 8)) && ((bus >= 0) && (bus < 8)))
		return 1;
	else
		return 0;
}

void spi_init_f(void)
{
}

void spi_init_r(void)
{
}

void spi_init(void)
{
	cfspi_init();
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct cf_spi_slave *cfslave;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	cfslave = spi_alloc_slave(struct cf_spi_slave, bus, cs);
	if (!cfslave)
		return NULL;

	cfslave->baudrate = max_hz;

	/* specific setup */
	return cfspi_setup_slave(cfslave, mode);
}

void spi_free_slave(struct spi_slave *slave)
{
	struct cf_spi_slave *cfslave = to_cf_spi_slave(slave);

	free(cfslave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return cfspi_claim_bus(slave->bus, slave->cs);
}

void spi_release_bus(struct spi_slave *slave)
{
	cfspi_release_bus(slave->bus, slave->cs);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	return cfspi_xfer(slave, bitlen, dout, din, flags);
}
#endif				/* CONFIG_CMD_SPI */
