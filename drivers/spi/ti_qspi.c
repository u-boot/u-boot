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
#include <dm.h>
#include <asm/gpio.h>
#include <asm/omap_gpio.h>
#include <asm/omap_common.h>
#include <asm/ti-common/ti-edma3.h>
#include <linux/kernel.h>
#include <regmap.h>
#include <syscon.h>

DECLARE_GLOBAL_DATA_PTR;

/* ti qpsi register bit masks */
#define QSPI_TIMEOUT                    2000000
#define QSPI_FCLK			192000000
#define QSPI_DRA7XX_FCLK                76800000
#define QSPI_WLEN_MAX_BITS		128
#define QSPI_WLEN_MAX_BYTES		(QSPI_WLEN_MAX_BITS >> 3)
#define QSPI_WLEN_MASK			QSPI_WLEN(QSPI_WLEN_MAX_BITS)
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
#define MEM_CS_UNSELECT                 0xfffff8ff
#define MMAP_START_ADDR_DRA		0x5c000000
#define MMAP_START_ADDR_AM43x		0x30000000
#define CORE_CTRL_IO                    0x4a002558

#define QSPI_CMD_READ                   (0x3 << 0)
#define QSPI_CMD_READ_DUAL		(0x6b << 0)
#define QSPI_CMD_READ_QUAD              (0x6c << 0)
#define QSPI_CMD_READ_FAST              (0x0b << 0)
#define QSPI_SETUP0_NUM_A_BYTES         (0x3 << 8)
#define QSPI_SETUP0_NUM_D_BYTES_NO_BITS (0x0 << 10)
#define QSPI_SETUP0_NUM_D_BYTES_8_BITS  (0x1 << 10)
#define QSPI_SETUP0_READ_NORMAL         (0x0 << 12)
#define QSPI_SETUP0_READ_DUAL           (0x1 << 12)
#define QSPI_SETUP0_READ_QUAD           (0x3 << 12)
#define QSPI_CMD_WRITE                  (0x12 << 16)
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
#ifndef CONFIG_DM_SPI
	struct spi_slave slave;
#else
	void *memory_map;
	uint max_hz;
	u32 num_cs;
#endif
	struct ti_qspi_regs *base;
	void *ctrl_mod_mmap;
	ulong fclk;
	unsigned int mode;
	u32 cmd;
	u32 dc;
};

static void ti_spi_set_speed(struct ti_qspi_priv *priv, uint hz)
{
	uint clk_div;

	if (!hz)
		clk_div = 0;
	else
		clk_div = DIV_ROUND_UP(priv->fclk, hz) - 1;

	/* truncate clk_div value to QSPI_CLK_DIV_MAX */
	if (clk_div > QSPI_CLK_DIV_MAX)
		clk_div = QSPI_CLK_DIV_MAX;

	debug("ti_spi_set_speed: hz: %d, clock divider %d\n", hz, clk_div);

	/* disable SCLK */
	writel(readl(&priv->base->clk_ctrl) & ~QSPI_CLK_EN,
	       &priv->base->clk_ctrl);
	/* enable SCLK and program the clk divider */
	writel(QSPI_CLK_EN | clk_div, &priv->base->clk_ctrl);
}

static void ti_qspi_cs_deactivate(struct ti_qspi_priv *priv)
{
	writel(priv->cmd | QSPI_INVAL, &priv->base->cmd);
	/* dummy readl to ensure bus sync */
	readl(&priv->base->cmd);
}

static int __ti_qspi_set_mode(struct ti_qspi_priv *priv, unsigned int mode)
{
	priv->dc = 0;
	if (mode & SPI_CPHA)
		priv->dc |= QSPI_CKPHA(0);
	if (mode & SPI_CPOL)
		priv->dc |= QSPI_CKPOL(0);
	if (mode & SPI_CS_HIGH)
		priv->dc |= QSPI_CSPOL(0);

	return 0;
}

static int __ti_qspi_claim_bus(struct ti_qspi_priv *priv, int cs)
{
	writel(priv->dc, &priv->base->dc);
	writel(0, &priv->base->cmd);
	writel(0, &priv->base->data);

	priv->dc <<= cs * 8;
	writel(priv->dc, &priv->base->dc);

	return 0;
}

static void __ti_qspi_release_bus(struct ti_qspi_priv *priv)
{
	writel(0, &priv->base->dc);
	writel(0, &priv->base->cmd);
	writel(0, &priv->base->data);
}

