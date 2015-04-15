/*
 * (C) Copyright 2013
 * Faraday Technology Corporation. <http://www.faraday-tech.com/tw/>
 * Kuo-Jung Su <dantesu@gmail.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <linux/compat.h>
#include <asm/io.h>
#include <malloc.h>
#include <spi.h>

#ifndef CONFIG_FTSSP010_BASE_LIST
#define CONFIG_FTSSP010_BASE_LIST   { CONFIG_FTSSP010_BASE }
#endif

#ifndef CONFIG_FTSSP010_GPIO_BASE
#define CONFIG_FTSSP010_GPIO_BASE   0
#endif

#ifndef CONFIG_FTSSP010_GPIO_LIST
#define CONFIG_FTSSP010_GPIO_LIST   { CONFIG_FTSSP010_GPIO_BASE }
#endif

#ifndef CONFIG_FTSSP010_CLOCK
#define CONFIG_FTSSP010_CLOCK       clk_get_rate("SSP");
#endif

#ifndef CONFIG_FTSSP010_TIMEOUT
#define CONFIG_FTSSP010_TIMEOUT     100
#endif

/* FTSSP010 chip registers */
struct ftssp010_regs {
	uint32_t cr[3];/* control register */
	uint32_t sr;   /* status register */
	uint32_t icr;  /* interrupt control register */
	uint32_t isr;  /* interrupt status register */
	uint32_t dr;   /* data register */
	uint32_t rsvd[17];
	uint32_t revr; /* revision register */
	uint32_t fear; /* feature register */
};

/* Control Register 0  */
#define CR0_FFMT_MASK       (7 << 12)
#define CR0_FFMT_SSP        (0 << 12)
#define CR0_FFMT_SPI        (1 << 12)
#define CR0_FFMT_MICROWIRE  (2 << 12)
#define CR0_FFMT_I2S        (3 << 12)
#define CR0_FFMT_AC97       (4 << 12)
#define CR0_FLASH           (1 << 11)
#define CR0_FSDIST(x)       (((x) & 0x03) << 8)
#define CR0_LOOP            (1 << 7)  /* loopback mode */
#define CR0_LSB             (1 << 6)  /* LSB */
#define CR0_FSPO            (1 << 5)  /* fs atcive low (I2S only) */
#define CR0_FSJUSTIFY       (1 << 4)
#define CR0_OPM_SLAVE       (0 << 2)
#define CR0_OPM_MASTER      (3 << 2)
#define CR0_OPM_I2S_MSST    (3 << 2)  /* master stereo mode */
#define CR0_OPM_I2S_MSMO    (2 << 2)  /* master mono mode */
#define CR0_OPM_I2S_SLST    (1 << 2)  /* slave stereo mode */
#define CR0_OPM_I2S_SLMO    (0 << 2)  /* slave mono mode */
#define CR0_SCLKPO          (1 << 1)  /* clock polarity */
#define CR0_SCLKPH          (1 << 0)  /* clock phase */

/* Control Register 1 */
#define CR1_PDL(x)   (((x) & 0xff) << 24) /* padding length */
#define CR1_SDL(x)   ((((x) - 1) & 0x1f) << 16) /* data length */
#define CR1_DIV(x)   (((x) - 1) & 0xffff) /* clock divider */

/* Control Register 2 */
#define CR2_CS(x)    (((x) & 3) << 10) /* CS/FS select */
#define CR2_FS       (1 << 9) /* CS/FS signal level */
#define CR2_TXEN     (1 << 8) /* tx enable */
#define CR2_RXEN     (1 << 7) /* rx enable */
#define CR2_RESET    (1 << 6) /* chip reset */
#define CR2_TXFC     (1 << 3) /* tx fifo Clear */
#define CR2_RXFC     (1 << 2) /* rx fifo Clear */
#define CR2_TXDOE    (1 << 1) /* tx data output enable */
#define CR2_EN       (1 << 0) /* chip enable */

/* Status Register */
#define SR_RFF       (1 << 0) /* rx fifo full */
#define SR_TFNF      (1 << 1) /* tx fifo not full */
#define SR_BUSY      (1 << 2) /* chip busy */
#define SR_RFVE(reg) (((reg) >> 4) & 0x1f)  /* rx fifo valid entries */
#define SR_TFVE(reg) (((reg) >> 12) & 0x1f) /* tx fifo valid entries */

