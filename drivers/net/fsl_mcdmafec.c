// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * Conversion to DM
 * (C) 2019 Angelo Dureghello <angelo.dureghello@timesys.com>
 */

#include <common.h>
#include <env.h>
#include <hang.h>
#include <malloc.h>
#include <command.h>
#include <config.h>
#include <net.h>
#include <miiphy.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <asm/immap.h>
#include <asm/fsl_mcdmafec.h>

#include "MCD_dma.h"

#undef	ET_DEBUG
#undef	MII_DEBUG

/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH		1520
#define PKT_MAXBUF_SIZE		1518
#define FIFO_ERRSTAT		(FIFO_STAT_RXW | FIFO_STAT_UF | FIFO_STAT_OF)

/* RxBD bits definitions */
#define BD_ENET_RX_ERR	(BD_ENET_RX_LG | BD_ENET_RX_NO | BD_ENET_RX_CR | \
			 BD_ENET_RX_OV | BD_ENET_RX_TR)

DECLARE_GLOBAL_DATA_PTR;

static void init_eth_info(struct fec_info_dma *info)
{
	/* setup Receive and Transmit buffer descriptor */
#ifdef CONFIG_SYS_FEC_BUF_USE_SRAM
	static u32 tmp;

	if (info->index == 0)
		tmp = CONFIG_SYS_INIT_RAM_ADDR + 0x1000;
	else
		info->rxbd = (cbd_t *)DBUF_LENGTH;

	info->rxbd = (cbd_t *)((u32)info->rxbd + tmp);
	tmp = (u32)info->rxbd;
	info->txbd =
	    (cbd_t *)((u32)info->txbd + tmp +
	    (PKTBUFSRX * sizeof(cbd_t)));
	tmp = (u32)info->txbd;
	info->txbuf =
	    (char *)((u32)info->txbuf + tmp +
	    (CONFIG_SYS_TX_ETH_BUFFER * sizeof(cbd_t)));
	tmp = (u32)info->txbuf;
#else
	info->rxbd =
	    (cbd_t *)memalign(CONFIG_SYS_CACHELINE_SIZE,
			       (PKTBUFSRX * sizeof(cbd_t)));
	info->txbd =
	    (cbd_t *)memalign(CONFIG_SYS_CACHELINE_SIZE,
			       (CONFIG_SYS_TX_ETH_BUFFER * sizeof(cbd_t)));
	info->txbuf =
	    (char *)memalign(CONFIG_SYS_CACHELINE_SIZE, DBUF_LENGTH);
#endif

#ifdef ET_DEBUG
	printf("rxbd %x txbd %x\n", (int)info->rxbd, (int)info->txbd);
#endif
	info->phy_name = (char *)memalign(CONFIG_SYS_CACHELINE_SIZE, 32);
}

static void fec_halt(struct udevice *dev)
{
	struct fec_info_dma *info = dev_get_priv(dev);
	volatile fecdma_t *fecp = (fecdma_t *)info->iobase;
	int counter = 0xffff;

	/* issue graceful stop command to the FEC transmitter if necessary */
	fecp->tcr |= FEC_TCR_GTS;

	/* wait for graceful stop to register */
	while ((counter--) && (!(fecp->eir & FEC_EIR_GRA)))
		;

	/* Disable DMA tasks */
	MCD_killDma(info->tx_task);
	MCD_killDma(info->rx_task);

	/* Disable the Ethernet Controller */
	fecp->ecr &= ~FEC_ECR_ETHER_EN;

	/* Clear FIFO status registers */
	fecp->rfsr &= FIFO_ERRSTAT;
	fecp->tfsr &= FIFO_ERRSTAT;

	fecp->frst = 0x01000000;

	/* Issue a reset command to the FEC chip */
	fecp->ecr |= FEC_ECR_RESET;

	/* wait at least 20 clock cycles */
	mdelay(10);

#ifdef ET_DEBUG
	printf("Ethernet task stopped\n");
#endif
}

