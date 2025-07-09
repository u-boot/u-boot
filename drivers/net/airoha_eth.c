// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Based on Linux airoha_eth.c majorly rewritten
 * and simplified for U-Boot usage for single TX/RX ring.
 *
 * Copyright (c) 2024 AIROHA Inc
 * Author: Lorenzo Bianconi <lorenzo@kernel.org>
 *         Christian Marangi <ansuelsmth@gmail.org>
 */

#include <dm.h>
#include <dm/devres.h>
#include <mapmem.h>
#include <net.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/time.h>

#define AIROHA_MAX_NUM_GDM_PORTS	1
#define AIROHA_MAX_NUM_QDMA		1
#define AIROHA_MAX_NUM_RSTS		3
#define AIROHA_MAX_NUM_XSI_RSTS		4

#define AIROHA_MAX_PACKET_SIZE		2048
#define AIROHA_NUM_TX_RING		1
#define AIROHA_NUM_RX_RING		1
#define AIROHA_NUM_TX_IRQ		1
#define HW_DSCP_NUM			32
#define IRQ_QUEUE_LEN			1
#define TX_DSCP_NUM			16
#define RX_DSCP_NUM			PKTBUFSRX

/* SCU */
#define SCU_SHARE_FEMEM_SEL		0x958

/* SWITCH */
#define SWITCH_MFC			0x10
#define   SWITCH_BC_FFP			GENMASK(31, 24)
#define   SWITCH_UNM_FFP		GENMASK(23, 16)
#define   SWITCH_UNU_FFP		GENMASK(15, 8)
#define SWITCH_PMCR(_n)			0x3000 + ((_n) * 0x100)
#define   SWITCH_IPG_CFG		GENMASK(19, 18)
#define     SWITCH_IPG_CFG_NORMAL	FIELD_PREP(SWITCH_IPG_CFG, 0x0)
#define     SWITCH_IPG_CFG_SHORT	FIELD_PREP(SWITCH_IPG_CFG, 0x1)
#define     SWITCH_IPG_CFG_SHRINK	FIELD_PREP(SWITCH_IPG_CFG, 0x2)
#define   SWITCH_MAC_MODE		BIT(16)
#define   SWITCH_FORCE_MODE		BIT(15)
#define   SWITCH_MAC_TX_EN		BIT(14)
#define   SWITCH_MAC_RX_EN		BIT(13)
#define   SWITCH_BKOFF_EN		BIT(9)
#define   SWITCH_BKPR_EN		BIT(8)
#define   SWITCH_FORCE_RX_FC		BIT(5)
#define   SWITCH_FORCE_TX_FC		BIT(4)
#define   SWITCH_FORCE_SPD		GENMASK(3, 2)
#define     SWITCH_FORCE_SPD_10		FIELD_PREP(SWITCH_FORCE_SPD, 0x0)
#define     SWITCH_FORCE_SPD_100	FIELD_PREP(SWITCH_FORCE_SPD, 0x1)
#define     SWITCH_FORCE_SPD_1000	FIELD_PREP(SWITCH_FORCE_SPD, 0x2)
#define   SWITCH_FORCE_DPX		BIT(1)
#define   SWITCH_FORCE_LNK		BIT(0)
#define SWITCH_SMACCR0			0x30e4
#define   SMACCR0_MAC2			GENMASK(31, 24)
#define   SMACCR0_MAC3			GENMASK(23, 16)
#define   SMACCR0_MAC4			GENMASK(15, 8)
#define   SMACCR0_MAC5			GENMASK(7, 0)
#define SWITCH_SMACCR1			0x30e8
#define   SMACCR1_MAC0			GENMASK(15, 8)
#define   SMACCR1_MAC1			GENMASK(7, 0)
#define SWITCH_PHY_POLL			0x7018
#define   SWITCH_PHY_AP_EN		GENMASK(30, 24)
#define   SWITCH_EEE_POLL_EN		GENMASK(22, 16)
#define   SWITCH_PHY_PRE_EN		BIT(15)
#define   SWITCH_PHY_END_ADDR		GENMASK(12, 8)
#define   SWITCH_PHY_ST_ADDR		GENMASK(4, 0)

/* FE */
#define PSE_BASE			0x0100
#define CSR_IFC_BASE			0x0200
#define CDM1_BASE			0x0400
#define GDM1_BASE			0x0500
#define PPE1_BASE			0x0c00

#define CDM2_BASE			0x1400
#define GDM2_BASE			0x1500

#define GDM3_BASE			0x1100
#define GDM4_BASE			0x2500

#define GDM_BASE(_n)			\
	((_n) == 4 ? GDM4_BASE :	\
	 (_n) == 3 ? GDM3_BASE :	\
	 (_n) == 2 ? GDM2_BASE : GDM1_BASE)

#define REG_GDM_FWD_CFG(_n)		GDM_BASE(_n)
#define GDM_PAD_EN			BIT(28)
#define GDM_DROP_CRC_ERR		BIT(23)
#define GDM_IP4_CKSUM			BIT(22)
#define GDM_TCP_CKSUM			BIT(21)
#define GDM_UDP_CKSUM			BIT(20)
#define GDM_UCFQ_MASK			GENMASK(15, 12)
#define GDM_BCFQ_MASK			GENMASK(11, 8)
#define GDM_MCFQ_MASK			GENMASK(7, 4)
#define GDM_OCFQ_MASK			GENMASK(3, 0)

