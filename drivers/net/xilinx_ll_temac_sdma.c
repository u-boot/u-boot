/*
 * Xilinx xps_ll_temac ethernet driver for u-boot
 *
 * SDMA sub-controller
 *
 * Copyright (C) 2011 - 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (C) 2008 - 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008 - 2011 PetaLogix
 *
 * Based on Yoshio Kashiwagi kashiwagi@co-nss.co.jp driver
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * CREDITS: tsec driver
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [M]:	[0]/ip_documentation/mpmc.pdf
 * [S]:	[0]/ip_documentation/xps_ll_temac.pdf
 * [A]:	[0]/application_notes/xapp1041.pdf
 */

#include <config.h>
#include <common.h>
#include <net.h>

#include <asm/types.h>
#include <asm/io.h>

#include "xilinx_ll_temac.h"
#include "xilinx_ll_temac_sdma.h"

#define TX_BUF_CNT		2

static unsigned int rx_idx;	/* index of the current RX buffer */
static unsigned int tx_idx;	/* index of the current TX buffer */

struct rtx_cdmac_bd {
	struct cdmac_bd rx[PKTBUFSRX];
	struct cdmac_bd tx[TX_BUF_CNT];
};

/*
 * DMA Buffer Descriptor alignment
 *
 * If the address contained in the Next Descriptor Pointer register is not
 * 8-word aligned or reaches beyond the range of available memory, the SDMA
 * halts processing and sets the CDMAC_BD_STCTRL_ERROR bit in the respective
 * status register (tx_chnl_sts or rx_chnl_sts).
 *
 * [1]: [0]/ip_documentation/mpmc.pdf
 *      page 161, Next Descriptor Pointer
 */
static struct rtx_cdmac_bd cdmac_bd __aligned(32);

#if defined(CONFIG_XILINX_440) || defined(CONFIG_XILINX_405)

/*
 * Indirect DCR access operations mi{ft}dcr_xilinx() espacialy
 * for Xilinx PowerPC implementations on FPGA.
 *
 * FIXME: This part should go up to arch/powerpc -- but where?
 */
#include <asm/processor.h>
#define XILINX_INDIRECT_DCR_ADDRESS_REG	0
#define XILINX_INDIRECT_DCR_ACCESS_REG	1
inline unsigned mifdcr_xilinx(const unsigned dcrn)
{
	mtdcr(XILINX_INDIRECT_DCR_ADDRESS_REG, dcrn);
	return mfdcr(XILINX_INDIRECT_DCR_ACCESS_REG);
}
inline void mitdcr_xilinx(const unsigned dcrn, int val)
{
	mtdcr(XILINX_INDIRECT_DCR_ADDRESS_REG, dcrn);
	mtdcr(XILINX_INDIRECT_DCR_ACCESS_REG, val);
}

/* Xilinx Device Control Register (DCR) in/out accessors */
inline unsigned ll_temac_xldcr_in32(phys_addr_t addr)
{
	return mifdcr_xilinx((const unsigned)addr);
}
inline void ll_temac_xldcr_out32(phys_addr_t addr, unsigned value)
{
	mitdcr_xilinx((const unsigned)addr, value);
}

void ll_temac_collect_xldcr_sdma_reg_addr(struct eth_device *dev)
{
	struct ll_temac *ll_temac = dev->priv;
	phys_addr_t dmac_ctrl = ll_temac->ctrladdr;
	phys_addr_t *ra = ll_temac->sdma_reg_addr;

	ra[TX_NXTDESC_PTR]   = dmac_ctrl + TX_NXTDESC_PTR;
	ra[TX_CURBUF_ADDR]   = dmac_ctrl + TX_CURBUF_ADDR;
	ra[TX_CURBUF_LENGTH] = dmac_ctrl + TX_CURBUF_LENGTH;
	ra[TX_CURDESC_PTR]   = dmac_ctrl + TX_CURDESC_PTR;
	ra[TX_TAILDESC_PTR]  = dmac_ctrl + TX_TAILDESC_PTR;
	ra[TX_CHNL_CTRL]     = dmac_ctrl + TX_CHNL_CTRL;
	ra[TX_IRQ_REG]       = dmac_ctrl + TX_IRQ_REG;
	ra[TX_CHNL_STS]      = dmac_ctrl + TX_CHNL_STS;
	ra[RX_NXTDESC_PTR]   = dmac_ctrl + RX_NXTDESC_PTR;
	ra[RX_CURBUF_ADDR]   = dmac_ctrl + RX_CURBUF_ADDR;
	ra[RX_CURBUF_LENGTH] = dmac_ctrl + RX_CURBUF_LENGTH;
	ra[RX_CURDESC_PTR]   = dmac_ctrl + RX_CURDESC_PTR;
	ra[RX_TAILDESC_PTR]  = dmac_ctrl + RX_TAILDESC_PTR;
	ra[RX_CHNL_CTRL]     = dmac_ctrl + RX_CHNL_CTRL;
	ra[RX_IRQ_REG]       = dmac_ctrl + RX_IRQ_REG;
	ra[RX_CHNL_STS]      = dmac_ctrl + RX_CHNL_STS;
	ra[DMA_CONTROL_REG]  = dmac_ctrl + DMA_CONTROL_REG;
}