#ifdef ET_DEBUG
static void dbg_fec_regs(struct eth_device *dev)
{
	struct fec_info_dma *info = dev->priv;
	volatile fecdma_t *fecp = (fecdma_t *)info->iobase;

	printf("=====\n");
	printf("ievent       %x - %x\n", (int)&fecp->eir, fecp->eir);
	printf("imask        %x - %x\n", (int)&fecp->eimr, fecp->eimr);
	printf("ecntrl       %x - %x\n", (int)&fecp->ecr, fecp->ecr);
	printf("mii_mframe   %x - %x\n", (int)&fecp->mmfr, fecp->mmfr);
	printf("mii_speed    %x - %x\n", (int)&fecp->mscr, fecp->mscr);
	printf("mii_ctrlstat %x - %x\n", (int)&fecp->mibc, fecp->mibc);
	printf("r_cntrl      %x - %x\n", (int)&fecp->rcr, fecp->rcr);
	printf("r hash       %x - %x\n", (int)&fecp->rhr, fecp->rhr);
	printf("x_cntrl      %x - %x\n", (int)&fecp->tcr, fecp->tcr);
	printf("padr_l       %x - %x\n", (int)&fecp->palr, fecp->palr);
	printf("padr_u       %x - %x\n", (int)&fecp->paur, fecp->paur);
	printf("op_pause     %x - %x\n", (int)&fecp->opd, fecp->opd);
	printf("iadr_u       %x - %x\n", (int)&fecp->iaur, fecp->iaur);
	printf("iadr_l       %x - %x\n", (int)&fecp->ialr, fecp->ialr);
	printf("gadr_u       %x - %x\n", (int)&fecp->gaur, fecp->gaur);
	printf("gadr_l       %x - %x\n", (int)&fecp->galr, fecp->galr);
	printf("x_wmrk       %x - %x\n", (int)&fecp->tfwr, fecp->tfwr);
	printf("r_fdata      %x - %x\n", (int)&fecp->rfdr, fecp->rfdr);
	printf("r_fstat      %x - %x\n", (int)&fecp->rfsr, fecp->rfsr);
	printf("r_fctrl      %x - %x\n", (int)&fecp->rfcr, fecp->rfcr);
	printf("r_flrfp      %x - %x\n", (int)&fecp->rlrfp, fecp->rlrfp);
	printf("r_flwfp      %x - %x\n", (int)&fecp->rlwfp, fecp->rlwfp);
	printf("r_frfar      %x - %x\n", (int)&fecp->rfar, fecp->rfar);
	printf("r_frfrp      %x - %x\n", (int)&fecp->rfrp, fecp->rfrp);
	printf("r_frfwp      %x - %x\n", (int)&fecp->rfwp, fecp->rfwp);
	printf("t_fdata      %x - %x\n", (int)&fecp->tfdr, fecp->tfdr);
	printf("t_fstat      %x - %x\n", (int)&fecp->tfsr, fecp->tfsr);
	printf("t_fctrl      %x - %x\n", (int)&fecp->tfcr, fecp->tfcr);
	printf("t_flrfp      %x - %x\n", (int)&fecp->tlrfp, fecp->tlrfp);
	printf("t_flwfp      %x - %x\n", (int)&fecp->tlwfp, fecp->tlwfp);
	printf("t_ftfar      %x - %x\n", (int)&fecp->tfar, fecp->tfar);
	printf("t_ftfrp      %x - %x\n", (int)&fecp->tfrp, fecp->tfrp);
	printf("t_ftfwp      %x - %x\n", (int)&fecp->tfwp, fecp->tfwp);
	printf("frst         %x - %x\n", (int)&fecp->frst, fecp->frst);
	printf("ctcwr        %x - %x\n", (int)&fecp->ctcwr, fecp->ctcwr);
}
#endif

static void set_fec_duplex_speed(volatile fecdma_t *fecp, int dup_spd)
{
	struct bd_info *bd = gd->bd;

	if ((dup_spd >> 16) == FULL) {
		/* Set maximum frame length */
		fecp->rcr = FEC_RCR_MAX_FL(PKT_MAXBUF_SIZE) | FEC_RCR_MII_MODE |
		    FEC_RCR_PROM | 0x100;
		fecp->tcr = FEC_TCR_FDEN;
	} else {
		/* Half duplex mode */
		fecp->rcr = FEC_RCR_MAX_FL(PKT_MAXBUF_SIZE) |
		    FEC_RCR_MII_MODE | FEC_RCR_DRT;
		fecp->tcr &= ~FEC_TCR_FDEN;
	}

	if ((dup_spd & 0xFFFF) == _100BASET) {
#ifdef MII_DEBUG
		printf("100Mbps\n");
#endif
		bd->bi_ethspeed = 100;
	} else {
#ifdef MII_DEBUG
		printf("10Mbps\n");
#endif
		bd->bi_ethspeed = 10;
	}
}