/* QDMA */
#define REG_QDMA_GLOBAL_CFG			0x0004
#define GLOBAL_CFG_RX_2B_OFFSET_MASK		BIT(31)
#define GLOBAL_CFG_DMA_PREFERENCE_MASK		GENMASK(30, 29)
#define GLOBAL_CFG_CPU_TXR_RR_MASK		BIT(28)
#define GLOBAL_CFG_DSCP_BYTE_SWAP_MASK		BIT(27)
#define GLOBAL_CFG_PAYLOAD_BYTE_SWAP_MASK	BIT(26)
#define GLOBAL_CFG_MULTICAST_MODIFY_FP_MASK	BIT(25)
#define GLOBAL_CFG_OAM_MODIFY_MASK		BIT(24)
#define GLOBAL_CFG_RESET_MASK			BIT(23)
#define GLOBAL_CFG_RESET_DONE_MASK		BIT(22)
#define GLOBAL_CFG_MULTICAST_EN_MASK		BIT(21)
#define GLOBAL_CFG_IRQ1_EN_MASK			BIT(20)
#define GLOBAL_CFG_IRQ0_EN_MASK			BIT(19)
#define GLOBAL_CFG_LOOPCNT_EN_MASK		BIT(18)
#define GLOBAL_CFG_RD_BYPASS_WR_MASK		BIT(17)
#define GLOBAL_CFG_QDMA_LOOPBACK_MASK		BIT(16)
#define GLOBAL_CFG_LPBK_RXQ_SEL_MASK		GENMASK(13, 8)
#define GLOBAL_CFG_CHECK_DONE_MASK		BIT(7)
#define GLOBAL_CFG_TX_WB_DONE_MASK		BIT(6)
#define GLOBAL_CFG_MAX_ISSUE_NUM_MASK		GENMASK(5, 4)
#define GLOBAL_CFG_RX_DMA_BUSY_MASK		BIT(3)
#define GLOBAL_CFG_RX_DMA_EN_MASK		BIT(2)
#define GLOBAL_CFG_TX_DMA_BUSY_MASK		BIT(1)
#define GLOBAL_CFG_TX_DMA_EN_MASK		BIT(0)

#define REG_FWD_DSCP_BASE			0x0010
#define REG_FWD_BUF_BASE			0x0014

#define REG_HW_FWD_DSCP_CFG			0x0018
#define HW_FWD_DSCP_PAYLOAD_SIZE_MASK		GENMASK(29, 28)
#define HW_FWD_DSCP_SCATTER_LEN_MASK		GENMASK(17, 16)
#define HW_FWD_DSCP_MIN_SCATTER_LEN_MASK	GENMASK(15, 0)

#define REG_INT_STATUS(_n)		\
	(((_n) == 4) ? 0x0730 :		\
	 ((_n) == 3) ? 0x0724 :		\
	 ((_n) == 2) ? 0x0720 :		\
	 ((_n) == 1) ? 0x0024 : 0x0020)

#define REG_TX_IRQ_BASE(_n)		((_n) ? 0x0048 : 0x0050)

#define REG_TX_IRQ_CFG(_n)		((_n) ? 0x004c : 0x0054)
#define TX_IRQ_THR_MASK			GENMASK(27, 16)
#define TX_IRQ_DEPTH_MASK		GENMASK(11, 0)

#define REG_IRQ_CLEAR_LEN(_n)		((_n) ? 0x0064 : 0x0058)
#define IRQ_CLEAR_LEN_MASK		GENMASK(7, 0)

#define REG_TX_RING_BASE(_n)	\
	(((_n) < 8) ? 0x0100 + ((_n) << 5) : 0x0b00 + (((_n) - 8) << 5))

#define REG_TX_CPU_IDX(_n)	\
	(((_n) < 8) ? 0x0108 + ((_n) << 5) : 0x0b08 + (((_n) - 8) << 5))

#define TX_RING_CPU_IDX_MASK		GENMASK(15, 0)

#define REG_TX_DMA_IDX(_n)	\
	(((_n) < 8) ? 0x010c + ((_n) << 5) : 0x0b0c + (((_n) - 8) << 5))

#define TX_RING_DMA_IDX_MASK		GENMASK(15, 0)

#define IRQ_RING_IDX_MASK		GENMASK(20, 16)
#define IRQ_DESC_IDX_MASK		GENMASK(15, 0)

#define REG_RX_RING_BASE(_n)	\
	(((_n) < 16) ? 0x0200 + ((_n) << 5) : 0x0e00 + (((_n) - 16) << 5))

#define REG_RX_RING_SIZE(_n)	\
	(((_n) < 16) ? 0x0204 + ((_n) << 5) : 0x0e04 + (((_n) - 16) << 5))

#define RX_RING_THR_MASK		GENMASK(31, 16)
#define RX_RING_SIZE_MASK		GENMASK(15, 0)

#define REG_RX_CPU_IDX(_n)	\
	(((_n) < 16) ? 0x0208 + ((_n) << 5) : 0x0e08 + (((_n) - 16) << 5))

#define RX_RING_CPU_IDX_MASK		GENMASK(15, 0)

