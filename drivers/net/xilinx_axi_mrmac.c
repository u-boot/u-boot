// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx Multirate Ethernet MAC(MRMAC) driver
 *
 * Author(s):   Ashok Reddy Soma <ashok.reddy.soma@xilinx.com>
 *              Michal Simek <michal.simek@xilinx.com>
 *
 * Copyright (C) 2021 Xilinx, Inc. All rights reserved.
 */

#include <config.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <net.h>
#include <malloc.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include "xilinx_axi_mrmac.h"

static u8 rxframe[RX_DESC * PKTSIZE_ALIGN] __attribute((aligned(DMAALIGN)));
static u8 txminframe[MIN_PKT_SIZE] __attribute((aligned(DMAALIGN)));

/* Static buffer descriptors:
 * MRMAC needs atleast two buffer descriptors for the TX/RX to happen.
 * Otherwise MRMAC will drop the packets. So, have two tx and rx bd's here.
 */
static struct mcdma_bd tx_bd[TX_DESC] __attribute((aligned(DMAALIGN)));
static struct mcdma_bd rx_bd[RX_DESC] __attribute((aligned(DMAALIGN)));

static void axi_mrmac_dma_write(struct mcdma_bd *bd, u32 *desc)
{
	if (IS_ENABLED(CONFIG_PHYS_64BIT))
		writeq((unsigned long)bd, desc);
	else
		writel((uintptr_t)bd, desc);
}

static int axi_mrmac_ethernet_init(struct axi_mrmac_priv *priv)
{
	struct mrmac_regs *regs = priv->iobase;
	u32 val, reg;
	u32 ret;

	/* Perform all the RESET's required */
	val = readl(&regs->reset);
	val |= MRMAC_RX_SERDES_RST_MASK | MRMAC_TX_SERDES_RST_MASK |
		MRMAC_RX_RST_MASK | MRMAC_TX_RST_MASK;
	writel(val, &regs->reset);

	mdelay(MRMAC_RESET_DELAY);

	/* Configure Mode register */
	reg = readl(&regs->mode);

	debug("Configuring MRMAC speed to %d\n", priv->mrmac_rate);

	if (priv->mrmac_rate == SPEED_25000) {
		reg &= ~MRMAC_CTL_RATE_CFG_MASK;
		reg |= MRMAC_CTL_DATA_RATE_25G;
		reg |= (MRMAC_CTL_AXIS_CFG_25G_IND << MRMAC_CTL_AXIS_CFG_SHIFT);
		reg |= (MRMAC_CTL_SERDES_WIDTH_25G <<
			MRMAC_CTL_SERDES_WIDTH_SHIFT);
	} else {
		reg &= ~MRMAC_CTL_RATE_CFG_MASK;
		reg |= MRMAC_CTL_DATA_RATE_10G;
		reg |= (MRMAC_CTL_AXIS_CFG_10G_IND << MRMAC_CTL_AXIS_CFG_SHIFT);
		reg |= (MRMAC_CTL_SERDES_WIDTH_10G <<
			MRMAC_CTL_SERDES_WIDTH_SHIFT);
	}

	/* For tick reg */
	reg |= MRMAC_CTL_PM_TICK_MASK;
	writel(reg, &regs->mode);

	val = readl(&regs->reset);
	val &= ~(MRMAC_RX_SERDES_RST_MASK | MRMAC_TX_SERDES_RST_MASK |
		 MRMAC_RX_RST_MASK | MRMAC_TX_RST_MASK);
	writel(val, &regs->reset);

	mdelay(MRMAC_RESET_DELAY);

	/* Setup MRMAC hardware options */
	writel(readl(&regs->rx_config) | MRMAC_RX_DEL_FCS_MASK,
	       &regs->rx_config);
	writel(readl(&regs->tx_config) | MRMAC_TX_INS_FCS_MASK,
	       &regs->tx_config);
	writel(readl(&regs->tx_config) | MRMAC_TX_EN_MASK, &regs->tx_config);
	writel(readl(&regs->rx_config) | MRMAC_RX_EN_MASK, &regs->rx_config);

	/* Check for block lock bit to be set. This ensures that
	 * MRMAC ethernet IP is functioning normally.
	 */
	writel(MRMAC_STS_ALL_MASK, (phys_addr_t)priv->iobase +
		MRMAC_TX_STS_OFFSET);
	writel(MRMAC_STS_ALL_MASK, (phys_addr_t)priv->iobase +
		MRMAC_RX_STS_OFFSET);
	writel(MRMAC_STS_ALL_MASK, (phys_addr_t)priv->iobase +
		MRMAC_STATRX_BLKLCK_OFFSET);

	ret = wait_for_bit_le32((u32 *)((phys_addr_t)priv->iobase +
				MRMAC_STATRX_BLKLCK_OFFSET),
				MRMAC_RX_BLKLCK_MASK, true,
				MRMAC_BLKLCK_TIMEOUT, true);
	if (ret) {
		printf("%s: MRMAC block lock not complete!\n", __func__);
		return 1;
	}

	writel(MRMAC_TICK_TRIGGER, &regs->tick_reg);

	return 0;
}