#endif /* CONFIG_XILINX_440 || ONFIG_XILINX_405 */

/* Xilinx Processor Local Bus (PLB) in/out accessors */
inline unsigned ll_temac_xlplb_in32(phys_addr_t addr)
{
	return in_be32((void *)addr);
}
inline void ll_temac_xlplb_out32(phys_addr_t addr, unsigned value)
{
	out_be32((void *)addr, value);
}

/* collect all register addresses for Xilinx PLB in/out accessors */
void ll_temac_collect_xlplb_sdma_reg_addr(struct eth_device *dev)
{
	struct ll_temac *ll_temac = dev->priv;
	struct sdma_ctrl *sdma_ctrl = (void *)ll_temac->ctrladdr;
	phys_addr_t *ra = ll_temac->sdma_reg_addr;

	ra[TX_NXTDESC_PTR]   = (phys_addr_t)&sdma_ctrl->tx_nxtdesc_ptr;
	ra[TX_CURBUF_ADDR]   = (phys_addr_t)&sdma_ctrl->tx_curbuf_addr;
	ra[TX_CURBUF_LENGTH] = (phys_addr_t)&sdma_ctrl->tx_curbuf_length;
	ra[TX_CURDESC_PTR]   = (phys_addr_t)&sdma_ctrl->tx_curdesc_ptr;
	ra[TX_TAILDESC_PTR]  = (phys_addr_t)&sdma_ctrl->tx_taildesc_ptr;
	ra[TX_CHNL_CTRL]     = (phys_addr_t)&sdma_ctrl->tx_chnl_ctrl;
	ra[TX_IRQ_REG]       = (phys_addr_t)&sdma_ctrl->tx_irq_reg;
	ra[TX_CHNL_STS]      = (phys_addr_t)&sdma_ctrl->tx_chnl_sts;
	ra[RX_NXTDESC_PTR]   = (phys_addr_t)&sdma_ctrl->rx_nxtdesc_ptr;
	ra[RX_CURBUF_ADDR]   = (phys_addr_t)&sdma_ctrl->rx_curbuf_addr;
	ra[RX_CURBUF_LENGTH] = (phys_addr_t)&sdma_ctrl->rx_curbuf_length;
	ra[RX_CURDESC_PTR]   = (phys_addr_t)&sdma_ctrl->rx_curdesc_ptr;
	ra[RX_TAILDESC_PTR]  = (phys_addr_t)&sdma_ctrl->rx_taildesc_ptr;
	ra[RX_CHNL_CTRL]     = (phys_addr_t)&sdma_ctrl->rx_chnl_ctrl;
	ra[RX_IRQ_REG]       = (phys_addr_t)&sdma_ctrl->rx_irq_reg;
	ra[RX_CHNL_STS]      = (phys_addr_t)&sdma_ctrl->rx_chnl_sts;
	ra[DMA_CONTROL_REG]  = (phys_addr_t)&sdma_ctrl->dma_control_reg;
}

/* Check for TX and RX channel errors. */
static inline int ll_temac_sdma_error(struct eth_device *dev)
{
	int err;
	struct ll_temac *ll_temac = dev->priv;
	phys_addr_t *ra = ll_temac->sdma_reg_addr;

	err = ll_temac->in32(ra[TX_CHNL_STS]) & CHNL_STS_ERROR;
	err |= ll_temac->in32(ra[RX_CHNL_STS]) & CHNL_STS_ERROR;

	return err;
}