#define REG_RX_DMA_IDX(_n)	\
	(((_n) < 16) ? 0x020c + ((_n) << 5) : 0x0e0c + (((_n) - 16) << 5))

#define REG_RX_DELAY_INT_IDX(_n)	\
	(((_n) < 16) ? 0x0210 + ((_n) << 5) : 0x0e10 + (((_n) - 16) << 5))

#define RX_DELAY_INT_MASK		GENMASK(15, 0)

#define RX_RING_DMA_IDX_MASK		GENMASK(15, 0)

#define REG_LMGR_INIT_CFG		0x1000
#define LMGR_INIT_START			BIT(31)
#define LMGR_SRAM_MODE_MASK		BIT(30)
#define HW_FWD_PKTSIZE_OVERHEAD_MASK	GENMASK(27, 20)
#define HW_FWD_DESC_NUM_MASK		GENMASK(16, 0)

/* CTRL */
#define QDMA_DESC_DONE_MASK		BIT(31)
#define QDMA_DESC_DROP_MASK		BIT(30) /* tx: drop - rx: overflow */
#define QDMA_DESC_MORE_MASK		BIT(29) /* more SG elements */
#define QDMA_DESC_DEI_MASK		BIT(25)
#define QDMA_DESC_NO_DROP_MASK		BIT(24)
#define QDMA_DESC_LEN_MASK		GENMASK(15, 0)
/* DATA */
#define QDMA_DESC_NEXT_ID_MASK		GENMASK(15, 0)
/* TX MSG0 */
#define QDMA_ETH_TXMSG_MIC_IDX_MASK	BIT(30)
#define QDMA_ETH_TXMSG_SP_TAG_MASK	GENMASK(29, 14)
#define QDMA_ETH_TXMSG_ICO_MASK		BIT(13)
#define QDMA_ETH_TXMSG_UCO_MASK		BIT(12)
#define QDMA_ETH_TXMSG_TCO_MASK		BIT(11)
#define QDMA_ETH_TXMSG_TSO_MASK		BIT(10)
#define QDMA_ETH_TXMSG_FAST_MASK	BIT(9)
#define QDMA_ETH_TXMSG_OAM_MASK		BIT(8)
#define QDMA_ETH_TXMSG_CHAN_MASK	GENMASK(7, 3)
#define QDMA_ETH_TXMSG_QUEUE_MASK	GENMASK(2, 0)
/* TX MSG1 */
#define QDMA_ETH_TXMSG_NO_DROP		BIT(31)
#define QDMA_ETH_TXMSG_METER_MASK	GENMASK(30, 24)	/* 0x7f no meters */
#define QDMA_ETH_TXMSG_FPORT_MASK	GENMASK(23, 20)
#define QDMA_ETH_TXMSG_NBOQ_MASK	GENMASK(19, 15)
#define QDMA_ETH_TXMSG_HWF_MASK		BIT(14)
#define QDMA_ETH_TXMSG_HOP_MASK		BIT(13)
#define QDMA_ETH_TXMSG_PTP_MASK		BIT(12)
#define QDMA_ETH_TXMSG_ACNT_G1_MASK	GENMASK(10, 6)	/* 0x1f do not count */
#define QDMA_ETH_TXMSG_ACNT_G0_MASK	GENMASK(5, 0)	/* 0x3f do not count */

/* RX MSG1 */
#define QDMA_ETH_RXMSG_DEI_MASK		BIT(31)
#define QDMA_ETH_RXMSG_IP6_MASK		BIT(30)
#define QDMA_ETH_RXMSG_IP4_MASK		BIT(29)
#define QDMA_ETH_RXMSG_IP4F_MASK	BIT(28)
#define QDMA_ETH_RXMSG_L4_VALID_MASK	BIT(27)
#define QDMA_ETH_RXMSG_L4F_MASK		BIT(26)
#define QDMA_ETH_RXMSG_SPORT_MASK	GENMASK(25, 21)
#define QDMA_ETH_RXMSG_CRSN_MASK	GENMASK(20, 16)
#define QDMA_ETH_RXMSG_PPE_ENTRY_MASK	GENMASK(15, 0)

struct airoha_qdma_desc {
	__le32 rsv;
	__le32 ctrl;
	__le32 addr;
	__le32 data;
	__le32 msg0;
	__le32 msg1;
	__le32 msg2;
	__le32 msg3;
};

struct airoha_qdma_fwd_desc {
	__le32 addr;
	__le32 ctrl0;
	__le32 ctrl1;
	__le32 ctrl2;
	__le32 msg0;
	__le32 msg1;
	__le32 rsv0;
	__le32 rsv1;
};

struct airoha_queue {
	struct airoha_qdma_desc *desc;
	u16 head;

	int ndesc;
};

struct airoha_tx_irq_queue {
	struct airoha_qdma *qdma;

	int size;
	u32 *q;
};

struct airoha_qdma {
	struct airoha_eth *eth;
	void __iomem *regs;

	struct airoha_tx_irq_queue q_tx_irq[AIROHA_NUM_TX_IRQ];

	struct airoha_queue q_tx[AIROHA_NUM_TX_RING];
	struct airoha_queue q_rx[AIROHA_NUM_RX_RING];

	/* descriptor and packet buffers for qdma hw forward */
	struct {
		void *desc;
		void *q;
	} hfwd;
};