static void ti_qspi_ctrl_mode_mmap(void *ctrl_mod_mmap, int cs, bool enable)
{
	u32 val;

	val = readl(ctrl_mod_mmap);
	if (enable)
		val |= MEM_CS(cs);
	else
		val &= MEM_CS_UNSELECT;
	writel(val, ctrl_mod_mmap);
}

static int __ti_qspi_xfer(struct ti_qspi_priv *priv, unsigned int bitlen,
			const void *dout, void *din, unsigned long flags,
			u32 cs)
{
	uint words = bitlen >> 3; /* fixed 8-bit word length */
	const uchar *txp = dout;
	uchar *rxp = din;
	uint status;
	int timeout;

	/* Setup mmap flags */
	if (flags & SPI_XFER_MMAP) {
		writel(MM_SWITCH, &priv->base->memswitch);
		if (priv->ctrl_mod_mmap)
			ti_qspi_ctrl_mode_mmap(priv->ctrl_mod_mmap, cs, true);
		return 0;
	} else if (flags & SPI_XFER_MMAP_END) {
		writel(~MM_SWITCH, &priv->base->memswitch);
		if (priv->ctrl_mod_mmap)
			ti_qspi_ctrl_mode_mmap(priv->ctrl_mod_mmap, cs, false);
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
	priv->cmd |= QSPI_EN_CS(cs);
	if (priv->mode & SPI_3WIRE)
		priv->cmd |= QSPI_3_PIN;
	priv->cmd |= 0xfff;

	while (words) {
		u8 xfer_len = 0;

		if (txp) {
			u32 cmd = priv->cmd;

			if (words >= QSPI_WLEN_MAX_BYTES) {
				u32 *txbuf = (u32 *)txp;
				u32 data;

				data = cpu_to_be32(*txbuf++);
				writel(data, &priv->base->data3);
				data = cpu_to_be32(*txbuf++);
				writel(data, &priv->base->data2);
				data = cpu_to_be32(*txbuf++);
				writel(data, &priv->base->data1);
				data = cpu_to_be32(*txbuf++);
				writel(data, &priv->base->data);
				cmd &= ~QSPI_WLEN_MASK;
				cmd |= QSPI_WLEN(QSPI_WLEN_MAX_BITS);
				xfer_len = QSPI_WLEN_MAX_BYTES;
			} else {
				writeb(*txp, &priv->base->data);
				xfer_len = 1;
			}
			debug("tx cmd %08x dc %08x\n",
			      cmd | QSPI_WR_SNGL, priv->dc);
			writel(cmd | QSPI_WR_SNGL, &priv->base->cmd);
			status = readl(&priv->base->status);
			timeout = QSPI_TIMEOUT;
			while ((status & QSPI_WC_BUSY) != QSPI_XFER_DONE) {
				if (--timeout < 0) {
					printf("spi_xfer: TX timeout!\n");
					return -1;
				}
				status = readl(&priv->base->status);
			}
			txp += xfer_len;
			debug("tx done, status %08x\n", status);
		}
		if (rxp) {
			debug("rx cmd %08x dc %08x\n",
			      ((u32)(priv->cmd | QSPI_RD_SNGL)), priv->dc);
			writel(priv->cmd | QSPI_RD_SNGL, &priv->base->cmd);
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
			xfer_len = 1;
			debug("rx done, status %08x, read %02x\n",
			      status, *(rxp-1));
		}
		words -= xfer_len;
	}

	/* Terminate frame */
	if (flags & SPI_XFER_END)
		ti_qspi_cs_deactivate(priv);

	return 0;
}

/* TODO: control from sf layer to here through dm-spi */
#if defined(CONFIG_TI_EDMA3) && !defined(CONFIG_DMA)
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

#ifndef CONFIG_DM_SPI

static inline struct ti_qspi_priv *to_ti_qspi_priv(struct spi_slave *slave)
{
	return container_of(slave, struct ti_qspi_priv, slave);
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
	ti_qspi_cs_deactivate(priv);
}

void spi_init(void)
{
	/* nothing to do */
}

static void ti_spi_setup_spi_register(struct ti_qspi_priv *priv)
{
	u32 memval = 0;

#ifdef CONFIG_QSPI_QUAD_SUPPORT
	struct spi_slave *slave = &priv->slave;
	memval |= (QSPI_CMD_READ_QUAD | QSPI_SETUP0_NUM_A_BYTES |
			QSPI_SETUP0_NUM_D_BYTES_8_BITS |
			QSPI_SETUP0_READ_QUAD | QSPI_CMD_WRITE |
			QSPI_NUM_DUMMY_BITS);
	slave->mode |= SPI_RX_QUAD;
#else
	memval |= QSPI_CMD_READ | QSPI_SETUP0_NUM_A_BYTES |
			QSPI_SETUP0_NUM_D_BYTES_NO_BITS |
			QSPI_SETUP0_READ_NORMAL | QSPI_CMD_WRITE |
			QSPI_NUM_DUMMY_BITS;
#endif

	writel(memval, &priv->base->setup0);
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
#if defined(CONFIG_DRA7XX)
	priv->ctrl_mod_mmap = (void *)CORE_CTRL_IO;
	priv->slave.memory_map = (void *)MMAP_START_ADDR_DRA;
	priv->fclk = QSPI_DRA7XX_FCLK;
#else
	priv->slave.memory_map = (void *)MMAP_START_ADDR_AM43x;
	priv->fclk = QSPI_FCLK;
#endif

	ti_spi_set_speed(priv, max_hz);

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

	debug("%s: bus:%i cs:%i\n", __func__, priv->slave.bus, priv->slave.cs);
	__ti_qspi_set_mode(priv, priv->mode);
	return __ti_qspi_claim_bus(priv, priv->slave.cs);
}
void spi_release_bus(struct spi_slave *slave)
{
	struct ti_qspi_priv *priv = to_ti_qspi_priv(slave);

	debug("%s: bus:%i cs:%i\n", __func__, priv->slave.bus, priv->slave.cs);
	__ti_qspi_release_bus(priv);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct ti_qspi_priv *priv = to_ti_qspi_priv(slave);

	debug("spi_xfer: bus:%i cs:%i bitlen:%i flags:%lx\n",
	      priv->slave.bus, priv->slave.cs, bitlen, flags);
	return __ti_qspi_xfer(priv, bitlen, dout, din, flags, priv->slave.cs);
}

#else /* CONFIG_DM_SPI */

static void __ti_qspi_setup_memorymap(struct ti_qspi_priv *priv,
				      struct spi_slave *slave,
				      bool enable)
{
	u32 memval;
	u32 mode = slave->mode & (SPI_RX_QUAD | SPI_RX_DUAL);

	if (!enable) {
		writel(0, &priv->base->setup0);
		return;
	}

	memval = QSPI_SETUP0_NUM_A_BYTES | QSPI_CMD_WRITE | QSPI_NUM_DUMMY_BITS;

	switch (mode) {
	case SPI_RX_QUAD:
		memval |= QSPI_CMD_READ_QUAD;
		memval |= QSPI_SETUP0_NUM_D_BYTES_8_BITS;
		memval |= QSPI_SETUP0_READ_QUAD;
		slave->mode |= SPI_RX_QUAD;
		break;
	case SPI_RX_DUAL:
		memval |= QSPI_CMD_READ_DUAL;
		memval |= QSPI_SETUP0_NUM_D_BYTES_8_BITS;
		memval |= QSPI_SETUP0_READ_DUAL;
		break;
	default:
		memval |= QSPI_CMD_READ;
		memval |= QSPI_SETUP0_NUM_D_BYTES_NO_BITS;
		memval |= QSPI_SETUP0_READ_NORMAL;
		break;
	}

	writel(memval, &priv->base->setup0);
}


static int ti_qspi_set_speed(struct udevice *bus, uint max_hz)
{
	struct ti_qspi_priv *priv = dev_get_priv(bus);

	ti_spi_set_speed(priv, max_hz);

	return 0;
}

static int ti_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct ti_qspi_priv *priv = dev_get_priv(bus);
	return __ti_qspi_set_mode(priv, mode);
}

