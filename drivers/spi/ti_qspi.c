/*
 * TI QSPI driver
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/omap.h>
#include <malloc.h>
#include <spi.h>
#include <asm/gpio.h>
#include <asm/omap_gpio.h>
#include <asm/omap_common.h>
#include <asm/ti-common/ti-edma3.h>

/* ti qpsi register bit masks */
#define QSPI_TIMEOUT                    2000000
#define QSPI_FCLK                       192000000
/* clock control */
#define QSPI_CLK_EN                     BIT(31)
#define QSPI_CLK_DIV_MAX                0xffff
/* command */
#define QSPI_EN_CS(n)                   (n << 28)
#define QSPI_WLEN(n)                    ((n-1) << 19)
#define QSPI_3_PIN                      BIT(18)
#define QSPI_RD_SNGL                    BIT(16)
#define QSPI_WR_SNGL                    (2 << 16)
#define QSPI_INVAL                      (4 << 16)
#define QSPI_RD_QUAD                    (7 << 16)
/* device control */
#define QSPI_DD(m, n)                   (m << (3 + n*8))
#define QSPI_CKPHA(n)                   (1 << (2 + n*8))
#define QSPI_CSPOL(n)                   (1 << (1 + n*8))
#define QSPI_CKPOL(n)                   (1 << (n*8))
/* status */
#define QSPI_WC                         BIT(1)
#define QSPI_BUSY                       BIT(0)
#define QSPI_WC_BUSY                    (QSPI_WC | QSPI_BUSY)
#define QSPI_XFER_DONE                  QSPI_WC
#define MM_SWITCH                       0x01
#define MEM_CS(cs)                      ((cs + 1) << 8)
#define MEM_CS_UNSELECT                 0xfffff0ff
#define MMAP_START_ADDR_DRA		0x5c000000
#define MMAP_START_ADDR_AM43x		0x30000000
#define CORE_CTRL_IO                    0x4a002558

#define QSPI_CMD_READ                   (0x3 << 0)
#define QSPI_CMD_READ_QUAD              (0x6b << 0)
#define QSPI_CMD_READ_FAST              (0x0b << 0)
#define QSPI_SETUP0_NUM_A_BYTES         (0x2 << 8)
#define QSPI_SETUP0_NUM_D_BYTES_NO_BITS (0x0 << 10)
#define QSPI_SETUP0_NUM_D_BYTES_8_BITS  (0x1 << 10)
#define QSPI_SETUP0_READ_NORMAL         (0x0 << 12)
#define QSPI_SETUP0_READ_QUAD           (0x3 << 12)
#define QSPI_CMD_WRITE                  (0x2 << 16)
#define QSPI_NUM_DUMMY_BITS             (0x0 << 24)

/* ti qspi register set */
struct ti_qspi_regs {
	u32 pid;
	u32 pad0[3];
	u32 sysconfig;
	u32 pad1[3];
	u32 int_stat_raw;
	u32 int_stat_en;
	u32 int_en_set;
	u32 int_en_ctlr;
	u32 intc_eoi;
	u32 pad2[3];
	u32 clk_ctrl;
	u32 dc;
	u32 cmd;
	u32 status;
	u32 data;
	u32 setup0;
	u32 setup1;
	u32 setup2;
	u32 setup3;
	u32 memswitch;
	u32 data1;
	u32 data2;
	u32 data3;
};

/* ti qspi priv */
struct ti_qspi_priv {
	struct spi_slave slave;
	struct ti_qspi_regs *base;
	unsigned int mode;
	u32 cmd;
	u32 dc;
};

static inline struct ti_qspi_priv *to_ti_qspi_priv(struct spi_slave *slave)
{
	return container_of(slave, struct ti_qspi_priv, slave);
}