struct airoha_gdm_port {
	struct airoha_qdma *qdma;
	int id;
};

struct airoha_eth {
	void __iomem *fe_regs;
	void __iomem *switch_regs;

	struct reset_ctl_bulk rsts;
	struct reset_ctl_bulk xsi_rsts;

	struct airoha_qdma qdma[AIROHA_MAX_NUM_QDMA];
	struct airoha_gdm_port *ports[AIROHA_MAX_NUM_GDM_PORTS];
};

static u32 airoha_rr(void __iomem *base, u32 offset)
{
	return readl(base + offset);
}

static void airoha_wr(void __iomem *base, u32 offset, u32 val)
{
	writel(val, base + offset);
}

static u32 airoha_rmw(void __iomem *base, u32 offset, u32 mask, u32 val)
{
	val |= (airoha_rr(base, offset) & ~mask);
	airoha_wr(base, offset, val);

	return val;
}

#define airoha_fe_rr(eth, offset)				\
	airoha_rr((eth)->fe_regs, (offset))
#define airoha_fe_wr(eth, offset, val)				\
	airoha_wr((eth)->fe_regs, (offset), (val))
#define airoha_fe_rmw(eth, offset, mask, val)			\
	airoha_rmw((eth)->fe_regs, (offset), (mask), (val))
#define airoha_fe_set(eth, offset, val)				\
	airoha_rmw((eth)->fe_regs, (offset), 0, (val))
#define airoha_fe_clear(eth, offset, val)			\
	airoha_rmw((eth)->fe_regs, (offset), (val), 0)

#define airoha_qdma_rr(qdma, offset)				\
	airoha_rr((qdma)->regs, (offset))
#define airoha_qdma_wr(qdma, offset, val)			\
	airoha_wr((qdma)->regs, (offset), (val))
#define airoha_qdma_rmw(qdma, offset, mask, val)		\
	airoha_rmw((qdma)->regs, (offset), (mask), (val))
#define airoha_qdma_set(qdma, offset, val)			\
	airoha_rmw((qdma)->regs, (offset), 0, (val))
#define airoha_qdma_clear(qdma, offset, val)			\
	airoha_rmw((qdma)->regs, (offset), (val), 0)

#define airoha_switch_wr(eth, offset, val)			\
	airoha_wr((eth)->switch_regs, (offset), (val))

static inline dma_addr_t dma_map_unaligned(void *vaddr, size_t len,
					   enum dma_data_direction dir)
{
	uintptr_t start, end;

	start = ALIGN_DOWN((uintptr_t)vaddr, ARCH_DMA_MINALIGN);
	end = ALIGN((uintptr_t)(vaddr + len), ARCH_DMA_MINALIGN);

	return dma_map_single((void *)start, end - start, dir);
}

static inline void dma_unmap_unaligned(dma_addr_t addr, size_t len,
				       enum dma_data_direction dir)
{
	uintptr_t start, end;

	start = ALIGN_DOWN((uintptr_t)addr, ARCH_DMA_MINALIGN);
	end = ALIGN((uintptr_t)(addr + len), ARCH_DMA_MINALIGN);
	dma_unmap_single(start, end - start, dir);
}

static void airoha_fe_maccr_init(struct airoha_eth *eth)
{
	int p;

	for (p = 1; p <= ARRAY_SIZE(eth->ports); p++) {
		/*
		 * Disable any kind of CRC drop or offload.
		 * Enable padding of short TX packets to 60 bytes.
		 */
		airoha_fe_wr(eth, REG_GDM_FWD_CFG(p), GDM_PAD_EN);
	}
}

static int airoha_fe_init(struct airoha_eth *eth)
{
	airoha_fe_maccr_init(eth);

	return 0;
}

static void airoha_qdma_reset_rx_desc(struct airoha_queue *q, int index)
{
	struct airoha_qdma_desc *desc;
	uchar *rx_packet;
	u32 val;

	desc = &q->desc[index];
	rx_packet = net_rx_packets[index];
	index = (index + 1) % q->ndesc;

	dma_map_single(rx_packet, PKTSIZE_ALIGN, DMA_TO_DEVICE);

	WRITE_ONCE(desc->msg0, cpu_to_le32(0));
	WRITE_ONCE(desc->msg1, cpu_to_le32(0));
	WRITE_ONCE(desc->msg2, cpu_to_le32(0));
	WRITE_ONCE(desc->msg3, cpu_to_le32(0));
	WRITE_ONCE(desc->addr, cpu_to_le32(virt_to_phys(rx_packet)));
	WRITE_ONCE(desc->data, cpu_to_le32(index));
	val = FIELD_PREP(QDMA_DESC_LEN_MASK, PKTSIZE_ALIGN);
	WRITE_ONCE(desc->ctrl, cpu_to_le32(val));

	dma_map_unaligned(desc, sizeof(*desc), DMA_TO_DEVICE);
}

static void airoha_qdma_init_rx_desc(struct airoha_queue *q)
{
	int i;

	for (i = 0; i < q->ndesc; i++)
		airoha_qdma_reset_rx_desc(q, i);
}

static int airoha_qdma_init_rx_queue(struct airoha_queue *q,
				     struct airoha_qdma *qdma, int ndesc)
{
	int qid = q - &qdma->q_rx[0];
	unsigned long dma_addr;

	q->ndesc = ndesc;
	q->head = 0;