static void fec_set_hwaddr(volatile fecdma_t *fecp, u8 *mac)
{
	u8 curr_byte;		/* byte for which to compute the CRC */
	int byte;		/* loop - counter */
	int bit;		/* loop - counter */
	u32 crc = 0xffffffff;	/* initial value */

	for (byte = 0; byte < 6; byte++) {
		curr_byte = mac[byte];
		for (bit = 0; bit < 8; bit++) {
			if ((curr_byte & 0x01) ^ (crc & 0x01)) {
				crc >>= 1;
				crc = crc ^ 0xedb88320;
			} else {
				crc >>= 1;
			}
			curr_byte >>= 1;
		}
	}

	crc = crc >> 26;

	/* Set individual hash table register */
	if (crc >= 32) {
		fecp->ialr = (1 << (crc - 32));
		fecp->iaur = 0;
	} else {
		fecp->ialr = 0;
		fecp->iaur = (1 << crc);
	}

	/* Set physical address */
	fecp->palr = (mac[0] << 24) + (mac[1] << 16) + (mac[2] << 8) + mac[3];
	fecp->paur = (mac[4] << 24) + (mac[5] << 16) + 0x8808;

	/* Clear multicast address hash table */
	fecp->gaur = 0;
	fecp->galr = 0;
}

static int fec_init(struct udevice *dev)
{
	struct fec_info_dma *info = dev_get_priv(dev);
	volatile fecdma_t *fecp = (fecdma_t *)info->iobase;
	int rval, i;
	uchar enetaddr[6];

#ifdef ET_DEBUG
	printf("fec_init: iobase 0x%08x ...\n", info->iobase);
#endif

	fecpin_setclear(info, 1);
	fec_halt(dev);

	mii_init();
	set_fec_duplex_speed(fecp, info->dup_spd);

	/* We use strictly polling mode only */
	fecp->eimr = 0;

	/* Clear any pending interrupt */
	fecp->eir = 0xffffffff;

	/* Set station address   */
	if (info->index == 0)
		rval = eth_env_get_enetaddr("ethaddr", enetaddr);
	else
		rval = eth_env_get_enetaddr("eth1addr", enetaddr);

	if (!rval) {
		puts("Please set a valid MAC address\n");
		return -EINVAL;
	}

	fec_set_hwaddr(fecp, enetaddr);

	/* Set Opcode/Pause Duration Register */
	fecp->opd = 0x00010020;

	/* Setup Buffers and Buffer Descriptors */
	info->rx_idx = 0;
	info->tx_idx = 0;

	/* Setup Receiver Buffer Descriptors (13.14.24.18)
	 * Settings:     Empty, Wrap */
	for (i = 0; i < PKTBUFSRX; i++) {
		info->rxbd[i].cbd_sc = BD_ENET_RX_EMPTY;
		info->rxbd[i].cbd_datlen = PKTSIZE_ALIGN;
		info->rxbd[i].cbd_bufaddr = (uint) net_rx_packets[i];
	}
	info->rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

	/* Setup Ethernet Transmitter Buffer Descriptors (13.14.24.19)
	 * Settings:    Last, Tx CRC */
	for (i = 0; i < CONFIG_SYS_TX_ETH_BUFFER; i++) {
		info->txbd[i].cbd_sc = 0;
		info->txbd[i].cbd_datlen = 0;
		info->txbd[i].cbd_bufaddr = (uint) (&info->txbuf[0]);
	}
	info->txbd[CONFIG_SYS_TX_ETH_BUFFER - 1].cbd_sc |= BD_ENET_TX_WRAP;

	info->used_tbd_idx = 0;
	info->clean_tbd_num = CONFIG_SYS_TX_ETH_BUFFER;

	/* Set Rx FIFO alarm and granularity value */
	fecp->rfcr = 0x0c000000;
	fecp->rfar = 0x0000030c;

	/* Set Tx FIFO granularity value */
	fecp->tfcr = FIFO_CTRL_FRAME | FIFO_CTRL_GR(6) | 0x00040000;
	fecp->tfar = 0x00000080;

	fecp->tfwr = 0x2;
	fecp->ctcwr = 0x03000000;

	/* Enable DMA receive task */
	MCD_startDma(info->rx_task,
		     (s8 *)info->rxbd,
		     0,
		     (s8 *)&fecp->rfdr,
		     4,
		     0,
		     4,
		     info->rx_init,
		     info->rx_pri,
		     (MCD_FECRX_DMA | MCD_TT_FLAGS_DEF),
		     (MCD_NO_CSUM | MCD_NO_BYTE_SWAP)
	    );

	/* Enable DMA tx task with no ready buffer descriptors */
	MCD_startDma(info->tx_task,
		     (s8 *)info->txbd,
		     0,
		     (s8 *)&fecp->tfdr,
		     4,
		     0,
		     4,
		     info->tx_init,
		     info->tx_pri,
		     (MCD_FECTX_DMA | MCD_TT_FLAGS_DEF),
		     (MCD_NO_CSUM | MCD_NO_BYTE_SWAP)
	    );

	/* Now enable the transmit and receive processing */
	fecp->ecr |= FEC_ECR_ETHER_EN;

	return 0;
}