static int ti_qspi_claim_bus(struct udevice *dev)
{
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct ti_qspi_priv *priv;
	struct udevice *bus;

	bus = dev->parent;
	priv = dev_get_priv(bus);

	if (slave_plat->cs > priv->num_cs) {
		debug("invalid qspi chip select\n");
		return -EINVAL;
	}

	__ti_qspi_setup_memorymap(priv, slave, true);

	return __ti_qspi_claim_bus(priv, slave_plat->cs);
}

static int ti_qspi_release_bus(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct ti_qspi_priv *priv;
	struct udevice *bus;

	bus = dev->parent;
	priv = dev_get_priv(bus);

	__ti_qspi_setup_memorymap(priv, slave, false);
	__ti_qspi_release_bus(priv);

	return 0;
}

static int ti_qspi_xfer(struct udevice *dev, unsigned int bitlen,
			const void *dout, void *din, unsigned long flags)
{
	struct dm_spi_slave_platdata *slave = dev_get_parent_platdata(dev);
	struct ti_qspi_priv *priv;
	struct udevice *bus;

	bus = dev->parent;
	priv = dev_get_priv(bus);

	if (slave->cs > priv->num_cs) {
		debug("invalid qspi chip select\n");
		return -EINVAL;
	}

	return __ti_qspi_xfer(priv, bitlen, dout, din, flags, slave->cs);
}