	q->desc = dma_alloc_coherent(q->ndesc * sizeof(*q->desc), &dma_addr);
	if (!q->desc)
		return -ENOMEM;

	memset(q->desc, 0, q->ndesc * sizeof(*q->desc));
	dma_map_single(q->desc, q->ndesc * sizeof(*q->desc), DMA_TO_DEVICE);

	airoha_qdma_wr(qdma, REG_RX_RING_BASE(qid), dma_addr);
	airoha_qdma_rmw(qdma, REG_RX_RING_SIZE(qid),
			RX_RING_SIZE_MASK,
			FIELD_PREP(RX_RING_SIZE_MASK, ndesc));

	/*
	 * See arht_eth_free_pkt() for the reasons used to fill
	 * REG_RX_CPU_IDX(qid) register.
	 */
	airoha_qdma_rmw(qdma, REG_RX_RING_SIZE(qid), RX_RING_THR_MASK,
			FIELD_PREP(RX_RING_THR_MASK, 0));
	airoha_qdma_rmw(qdma, REG_RX_CPU_IDX(qid), RX_RING_CPU_IDX_MASK,
			FIELD_PREP(RX_RING_CPU_IDX_MASK, q->ndesc - 3));
	airoha_qdma_rmw(qdma, REG_RX_DMA_IDX(qid), RX_RING_DMA_IDX_MASK,
			FIELD_PREP(RX_RING_DMA_IDX_MASK, q->head));

	return 0;
}

static int airoha_qdma_init_rx(struct airoha_qdma *qdma)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(qdma->q_rx); i++) {
		int err;

		err = airoha_qdma_init_rx_queue(&qdma->q_rx[i], qdma,
						RX_DSCP_NUM);
		if (err)
			return err;
	}

	return 0;
}

static int airoha_qdma_init_tx_queue(struct airoha_queue *q,
				     struct airoha_qdma *qdma, int size)
{
	int qid = q - &qdma->q_tx[0];
	unsigned long dma_addr;

	q->ndesc = size;
	q->head = 0;

	q->desc = dma_alloc_coherent(q->ndesc * sizeof(*q->desc), &dma_addr);
	if (!q->desc)
		return -ENOMEM;

	memset(q->desc, 0, q->ndesc * sizeof(*q->desc));
	dma_map_single(q->desc, q->ndesc * sizeof(*q->desc), DMA_TO_DEVICE);

	airoha_qdma_wr(qdma, REG_TX_RING_BASE(qid), dma_addr);
	airoha_qdma_rmw(qdma, REG_TX_CPU_IDX(qid), TX_RING_CPU_IDX_MASK,
			FIELD_PREP(TX_RING_CPU_IDX_MASK, q->head));
	airoha_qdma_rmw(qdma, REG_TX_DMA_IDX(qid), TX_RING_DMA_IDX_MASK,
			FIELD_PREP(TX_RING_DMA_IDX_MASK, q->head));

	return 0;
}

static int airoha_qdma_tx_irq_init(struct airoha_tx_irq_queue *irq_q,
				   struct airoha_qdma *qdma, int size)
{
	int id = irq_q - &qdma->q_tx_irq[0];
	unsigned long dma_addr;

	irq_q->q = dma_alloc_coherent(size * sizeof(u32), &dma_addr);
	if (!irq_q->q)
		return -ENOMEM;

	memset(irq_q->q, 0xffffffff, size * sizeof(u32));
	irq_q->size = size;
	irq_q->qdma = qdma;

	dma_map_single(irq_q->q, size * sizeof(u32), DMA_TO_DEVICE);

	airoha_qdma_wr(qdma, REG_TX_IRQ_BASE(id), dma_addr);
	airoha_qdma_rmw(qdma, REG_TX_IRQ_CFG(id), TX_IRQ_DEPTH_MASK,
			FIELD_PREP(TX_IRQ_DEPTH_MASK, size));

	return 0;
}

static int airoha_qdma_init_tx(struct airoha_qdma *qdma)
{
	int i, err;

	for (i = 0; i < ARRAY_SIZE(qdma->q_tx_irq); i++) {
		err = airoha_qdma_tx_irq_init(&qdma->q_tx_irq[i], qdma,
					      IRQ_QUEUE_LEN);
		if (err)
			return err;
	}

	for (i = 0; i < ARRAY_SIZE(qdma->q_tx); i++) {
		err = airoha_qdma_init_tx_queue(&qdma->q_tx[i], qdma,
						TX_DSCP_NUM);
		if (err)
			return err;
	}

	return 0;
}

