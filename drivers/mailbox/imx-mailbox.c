// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <mailbox-uclass.h>
#include <linux/bitfield.h>
#include <linux/bug.h>
#include <linux/iopoll.h>
#include <linux/compat.h>

/* This driver only exposes the status bits to keep with the
 * polling methodology of u-boot.
 */
DECLARE_GLOBAL_DATA_PTR;

#define IMX_MU_CHANS		24

#define IMX_MU_V2_PAR_OFF	0x4
#define IMX_MU_V2_TR_MASK	GENMASK(7, 0)
#define IMX_MU_V2_RR_MASK	GENMASK(15, 8)

enum imx_mu_chan_type {
	IMX_MU_TYPE_TX          = 0, /* Tx */
	IMX_MU_TYPE_RX          = 1, /* Rx */
	IMX_MU_TYPE_TXDB        = 2, /* Tx doorbell */
	IMX_MU_TYPE_RXDB        = 3, /* Rx doorbell */
	IMX_MU_TYPE_RST         = 4, /* Reset */
	IMX_MU_TYPE_TXDB_V2     = 5, /* Tx doorbell with S/W ACK */
};

enum imx_mu_xcr {
	IMX_MU_CR,
	IMX_MU_GIER,
	IMX_MU_GCR,
	IMX_MU_TCR,
	IMX_MU_RCR,
	IMX_MU_xCR_MAX,
};

enum imx_mu_xsr {
	IMX_MU_SR,
	IMX_MU_GSR,
	IMX_MU_TSR,
	IMX_MU_RSR,
	IMX_MU_xSR_MAX,
};

struct imx_mu_con_priv {
	unsigned int		idx;
	enum imx_mu_chan_type	type;
	struct mbox_chan	*chan;
};

enum imx_mu_type {
	IMX_MU_V1,
	IMX_MU_V2 = BIT(1),
	IMX_MU_V2_S4 = BIT(15),
	IMX_MU_V2_IRQ = BIT(16),
};

struct imx_mu {
	void __iomem *base;
	const struct imx_mu_dcfg *dcfg;
	u32 num_tr;
	u32 num_rr;
	/* use pointers to channel as a way to reserve channels */
	struct mbox_chan *channels[IMX_MU_CHANS];
	struct imx_mu_con_priv  con_priv[IMX_MU_CHANS];
};

struct imx_mu_dcfg {
	int (*tx)(struct imx_mu *plat, struct imx_mu_con_priv *cp, const void *data);
	int (*rx)(struct imx_mu *plat, struct imx_mu_con_priv *cp);
	int (*rxdb)(struct imx_mu *plat, struct imx_mu_con_priv *cp);
	int (*init)(struct imx_mu *plat);
	int (*of_xlate)(struct mbox_chan *chan, struct ofnode_phandle_args *args);
	enum imx_mu_type type;
	u32	xTR;			/* Transmit Register0 */
	u32	xRR;			/* Receive Register0 */
	u32	xSR[IMX_MU_xSR_MAX];	/* Status Registers */
	u32	xCR[IMX_MU_xCR_MAX];	/* Control Registers */
};

#define IMX_MU_xSR_GIPn(type, x) (type & IMX_MU_V2 ? BIT(x) : BIT(28 + (3 - (x))))
#define IMX_MU_xSR_RFn(type, x) (type & IMX_MU_V2 ? BIT(x) : BIT(24 + (3 - (x))))
#define IMX_MU_xSR_TEn(type, x) (type & IMX_MU_V2 ? BIT(x) : BIT(20 + (3 - (x))))

/* General Purpose Interrupt Enable */
#define IMX_MU_xCR_GIEn(type, x) (type & IMX_MU_V2 ? BIT(x) : BIT(28 + (3 - (x))))
/* Receive Interrupt Enable */
#define IMX_MU_xCR_RIEn(type, x) (type & IMX_MU_V2 ? BIT(x) : BIT(24 + (3 - (x))))
/* Transmit Interrupt Enable */
#define IMX_MU_xCR_TIEn(type, x) (type & IMX_MU_V2 ? BIT(x) : BIT(20 + (3 - (x))))
/* General Purpose Interrupt Request */
#define IMX_MU_xCR_GIRn(type, x) (type & IMX_MU_V2 ? BIT(x) : BIT(16 + (3 - (x))))
/* MU reset */
#define IMX_MU_xCR_RST(type)	(type & IMX_MU_V2 ? BIT(0) : BIT(5))
#define IMX_MU_xSR_RST(type)	(type & IMX_MU_V2 ? BIT(0) : BIT(7))

