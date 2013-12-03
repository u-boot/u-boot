/*
 * Simulate a SPI port
 *
 * Copyright (c) 2011-2013 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <os.h>

#include <asm/errno.h>
#include <asm/spi.h>
#include <asm/state.h>

#ifndef CONFIG_SPI_IDLE_VAL
# define CONFIG_SPI_IDLE_VAL 0xFF
#endif

struct sandbox_spi_slave {
	struct spi_slave slave;
	const struct sandbox_spi_emu_ops *ops;
	void *priv;
};

#define to_sandbox_spi_slave(s) container_of(s, struct sandbox_spi_slave, slave)

const char *sandbox_spi_parse_spec(const char *arg, unsigned long *bus,
				   unsigned long *cs)
{
	char *endp;

	*bus = simple_strtoul(arg, &endp, 0);
	if (*endp != ':' || *bus >= CONFIG_SANDBOX_SPI_MAX_BUS)
		return NULL;

	*cs = simple_strtoul(endp + 1, &endp, 0);
	if (*endp != ':' || *cs >= CONFIG_SANDBOX_SPI_MAX_CS)
		return NULL;

	return endp + 1;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus < CONFIG_SANDBOX_SPI_MAX_BUS &&
		cs < CONFIG_SANDBOX_SPI_MAX_CS;
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct sandbox_spi_slave *sss = to_sandbox_spi_slave(slave);

	debug("sandbox_spi: activating CS\n");
	if (sss->ops->cs_activate)
		sss->ops->cs_activate(sss->priv);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct sandbox_spi_slave *sss = to_sandbox_spi_slave(slave);

	debug("sandbox_spi: deactivating CS\n");
	if (sss->ops->cs_deactivate)
		sss->ops->cs_deactivate(sss->priv);
}

void spi_init(void)
{
}

void spi_set_speed(struct spi_slave *slave, uint hz)
{
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct sandbox_spi_slave *sss;
	struct sandbox_state *state = state_get_current();
	const char *spec;

	if (!spi_cs_is_valid(bus, cs)) {
		debug("sandbox_spi: Invalid SPI bus/cs\n");
		return NULL;
	}

	sss = spi_alloc_slave(struct sandbox_spi_slave, bus, cs);
	if (!sss) {
		debug("sandbox_spi: Out of memory\n");
		return NULL;
	}

	spec = state->spi[bus][cs].spec;
	sss->ops = state->spi[bus][cs].ops;
	if (!spec || !sss->ops || sss->ops->setup(&sss->priv, spec)) {
		free(sss);
		printf("sandbox_spi: unable to locate a slave client\n");
		return NULL;
	}

	return &sss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct sandbox_spi_slave *sss = to_sandbox_spi_slave(slave);

	debug("sandbox_spi: releasing slave\n");

	if (sss->ops->free)
		sss->ops->free(sss->priv);

	free(sss);
}

static int spi_bus_claim_cnt[CONFIG_SANDBOX_SPI_MAX_BUS];

int spi_claim_bus(struct spi_slave *slave)
{
	if (spi_bus_claim_cnt[slave->bus]++) {
		printf("sandbox_spi: error: bus already claimed: %d!\n",
		       spi_bus_claim_cnt[slave->bus]);
	}

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	if (--spi_bus_claim_cnt[slave->bus]) {
		printf("sandbox_spi: error: bus freed too often: %d!\n",
		       spi_bus_claim_cnt[slave->bus]);
	}
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct sandbox_spi_slave *sss = to_sandbox_spi_slave(slave);
	uint bytes = bitlen / 8, i;
	int ret = 0;
	u8 *tx = (void *)dout, *rx = din;

	if (bitlen == 0)
		goto done;

	/* we can only do 8 bit transfers */
	if (bitlen % 8) {
		printf("sandbox_spi: xfer: invalid bitlen size %u; needs to be 8bit\n",
		       bitlen);
		flags |= SPI_XFER_END;
		goto done;
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	/* make sure rx/tx buffers are full so clients can assume */
	if (!tx) {
		debug("sandbox_spi: xfer: auto-allocating tx scratch buffer\n");
		tx = malloc(bytes);
		if (!tx) {
			debug("sandbox_spi: Out of memory\n");
			return -ENOMEM;
		}
	}
	if (!rx) {
		debug("sandbox_spi: xfer: auto-allocating rx scratch buffer\n");
		rx = malloc(bytes);
		if (!rx) {
			debug("sandbox_spi: Out of memory\n");
			return -ENOMEM;
		}
	}

	debug("sandbox_spi: xfer: bytes = %u\n tx:", bytes);
	for (i = 0; i < bytes; ++i)
		debug(" %u:%02x", i, tx[i]);
	debug("\n");

	ret = sss->ops->xfer(sss->priv, tx, rx, bytes);

	debug("sandbox_spi: xfer: got back %i (that's %s)\n rx:",
	      ret, ret ? "bad" : "good");
	for (i = 0; i < bytes; ++i)
		debug(" %u:%02x", i, rx[i]);
	debug("\n");

	if (tx != dout)
		free(tx);
	if (rx != din)
		free(rx);

 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return ret;
}