static int airoha_qdma_init_hfwd_queues(struct airoha_qdma *qdma)
{
	unsigned long dma_addr;
	u32 status;
	int size;

	size = HW_DSCP_NUM * sizeof(struct airoha_qdma_fwd_desc);
	qdma->hfwd.desc = dma_alloc_coherent(size, &dma_addr);
	if (!qdma->hfwd.desc)
		return -ENOMEM;

	memset(qdma->hfwd.desc, 0, size);
	dma_map_single(qdma->hfwd.desc, size, DMA_TO_DEVICE);

	airoha_qdma_wr(qdma, REG_FWD_DSCP_BASE, dma_addr);

	size = AIROHA_MAX_PACKET_SIZE * HW_DSCP_NUM;
	qdma->hfwd.q = dma_alloc_coherent(size, &dma_addr);
	if (!qdma->hfwd.q)
		return -ENOMEM;

	memset(qdma->hfwd.q, 0, size);
	dma_map_single(qdma->hfwd.q, size, DMA_TO_DEVICE);

	airoha_qdma_wr(qdma, REG_FWD_BUF_BASE, dma_addr);

	airoha_qdma_rmw(qdma, REG_HW_FWD_DSCP_CFG,
			HW_FWD_DSCP_PAYLOAD_SIZE_MASK |
			HW_FWD_DSCP_MIN_SCATTER_LEN_MASK,
			FIELD_PREP(HW_FWD_DSCP_PAYLOAD_SIZE_MASK, 0) |
			FIELD_PREP(HW_FWD_DSCP_MIN_SCATTER_LEN_MASK, 1));
	airoha_qdma_rmw(qdma, REG_LMGR_INIT_CFG,
			LMGR_INIT_START | LMGR_SRAM_MODE_MASK |
			HW_FWD_DESC_NUM_MASK,
			FIELD_PREP(HW_FWD_DESC_NUM_MASK, HW_DSCP_NUM) |
			LMGR_INIT_START);

	udelay(1000);
	return read_poll_timeout(airoha_qdma_rr, status,
				 !(status & LMGR_INIT_START), USEC_PER_MSEC,
				 30 * USEC_PER_MSEC, qdma,
				 REG_LMGR_INIT_CFG);
}

static int airoha_qdma_hw_init(struct airoha_qdma *qdma)
{
	int i;

	/* clear pending irqs */
	for (i = 0; i < 2; i++)
		airoha_qdma_wr(qdma, REG_INT_STATUS(i), 0xffffffff);

	airoha_qdma_wr(qdma, REG_QDMA_GLOBAL_CFG,
		       GLOBAL_CFG_CPU_TXR_RR_MASK |
		       GLOBAL_CFG_PAYLOAD_BYTE_SWAP_MASK |
		       GLOBAL_CFG_IRQ0_EN_MASK |
		       GLOBAL_CFG_TX_WB_DONE_MASK |
		       FIELD_PREP(GLOBAL_CFG_MAX_ISSUE_NUM_MASK, 3));

	/* disable qdma rx delay interrupt */
	for (i = 0; i < ARRAY_SIZE(qdma->q_rx); i++) {
		if (!qdma->q_rx[i].ndesc)
			continue;

		airoha_qdma_clear(qdma, REG_RX_DELAY_INT_IDX(i),
				  RX_DELAY_INT_MASK);
	}

	return 0;
}

static int airoha_qdma_init(struct udevice *dev,
			    struct airoha_eth *eth,
			    struct airoha_qdma *qdma)
{
	int err;

	qdma->eth = eth;
	qdma->regs = dev_remap_addr_name(dev, "qdma0");
	if (IS_ERR(qdma->regs))
		return PTR_ERR(qdma->regs);

	err = airoha_qdma_init_rx(qdma);
	if (err)
		return err;

	err = airoha_qdma_init_tx(qdma);
	if (err)
		return err;

	err = airoha_qdma_init_hfwd_queues(qdma);
	if (err)
		return err;

	return airoha_qdma_hw_init(qdma);
}

static int airoha_hw_init(struct udevice *dev,
			  struct airoha_eth *eth)
{
	int ret, i;

	/* disable xsi */
	ret = reset_assert_bulk(&eth->xsi_rsts);
	if (ret)
		return ret;

	ret = reset_assert_bulk(&eth->rsts);
	if (ret)
		return ret;

	mdelay(20);

	ret = reset_deassert_bulk(&eth->rsts);
	if (ret)
		return ret;

	mdelay(20);

	ret = airoha_fe_init(eth);
	if (ret)
		return ret;

	for (i = 0; i < ARRAY_SIZE(eth->qdma); i++) {
		ret = airoha_qdma_init(dev, eth, &eth->qdma[i]);
		if (ret)
			return ret;
	}

	return 0;
}

static int airoha_switch_init(struct udevice *dev, struct airoha_eth *eth)
{
	ofnode switch_node;
	fdt_addr_t addr;

	switch_node = ofnode_by_compatible(ofnode_null(), "airoha,en7581-switch");
	if (!ofnode_valid(switch_node))
		return -EINVAL;

	addr = ofnode_get_addr(switch_node);
	if (addr == FDT_ADDR_T_NONE)
		return -ENOMEM;

	/* Switch doesn't have a DEV, gets address and setup Flood and CPU port */
	eth->switch_regs = map_sysmem(addr, 0);

	/* Set FLOOD, no CPU switch register */
	airoha_switch_wr(eth, SWITCH_MFC, SWITCH_BC_FFP | SWITCH_UNM_FFP |
			 SWITCH_UNU_FFP);

	/* Set CPU 6 PMCR */
	airoha_switch_wr(eth, SWITCH_PMCR(6),
			 SWITCH_IPG_CFG_SHORT | SWITCH_MAC_MODE |
			 SWITCH_FORCE_MODE | SWITCH_MAC_TX_EN |
			 SWITCH_MAC_RX_EN | SWITCH_BKOFF_EN | SWITCH_BKPR_EN |
			 SWITCH_FORCE_RX_FC | SWITCH_FORCE_TX_FC |
			 SWITCH_FORCE_SPD_1000 | SWITCH_FORCE_DPX |
			 SWITCH_FORCE_LNK);

	/* Sideband signal error for Port 3, which need the auto polling */
	airoha_switch_wr(eth, SWITCH_PHY_POLL,
			 FIELD_PREP(SWITCH_PHY_AP_EN, 0x7f) |
			 FIELD_PREP(SWITCH_EEE_POLL_EN, 0x7f) |
			 SWITCH_PHY_PRE_EN |
			 FIELD_PREP(SWITCH_PHY_END_ADDR, 0xc) |
			 FIELD_PREP(SWITCH_PHY_ST_ADDR, 0x8));

	return 0;
}