static void imx_mu_write(struct imx_mu *plat, u32 val, u32 offs)
{
	iowrite32(val, plat->base + offs);
}

static u32 imx_mu_read(struct imx_mu *plat, u32 offs)
{
	return ioread32(plat->base + offs);
}

static u32 imx_mu_xcr_rmw(struct imx_mu *plat, enum imx_mu_xcr type, u32 set, u32 clr)
{
	u32 val;

	val = imx_mu_read(plat, plat->dcfg->xCR[type]);
	val &= ~clr;
	val |= set;
	imx_mu_write(plat, val, plat->dcfg->xCR[type]);

	return val;
}

/* check that the channel is open or owned by caller */
static int imx_mu_check_channel(struct mbox_chan *chan)
{
	struct imx_mu *plat = dev_get_plat(chan->dev);

	if (plat->channels[chan->id]) {
		/* if reserved check that caller owns */
		if (plat->channels[chan->id] == chan)
			return 1; /* caller owns the channel */

		return -EACCES;
	}

	return 0; /* channel empty */
}

static int imx_mu_chan_request(struct mbox_chan *chan)
{
	struct imx_mu *plat = dev_get_plat(chan->dev);
	struct imx_mu_con_priv *cp;
	enum imx_mu_chan_type type;
	int idx;

	type = chan->id / 4;
	idx = chan->id % 4;

	if (imx_mu_check_channel(chan) < 0) /* check if channel already in use */
		return -EPERM;

	plat->channels[chan->id] = chan;
	chan->con_priv = kcalloc(1, sizeof(struct imx_mu_con_priv), 0);
	if (!chan->con_priv)
		return -ENOMEM;
	cp = chan->con_priv;
	cp->idx = idx;
	cp->type = type;
	cp->chan = chan;

	switch (type) {
	case IMX_MU_TYPE_RX:
		imx_mu_xcr_rmw(plat, IMX_MU_RCR, IMX_MU_xCR_RIEn(plat->dcfg->type, idx), 0);
		break;
	case IMX_MU_TYPE_TXDB_V2:
	case IMX_MU_TYPE_TXDB:
	case IMX_MU_TYPE_RXDB:
		imx_mu_xcr_rmw(plat, IMX_MU_GIER, IMX_MU_xCR_GIEn(plat->dcfg->type, idx), 0);
		break;
	default:
		break;
	}

	return 0;
}

static int imx_mu_chan_free(struct mbox_chan *chan)
{
	struct imx_mu *plat = dev_get_plat(chan->dev);
	struct imx_mu_con_priv *cp = chan->con_priv;

	if (imx_mu_check_channel(chan) <= 0) /* check that the channel is also not empty */
		return -EINVAL;

	/* if you own channel and  channel is NOT empty */
	plat->channels[chan->id] = NULL;
	switch (cp->type) {
	case IMX_MU_TYPE_TX:
		imx_mu_xcr_rmw(plat, IMX_MU_TCR, 0, IMX_MU_xCR_TIEn(plat->dcfg->type, cp->idx));
		break;
	case IMX_MU_TYPE_RX:
		imx_mu_xcr_rmw(plat, IMX_MU_RCR, 0, IMX_MU_xCR_RIEn(plat->dcfg->type, cp->idx));
		break;
	case IMX_MU_TYPE_TXDB_V2:
	case IMX_MU_TYPE_TXDB:
	case IMX_MU_TYPE_RXDB:
		imx_mu_xcr_rmw(plat, IMX_MU_GIER, 0, IMX_MU_xCR_GIEn(plat->dcfg->type, cp->idx));
		break;
	default:
		break;
	}

	kfree(cp);

	return 0;
}

