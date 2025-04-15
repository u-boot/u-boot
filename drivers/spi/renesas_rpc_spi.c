// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas R-Car Gen3 RPC QSPI driver
 *
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/of_access.h>
#include <dt-structs.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/errno.h>
#include <spi.h>
#include <spi-mem.h>
#include <wait_bit.h>

#define RPC_CMNCR		0x0000	/* R/W */
#define RPC_CMNCR_MD		BIT(31)
#define RPC_CMNCR_SFDE		BIT(24)
#define RPC_CMNCR_MOIIO3(val)	(((val) & 0x3) << 22)
#define RPC_CMNCR_MOIIO2(val)	(((val) & 0x3) << 20)
#define RPC_CMNCR_MOIIO1(val)	(((val) & 0x3) << 18)
#define RPC_CMNCR_MOIIO0(val)	(((val) & 0x3) << 16)
#define RPC_CMNCR_MOIIO_HIZ	(RPC_CMNCR_MOIIO0(3) | RPC_CMNCR_MOIIO1(3) | \
				 RPC_CMNCR_MOIIO2(3) | RPC_CMNCR_MOIIO3(3))
#define RPC_CMNCR_IO3FV(val)	(((val) & 0x3) << 14)
#define RPC_CMNCR_IO2FV(val)	(((val) & 0x3) << 12)
#define RPC_CMNCR_IO0FV(val)	(((val) & 0x3) << 8)
#define RPC_CMNCR_IOFV_HIZ	(RPC_CMNCR_IO0FV(3) | RPC_CMNCR_IO2FV(3) | \
				 RPC_CMNCR_IO3FV(3))
#define RPC_CMNCR_CPHAT		BIT(6)
#define RPC_CMNCR_CPHAR		BIT(5)
#define RPC_CMNCR_SSLP		BIT(4)
#define RPC_CMNCR_CPOL		BIT(3)
#define RPC_CMNCR_BSZ(val)	(((val) & 0x3) << 0)

#define RPC_SSLDR		0x0004	/* R/W */
#define RPC_SSLDR_SPNDL(d)	(((d) & 0x7) << 16)
#define RPC_SSLDR_SLNDL(d)	(((d) & 0x7) << 8)
#define RPC_SSLDR_SCKDL(d)	(((d) & 0x7) << 0)

#define RPC_DRCR		0x000C	/* R/W */
#define RPC_DRCR_SSLN		BIT(24)
#define RPC_DRCR_RBURST(v)	(((v) & 0x1F) << 16)
#define RPC_DRCR_RCF		BIT(9)
#define RPC_DRCR_RBE		BIT(8)
#define RPC_DRCR_SSLE		BIT(0)

#define RPC_DRCMR		0x0010	/* R/W */
#define RPC_DRCMR_CMD(c)	(((c) & 0xFF) << 16)
#define RPC_DRCMR_OCMD(c)	(((c) & 0xFF) << 0)

#define RPC_DREAR		0x0014	/* R/W */
#define RPC_DREAR_EAV(v)	(((v) & 0xFF) << 16)
#define RPC_DREAR_EAC(v)	(((v) & 0x7) << 0)

#define RPC_DROPR		0x0018	/* R/W */
#define RPC_DROPR_OPD3(o)	(((o) & 0xFF) << 24)
#define RPC_DROPR_OPD2(o)	(((o) & 0xFF) << 16)
#define RPC_DROPR_OPD1(o)	(((o) & 0xFF) << 8)
#define RPC_DROPR_OPD0(o)	(((o) & 0xFF) << 0)

#define RPC_DRENR		0x001C	/* R/W */
#define RPC_DRENR_CDB(o)	(u32)((((o) & 0x3) << 30))
#define RPC_DRENR_OCDB(o)	(((o) & 0x3) << 28)
#define RPC_DRENR_ADB(o)	(((o) & 0x3) << 24)
#define RPC_DRENR_OPDB(o)	(((o) & 0x3) << 20)
#define RPC_DRENR_SPIDB(o)	(((o) & 0x3) << 16)
#define RPC_DRENR_DME		BIT(15)
#define RPC_DRENR_CDE		BIT(14)
#define RPC_DRENR_OCDE		BIT(12)
#define RPC_DRENR_ADE(v)	(((v) & 0xF) << 8)
#define RPC_DRENR_OPDE(v)	(((v) & 0xF) << 4)