static int airoha_eth_probe(struct udevice *dev)
{
	struct airoha_eth *eth = dev_get_priv(dev);
	struct regmap *scu_regmap;
	ofnode scu_node;
	int ret;

	scu_node = ofnode_by_compatible(ofnode_null(), "airoha,en7581-scu");
	if (!ofnode_valid(scu_node))
		return -EINVAL;

	scu_regmap = syscon_node_to_regmap(scu_node);
	if (IS_ERR(scu_regmap))
		return PTR_ERR(scu_regmap);

	/* It seems by default the FEMEM_SEL is set to Memory (0x1)
	 * preventing any access to any QDMA and FrameEngine register
	 * reporting all 0xdeadbeef (poor cow :( )
	 */
	regmap_write(scu_regmap, SCU_SHARE_FEMEM_SEL, 0x0);

	eth->fe_regs = dev_remap_addr_name(dev, "fe");
	if (!eth->fe_regs)
		return -ENOMEM;

	eth->rsts.resets = devm_kcalloc(dev, AIROHA_MAX_NUM_RSTS,
					sizeof(struct reset_ctl), GFP_KERNEL);
	if (!eth->rsts.resets)
		return -ENOMEM;
	eth->rsts.count = AIROHA_MAX_NUM_RSTS;

	eth->xsi_rsts.resets = devm_kcalloc(dev, AIROHA_MAX_NUM_XSI_RSTS,
					    sizeof(struct reset_ctl), GFP_KERNEL);
	if (!eth->xsi_rsts.resets)
		return -ENOMEM;
	eth->xsi_rsts.count = AIROHA_MAX_NUM_XSI_RSTS;

	ret = reset_get_by_name(dev, "fe", &eth->rsts.resets[0]);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "pdma", &eth->rsts.resets[1]);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "qdma", &eth->rsts.resets[2]);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "hsi0-mac", &eth->xsi_rsts.resets[0]);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "hsi1-mac", &eth->xsi_rsts.resets[1]);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "hsi-mac", &eth->xsi_rsts.resets[2]);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "xfp-mac", &eth->xsi_rsts.resets[3]);
	if (ret)
		return ret;

	ret = airoha_hw_init(dev, eth);
	if (ret)
		return ret;

	return airoha_switch_init(dev, eth);
}

static int airoha_eth_init(struct udevice *dev)
{
	struct airoha_eth *eth = dev_get_priv(dev);
	struct airoha_qdma *qdma = &eth->qdma[0];
	struct airoha_queue *q;
	int qid;

	qid = 0;
	q = &qdma->q_rx[qid];

	airoha_qdma_init_rx_desc(q);

	airoha_qdma_set(qdma, REG_QDMA_GLOBAL_CFG,
			GLOBAL_CFG_TX_DMA_EN_MASK |
			GLOBAL_CFG_RX_DMA_EN_MASK);

	return 0;
}

static void airoha_eth_stop(struct udevice *dev)
{
	struct airoha_eth *eth = dev_get_priv(dev);
	struct airoha_qdma *qdma = &eth->qdma[0];

	airoha_qdma_clear(qdma, REG_QDMA_GLOBAL_CFG,
			  GLOBAL_CFG_TX_DMA_EN_MASK |
			  GLOBAL_CFG_RX_DMA_EN_MASK);
}