/* Reset DMA engine */
static int axi_mcdma_init(struct axi_mrmac_priv *priv)
{
	u32 ret;

	/* Reset the engine so the hardware starts from a known state */
	writel(XMCDMA_CR_RESET, &priv->mm2s_cmn->control);
	writel(XMCDMA_CR_RESET, &priv->s2mm_cmn->control);

	/* Check TX/RX MCDMA.RST. Reset is done when the reset bit is low */
	ret = wait_for_bit_le32(&priv->mm2s_cmn->control, XMCDMA_CR_RESET,
				false, MRMAC_DMARST_TIMEOUT, true);
	if (ret) {
		printf("%s: TX MCDMA reset Timeout\n", __func__);
		return -1;
	}

	ret = wait_for_bit_le32(&priv->s2mm_cmn->control, XMCDMA_CR_RESET,
				false, MRMAC_DMARST_TIMEOUT, true);
	if (ret) {
		printf("%s: RX MCDMA reset Timeout\n", __func__);
		return -1;
	}

	/* Enable channel 1 for TX and RX */
	writel(XMCDMA_CHANNEL_1, &priv->mm2s_cmn->chen);
	writel(XMCDMA_CHANNEL_1, &priv->s2mm_cmn->chen);

	return 0;
}

static int axi_mrmac_start(struct udevice *dev)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);
	struct mrmac_regs *regs = priv->iobase;
	u32 temp;

	/*
	 * Initialize MCDMA engine. MCDMA engine must be initialized before
	 * MRMAC. During MCDMA engine initialization, MCDMA hardware is reset,
	 * since MCDMA reset line is connected to MRMAC, this would ensure a
	 * reset of MRMAC.
	 */
	axi_mcdma_init(priv);

	/* Initialize MRMAC hardware */
	if (axi_mrmac_ethernet_init(priv))
		return -1;

	/* Disable all RX interrupts before RxBD space setup */
	temp = readl(&priv->mcdma_rx->control);
	temp &= ~XMCDMA_IRQ_ALL_MASK;
	writel(temp, &priv->mcdma_rx->control);

	/* Update current descriptor */
	axi_mrmac_dma_write(&rx_bd[0], &priv->mcdma_rx->current);

	/* Setup Rx BD. MRMAC needs atleast two descriptors */
	memset(&rx_bd[0], 0, sizeof(rx_bd));
	rx_bd[0].next_desc = lower_32_bits((u64)&rx_bd[1]);
	rx_bd[0].buf_addr = lower_32_bits((u64)&rxframe);

	rx_bd[1].next_desc = lower_32_bits((u64)&rx_bd[0]);
	rx_bd[1].buf_addr = lower_32_bits((u64)&rxframe[PKTSIZE_ALIGN]);

	if (IS_ENABLED(CONFIG_PHYS_64BIT)) {
		rx_bd[0].next_desc_msb = upper_32_bits((u64)&rx_bd[1]);
		rx_bd[0].buf_addr_msb = upper_32_bits((u64)&rxframe);

		rx_bd[1].next_desc_msb = upper_32_bits((u64)&rx_bd[0]);
		rx_bd[1].buf_addr_msb = upper_32_bits((u64)&rxframe[PKTSIZE_ALIGN]);
	}

	rx_bd[0].cntrl = PKTSIZE_ALIGN;
	rx_bd[1].cntrl = PKTSIZE_ALIGN;
	/* Flush the last BD so DMA core could see the updates */
	flush_cache((phys_addr_t)&rx_bd, sizeof(rx_bd));

	/* It is necessary to flush rxframe because if you don't do it
	 * then cache can contain uninitialized data
	 */
	flush_cache((phys_addr_t)&rxframe, sizeof(rxframe));

	/* Start the hardware */
	temp = readl(&priv->s2mm_cmn->control);
	temp |= XMCDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->s2mm_cmn->control);

	temp = readl(&priv->mm2s_cmn->control);
	temp |= XMCDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->mm2s_cmn->control);

	temp = readl(&priv->mcdma_rx->control);
	temp |= XMCDMA_IRQ_ALL_MASK;
	writel(temp, &priv->mcdma_rx->control);

	/* Channel fetch */
	temp = readl(&priv->mcdma_rx->control);
	temp |= XMCDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->mcdma_rx->control);

	/* Update tail descriptor. Now it's ready to receive data */
	axi_mrmac_dma_write(&rx_bd[1], &priv->mcdma_rx->tail);

	/* Enable TX */
	writel(readl(&regs->tx_config) | MRMAC_TX_EN_MASK, &regs->tx_config);

	/* Enable RX */
	writel(readl(&regs->rx_config) | MRMAC_RX_EN_MASK, &regs->rx_config);

	return 0;
}