#define RPC_SMCR		0x0020	/* R/W */
#define RPC_SMCR_SSLKP		BIT(8)
#define RPC_SMCR_SPIRE		BIT(2)
#define RPC_SMCR_SPIWE		BIT(1)
#define RPC_SMCR_SPIE		BIT(0)

#define RPC_SMCMR		0x0024	/* R/W */
#define RPC_SMCMR_CMD(c)	(((c) & 0xFF) << 16)
#define RPC_SMCMR_OCMD(c)	(((c) & 0xFF) << 0)

#define RPC_SMADR		0x0028	/* R/W */
#define RPC_SMOPR		0x002C	/* R/W */
#define RPC_SMOPR_OPD0(o)	(((o) & 0xFF) << 0)
#define RPC_SMOPR_OPD1(o)	(((o) & 0xFF) << 8)
#define RPC_SMOPR_OPD2(o)	(((o) & 0xFF) << 16)
#define RPC_SMOPR_OPD3(o)	(((o) & 0xFF) << 24)

#define RPC_SMENR		0x0030	/* R/W */
#define RPC_SMENR_CDB(o)	(((o) & 0x3) << 30)
#define RPC_SMENR_OCDB(o)	(((o) & 0x3) << 28)
#define RPC_SMENR_ADB(o)	(((o) & 0x3) << 24)
#define RPC_SMENR_OPDB(o)	(((o) & 0x3) << 20)
#define RPC_SMENR_SPIDB(o)	(((o) & 0x3) << 16)
#define RPC_SMENR_DME		BIT(15)
#define RPC_SMENR_CDE		BIT(14)
#define RPC_SMENR_OCDE		BIT(12)
#define RPC_SMENR_ADE(v)	(((v) & 0xF) << 8)
#define RPC_SMENR_OPDE(v)	(((v) & 0xF) << 4)
#define RPC_SMENR_SPIDE(v)	(((v) & 0xF) << 0)

#define RPC_SMRDR0		0x0038	/* R */
#define RPC_SMRDR1		0x003C	/* R */
#define RPC_SMWDR0		0x0040	/* R/W */
#define RPC_SMWDR1		0x0044	/* R/W */
#define RPC_CMNSR		0x0048	/* R */
#define RPC_CMNSR_SSLF		BIT(1)
#define	RPC_CMNSR_TEND		BIT(0)

#define RPC_DRDMCR		0x0058	/* R/W */
#define RPC_DRDMCR_DMCYC(v)	(((v) & 0xF) << 0)

#define RPC_DRDRENR		0x005C	/* R/W */
#define RPC_DRDRENR_HYPE	(0x5 << 12)
#define RPC_DRDRENR_ADDRE	BIT(8)
#define RPC_DRDRENR_OPDRE	BIT(4)
#define RPC_DRDRENR_DRDRE	BIT(0)

#define RPC_SMDMCR		0x0060	/* R/W */
#define RPC_SMDMCR_DMCYC(v)	(((v) & 0xF) << 0)

#define RPC_SMDRENR		0x0064	/* R/W */
#define RPC_SMDRENR_HYPE	(0x5 << 12)
#define RPC_SMDRENR_ADDRE	BIT(8)
#define RPC_SMDRENR_OPDRE	BIT(4)
#define RPC_SMDRENR_SPIDRE	BIT(0)

#define RPC_PHYCNT		0x007C	/* R/W */
#define RPC_PHYCNT_CAL		BIT(31)
#define PRC_PHYCNT_OCTA_AA	BIT(22)
#define PRC_PHYCNT_OCTA_SA	BIT(23)
#define PRC_PHYCNT_EXDS		BIT(21)
#define RPC_PHYCNT_OCT		BIT(20)
#define RPC_PHYCNT_STRTIM(v)	(((v) & 0x7) << 15)
#define RPC_PHYCNT_STRTIM2(v)	((((v) & 0x7) << 15) | (((v) & 0x8) << 24))
#define RPC_PHYCNT_WBUF2	BIT(4)
#define RPC_PHYCNT_WBUF		BIT(2)
#define RPC_PHYCNT_MEM(v)	(((v) & 0x3) << 0)