static void ti_spi_setup_spi_register(struct ti_qspi_priv *priv)
{
	struct spi_slave *slave = &priv->slave;
	u32 memval = 0;

#if defined(CONFIG_DRA7XX) || defined(CONFIG_AM57XX)
	slave->memory_map = (void *)MMAP_START_ADDR_DRA;
#else
	slave->memory_map = (void *)MMAP_START_ADDR_AM43x;
#endif

#ifdef CONFIG_QSPI_QUAD_SUPPORT
	memval |= (QSPI_CMD_READ_QUAD | QSPI_SETUP0_NUM_A_BYTES |
			QSPI_SETUP0_NUM_D_BYTES_8_BITS |
			QSPI_SETUP0_READ_QUAD | QSPI_CMD_WRITE |
			QSPI_NUM_DUMMY_BITS);
	slave->mode_rx = SPI_RX_QUAD;
#else
	memval |= QSPI_CMD_READ | QSPI_SETUP0_NUM_A_BYTES |
			QSPI_SETUP0_NUM_D_BYTES_NO_BITS |
			QSPI_SETUP0_READ_NORMAL | QSPI_CMD_WRITE |
			QSPI_NUM_DUMMY_BITS;
#endif

	writel(memval, &priv->base->setup0);
}

static void ti_spi_set_speed(struct spi_slave *slave, uint hz)
{
	struct ti_qspi_priv *priv = to_ti_qspi_priv(slave);
	uint clk_div;

	debug("ti_spi_set_speed: hz: %d, clock divider %d\n", hz, clk_div);

	if (!hz)
		clk_div = 0;
	else
		clk_div = (QSPI_FCLK / hz) - 1;

	/* disable SCLK */
	writel(readl(&priv->base->clk_ctrl) & ~QSPI_CLK_EN,
	       &priv->base->clk_ctrl);

	/* assign clk_div values */
	if (clk_div < 0)
		clk_div = 0;
	else if (clk_div > QSPI_CLK_DIV_MAX)
		clk_div = QSPI_CLK_DIV_MAX;

	/* enable SCLK */
	writel(QSPI_CLK_EN | clk_div, &priv->base->clk_ctrl);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
	/* CS handled in xfer */
	return;
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct ti_qspi_priv *priv = to_ti_qspi_priv(slave);

	debug("spi_cs_deactivate: 0x%08x\n", (u32)slave);

	writel(priv->cmd | QSPI_INVAL, &priv->base->cmd);
	/* dummy readl to ensure bus sync */
	readl(&qslave->base->cmd);
}