static int axi_mrmac_send(struct udevice *dev, void *ptr, int len)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);
	u32 val;
	u32 ret;
	u32 temp;

#ifdef DEBUG
	print_buffer(ptr, ptr, 1, len, 16);
#endif
	if (len > PKTSIZE_ALIGN)
		len = PKTSIZE_ALIGN;

	/* If size is less than min packet size, pad to min size */
	if (len < MIN_PKT_SIZE) {
		memset(txminframe, 0, MIN_PKT_SIZE);
		memcpy(txminframe, ptr, len);
		len = MIN_PKT_SIZE;
		ptr = txminframe;
	}

	writel(XMCDMA_IRQ_ALL_MASK, &priv->mcdma_tx->status);

	temp = readl(&priv->mcdma_tx->control);
	temp &= ~XMCDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->mcdma_tx->control);

	/* Flush packet to main memory to be trasfered by DMA */
	flush_cache((phys_addr_t)ptr, len);

	/* Setup Tx BD. MRMAC needs atleast two descriptors */
	memset(&tx_bd[0], 0, sizeof(tx_bd));
	tx_bd[0].next_desc = lower_32_bits((u64)&tx_bd[1]);
	tx_bd[0].buf_addr = lower_32_bits((u64)ptr);

	/* At the end of the ring, link the last BD back to the top */
	tx_bd[1].next_desc = lower_32_bits((u64)&tx_bd[0]);
	tx_bd[1].buf_addr = lower_32_bits((u64)ptr + len / 2);

	if (IS_ENABLED(CONFIG_PHYS_64BIT)) {
		tx_bd[0].next_desc_msb = upper_32_bits((u64)&tx_bd[1]);
		tx_bd[0].buf_addr_msb = upper_32_bits((u64)ptr);

		tx_bd[1].next_desc_msb = upper_32_bits((u64)&tx_bd[0]);
		tx_bd[1].buf_addr_msb = upper_32_bits((u64)ptr + len / 2);
	}

	/* Split TX data in to half and send in two descriptors */
	tx_bd[0].cntrl = (len / 2) | XMCDMA_BD_CTRL_TXSOF_MASK;
	tx_bd[1].cntrl = (len - len / 2) | XMCDMA_BD_CTRL_TXEOF_MASK;

	/* Flush the last BD so DMA core could see the updates */
	flush_cache((phys_addr_t)&tx_bd, sizeof(tx_bd));

	if (readl(&priv->mcdma_tx->status) & XMCDMA_CH_IDLE) {
		axi_mrmac_dma_write(&tx_bd[0], &priv->mcdma_tx->current);
		/* Channel fetch */
		temp = readl(&priv->mcdma_tx->control);
		temp |= XMCDMA_CR_RUNSTOP_MASK;
		writel(temp, &priv->mcdma_tx->control);
	} else {
		printf("Error: current desc is not updated\n");
		return 1;
	}

	val = readl(&priv->mcdma_tx->control);
	val |= XMCDMA_IRQ_ALL_MASK;
	writel(val, &priv->mcdma_tx->control);

	/* Start transfer */
	axi_mrmac_dma_write(&tx_bd[1], &priv->mcdma_tx->tail);

	/* Wait for transmission to complete */
	ret = wait_for_bit_le32(&priv->mcdma_tx->status, XMCDMA_IRQ_IOC_MASK,
				true, 1, true);
	if (ret) {
		printf("%s: Timeout\n", __func__);
		return 1;
	}

	/* Clear status */
	tx_bd[0].sband_stats = 0;
	tx_bd[1].sband_stats = 0;

	debug("axi mrmac: Sending complete\n");

	return 0;
}