/* Feature Register */
#define FEAR_BITS(reg)   ((((reg) >>  0) & 0xff) + 1) /* data width */
#define FEAR_RFSZ(reg)   ((((reg) >>  8) & 0xff) + 1) /* rx fifo size */
#define FEAR_TFSZ(reg)   ((((reg) >> 16) & 0xff) + 1) /* tx fifo size */
#define FEAR_AC97        (1 << 24)
#define FEAR_I2S         (1 << 25)
#define FEAR_SPI_MWR     (1 << 26)
#define FEAR_SSP         (1 << 27)
#define FEAR_SPDIF       (1 << 28)

/* FTGPIO010 chip registers */
struct ftgpio010_regs {
	uint32_t out;     /* 0x00: Data Output */
	uint32_t in;      /* 0x04: Data Input */
	uint32_t dir;     /* 0x08: Direction */
	uint32_t bypass;  /* 0x0c: Bypass */
	uint32_t set;     /* 0x10: Data Set */
	uint32_t clr;     /* 0x14: Data Clear */
	uint32_t pull_up; /* 0x18: Pull-Up Enabled */
	uint32_t pull_st; /* 0x1c: Pull State (0=pull-down, 1=pull-up) */
};

struct ftssp010_gpio {
	struct ftgpio010_regs *regs;
	uint32_t pin;
};

struct ftssp010_spi {
	struct spi_slave slave;
	struct ftssp010_gpio gpio;
	struct ftssp010_regs *regs;
	uint32_t fifo;
	uint32_t mode;
	uint32_t div;
	uint32_t clk;
	uint32_t speed;
	uint32_t revision;
};

static inline struct ftssp010_spi *to_ftssp010_spi(struct spi_slave *slave)
{
	return container_of(slave, struct ftssp010_spi, slave);
}

static int get_spi_chip(int bus, struct ftssp010_spi *chip)
{
	uint32_t fear, base[] = CONFIG_FTSSP010_BASE_LIST;

	if (bus >= ARRAY_SIZE(base) || !base[bus])
		return -1;

	chip->regs = (struct ftssp010_regs *)base[bus];

	chip->revision = readl(&chip->regs->revr);

	fear = readl(&chip->regs->fear);
	chip->fifo = min_t(uint32_t, FEAR_TFSZ(fear), FEAR_RFSZ(fear));

	return 0;
}

static int get_spi_gpio(int bus, struct ftssp010_gpio *chip)
{
	uint32_t base[] = CONFIG_FTSSP010_GPIO_LIST;

	if (bus >= ARRAY_SIZE(base) || !base[bus])
		return -1;

	chip->regs = (struct ftgpio010_regs *)(base[bus] & 0xfff00000);
	chip->pin = base[bus] & 0x1f;

	/* make it an output pin */
	setbits_le32(&chip->regs->dir, 1 << chip->pin);

	return 0;
}

static int ftssp010_wait(struct ftssp010_spi *chip)
{
	struct ftssp010_regs *regs = chip->regs;
	ulong t;

	/* wait until device idle */
	for (t = get_timer(0); get_timer(t) < CONFIG_FTSSP010_TIMEOUT; ) {
		if (!(readl(&regs->sr) & SR_BUSY))
			return 0;
	}

	puts("ftspi010: busy timeout\n");

	return -1;
}

static int ftssp010_wait_tx(struct ftssp010_spi *chip)
{
	struct ftssp010_regs *regs = chip->regs;
	ulong t;

	/* wait until tx fifo not full */
	for (t = get_timer(0); get_timer(t) < CONFIG_FTSSP010_TIMEOUT; ) {
		if (readl(&regs->sr) & SR_TFNF)
			return 0;
	}

	puts("ftssp010: tx timeout\n");

	return -1;
}

static int ftssp010_wait_rx(struct ftssp010_spi *chip)
{
	struct ftssp010_regs *regs = chip->regs;
	ulong t;

	/* wait until rx fifo not empty */
	for (t = get_timer(0); get_timer(t) < CONFIG_FTSSP010_TIMEOUT; ) {
		if (SR_RFVE(readl(&regs->sr)))
			return 0;
	}

	puts("ftssp010: rx timeout\n");

	return -1;
}