static int mcdmafec_init(struct udevice *dev)
{
	return fec_init(dev);
}

static int mcdmafec_send(struct udevice *dev, void *packet, int length)
{
	struct fec_info_dma *info = dev_get_priv(dev);
	cbd_t *p_tbd, *p_used_tbd;
	u16 phy_status;

	miiphy_read(dev->name, info->phy_addr, MII_BMSR, &phy_status);

	/* process all the consumed TBDs */
	while (info->clean_tbd_num < CONFIG_SYS_TX_ETH_BUFFER) {
		p_used_tbd = &info->txbd[info->used_tbd_idx];
		if (p_used_tbd->cbd_sc & BD_ENET_TX_READY) {
#ifdef ET_DEBUG
			printf("Cannot clean TBD %d, in use\n",
			       info->clean_tbd_num);
#endif
			return 0;
		}

		/* clean this buffer descriptor */
		if (info->used_tbd_idx == (CONFIG_SYS_TX_ETH_BUFFER - 1))
			p_used_tbd->cbd_sc = BD_ENET_TX_WRAP;
		else
			p_used_tbd->cbd_sc = 0;

		/* update some indeces for a correct handling of TBD ring */
		info->clean_tbd_num++;
		info->used_tbd_idx = (info->used_tbd_idx + 1)
			% CONFIG_SYS_TX_ETH_BUFFER;
	}

	/* Check for valid length of data. */
	if (length > 1500 || length <= 0)
		return -1;

	/* Check the number of vacant TxBDs. */
	if (info->clean_tbd_num < 1) {
		printf("No available TxBDs ...\n");
		return -1;
	}

	/* Get the first TxBD to send the mac header */
	p_tbd = &info->txbd[info->tx_idx];
	p_tbd->cbd_datlen = length;
	p_tbd->cbd_bufaddr = (u32)packet;
	p_tbd->cbd_sc |= BD_ENET_TX_LAST | BD_ENET_TX_TC | BD_ENET_TX_READY;
	info->tx_idx = (info->tx_idx + 1) % CONFIG_SYS_TX_ETH_BUFFER;

	/* Enable DMA transmit task */
	MCD_continDma(info->tx_task);

	info->clean_tbd_num -= 1;

	/* wait until frame is sent . */
	while (p_tbd->cbd_sc & BD_ENET_TX_READY)
		udelay(10);

	return (int)(info->txbd[info->tx_idx].cbd_sc & BD_ENET_TX_STATS);
}

static int mcdmafec_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct fec_info_dma *info = dev_get_priv(dev);
	volatile fecdma_t *fecp = (fecdma_t *)info->iobase;

	cbd_t *prbd = &info->rxbd[info->rx_idx];
	u32 ievent;
	int frame_length, len = 0;

	/* Check if any critical events have happened */
	ievent = fecp->eir;
	if (ievent != 0) {
		fecp->eir = ievent;

		if (ievent & (FEC_EIR_BABT | FEC_EIR_TXERR | FEC_EIR_RXERR)) {
			printf("fec_recv: error\n");
			fec_halt(dev);
			fec_init(dev);
			return 0;
		}

		if (ievent & FEC_EIR_HBERR) {
			/* Heartbeat error */
			fecp->tcr |= FEC_TCR_GTS;
		}

		if (ievent & FEC_EIR_GRA) {
			/* Graceful stop complete */
			if (fecp->tcr & FEC_TCR_GTS) {
				printf("fec_recv: tcr_gts\n");
				fec_halt(dev);
				fecp->tcr &= ~FEC_TCR_GTS;
				fec_init(dev);
			}
		}
	}

	if (!(prbd->cbd_sc & BD_ENET_RX_EMPTY)) {
		if ((prbd->cbd_sc & BD_ENET_RX_LAST) &&
		    !(prbd->cbd_sc & BD_ENET_RX_ERR) &&
		    ((prbd->cbd_datlen - 4) > 14)) {
			/* Get buffer address and size */
			frame_length = prbd->cbd_datlen - 4;

			/* Fill the buffer and pass it to upper layers */
			net_process_received_packet((uchar *)prbd->cbd_bufaddr,
						    frame_length);
			len = frame_length;
		}

		/* Reset buffer descriptor as empty */
		if (info->rx_idx == (PKTBUFSRX - 1))
			prbd->cbd_sc = (BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY);
		else
			prbd->cbd_sc = BD_ENET_RX_EMPTY;

		prbd->cbd_datlen = PKTSIZE_ALIGN;

		/* Now, we have an empty RxBD, restart the DMA receive task */
		MCD_continDma(info->rx_task);

		/* Increment BD count */
		info->rx_idx = (info->rx_idx + 1) % PKTBUFSRX;
	}

	return len;
}