void spi_init(void)
{
	/* nothing to do */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct ti_qspi_priv *priv;

#ifdef CONFIG_AM43XX
	gpio_request(CONFIG_QSPI_SEL_GPIO, "qspi_gpio");
	gpio_direction_output(CONFIG_QSPI_SEL_GPIO, 1);
#endif

	priv = spi_alloc_slave(struct ti_qspi_priv, bus, cs);
	if (!priv) {
		printf("SPI_error: Fail to allocate ti_qspi_priv\n");
		return NULL;
	}

	priv->base = (struct ti_qspi_regs *)QSPI_BASE;
	priv->mode = mode;

	ti_spi_set_speed(&priv->slave, max_hz);

#ifdef CONFIG_TI_SPI_MMAP
	ti_spi_setup_spi_register(priv);
#endif

	return &priv->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct ti_qspi_priv *priv = to_ti_qspi_priv(slave);
	free(priv);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct ti_qspi_priv *priv = to_ti_qspi_priv(slave);

	debug("spi_claim_bus: bus:%i cs:%i\n", slave->bus, slave->cs);

	priv->dc = 0;
	if (priv->mode & SPI_CPHA)
		priv->dc |= QSPI_CKPHA(slave->cs);
	if (priv->mode & SPI_CPOL)
		priv->dc |= QSPI_CKPOL(slave->cs);
	if (priv->mode & SPI_CS_HIGH)
		priv->dc |= QSPI_CSPOL(slave->cs);

	writel(priv->dc, &priv->base->dc);
	writel(0, &priv->base->cmd);
	writel(0, &priv->base->data);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct ti_qspi_priv *priv = to_ti_qspi_priv(slave);

	debug("spi_release_bus: bus:%i cs:%i\n", slave->bus, slave->cs);

	writel(0, &priv->base->dc);
	writel(0, &priv->base->cmd);
	writel(0, &priv->base->data);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct ti_qspi_priv *priv = to_ti_qspi_priv(slave);
	uint words = bitlen >> 3; /* fixed 8-bit word length */
	const uchar *txp = dout;
	uchar *rxp = din;
	uint status;
	int timeout;

#if defined(CONFIG_DRA7XX) || defined(CONFIG_AM57XX)
	int val;
#endif

	debug("spi_xfer: bus:%i cs:%i bitlen:%i words:%i flags:%lx\n",
	      slave->bus, slave->cs, bitlen, words, flags);

	/* Setup mmap flags */
	if (flags & SPI_XFER_MMAP) {
		writel(MM_SWITCH, &priv->base->memswitch);
#if defined(CONFIG_DRA7XX) || defined(CONFIG_AM57XX)
		val = readl(CORE_CTRL_IO);
		val |= MEM_CS(slave->cs);
		writel(val, CORE_CTRL_IO);
#endif
		return 0;
	} else if (flags & SPI_XFER_MMAP_END) {
		writel(~MM_SWITCH, &priv->base->memswitch);
#if defined(CONFIG_DRA7XX) || defined(CONFIG_AM57XX)
		val = readl(CORE_CTRL_IO);
		val &= MEM_CS_UNSELECT;
		writel(val, CORE_CTRL_IO);
#endif
		return 0;
	}

	if (bitlen == 0)
		return -1;

	if (bitlen % 8) {
		debug("spi_xfer: Non byte aligned SPI transfer\n");
		return -1;
	}

	/* Setup command reg */
	priv->cmd = 0;
	priv->cmd |= QSPI_WLEN(8);
	priv->cmd |= QSPI_EN_CS(slave->cs);
	if (priv->mode & SPI_3WIRE)
		priv->cmd |= QSPI_3_PIN;
	priv->cmd |= 0xfff;

/* FIXME: This delay is required for successfull
 * completion of read/write/erase. Once its root
 * caused, it will be remove from the driver.
 */
#ifdef CONFIG_AM43XX
	udelay(100);
#endif
	while (words--) {
		if (txp) {
			debug("tx cmd %08x dc %08x data %02x\n",
			      priv->cmd | QSPI_WR_SNGL, priv->dc, *txp);
			writel(*txp++, &priv->base->data);
			writel(priv->cmd | QSPI_WR_SNGL,
			       &priv->base->cmd);
			status = readl(&priv->base->status);
			timeout = QSPI_TIMEOUT;
			while ((status & QSPI_WC_BUSY) != QSPI_XFER_DONE) {
				if (--timeout < 0) {
					printf("spi_xfer: TX timeout!\n");
					return -1;
				}
				status = readl(&priv->base->status);
			}
			debug("tx done, status %08x\n", status);
		}
		if (rxp) {
			priv->cmd |= QSPI_RD_SNGL;
			debug("rx cmd %08x dc %08x\n",
			      priv->cmd, priv->dc);
			#ifdef CONFIG_DRA7XX
				udelay(500);
			#endif
			writel(priv->cmd, &priv->base->cmd);
			status = readl(&priv->base->status);
			timeout = QSPI_TIMEOUT;
			while ((status & QSPI_WC_BUSY) != QSPI_XFER_DONE) {
				if (--timeout < 0) {
					printf("spi_xfer: RX timeout!\n");
					return -1;
				}
				status = readl(&priv->base->status);
			}
			*rxp++ = readl(&priv->base->data);
			debug("rx done, status %08x, read %02x\n",
			      status, *(rxp-1));
		}
	}

	/* Terminate frame */
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}

/* TODO: control from sf layer to here through dm-spi */
#ifdef CONFIG_TI_EDMA3
void spi_flash_copy_mmap(void *data, void *offset, size_t len)
{
	unsigned int			addr = (unsigned int) (data);
	unsigned int			edma_slot_num = 1;

	/* Invalidate the area, so no writeback into the RAM races with DMA */
	invalidate_dcache_range(addr, addr + roundup(len, ARCH_DMA_MINALIGN));

	/* enable edma3 clocks */
	enable_edma3_clocks();

	/* Call edma3 api to do actual DMA transfer	*/
	edma3_transfer(EDMA3_BASE, edma_slot_num, data, offset, len);

	/* disable edma3 clocks */
	disable_edma3_clocks();

	*((unsigned int *)offset) += len;
}
#endif