int ll_temac_init_sdma(struct eth_device *dev)
{
	struct ll_temac *ll_temac = dev->priv;
	struct cdmac_bd *rx_dp;
	struct cdmac_bd *tx_dp;
	phys_addr_t *ra = ll_temac->sdma_reg_addr;
	int i;

	printf("%s: SDMA: %d Rx buffers, %d Tx buffers\n",
			dev->name, PKTBUFSRX, TX_BUF_CNT);

	/* Initialize the Rx Buffer descriptors */
	for (i = 0; i < PKTBUFSRX; i++) {
		rx_dp = &cdmac_bd.rx[i];
		memset(rx_dp, 0, sizeof(*rx_dp));
		rx_dp->next_p = rx_dp;
		rx_dp->buf_len = PKTSIZE_ALIGN;
		rx_dp->phys_buf_p = (u8 *)NetRxPackets[i];
		flush_cache((u32)rx_dp->phys_buf_p, PKTSIZE_ALIGN);
	}
	flush_cache((u32)cdmac_bd.rx, sizeof(cdmac_bd.rx));

	/* Initialize the TX Buffer Descriptors */
	for (i = 0; i < TX_BUF_CNT; i++) {
		tx_dp = &cdmac_bd.tx[i];
		memset(tx_dp, 0, sizeof(*tx_dp));
		tx_dp->next_p = tx_dp;
	}
	flush_cache((u32)cdmac_bd.tx, sizeof(cdmac_bd.tx));

	/* Reset index counter to the Rx and Tx Buffer descriptors */
	rx_idx = tx_idx = 0;

	/* initial Rx DMA start by writing to respective TAILDESC_PTR */
	ll_temac->out32(ra[RX_CURDESC_PTR], (int)&cdmac_bd.rx[rx_idx]);
	ll_temac->out32(ra[RX_TAILDESC_PTR], (int)&cdmac_bd.rx[rx_idx]);

	return 0;
}

int ll_temac_halt_sdma(struct eth_device *dev)
{
	unsigned timeout = 50;	/* 1usec * 50 = 50usec */
	struct ll_temac *ll_temac = dev->priv;
	phys_addr_t *ra = ll_temac->sdma_reg_addr;

	/*
	 * Soft reset the DMA
	 *
	 * Quote from MPMC documentation: Writing a 1 to this field
	 * forces the DMA engine to shutdown and reset itself. After
	 * setting this bit, software must poll it until the bit is
	 * cleared by the DMA. This indicates that the reset process
	 * is done and the pipeline has been flushed.
	 */
	ll_temac->out32(ra[DMA_CONTROL_REG], DMA_CONTROL_RESET);
	while (timeout && (ll_temac->in32(ra[DMA_CONTROL_REG])
					& DMA_CONTROL_RESET)) {
		timeout--;
		udelay(1);
	}

	if (!timeout) {
		printf("%s: Timeout\n", __func__);
		return -1;
	}

	return 0;
}

int ll_temac_reset_sdma(struct eth_device *dev)
{
	u32 r;
	struct ll_temac *ll_temac = dev->priv;
	phys_addr_t *ra = ll_temac->sdma_reg_addr;

	/* Soft reset the DMA.  */
	if (ll_temac_halt_sdma(dev))
		return -1;

	/* Now clear the interrupts.  */
	r = ll_temac->in32(ra[TX_CHNL_CTRL]);
	r &= ~CHNL_CTRL_IRQ_MASK;
	ll_temac->out32(ra[TX_CHNL_CTRL], r);

	r = ll_temac->in32(ra[RX_CHNL_CTRL]);
	r &= ~CHNL_CTRL_IRQ_MASK;
	ll_temac->out32(ra[RX_CHNL_CTRL], r);

	/* Now ACK pending IRQs.  */
	ll_temac->out32(ra[TX_IRQ_REG], IRQ_REG_IRQ_MASK);
	ll_temac->out32(ra[RX_IRQ_REG], IRQ_REG_IRQ_MASK);

	/* Set tail-ptr mode, disable errors for both channels.  */
	ll_temac->out32(ra[DMA_CONTROL_REG],
			/* Enable use of tail pointer register */
			DMA_CONTROL_TPE |
			/* Disable error when 2 or 4 bit coalesce cnt overfl */
			DMA_CONTROL_RXOCEID |
			/* Disable error when 2 or 4 bit coalesce cnt overfl */
			DMA_CONTROL_TXOCEID);

	return 0;
}