#define RPCIF_PHYOFFSET1	0x0080	/* R/W */
#define RPCIF_PHYOFFSET1_DDRTMG(v) (((v) & 0x3) << 28)

#define RPCIF_PHYOFFSET2	0x0084	/* R/W */
#define RPCIF_PHYOFFSET2_OCTTMG(v) (((v) & 0x7) << 8)

#define RPC_PHYINT		0x0088	/* R/W */
#define RPC_PHYINT_RSTEN	BIT(18)
#define RPC_PHYINT_WPEN		BIT(17)
#define RPC_PHYINT_INTEN	BIT(16)
#define RPC_PHYINT_RST		BIT(2)
#define RPC_PHYINT_WP		BIT(1)
#define RPC_PHYINT_INT		BIT(0)

#define RPC_WBUF		0x8000	/* R/W size=4/8/16/32/64Bytes */
#define RPC_WBUF_SIZE		0x100

DECLARE_GLOBAL_DATA_PTR;

struct rpc_spi_plat {
	fdt_addr_t	regs;
	fdt_addr_t	extr;
	s32		freq;	/* Default clock freq, -1 for none */
};

struct rpc_spi_priv {
	fdt_addr_t	regs;
	fdt_addr_t	extr;
	struct clk	clk;
};

static int rpc_spi_wait_sslf(struct udevice *dev)
{
	struct rpc_spi_priv *priv = dev_get_priv(dev->parent);

	return wait_for_bit_le32((void *)priv->regs + RPC_CMNSR, RPC_CMNSR_SSLF,
				 false, 1000, false);
}

static int rpc_spi_wait_tend(struct udevice *dev)
{
	struct rpc_spi_priv *priv = dev_get_priv(dev->parent);

	return wait_for_bit_le32((void *)priv->regs + RPC_CMNSR, RPC_CMNSR_TEND,
				 true, 1000, false);
}

static void rpc_spi_flush_read_cache(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct rpc_spi_priv *priv = dev_get_priv(bus);

	/* Flush read cache */
	writel(RPC_DRCR_SSLN | RPC_DRCR_RBURST(0x1f) |
	       RPC_DRCR_RCF | RPC_DRCR_RBE | RPC_DRCR_SSLE,
	       priv->regs + RPC_DRCR);
	readl(priv->regs + RPC_DRCR);

}

static u32 rpc_spi_get_strobe_delay(void)
{
#ifndef CONFIG_RZA1
	u32 cpu_type = renesas_get_cpu_type();

	/*
	 * NOTE: RPC_PHYCNT_STRTIM value:
	 *       0: On H3 ES1.x (not supported in mainline U-Boot)
	 *       6: On M3 ES1.x
	 *       7: On other R-Car Gen3
	 *      15: On R-Car Gen4
	 */
	if (cpu_type == RENESAS_CPU_TYPE_R8A7796 && renesas_get_cpu_rev_integer() == 1)
		return RPC_PHYCNT_STRTIM(6);
	else if (cpu_type == RENESAS_CPU_TYPE_R8A779F0 ||
		 cpu_type == RENESAS_CPU_TYPE_R8A779G0 ||
		 cpu_type == RENESAS_CPU_TYPE_R8A779H0)
		return RPC_PHYCNT_STRTIM2(15);
	else
#endif
		return RPC_PHYCNT_STRTIM(7);
}

static int rpc_spi_claim_bus(struct udevice *dev, bool manual)
{
	struct udevice *bus = dev->parent;
	struct rpc_spi_priv *priv = dev_get_priv(bus);

	setbits_le32(priv->regs + RPCIF_PHYOFFSET1,
		     RPCIF_PHYOFFSET1_DDRTMG(3));
	clrsetbits_le32(priv->regs + RPCIF_PHYOFFSET2,
			RPCIF_PHYOFFSET2_OCTTMG(7),
			RPCIF_PHYOFFSET2_OCTTMG(4));

	/* NOTE: The 0x260 are undocumented bits, but they must be set. */
	writel(RPC_PHYCNT_CAL | rpc_spi_get_strobe_delay() | 0x260,
	       priv->regs + RPC_PHYCNT);
	writel((manual ? RPC_CMNCR_MD : 0) | RPC_CMNCR_SFDE |
		 RPC_CMNCR_MOIIO_HIZ | RPC_CMNCR_IOFV_HIZ | RPC_CMNCR_BSZ(0),
		 priv->regs + RPC_CMNCR);

	writel(RPC_SSLDR_SPNDL(7) | RPC_SSLDR_SLNDL(7) |
	       RPC_SSLDR_SCKDL(7), priv->regs + RPC_SSLDR);

	rpc_spi_flush_read_cache(dev);

	return 0;
}