static void mcdmafec_halt(struct udevice *dev)
{
	fec_halt(dev);
}

static const struct eth_ops mcdmafec_ops = {
	.start	= mcdmafec_init,
	.send	= mcdmafec_send,
	.recv	= mcdmafec_recv,
	.stop	= mcdmafec_halt,
};

/*
 * Boot sequence, called just after mcffec_of_to_plat,
 * as DM way, it replaces old mcffec_initialize.
 */
static int mcdmafec_probe(struct udevice *dev)
{
	struct fec_info_dma *info = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	int node = dev_of_offset(dev);
	int retval;
	const u32 *val;

	info->index = dev_seq(dev);
	info->iobase = pdata->iobase;
	info->miibase = pdata->iobase;
	info->phy_addr = -1;

	val = fdt_getprop(gd->fdt_blob, node, "rx-task", NULL);
	if (val)
		info->rx_task = fdt32_to_cpu(*val);

	val = fdt_getprop(gd->fdt_blob, node, "tx-task", NULL);
	if (val)
		info->tx_task = fdt32_to_cpu(*val);

	val = fdt_getprop(gd->fdt_blob, node, "rx-prioprity", NULL);
	if (val)
		info->rx_pri = fdt32_to_cpu(*val);

	val = fdt_getprop(gd->fdt_blob, node, "tx-prioprity", NULL);
	if (val)
		info->tx_pri = fdt32_to_cpu(*val);

	val = fdt_getprop(gd->fdt_blob, node, "rx-init", NULL);
	if (val)
		info->rx_init = fdt32_to_cpu(*val);

	val = fdt_getprop(gd->fdt_blob, node, "tx-init", NULL);
	if (val)
		info->tx_init = fdt32_to_cpu(*val);

#ifdef CONFIG_SYS_FEC_BUF_USE_SRAM
	u32 tmp = CONFIG_SYS_INIT_RAM_ADDR + 0x1000;
#endif
	init_eth_info(info);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	info->bus = mdio_alloc();
	if (!info->bus)
		return -ENOMEM;
	strlcpy(info->bus->name, dev->name, MDIO_NAME_LEN);
	info->bus->read = mcffec_miiphy_read;
	info->bus->write = mcffec_miiphy_write;

	retval = mdio_register(info->bus);
	if (retval < 0)
		return retval;
#endif

	return 0;
}

static int mcdmafec_remove(struct udevice *dev)
{
	struct fec_info_dma *priv = dev_get_priv(dev);

	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	return 0;
}

/*
 * Boot sequence, called 1st
 */
static int mcdmafec_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	const u32 *val;

	pdata->iobase = dev_read_addr(dev);
	/* Default to 10Mbit/s */
	pdata->max_speed = 10;

	val = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "max-speed", NULL);
	if (val)
		pdata->max_speed = fdt32_to_cpu(*val);

	return 0;
}

static const struct udevice_id mcdmafec_ids[] = {
	{ .compatible = "fsl,mcf-dma-fec" },
	{ }
};

U_BOOT_DRIVER(mcffec) = {
	.name	= "mcdmafec",
	.id	= UCLASS_ETH,
	.of_match = mcdmafec_ids,
	.of_to_plat = mcdmafec_of_to_plat,
	.probe	= mcdmafec_probe,
	.remove	= mcdmafec_remove,
	.ops	= &mcdmafec_ops,
	.priv_auto	= sizeof(struct fec_info_dma),
	.plat_auto	= sizeof(struct eth_pdata),
};