int ll_temac_recv_sdma(struct eth_device *dev)
{
	int length, pb_idx;
	struct cdmac_bd *rx_dp = &cdmac_bd.rx[rx_idx];
	struct ll_temac *ll_temac = dev->priv;
	phys_addr_t *ra = ll_temac->sdma_reg_addr;

	if (ll_temac_sdma_error(dev)) {

		if (ll_temac_reset_sdma(dev))
			return -1;

		ll_temac_init_sdma(dev);
	}

	flush_cache((u32)rx_dp, sizeof(*rx_dp));

	if (!(rx_dp->sca.stctrl & CDMAC_BD_STCTRL_COMPLETED))
		return 0;

	if (rx_dp->sca.stctrl & (CDMAC_BD_STCTRL_SOP | CDMAC_BD_STCTRL_EOP)) {
		pb_idx = rx_idx;
		length = rx_dp->sca.app[4] & CDMAC_BD_APP4_RXBYTECNT_MASK;
	} else {
		pb_idx = -1;
		length = 0;
		printf("%s: Got part of package, unsupported (%x)\n",
				__func__, rx_dp->sca.stctrl);
	}

	/* flip the buffer */
	flush_cache((u32)rx_dp->phys_buf_p, length);

	/* reset the current descriptor */
	rx_dp->sca.stctrl = 0;
	rx_dp->sca.app[4] = 0;
	flush_cache((u32)rx_dp, sizeof(*rx_dp));

	/* Find next empty buffer descriptor, preparation for next iteration */
	rx_idx = (rx_idx + 1) % PKTBUFSRX;
	rx_dp = &cdmac_bd.rx[rx_idx];
	flush_cache((u32)rx_dp, sizeof(*rx_dp));

	/* DMA start by writing to respective TAILDESC_PTR */
	ll_temac->out32(ra[RX_CURDESC_PTR], (int)&cdmac_bd.rx[rx_idx]);
	ll_temac->out32(ra[RX_TAILDESC_PTR], (int)&cdmac_bd.rx[rx_idx]);

	if (length > 0 && pb_idx != -1)
		NetReceive(NetRxPackets[pb_idx], length);

	return 0;
}

int ll_temac_send_sdma(struct eth_device *dev, volatile void *packet,
							int length)
{
	unsigned timeout = 50;	/* 1usec * 50 = 50usec */
	struct cdmac_bd *tx_dp = &cdmac_bd.tx[tx_idx];
	struct ll_temac *ll_temac = dev->priv;
	phys_addr_t *ra = ll_temac->sdma_reg_addr;

	if (ll_temac_sdma_error(dev)) {

		if (ll_temac_reset_sdma(dev))
			return -1;

		ll_temac_init_sdma(dev);
	}

	tx_dp->phys_buf_p = (u8 *)packet;
	tx_dp->buf_len = length;
	tx_dp->sca.stctrl = CDMAC_BD_STCTRL_SOP | CDMAC_BD_STCTRL_EOP |
			CDMAC_BD_STCTRL_STOP_ON_END;

	flush_cache((u32)packet, length);
	flush_cache((u32)tx_dp, sizeof(*tx_dp));

	/* DMA start by writing to respective TAILDESC_PTR */
	ll_temac->out32(ra[TX_CURDESC_PTR], (int)tx_dp);
	ll_temac->out32(ra[TX_TAILDESC_PTR], (int)tx_dp);

	/* Find next empty buffer descriptor, preparation for next iteration */
	tx_idx = (tx_idx + 1) % TX_BUF_CNT;
	tx_dp = &cdmac_bd.tx[tx_idx];

	do {
		flush_cache((u32)tx_dp, sizeof(*tx_dp));
		udelay(1);
	} while (timeout-- && !(tx_dp->sca.stctrl & CDMAC_BD_STCTRL_COMPLETED));

	if (!timeout) {
		printf("%s: Timeout\n", __func__);
		return -1;
	}

	return 0;
}