static int rpc_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct rpc_spi_priv *priv = dev_get_priv(bus);

	/* NOTE: The 0x260 are undocumented bits, but they must be set. */
	writel(rpc_spi_get_strobe_delay() | 0x260, priv->regs + RPC_PHYCNT);

	rpc_spi_flush_read_cache(dev);

	return 0;
}

static int rpc_spi_mem_exec_op(struct spi_slave *spi,
			       const struct spi_mem_op *op)
{
	struct udevice *bus = spi->dev->parent;
	struct rpc_spi_priv *priv = dev_get_priv(bus);
	const void *dout = op->data.buf.out ? op->data.buf.out : NULL;
	void *din = op->data.buf.in ? op->data.buf.in : NULL;
	int ret = 0;
	u32 offset = 0;
	u32 smenr, smcr;

	smenr = 0;
	offset = op->addr.val;

	switch (op->data.dir) {
	case SPI_MEM_DATA_IN:
		rpc_spi_claim_bus(spi->dev, false);

		writel(0, priv->regs + RPC_DRCMR);
		writel(RPC_DRCMR_CMD(op->cmd.opcode), priv->regs + RPC_DRCMR);
		smenr |= RPC_DRENR_CDE;

		if (op->addr.nbytes == 4) {
			writel(RPC_DREAR_EAV(offset >> 25) | RPC_DREAR_EAC(1),
			       priv->regs + RPC_DREAR);
			smenr |= RPC_DRENR_ADE(0xF);
		} else if (op->addr.nbytes == 3) {
			writel(0, priv->regs + RPC_DREAR);
			smenr |= RPC_DRENR_ADE(0x7);
		} else {
			writel(0, priv->regs + RPC_DREAR);
			smenr |= RPC_DRENR_ADE(0);
		}

		if (op->dummy.nbytes)
			smenr |= RPC_DRENR_DME;

		writel(8 * op->dummy.nbytes - 1, priv->regs + RPC_DRDMCR);
		writel(0, priv->regs + RPC_DROPR);
		writel(0, priv->regs + RPC_DRDRENR);
		writel(smenr, priv->regs + RPC_DRENR);

		memcpy_fromio(din, (void *)(priv->extr + offset), op->data.nbytes);

		rpc_spi_release_bus(spi->dev);
		break;
	case SPI_MEM_DATA_OUT:
	case SPI_MEM_NO_DATA:
		rpc_spi_claim_bus(spi->dev, true);

		writel(0, priv->regs + RPC_SMCR);
		writel(0, priv->regs + RPC_SMCMR);
		writel(RPC_SMCMR_CMD(op->cmd.opcode), priv->regs + RPC_SMCMR);
		smenr |= RPC_SMENR_CDE;

		writel(0, priv->regs + RPC_SMADR);
		if (op->addr.nbytes == 4)
			smenr |= RPC_SMENR_ADE(0xF);
		else if (op->addr.nbytes == 3)
			smenr |= RPC_SMENR_ADE(0x7);
		else
			smenr |= RPC_SMENR_ADE(0);
		writel(offset, priv->regs + RPC_SMADR);

		writel(0, priv->regs + RPC_SMDMCR);
		if (op->dummy.nbytes) {
			writel(8 * op->dummy.nbytes - 1, priv->regs + RPC_SMDMCR);
			smenr |= RPC_SMENR_DME;
		}

		writel(0, priv->regs + RPC_SMOPR);
		writel(0, priv->regs + RPC_SMDRENR);

		if (dout && op->data.nbytes) {
			u32 *datout = (u32 *)dout;
			u32 wloop = DIV_ROUND_UP(op->data.nbytes, 4);

			smenr |= RPC_SMENR_SPIDE(0xF);

			while (wloop--) {
				smcr = RPC_SMCR_SPIWE | RPC_SMCR_SPIE;
				if (wloop >= 1)
					smcr |= RPC_SMCR_SSLKP;
				writel(smenr, priv->regs + RPC_SMENR);
				writel(*datout, priv->regs + RPC_SMWDR0);
				writel(smcr, priv->regs + RPC_SMCR);
				ret = rpc_spi_wait_tend(spi->dev);
				if (ret) {
					rpc_spi_release_bus(spi->dev);
					return ret;
				}
				datout++;
				smenr &= (~RPC_SMENR_CDE & ~RPC_SMENR_ADE(0xF));
			}

			ret = rpc_spi_wait_sslf(spi->dev);
		} else {
			writel(smenr, priv->regs + RPC_SMENR);
			writel(RPC_SMCR_SPIE, priv->regs + RPC_SMCR);
			ret = rpc_spi_wait_tend(spi->dev);
		}

		rpc_spi_release_bus(spi->dev);
		break;
	default:
		break;
	}

	return ret;
}