static int isrxready(struct axi_mrmac_priv *priv)
{
	u32 status;

	/* Read pending interrupts */
	status = readl(&priv->mcdma_rx->status);

	/* Acknowledge pending interrupts */
	writel(status & XMCDMA_IRQ_ALL_MASK, &priv->mcdma_rx->status);

	/*
	 * If Reception done interrupt is asserted, call RX call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
	if (status & (XMCDMA_IRQ_IOC_MASK | XMCDMA_IRQ_DELAY_MASK))
		return 1;

	return 0;
}

static int axi_mrmac_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);
	u32 length;
	u32 temp;

	/* Wait for an incoming packet */
	if (!isrxready(priv))
		return -1;

	/* Clear all interrupts */
	writel(XMCDMA_IRQ_ALL_MASK, &priv->mcdma_rx->status);

	/* Disable IRQ for a moment till packet is handled */
	temp = readl(&priv->mcdma_rx->control);
	temp &= ~XMCDMA_IRQ_ALL_MASK;
	writel(temp, &priv->mcdma_rx->control);

	/* Disable channel fetch */
	temp = readl(&priv->mcdma_rx->control);
	temp &= ~XMCDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->mcdma_rx->control);

	length = rx_bd[0].status & XMCDMA_BD_STS_ACTUAL_LEN_MASK;
	*packetp = rxframe;

	if (!length) {
		length = rx_bd[1].status & XMCDMA_BD_STS_ACTUAL_LEN_MASK;
		*packetp = &rxframe[PKTSIZE_ALIGN];
	}

#ifdef DEBUG
	print_buffer(*packetp, *packetp, 1, length, 16);
#endif

	/* Clear status */
	rx_bd[0].status = 0;
	rx_bd[1].status = 0;

	return length;
}

static int axi_mrmac_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);
	u32 temp;

#ifdef DEBUG
	/* It is useful to clear buffer to be sure that it is consistent */
	memset(rxframe, 0, sizeof(rxframe));
#endif
	/* Disable all RX interrupts before RxBD space setup */
	temp = readl(&priv->mcdma_rx->control);
	temp &= ~XMCDMA_IRQ_ALL_MASK;
	writel(temp, &priv->mcdma_rx->control);

	/* Disable channel fetch */
	temp = readl(&priv->mcdma_rx->control);
	temp &= ~XMCDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->mcdma_rx->control);

	/* Update current descriptor */
	axi_mrmac_dma_write(&rx_bd[0], &priv->mcdma_rx->current);

	/* Write bd to HW */
	flush_cache((phys_addr_t)&rx_bd, sizeof(rx_bd));

	/* It is necessary to flush rxframe because if you don't do it
	 * then cache will contain previous packet
	 */
	flush_cache((phys_addr_t)&rxframe, sizeof(rxframe));

	/* Enable all IRQ */
	temp = readl(&priv->mcdma_rx->control);
	temp |= XMCDMA_IRQ_ALL_MASK;
	writel(temp, &priv->mcdma_rx->control);

	/* Channel fetch */
	temp = readl(&priv->mcdma_rx->control);
	temp |= XMCDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->mcdma_rx->control);

	/* Update tail descriptor. Now it's ready to receive data */
	axi_mrmac_dma_write(&rx_bd[1], &priv->mcdma_rx->tail);

	debug("axi mrmac: RX completed, framelength = %x\n", length);

	return 0;
}