static int ftssp010_spi_work_transfer_v2(struct ftssp010_spi *chip,
	const void *tx_buf, void *rx_buf, int len, uint flags)
{
	struct ftssp010_regs *regs = chip->regs;
	const uint8_t *txb = tx_buf;
	uint8_t       *rxb = rx_buf;

	while (len > 0) {
		int i, depth = min(chip->fifo >> 2, len);
		uint32_t xmsk = 0;

		if (tx_buf) {
			for (i = 0; i < depth; ++i) {
				ftssp010_wait_tx(chip);
				writel(*txb++, &regs->dr);
			}
			xmsk |= CR2_TXEN | CR2_TXDOE;
			if ((readl(&regs->cr[2]) & xmsk) != xmsk)
				setbits_le32(&regs->cr[2], xmsk);
		}
		if (rx_buf) {
			xmsk |= CR2_RXEN;
			if ((readl(&regs->cr[2]) & xmsk) != xmsk)
				setbits_le32(&regs->cr[2], xmsk);
			for (i = 0; i < depth; ++i) {
				ftssp010_wait_rx(chip);
				*rxb++ = (uint8_t)readl(&regs->dr);
			}
		}

		len -= depth;
	}

	return 0;
}

static int ftssp010_spi_work_transfer_v1(struct ftssp010_spi *chip,
	const void *tx_buf, void *rx_buf, int len, uint flags)
{
	struct ftssp010_regs *regs = chip->regs;
	const uint8_t *txb = tx_buf;
	uint8_t       *rxb = rx_buf;

	while (len > 0) {
		int i, depth = min(chip->fifo >> 2, len);
		uint32_t tmp;

		for (i = 0; i < depth; ++i) {
			ftssp010_wait_tx(chip);
			writel(txb ? (*txb++) : 0, &regs->dr);
		}
		for (i = 0; i < depth; ++i) {
			ftssp010_wait_rx(chip);
			tmp = readl(&regs->dr);
			if (rxb)
				*rxb++ = (uint8_t)tmp;
		}

		len -= depth;
	}

	return 0;
}

static void ftssp010_cs_set(struct ftssp010_spi *chip, int high)
{
	struct ftssp010_regs *regs = chip->regs;
	struct ftssp010_gpio *gpio = &chip->gpio;
	uint32_t mask;

	/* cs pull high/low */
	if (chip->revision >= 0x11900) {
		mask = CR2_CS(chip->slave.cs) | (high ? CR2_FS : 0);
		writel(mask, &regs->cr[2]);
	} else if (gpio->regs) {
		mask = 1 << gpio->pin;
		if (high)
			writel(mask, &gpio->regs->set);
		else
			writel(mask, &gpio->regs->clr);
	}

	/* extra delay for signal propagation */
	udelay_masked(1);
}

/*
 * Determine if a SPI chipselect is valid.
 * This function is provided by the board if the low-level SPI driver
 * needs it to determine if a given chipselect is actually valid.
 *
 * Returns: 1 if bus:cs identifies a valid chip on this board, 0
 * otherwise.
 */
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	struct ftssp010_spi chip;

	if (get_spi_chip(bus, &chip))
		return 0;

	if (!cs)
		return 1;
	else if ((cs < 4) && (chip.revision >= 0x11900))
		return 1;

	return 0;
}

/*
 * Activate a SPI chipselect.
 * This function is provided by the board code when using a driver
 * that can't control its chipselects automatically (e.g.
 * common/soft_spi.c). When called, it should activate the chip select
 * to the device identified by "slave".
 */
void spi_cs_activate(struct spi_slave *slave)
{
	struct ftssp010_spi *chip = to_ftssp010_spi(slave);
	struct ftssp010_regs *regs = chip->regs;

	/* cs pull */
	if (chip->mode & SPI_CS_HIGH)
		ftssp010_cs_set(chip, 1);
	else
		ftssp010_cs_set(chip, 0);

	/* chip enable + fifo clear */
	setbits_le32(&regs->cr[2], CR2_EN | CR2_TXFC | CR2_RXFC);
}