static int rpc_spi_set_speed(struct udevice *bus, uint speed)
{
	/* This is a SPI NOR controller, do nothing. */
	return 0;
}

static int rpc_spi_set_mode(struct udevice *bus, uint mode)
{
	/* This is a SPI NOR controller, do nothing. */
	return 0;
}

static const struct spi_controller_mem_ops rpc_spi_mem_ops = {
	.exec_op	= rpc_spi_mem_exec_op
};

static int rpc_spi_bind(struct udevice *parent)
{
	const void *fdt = gd->fdt_blob;
	ofnode node;
	int ret, off;

	/*
	 * Check if there are any SPI NOR child nodes, if so, bind as
	 * this controller will be operated in SPI mode.
	 */
	dev_for_each_subnode(node, parent) {
		off = ofnode_to_offset(node);

		ret = fdt_node_check_compatible(fdt, off, "spi-flash");
		if (!ret)
			return 0;

		ret = fdt_node_check_compatible(fdt, off, "jedec,spi-nor");
		if (!ret)
			return 0;
	}

	return -ENODEV;
}

static int rpc_spi_probe(struct udevice *dev)
{
	struct rpc_spi_plat *plat = dev_get_plat(dev);
	struct rpc_spi_priv *priv = dev_get_priv(dev);

	priv->regs = plat->regs;
	priv->extr = plat->extr;
#if CONFIG_IS_ENABLED(CLK)
	clk_enable(&priv->clk);
#endif
	return 0;
}

static int rpc_spi_of_to_plat(struct udevice *bus)
{
	struct rpc_spi_plat *plat = dev_get_plat(bus);

	plat->regs = dev_read_addr_index(bus, 0);
	plat->extr = dev_read_addr_index(bus, 1);

#if CONFIG_IS_ENABLED(CLK)
	struct rpc_spi_priv *priv = dev_get_priv(bus);
	int ret;

	ret = clk_get_by_index(bus, 0, &priv->clk);
	if (ret < 0) {
		printf("%s: Could not get clock for %s: %d\n",
		       __func__, bus->name, ret);
		return ret;
	}
#endif

	plat->freq = dev_read_u32_default(bus, "spi-max-freq", 50000000);

	return 0;
}

static const struct dm_spi_ops rpc_spi_ops = {
	.set_speed	= rpc_spi_set_speed,
	.set_mode	= rpc_spi_set_mode,
	.mem_ops        = &rpc_spi_mem_ops
};

static const struct udevice_id rpc_spi_ids[] = {
	{ .compatible = "renesas,r7s72100-rpc-if" },
	{ .compatible = "renesas,rcar-gen3-rpc-if" },
	{ .compatible = "renesas,rcar-gen4-rpc-if" },
	{ }
};

U_BOOT_DRIVER(rpc_spi) = {
	.name		= "rpc_spi",
	.id		= UCLASS_SPI,
	.of_match	= rpc_spi_ids,
	.ops		= &rpc_spi_ops,
	.of_to_plat = rpc_spi_of_to_plat,
	.plat_auto	= sizeof(struct rpc_spi_plat),
	.priv_auto	= sizeof(struct rpc_spi_priv),
	.bind		= rpc_spi_bind,
	.probe		= rpc_spi_probe,
};