static int ti_qspi_probe(struct udevice *bus)
{
	struct ti_qspi_priv *priv = dev_get_priv(bus);

	priv->fclk = dev_get_driver_data(bus);

	return 0;
}

static void *map_syscon_chipselects(struct udevice *bus)
{
#if CONFIG_IS_ENABLED(SYSCON)
	struct udevice *syscon;
	struct regmap *regmap;
	const fdt32_t *cell;
	int len, err;

	err = uclass_get_device_by_phandle(UCLASS_SYSCON, bus,
					   "syscon-chipselects", &syscon);
	if (err) {
		debug("%s: unable to find syscon device (%d)\n", __func__,
		      err);
		return NULL;
	}

	regmap = syscon_get_regmap(syscon);
	if (IS_ERR(regmap)) {
		debug("%s: unable to find regmap (%ld)\n", __func__,
		      PTR_ERR(regmap));
		return NULL;
	}

	cell = fdt_getprop(gd->fdt_blob, bus->of_offset, "syscon-chipselects",
			   &len);
	if (len < 2*sizeof(fdt32_t)) {
		debug("%s: offset not available\n", __func__);
		return NULL;
	}

	return fdtdec_get_number(cell + 1, 1) + regmap_get_range(regmap, 0);
#else
	fdt_addr_t addr;
	addr = dev_get_addr_index(bus, 2);
	return (addr == FDT_ADDR_T_NONE) ? NULL :
		map_physmem(addr, 0, MAP_NOCACHE);
#endif
}

static int ti_qspi_ofdata_to_platdata(struct udevice *bus)
{
	struct ti_qspi_priv *priv = dev_get_priv(bus);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);

	priv->ctrl_mod_mmap = map_syscon_chipselects(bus);
	priv->base = map_physmem(dev_get_addr(bus), sizeof(struct ti_qspi_regs),
				 MAP_NOCACHE);
	priv->memory_map = map_physmem(dev_get_addr_index(bus, 1), 0,
				       MAP_NOCACHE);

	priv->max_hz = fdtdec_get_int(blob, node, "spi-max-frequency", -1);
	if (priv->max_hz < 0) {
		debug("Error: Max frequency missing\n");
		return -ENODEV;
	}
	priv->num_cs = fdtdec_get_int(blob, node, "num-cs", 4);

	debug("%s: regs=<0x%x>, max-frequency=%d\n", __func__,
	      (int)priv->base, priv->max_hz);

	return 0;
}

static int ti_qspi_child_pre_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct ti_qspi_priv *priv = dev_get_priv(bus);

	slave->memory_map = priv->memory_map;
	return 0;
}

static const struct dm_spi_ops ti_qspi_ops = {
	.claim_bus	= ti_qspi_claim_bus,
	.release_bus	= ti_qspi_release_bus,
	.xfer		= ti_qspi_xfer,
	.set_speed	= ti_qspi_set_speed,
	.set_mode	= ti_qspi_set_mode,
};

static const struct udevice_id ti_qspi_ids[] = {
	{ .compatible = "ti,dra7xxx-qspi",	.data = QSPI_DRA7XX_FCLK},
	{ .compatible = "ti,am4372-qspi",	.data = QSPI_FCLK},
	{ }
};

U_BOOT_DRIVER(ti_qspi) = {
	.name	= "ti_qspi",
	.id	= UCLASS_SPI,
	.of_match = ti_qspi_ids,
	.ops	= &ti_qspi_ops,
	.ofdata_to_platdata = ti_qspi_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct ti_qspi_priv),
	.probe	= ti_qspi_probe,
	.child_pre_probe = ti_qspi_child_pre_probe,
};
#endif /* CONFIG_DM_SPI */