/*
 * Deactivate a SPI chipselect.
 * This function is provided by the board code when using a driver
 * that can't control its chipselects automatically (e.g.
 * common/soft_spi.c). When called, it should deactivate the chip
 * select to the device identified by "slave".
 */
void spi_cs_deactivate(struct spi_slave *slave)
{
	struct ftssp010_spi *chip = to_ftssp010_spi(slave);

	/* wait until chip idle */
	ftssp010_wait(chip);

	/* cs pull */
	if (chip->mode & SPI_CS_HIGH)
		ftssp010_cs_set(chip, 0);
	else
		ftssp010_cs_set(chip, 1);
}

void spi_init(void)
{
	/* nothing to do */
}

struct spi_slave *spi_setup_slave(uint bus, uint cs, uint max_hz, uint mode)
{
	struct ftssp010_spi *chip;

	if (mode & SPI_3WIRE) {
		puts("ftssp010: can't do 3-wire\n");
		return NULL;
	}

	if (mode & SPI_SLAVE) {
		puts("ftssp010: can't do slave mode\n");
		return NULL;
	}

	if (mode & SPI_PREAMBLE) {
		puts("ftssp010: can't skip preamble bytes\n");
		return NULL;
	}

	if (!spi_cs_is_valid(bus, cs)) {
		puts("ftssp010: invalid (bus, cs)\n");
		return NULL;
	}

	chip = spi_alloc_slave(struct ftssp010_spi, bus, cs);
	if (!chip)
		return NULL;

	if (get_spi_chip(bus, chip))
		goto free_out;

	if (chip->revision < 0x11900 && get_spi_gpio(bus, &chip->gpio)) {
		puts("ftssp010: Before revision 1.19.0, its clock & cs are\n"
		"controlled by tx engine which is not synced with rx engine,\n"
		"so the clock & cs might be shutdown before rx engine\n"
		"finishs its jobs.\n"
		"If possible, please add a dedicated gpio for it.\n");
	}

	chip->mode = mode;
	chip->clk = CONFIG_FTSSP010_CLOCK;
	chip->div = 2;
	if (max_hz) {
		while (chip->div < 0xffff) {
			if ((chip->clk / (2 * chip->div)) <= max_hz)
				break;
			chip->div += 1;
		}
	}
	chip->speed = chip->clk / (2 * chip->div);

	return &chip->slave;

free_out:
	free(chip);
	return NULL;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct ftssp010_spi *chip = to_ftssp010_spi(slave);

	free(chip);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct ftssp010_spi *chip = to_ftssp010_spi(slave);
	struct ftssp010_regs *regs = chip->regs;

	writel(CR1_SDL(8) | CR1_DIV(chip->div), &regs->cr[1]);

	if (chip->revision >= 0x11900) {
		writel(CR0_OPM_MASTER | CR0_FFMT_SPI | CR0_FSPO | CR0_FLASH,
		       &regs->cr[0]);
		writel(CR2_TXFC | CR2_RXFC,
		       &regs->cr[2]);
	} else {
		writel(CR0_OPM_MASTER | CR0_FFMT_SPI | CR0_FSPO,
		       &regs->cr[0]);
		writel(CR2_TXFC | CR2_RXFC | CR2_EN | CR2_TXDOE,
		       &regs->cr[2]);
	}

	if (chip->mode & SPI_LOOP)
		setbits_le32(&regs->cr[0], CR0_LOOP);

	if (chip->mode & SPI_CPOL)
		setbits_le32(&regs->cr[0], CR0_SCLKPO);

	if (chip->mode & SPI_CPHA)
		setbits_le32(&regs->cr[0], CR0_SCLKPH);

	spi_cs_deactivate(slave);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct ftssp010_spi *chip = to_ftssp010_spi(slave);
	struct ftssp010_regs *regs = chip->regs;

	writel(0, &regs->cr[2]);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
			 const void *dout, void *din, unsigned long flags)
{
	struct ftssp010_spi *chip = to_ftssp010_spi(slave);
	uint32_t len = bitlen >> 3;

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	if (chip->revision >= 0x11900)
		ftssp010_spi_work_transfer_v2(chip, dout, din, len, flags);
	else
		ftssp010_spi_work_transfer_v1(chip, dout, din, len, flags);

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