static int imx_mu_send(struct mbox_chan *chan, const void *data)
{
	struct imx_mu *plat = dev_get_plat(chan->dev);
	struct imx_mu_con_priv *cp = chan->con_priv;

	if (imx_mu_check_channel(chan) < 1) /* return if channel isn't owned */
		return -EPERM;

	return plat->dcfg->tx(plat, cp, data);
}

static int imx_mu_recv(struct mbox_chan *chan, void *data)
{
	struct imx_mu *plat = dev_get_plat(chan->dev);
	struct imx_mu_con_priv *cp = chan->con_priv;
	u32 ctrl, val;

	if (imx_mu_check_channel(chan) < 1) /* return if channel isn't owned */
		return -EPERM;

	switch (cp->type) {
	case IMX_MU_TYPE_TXDB_V2:
	case IMX_MU_TYPE_RXDB:
		/* check if GSR[GIRn] bit is set */
		if (readx_poll_timeout(ioread32, plat->base + plat->dcfg->xSR[IMX_MU_GSR],
			val, val & BIT(cp->idx), 1000000) < 0)
			return -EBUSY;

		ctrl = imx_mu_read(plat, plat->dcfg->xCR[IMX_MU_GIER]);
		val = imx_mu_read(plat, plat->dcfg->xSR[IMX_MU_GSR]);
		val &= IMX_MU_xSR_GIPn(plat->dcfg->type, cp->idx) &
			(ctrl & IMX_MU_xCR_GIEn(plat->dcfg->type, cp->idx));
		break;
	default:
		dev_warn(chan->dev, "Unhandled channel type %d\n", cp->type);
		return -EOPNOTSUPP;
	};

	if (val == IMX_MU_xSR_GIPn(plat->dcfg->type, cp->idx))
		plat->dcfg->rxdb(plat, cp);

	return 0;
}

static int imx_mu_of_to_plat(struct udevice *dev)
{
	struct imx_mu *plat = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -ENODEV;

	plat->base = (struct mu_type *)addr;

	return 0;
}

static int imx_mu_init_generic(struct imx_mu *plat)
{
	unsigned int i;
	unsigned int val;

	if (plat->num_rr > 4 || plat->num_tr > 4) {
		WARN_ONCE(true, "%s not support TR/RR larger than 4\n", __func__);
		return -EOPNOTSUPP;
	}

	/* Set default MU configuration */
	for (i = 0; i < IMX_MU_xCR_MAX; i++)
		imx_mu_write(plat, 0, plat->dcfg->xCR[i]);

	/* Clear any pending GIP */
	val = imx_mu_read(plat, plat->dcfg->xSR[IMX_MU_GSR]);
	imx_mu_write(plat, val, plat->dcfg->xSR[IMX_MU_GSR]);

	/* Clear any pending RSR */
	for (i = 0; i < plat->num_rr; i++)
		imx_mu_read(plat, plat->dcfg->xRR + i * 4);

	return 0;
}

static int imx_mu_generic_of_xlate(struct mbox_chan *chan, struct ofnode_phandle_args *args)
{
	enum imx_mu_chan_type type;
	int idx, cid;

	if (args->args_count != 2) {
		dev_err(chan->dev, "Invalid argument count %d\n", args->args_count);
		return -EINVAL;
	}

	type = args->args[0]; /* channel type */
	idx = args->args[1]; /* index */

	cid = type * 4 + idx;
	if (cid >= IMX_MU_CHANS) {
		dev_err(chan->dev, "Not supported channel number: %d. (type: %d, idx: %d)\n",
			cid, type, idx);
		return -EINVAL;
	}

	chan->id = cid;

	return 0;
}

static int imx_mu_generic_tx(struct imx_mu *plat, struct imx_mu_con_priv *cp,
			     const void *data)
{
	switch (cp->type) {
	case IMX_MU_TYPE_TXDB_V2:
		imx_mu_xcr_rmw(plat, IMX_MU_GCR, IMX_MU_xCR_GIRn(plat->dcfg->type, cp->idx), 0);
		break;
	default:
		dev_warn(cp->chan->dev, "Send data on wrong channel type: %d\n", cp->type);
		return -EINVAL;
	}

	return 0;
}