static int airoha_eth_send(struct udevice *dev, void *packet, int length)
{
	struct airoha_eth *eth = dev_get_priv(dev);
	struct airoha_qdma *qdma = &eth->qdma[0];
	struct airoha_qdma_desc *desc;
	struct airoha_queue *q;
	dma_addr_t dma_addr;
	u32 msg0, msg1;
	int qid, index;
	u8 fport;
	u32 val;
	int i;

	/*
	 * There is no need to pad short TX packets to 60 bytes since the
	 * GDM_PAD_EN bit set in the corresponding REG_GDM_FWD_CFG(n) register.
	 */

	dma_addr = dma_map_single(packet, length, DMA_TO_DEVICE);

	qid = 0;
	q = &qdma->q_tx[qid];
	desc = &q->desc[q->head];
	index = (q->head + 1) % q->ndesc;

	fport = 1;

	msg0 = 0;
	msg1 = FIELD_PREP(QDMA_ETH_TXMSG_FPORT_MASK, fport) |
	       FIELD_PREP(QDMA_ETH_TXMSG_METER_MASK, 0x7f);

	val = FIELD_PREP(QDMA_DESC_LEN_MASK, length);
	WRITE_ONCE(desc->ctrl, cpu_to_le32(val));
	WRITE_ONCE(desc->addr, cpu_to_le32(dma_addr));
	val = FIELD_PREP(QDMA_DESC_NEXT_ID_MASK, index);
	WRITE_ONCE(desc->data, cpu_to_le32(val));
	WRITE_ONCE(desc->msg0, cpu_to_le32(msg0));
	WRITE_ONCE(desc->msg1, cpu_to_le32(msg1));
	WRITE_ONCE(desc->msg2, cpu_to_le32(0xffff));

	dma_map_unaligned(desc, sizeof(*desc), DMA_TO_DEVICE);

	airoha_qdma_rmw(qdma, REG_TX_CPU_IDX(qid), TX_RING_CPU_IDX_MASK,
			FIELD_PREP(TX_RING_CPU_IDX_MASK, index));

	for (i = 0; i < 100; i++) {
		dma_unmap_unaligned(virt_to_phys(desc), sizeof(*desc),
				    DMA_FROM_DEVICE);
		if (desc->ctrl & QDMA_DESC_DONE_MASK)
			break;

		udelay(1);
	}

	/* Return error if for some reason the descriptor never ACK */
	if (!(desc->ctrl & QDMA_DESC_DONE_MASK))
		return -EAGAIN;

	q->head = index;
	airoha_qdma_rmw(qdma, REG_IRQ_CLEAR_LEN(0),
			IRQ_CLEAR_LEN_MASK, 1);

	return 0;
}

static int airoha_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct airoha_eth *eth = dev_get_priv(dev);
	struct airoha_qdma *qdma = &eth->qdma[0];
	struct airoha_qdma_desc *desc;
	struct airoha_queue *q;
	u16 length;
	int qid;

	qid = 0;
	q = &qdma->q_rx[qid];
	desc = &q->desc[q->head];

	dma_unmap_unaligned(virt_to_phys(desc), sizeof(*desc),
			    DMA_FROM_DEVICE);

	if (!(desc->ctrl & QDMA_DESC_DONE_MASK))
		return -EAGAIN;

	length = FIELD_GET(QDMA_DESC_LEN_MASK, desc->ctrl);
	dma_unmap_single(desc->addr, length,
			 DMA_FROM_DEVICE);

	*packetp = phys_to_virt(desc->addr);

	return length;
}

static int arht_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct airoha_eth *eth = dev_get_priv(dev);
	struct airoha_qdma *qdma = &eth->qdma[0];
	struct airoha_queue *q;
	int qid;
	u16 prev, pprev;

	if (!packet)
		return 0;

	qid = 0;
	q = &qdma->q_rx[qid];

	/*
	 * Due to cpu cache issue the airoha_qdma_reset_rx_desc() function
	 * will always touch 2 descriptors:
	 *   - if current descriptor is even, then the previous and the one
	 *     before previous descriptors will be touched (previous cacheline)
	 *   - if current descriptor is odd, then only current and previous
	 *     descriptors will be touched (current cacheline)
	 *
	 * Thus, to prevent possible destroying of rx queue, only (q->ndesc - 2)
	 * descriptors might be used for packet receiving.
	 */
	prev  = (q->head + q->ndesc - 1) % q->ndesc;
	pprev = (q->head + q->ndesc - 2) % q->ndesc;
	q->head = (q->head + 1) % q->ndesc;

	airoha_qdma_reset_rx_desc(q, prev);
	airoha_qdma_rmw(qdma, REG_RX_CPU_IDX(qid), RX_RING_CPU_IDX_MASK,
			FIELD_PREP(RX_RING_CPU_IDX_MASK, pprev));

	return 0;
}

static int arht_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct airoha_eth *eth = dev_get_priv(dev);
	unsigned char *mac = pdata->enetaddr;
	u32 macaddr_lsb, macaddr_msb;

	macaddr_lsb = FIELD_PREP(SMACCR0_MAC2, mac[2]) |
		      FIELD_PREP(SMACCR0_MAC3, mac[3]) |
		      FIELD_PREP(SMACCR0_MAC4, mac[4]) |
		      FIELD_PREP(SMACCR0_MAC5, mac[5]);
	macaddr_msb = FIELD_PREP(SMACCR1_MAC1, mac[1]) |
		      FIELD_PREP(SMACCR1_MAC0, mac[0]);

	/* Set MAC for Switch */
	airoha_switch_wr(eth, SWITCH_SMACCR0, macaddr_lsb);
	airoha_switch_wr(eth, SWITCH_SMACCR1, macaddr_msb);

	return 0;
}

static const struct udevice_id airoha_eth_ids[] = {
	{ .compatible = "airoha,en7581-eth" },
	{ }
};

static const struct eth_ops airoha_eth_ops = {
	.start = airoha_eth_init,
	.stop = airoha_eth_stop,
	.send = airoha_eth_send,
	.recv = airoha_eth_recv,
	.free_pkt = arht_eth_free_pkt,
	.write_hwaddr = arht_eth_write_hwaddr,
};

U_BOOT_DRIVER(airoha_eth) = {
	.name = "airoha-eth",
	.id = UCLASS_ETH,
	.of_match = airoha_eth_ids,
	.probe = airoha_eth_probe,
	.ops = &airoha_eth_ops,
	.priv_auto = sizeof(struct airoha_eth),
	.plat_auto = sizeof(struct eth_pdata),
};