/* STOP DMA transfers */
static void axi_mrmac_stop(struct udevice *dev)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);
	u32 temp;

	/* Stop the hardware */
	temp = readl(&priv->mcdma_tx->control);
	temp &= ~XMCDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->mcdma_tx->control);

	temp = readl(&priv->mcdma_rx->control);
	temp &= ~XMCDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->mcdma_rx->control);

	debug("%s: Halted\n", __func__);
}

static int axi_mrmac_probe(struct udevice *dev)
{
	struct axi_mrmac_plat *plat = dev_get_platdata(dev);
	struct eth_pdata *pdata = &plat->eth_pdata;
	struct axi_mrmac_priv *priv = dev_get_priv(dev);

	priv->iobase = (struct mrmac_regs *)pdata->iobase;

	priv->mm2s_cmn = plat->mm2s_cmn;
	priv->mcdma_tx = (struct mcdma_chan_reg *)((phys_addr_t)priv->mm2s_cmn
						   + XMCDMA_CHAN_OFFSET);
	priv->s2mm_cmn = (struct mcdma_common_regs *)((phys_addr_t)priv->mm2s_cmn
						      + XMCDMA_RX_OFFSET);
	priv->mcdma_rx = (struct mcdma_chan_reg *)((phys_addr_t)priv->s2mm_cmn
						   + XMCDMA_CHAN_OFFSET);
	priv->mrmac_rate = plat->mrmac_rate;

	return 0;
}

static int axi_mrmac_remove(struct udevice *dev)
{
	return 0;
}

static int axi_mrmac_ofdata_to_platdata(struct udevice *dev)
{
	struct axi_mrmac_plat *plat = dev_get_platdata(dev);
	struct eth_pdata *pdata = &plat->eth_pdata;
	struct ofnode_phandle_args phandle_args;
	int ret = 0;

	pdata->iobase = dev_read_addr(dev);

	ret = dev_read_phandle_with_args(dev, "axistream-connected", NULL, 0, 0,
					 &phandle_args);
	if (ret) {
		printf("%s: axistream not found\n", __func__);
		return -EINVAL;
	}

	plat->mm2s_cmn = (struct mcdma_common_regs *)ofnode_read_u64_default
						(phandle_args.node, "reg", -1);
	if (!plat->mm2s_cmn) {
		printf("%s: MRMAC dma register space not found\n", __func__);
		return -EINVAL;
	}

	/* Set default MRMAC rate to 10000 */
	plat->mrmac_rate = dev_read_u32_default(dev, "xlnx,mrmac-rate", 10000);

	return 0;
}

static const struct eth_ops axi_mrmac_ops = {
	.start			= axi_mrmac_start,
	.send			= axi_mrmac_send,
	.recv			= axi_mrmac_recv,
	.free_pkt		= axi_mrmac_free_pkt,
	.stop			= axi_mrmac_stop,
};

static const struct udevice_id axi_mrmac_ids[] = {
	{ .compatible = "xlnx,mrmac-ethernet-1.0" },
	{ }
};

U_BOOT_DRIVER(axi_mrmac) = {
	.name   = "axi_mrmac",
	.id     = UCLASS_ETH,
	.of_match = axi_mrmac_ids,
	.ofdata_to_platdata = axi_mrmac_ofdata_to_platdata,
	.probe  = axi_mrmac_probe,
	.remove = axi_mrmac_remove,
	.ops    = &axi_mrmac_ops,
	.priv_auto_alloc_size = sizeof(struct axi_mrmac_priv),
	.platdata_auto_alloc_size = sizeof(struct axi_mrmac_plat),
};
