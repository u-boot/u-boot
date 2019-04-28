// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2006 Ben Warren, Qstreams Networks Inc.
 * With help from the common/soft_spi and arch/powerpc/cpu/mpc8260 drivers
 */

#include <common.h>

#include <malloc.h>
#include <spi.h>
#include <asm/mpc8xxx_spi.h>

#define SPI_EV_NE	(0x80000000 >> 22)	/* Receiver Not Empty */
#define SPI_EV_NF	(0x80000000 >> 23)	/* Transmitter Not Full */

#define SPI_MODE_LOOP	(0x80000000 >> 1)	/* Loopback mode */
#define SPI_MODE_REV	(0x80000000 >> 5)	/* Reverse mode - MSB first */
#define SPI_MODE_MS	(0x80000000 >> 6)	/* Always master */
#define SPI_MODE_EN	(0x80000000 >> 7)	/* Enable interface */

#define SPI_TIMEOUT	1000

struct spi_slave *spi_setup_slave(uint bus, uint cs, uint max_hz, uint mode)
{
	struct spi_slave *slave;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	slave = spi_alloc_slave_base(bus, cs);
	if (!slave)
		return NULL;

	/*
	 * TODO: Some of the code in spi_init() should probably move
	 * here, or into spi_claim_bus() below.
	 */

	return slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	free(slave);
}

void spi_init(void)
{
	volatile spi8xxx_t *spi = &((immap_t *) (CONFIG_SYS_IMMR))->spi;

	/*
	 * SPI pins on the MPC83xx are not muxed, so all we do is initialize
	 * some registers
	 */
	spi->mode = SPI_MODE_REV | SPI_MODE_MS | SPI_MODE_EN;
	/* Use SYSCLK / 8 (16.67MHz typ.) */
	spi->mode = (spi->mode & 0xfff0ffff) | BIT(16);
	/* Clear all SPI events */
	spi->event = 0xffffffff;
	/* Mask  all SPI interrupts */
	spi->mask = 0x00000000;
	/* LST bit doesn't do anything, so disregard */
	spi->com = 0;
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

int spi_xfer(struct spi_slave *slave, uint bitlen, const void *dout, void *din,
	     ulong flags)
{
	volatile spi8xxx_t *spi = &((immap_t *) (CONFIG_SYS_IMMR))->spi;
	uint tmpdout, tmpdin, event;
	int num_blks = DIV_ROUND_UP(bitlen, 32);
	int tm, is_read = 0;
	uchar char_size = 32;

	debug("spi_xfer: slave %u:%u dout %08X din %08X bitlen %u\n",
	      slave->bus, slave->cs, *(uint *) dout, *(uint *) din, bitlen);

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	/* Clear all SPI events */
	spi->event = 0xffffffff;

	/* Handle data in 32-bit chunks */
	while (num_blks--) {
		tmpdout = 0;
		char_size = (bitlen >= 32 ? 32 : bitlen);

		/* Shift data so it's msb-justified */
		tmpdout = *(u32 *) dout >> (32 - char_size);

		/* The LEN field of the SPMODE register is set as follows:
		 *
		 * Bit length             setting
		 * len <= 4               3
		 * 4 < len <= 16          len - 1
		 * len > 16               0
		 */

		spi->mode &= ~SPI_MODE_EN;

		if (bitlen <= 16) {
			if (bitlen <= 4)
				spi->mode = (spi->mode & 0xff0fffff) |
					    (3 << 20);
			else
				spi->mode = (spi->mode & 0xff0fffff) |
					    ((bitlen - 1) << 20);
		} else {
			spi->mode = (spi->mode & 0xff0fffff);
			/* Set up the next iteration if sending > 32 bits */
			bitlen -= 32;
			dout += 4;
		}

		spi->mode |= SPI_MODE_EN;

		/* Write the data out */
		spi->tx = tmpdout;

		debug("*** spi_xfer: ... %08x written\n", tmpdout);

		/*
		 * Wait for SPI transmit to get out
		 * or time out (1 second = 1000 ms)
		 * The NE event must be read and cleared first
		 */
		for (tm = 0, is_read = 0; tm < SPI_TIMEOUT; ++tm) {
			event = spi->event;
			if (event & SPI_EV_NE) {
				tmpdin = spi->rx;
				spi->event |= SPI_EV_NE;
				is_read = 1;

				*(u32 *) din = (tmpdin << (32 - char_size));
				if (char_size == 32) {
					/* Advance output buffer by 32 bits */
					din += 4;
				}
			}
			/*
			 * Only bail when we've had both NE and NF events.
			 * This will cause timeouts on RO devices, so maybe
			 * in the future put an arbitrary delay after writing
			 * the device.  Arbitrary delays suck, though...
			 */
			if (is_read && (event & SPI_EV_NF))
				break;
		}
		if (tm >= SPI_TIMEOUT)
			puts("*** spi_xfer: Time out during SPI transfer");

		debug("*** spi_xfer: transfer ended. Value=%08x\n", tmpdin);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