static int imx_mu_generic_rxdb(struct imx_mu *plat, struct imx_mu_con_priv *cp)
{
	imx_mu_write(plat, IMX_MU_xSR_GIPn(plat->dcfg->type, cp->idx),
		     plat->dcfg->xSR[IMX_MU_GSR]);

	return 0;
}

static const struct imx_mu_dcfg imx_mu_cfg_imx6sx = {
	.tx	= imx_mu_generic_tx,
	.rxdb	= imx_mu_generic_rxdb,
	.init	= imx_mu_init_generic,
	.of_xlate = imx_mu_generic_of_xlate,
	.type	= IMX_MU_V1,
	.xTR	= 0x0,
	.xRR	= 0x10,
	.xSR	= {0x20, 0x20, 0x20, 0x20},
	.xCR	= {0x24, 0x24, 0x24, 0x24, 0x24},
};

static const struct imx_mu_dcfg imx_mu_cfg_imx7ulp = {
	.tx	= imx_mu_generic_tx,
	.rxdb	= imx_mu_generic_rxdb,
	.init	= imx_mu_init_generic,
	.of_xlate = imx_mu_generic_of_xlate,
	.type	= IMX_MU_V1,
	.xTR	= 0x20,
	.xRR	= 0x40,
	.xSR	= {0x60, 0x60, 0x60, 0x60},
	.xCR	= {0x64, 0x64, 0x64, 0x64, 0x64},
};

static const struct imx_mu_dcfg imx_mu_cfg_imx95 = {
	.tx	= imx_mu_generic_tx,
	.rxdb	= imx_mu_generic_rxdb,
	.init	= imx_mu_init_generic,
	.of_xlate = imx_mu_generic_of_xlate,
	.type	= IMX_MU_V2,
	.xTR	= 0x200,
	.xRR	= 0x280,
	.xSR	= {0xC, 0x118, 0x124, 0x12C},
	.xCR	= {0x8, 0x110, 0x114, 0x120, 0x128},
};

static const struct udevice_id ids[] = {
	{ .compatible = "fsl,imx6sx-mu", .data = (ulong)&imx_mu_cfg_imx6sx },
	{ .compatible = "fsl,imx7ulp-mu", .data = (ulong)&imx_mu_cfg_imx7ulp },
	{ .compatible = "fsl,imx95-mu", .data = (ulong)&imx_mu_cfg_imx95 },
	{ }
};

int imx_mu_of_xlate(struct mbox_chan *chan, struct ofnode_phandle_args *args)
{
	struct imx_mu *plat = dev_get_plat(chan->dev);

	return plat->dcfg->of_xlate(chan, args);
}

struct mbox_ops imx_mu_ops = {
	.of_xlate = imx_mu_of_xlate,
	.request  = imx_mu_chan_request,
	.rfree    = imx_mu_chan_free,
	.send     = imx_mu_send,
	.recv     = imx_mu_recv,
};

static void imx_mu_get_tr_rr(struct imx_mu *plat)
{
	u32 val;

	if (plat->dcfg->type & IMX_MU_V2) {
		val = imx_mu_read(plat, IMX_MU_V2_PAR_OFF);
		plat->num_tr = FIELD_GET(IMX_MU_V2_TR_MASK, val);
		plat->num_rr = FIELD_GET(IMX_MU_V2_RR_MASK, val);
	} else {
		plat->num_tr = 4;
		plat->num_rr = 4;
	}
}

static int imx_mu_probe(struct udevice *dev)
{
	struct imx_mu *plat = dev_get_plat(dev);
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	plat->dcfg = (void *)dev_get_driver_data(dev);

	imx_mu_get_tr_rr(plat);

	ret = plat->dcfg->init(plat);
	if (ret) {
		dev_err(dev, "Failed to init MU\n");
		return ret;
	}

	return 0;
}

U_BOOT_DRIVER(imx_mu) = {
	.name = "imx-mu",
	.id = UCLASS_MAILBOX,
	.of_match = ids,
	.of_to_plat = imx_mu_of_to_plat,
	.plat_auto = sizeof(struct imx_mu),
	.probe = imx_mu_probe,
	.ops = &imx_mu_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
