/*
 * Driver for Marvell PPv2 network controller for Armada 375 SoC.
 *
 * Copyright (C) 2014 Marvell
 *
 * Marcin Wojtas <mw@semihalf.com>
 *
 * U-Boot version:
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <net.h>
#include <netdev.h>
#include <config.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <phy.h>
#include <miiphy.h>
#include <watchdog.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/compat.h>
#include <linux/mbus.h>

DECLARE_GLOBAL_DATA_PTR;

/* Some linux -> U-Boot compatibility stuff */
#define netdev_err(dev, fmt, args...)		\
	printf(fmt, ##args)
#define netdev_warn(dev, fmt, args...)		\
	printf(fmt, ##args)
#define netdev_info(dev, fmt, args...)		\
	printf(fmt, ##args)
#define netdev_dbg(dev, fmt, args...)		\
	printf(fmt, ##args)

#define ETH_ALEN	6		/* Octets in one ethernet addr	*/

#define __verify_pcpu_ptr(ptr)						\
do {									\
	const void __percpu *__vpp_verify = (typeof((ptr) + 0))NULL;	\
	(void)__vpp_verify;						\
} while (0)

#define VERIFY_PERCPU_PTR(__p)						\
({									\
	__verify_pcpu_ptr(__p);						\
	(typeof(*(__p)) __kernel __force *)(__p);			\
})

#define per_cpu_ptr(ptr, cpu)	({ (void)(cpu); VERIFY_PERCPU_PTR(ptr); })
#define smp_processor_id()	0
#define num_present_cpus()	1
#define for_each_present_cpu(cpu)			\
	for ((cpu) = 0; (cpu) < 1; (cpu)++)

#define NET_SKB_PAD	max(32, MVPP2_CPU_D_CACHE_LINE_SIZE)

#define CONFIG_NR_CPUS		1
#define ETH_HLEN		ETHER_HDR_SIZE	/* Total octets in header */

/* 2(HW hdr) 14(MAC hdr) 4(CRC) 32(extra for cache prefetch) */
#define WRAP			(2 + ETH_HLEN + 4 + 32)
#define MTU			1500
#define RX_BUFFER_SIZE		(ALIGN(MTU + WRAP, ARCH_DMA_MINALIGN))

#define MVPP2_SMI_TIMEOUT			10000

/* RX Fifo Registers */
#define MVPP2_RX_DATA_FIFO_SIZE_REG(port)	(0x00 + 4 * (port))
#define MVPP2_RX_ATTR_FIFO_SIZE_REG(port)	(0x20 + 4 * (port))
#define MVPP2_RX_MIN_PKT_SIZE_REG		0x60
#define MVPP2_RX_FIFO_INIT_REG			0x64

/* RX DMA Top Registers */
#define MVPP2_RX_CTRL_REG(port)			(0x140 + 4 * (port))
#define     MVPP2_RX_LOW_LATENCY_PKT_SIZE(s)	(((s) & 0xfff) << 16)
#define     MVPP2_RX_USE_PSEUDO_FOR_CSUM_MASK	BIT(31)
#define MVPP2_POOL_BUF_SIZE_REG(pool)		(0x180 + 4 * (pool))
#define     MVPP2_POOL_BUF_SIZE_OFFSET		5
#define MVPP2_RXQ_CONFIG_REG(rxq)		(0x800 + 4 * (rxq))
#define     MVPP2_SNOOP_PKT_SIZE_MASK		0x1ff
#define     MVPP2_SNOOP_BUF_HDR_MASK		BIT(9)
#define     MVPP2_RXQ_POOL_SHORT_OFFS		20
#define     MVPP2_RXQ_POOL_SHORT_MASK		0x700000
#define     MVPP2_RXQ_POOL_LONG_OFFS		24
#define     MVPP2_RXQ_POOL_LONG_MASK		0x7000000
#define     MVPP2_RXQ_PACKET_OFFSET_OFFS	28
#define     MVPP2_RXQ_PACKET_OFFSET_MASK	0x70000000
#define     MVPP2_RXQ_DISABLE_MASK		BIT(31)

/* Parser Registers */
#define MVPP2_PRS_INIT_LOOKUP_REG		0x1000
#define     MVPP2_PRS_PORT_LU_MAX		0xf
#define     MVPP2_PRS_PORT_LU_MASK(port)	(0xff << ((port) * 4))
#define     MVPP2_PRS_PORT_LU_VAL(port, val)	((val) << ((port) * 4))
#define MVPP2_PRS_INIT_OFFS_REG(port)		(0x1004 + ((port) & 4))
#define     MVPP2_PRS_INIT_OFF_MASK(port)	(0x3f << (((port) % 4) * 8))
#define     MVPP2_PRS_INIT_OFF_VAL(port, val)	((val) << (((port) % 4) * 8))
#define MVPP2_PRS_MAX_LOOP_REG(port)		(0x100c + ((port) & 4))
#define     MVPP2_PRS_MAX_LOOP_MASK(port)	(0xff << (((port) % 4) * 8))
#define     MVPP2_PRS_MAX_LOOP_VAL(port, val)	((val) << (((port) % 4) * 8))
#define MVPP2_PRS_TCAM_IDX_REG			0x1100
#define MVPP2_PRS_TCAM_DATA_REG(idx)		(0x1104 + (idx) * 4)
#define     MVPP2_PRS_TCAM_INV_MASK		BIT(31)
#define MVPP2_PRS_SRAM_IDX_REG			0x1200
#define MVPP2_PRS_SRAM_DATA_REG(idx)		(0x1204 + (idx) * 4)
#define MVPP2_PRS_TCAM_CTRL_REG			0x1230
#define     MVPP2_PRS_TCAM_EN_MASK		BIT(0)

/* Classifier Registers */
#define MVPP2_CLS_MODE_REG			0x1800
#define     MVPP2_CLS_MODE_ACTIVE_MASK		BIT(0)
#define MVPP2_CLS_PORT_WAY_REG			0x1810
#define     MVPP2_CLS_PORT_WAY_MASK(port)	(1 << (port))
#define MVPP2_CLS_LKP_INDEX_REG			0x1814
#define     MVPP2_CLS_LKP_INDEX_WAY_OFFS	6
#define MVPP2_CLS_LKP_TBL_REG			0x1818
#define     MVPP2_CLS_LKP_TBL_RXQ_MASK		0xff
#define     MVPP2_CLS_LKP_TBL_LOOKUP_EN_MASK	BIT(25)
#define MVPP2_CLS_FLOW_INDEX_REG		0x1820
#define MVPP2_CLS_FLOW_TBL0_REG			0x1824
#define MVPP2_CLS_FLOW_TBL1_REG			0x1828
#define MVPP2_CLS_FLOW_TBL2_REG			0x182c
#define MVPP2_CLS_OVERSIZE_RXQ_LOW_REG(port)	(0x1980 + ((port) * 4))
#define     MVPP2_CLS_OVERSIZE_RXQ_LOW_BITS	3
#define     MVPP2_CLS_OVERSIZE_RXQ_LOW_MASK	0x7
#define MVPP2_CLS_SWFWD_P2HQ_REG(port)		(0x19b0 + ((port) * 4))
#define MVPP2_CLS_SWFWD_PCTRL_REG		0x19d0
#define     MVPP2_CLS_SWFWD_PCTRL_MASK(port)	(1 << (port))

/* Descriptor Manager Top Registers */
#define MVPP2_RXQ_NUM_REG			0x2040
#define MVPP2_RXQ_DESC_ADDR_REG			0x2044
#define MVPP2_RXQ_DESC_SIZE_REG			0x2048
#define     MVPP2_RXQ_DESC_SIZE_MASK		0x3ff0
#define MVPP2_RXQ_STATUS_UPDATE_REG(rxq)	(0x3000 + 4 * (rxq))
#define     MVPP2_RXQ_NUM_PROCESSED_OFFSET	0
#define     MVPP2_RXQ_NUM_NEW_OFFSET		16
#define MVPP2_RXQ_STATUS_REG(rxq)		(0x3400 + 4 * (rxq))
#define     MVPP2_RXQ_OCCUPIED_MASK		0x3fff
#define     MVPP2_RXQ_NON_OCCUPIED_OFFSET	16
#define     MVPP2_RXQ_NON_OCCUPIED_MASK		0x3fff0000
#define MVPP2_RXQ_THRESH_REG			0x204c
#define     MVPP2_OCCUPIED_THRESH_OFFSET	0
#define     MVPP2_OCCUPIED_THRESH_MASK		0x3fff
#define MVPP2_RXQ_INDEX_REG			0x2050
#define MVPP2_TXQ_NUM_REG			0x2080
#define MVPP2_TXQ_DESC_ADDR_REG			0x2084
#define MVPP2_TXQ_DESC_SIZE_REG			0x2088
#define     MVPP2_TXQ_DESC_SIZE_MASK		0x3ff0
#define MVPP2_AGGR_TXQ_UPDATE_REG		0x2090
#define MVPP2_TXQ_THRESH_REG			0x2094
#define     MVPP2_TRANSMITTED_THRESH_OFFSET	16
#define     MVPP2_TRANSMITTED_THRESH_MASK	0x3fff0000
#define MVPP2_TXQ_INDEX_REG			0x2098
#define MVPP2_TXQ_PREF_BUF_REG			0x209c
#define     MVPP2_PREF_BUF_PTR(desc)		((desc) & 0xfff)
#define     MVPP2_PREF_BUF_SIZE_4		(BIT(12) | BIT(13))
#define     MVPP2_PREF_BUF_SIZE_16		(BIT(12) | BIT(14))
#define     MVPP2_PREF_BUF_THRESH(val)		((val) << 17)
#define     MVPP2_TXQ_DRAIN_EN_MASK		BIT(31)
#define MVPP2_TXQ_PENDING_REG			0x20a0
#define     MVPP2_TXQ_PENDING_MASK		0x3fff
#define MVPP2_TXQ_INT_STATUS_REG		0x20a4
#define MVPP2_TXQ_SENT_REG(txq)			(0x3c00 + 4 * (txq))
#define     MVPP2_TRANSMITTED_COUNT_OFFSET	16
#define     MVPP2_TRANSMITTED_COUNT_MASK	0x3fff0000
#define MVPP2_TXQ_RSVD_REQ_REG			0x20b0
#define     MVPP2_TXQ_RSVD_REQ_Q_OFFSET		16
#define MVPP2_TXQ_RSVD_RSLT_REG			0x20b4
#define     MVPP2_TXQ_RSVD_RSLT_MASK		0x3fff
#define MVPP2_TXQ_RSVD_CLR_REG			0x20b8
#define     MVPP2_TXQ_RSVD_CLR_OFFSET		16
#define MVPP2_AGGR_TXQ_DESC_ADDR_REG(cpu)	(0x2100 + 4 * (cpu))
#define MVPP2_AGGR_TXQ_DESC_SIZE_REG(cpu)	(0x2140 + 4 * (cpu))
#define     MVPP2_AGGR_TXQ_DESC_SIZE_MASK	0x3ff0
#define MVPP2_AGGR_TXQ_STATUS_REG(cpu)		(0x2180 + 4 * (cpu))
#define     MVPP2_AGGR_TXQ_PENDING_MASK		0x3fff
#define MVPP2_AGGR_TXQ_INDEX_REG(cpu)		(0x21c0 + 4 * (cpu))

/* MBUS bridge registers */
#define MVPP2_WIN_BASE(w)			(0x4000 + ((w) << 2))
#define MVPP2_WIN_SIZE(w)			(0x4020 + ((w) << 2))
#define MVPP2_WIN_REMAP(w)			(0x4040 + ((w) << 2))
#define MVPP2_BASE_ADDR_ENABLE			0x4060

/* Interrupt Cause and Mask registers */
#define MVPP2_ISR_RX_THRESHOLD_REG(rxq)		(0x5200 + 4 * (rxq))
#define MVPP2_ISR_RXQ_GROUP_REG(rxq)		(0x5400 + 4 * (rxq))
#define MVPP2_ISR_ENABLE_REG(port)		(0x5420 + 4 * (port))
#define     MVPP2_ISR_ENABLE_INTERRUPT(mask)	((mask) & 0xffff)
#define     MVPP2_ISR_DISABLE_INTERRUPT(mask)	(((mask) << 16) & 0xffff0000)
#define MVPP2_ISR_RX_TX_CAUSE_REG(port)		(0x5480 + 4 * (port))
#define     MVPP2_CAUSE_RXQ_OCCUP_DESC_ALL_MASK	0xffff
#define     MVPP2_CAUSE_TXQ_OCCUP_DESC_ALL_MASK	0xff0000
#define     MVPP2_CAUSE_RX_FIFO_OVERRUN_MASK	BIT(24)
#define     MVPP2_CAUSE_FCS_ERR_MASK		BIT(25)
#define     MVPP2_CAUSE_TX_FIFO_UNDERRUN_MASK	BIT(26)
#define     MVPP2_CAUSE_TX_EXCEPTION_SUM_MASK	BIT(29)
#define     MVPP2_CAUSE_RX_EXCEPTION_SUM_MASK	BIT(30)
#define     MVPP2_CAUSE_MISC_SUM_MASK		BIT(31)
#define MVPP2_ISR_RX_TX_MASK_REG(port)		(0x54a0 + 4 * (port))
#define MVPP2_ISR_PON_RX_TX_MASK_REG		0x54bc
#define     MVPP2_PON_CAUSE_RXQ_OCCUP_DESC_ALL_MASK	0xffff
#define     MVPP2_PON_CAUSE_TXP_OCCUP_DESC_ALL_MASK	0x3fc00000
#define     MVPP2_PON_CAUSE_MISC_SUM_MASK		BIT(31)
#define MVPP2_ISR_MISC_CAUSE_REG		0x55b0

/* Buffer Manager registers */
#define MVPP2_BM_POOL_BASE_REG(pool)		(0x6000 + ((pool) * 4))
#define     MVPP2_BM_POOL_BASE_ADDR_MASK	0xfffff80
#define MVPP2_BM_POOL_SIZE_REG(pool)		(0x6040 + ((pool) * 4))
#define     MVPP2_BM_POOL_SIZE_MASK		0xfff0
#define MVPP2_BM_POOL_READ_PTR_REG(pool)	(0x6080 + ((pool) * 4))
#define     MVPP2_BM_POOL_GET_READ_PTR_MASK	0xfff0
#define MVPP2_BM_POOL_PTRS_NUM_REG(pool)	(0x60c0 + ((pool) * 4))
#define     MVPP2_BM_POOL_PTRS_NUM_MASK		0xfff0
#define MVPP2_BM_BPPI_READ_PTR_REG(pool)	(0x6100 + ((pool) * 4))
#define MVPP2_BM_BPPI_PTRS_NUM_REG(pool)	(0x6140 + ((pool) * 4))
#define     MVPP2_BM_BPPI_PTR_NUM_MASK		0x7ff
#define     MVPP2_BM_BPPI_PREFETCH_FULL_MASK	BIT(16)
#define MVPP2_BM_POOL_CTRL_REG(pool)		(0x6200 + ((pool) * 4))
#define     MVPP2_BM_START_MASK			BIT(0)
#define     MVPP2_BM_STOP_MASK			BIT(1)
#define     MVPP2_BM_STATE_MASK			BIT(4)
#define     MVPP2_BM_LOW_THRESH_OFFS		8
#define     MVPP2_BM_LOW_THRESH_MASK		0x7f00
#define     MVPP2_BM_LOW_THRESH_VALUE(val)	((val) << \
						MVPP2_BM_LOW_THRESH_OFFS)
#define     MVPP2_BM_HIGH_THRESH_OFFS		16
#define     MVPP2_BM_HIGH_THRESH_MASK		0x7f0000
#define     MVPP2_BM_HIGH_THRESH_VALUE(val)	((val) << \
						MVPP2_BM_HIGH_THRESH_OFFS)
#define MVPP2_BM_INTR_CAUSE_REG(pool)		(0x6240 + ((pool) * 4))
#define     MVPP2_BM_RELEASED_DELAY_MASK	BIT(0)
#define     MVPP2_BM_ALLOC_FAILED_MASK		BIT(1)
#define     MVPP2_BM_BPPE_EMPTY_MASK		BIT(2)
#define     MVPP2_BM_BPPE_FULL_MASK		BIT(3)
#define     MVPP2_BM_AVAILABLE_BP_LOW_MASK	BIT(4)
#define MVPP2_BM_INTR_MASK_REG(pool)		(0x6280 + ((pool) * 4))
#define MVPP2_BM_PHY_ALLOC_REG(pool)		(0x6400 + ((pool) * 4))
#define     MVPP2_BM_PHY_ALLOC_GRNTD_MASK	BIT(0)
#define MVPP2_BM_VIRT_ALLOC_REG			0x6440
#define MVPP2_BM_PHY_RLS_REG(pool)		(0x6480 + ((pool) * 4))
#define     MVPP2_BM_PHY_RLS_MC_BUFF_MASK	BIT(0)
#define     MVPP2_BM_PHY_RLS_PRIO_EN_MASK	BIT(1)
#define     MVPP2_BM_PHY_RLS_GRNTD_MASK		BIT(2)
#define MVPP2_BM_VIRT_RLS_REG			0x64c0
#define MVPP2_BM_MC_RLS_REG			0x64c4
#define     MVPP2_BM_MC_ID_MASK			0xfff
#define     MVPP2_BM_FORCE_RELEASE_MASK		BIT(12)

/* TX Scheduler registers */
#define MVPP2_TXP_SCHED_PORT_INDEX_REG		0x8000
#define MVPP2_TXP_SCHED_Q_CMD_REG		0x8004
#define     MVPP2_TXP_SCHED_ENQ_MASK		0xff
#define     MVPP2_TXP_SCHED_DISQ_OFFSET		8
#define MVPP2_TXP_SCHED_CMD_1_REG		0x8010
#define MVPP2_TXP_SCHED_PERIOD_REG		0x8018
#define MVPP2_TXP_SCHED_MTU_REG			0x801c
#define     MVPP2_TXP_MTU_MAX			0x7FFFF
#define MVPP2_TXP_SCHED_REFILL_REG		0x8020
#define     MVPP2_TXP_REFILL_TOKENS_ALL_MASK	0x7ffff
#define     MVPP2_TXP_REFILL_PERIOD_ALL_MASK	0x3ff00000
#define     MVPP2_TXP_REFILL_PERIOD_MASK(v)	((v) << 20)
#define MVPP2_TXP_SCHED_TOKEN_SIZE_REG		0x8024
#define     MVPP2_TXP_TOKEN_SIZE_MAX		0xffffffff
#define MVPP2_TXQ_SCHED_REFILL_REG(q)		(0x8040 + ((q) << 2))
#define     MVPP2_TXQ_REFILL_TOKENS_ALL_MASK	0x7ffff
#define     MVPP2_TXQ_REFILL_PERIOD_ALL_MASK	0x3ff00000
#define     MVPP2_TXQ_REFILL_PERIOD_MASK(v)	((v) << 20)
#define MVPP2_TXQ_SCHED_TOKEN_SIZE_REG(q)	(0x8060 + ((q) << 2))
#define     MVPP2_TXQ_TOKEN_SIZE_MAX		0x7fffffff
#define MVPP2_TXQ_SCHED_TOKEN_CNTR_REG(q)	(0x8080 + ((q) << 2))
#define     MVPP2_TXQ_TOKEN_CNTR_MAX		0xffffffff

/* TX general registers */
#define MVPP2_TX_SNOOP_REG			0x8800
#define MVPP2_TX_PORT_FLUSH_REG			0x8810
#define     MVPP2_TX_PORT_FLUSH_MASK(port)	(1 << (port))

/* LMS registers */
#define MVPP2_SRC_ADDR_MIDDLE			0x24
#define MVPP2_SRC_ADDR_HIGH			0x28
#define MVPP2_PHY_AN_CFG0_REG			0x34
#define     MVPP2_PHY_AN_STOP_SMI0_MASK		BIT(7)
#define MVPP2_MNG_EXTENDED_GLOBAL_CTRL_REG	0x305c
#define MVPP2_EXT_GLOBAL_CTRL_DEFAULT		0x27

/* Per-port registers */
#define MVPP2_GMAC_CTRL_0_REG			0x0
#define      MVPP2_GMAC_PORT_EN_MASK		BIT(0)
#define      MVPP2_GMAC_MAX_RX_SIZE_OFFS	2
#define      MVPP2_GMAC_MAX_RX_SIZE_MASK	0x7ffc
#define      MVPP2_GMAC_MIB_CNTR_EN_MASK	BIT(15)
#define MVPP2_GMAC_CTRL_1_REG			0x4
#define      MVPP2_GMAC_PERIODIC_XON_EN_MASK	BIT(1)
#define      MVPP2_GMAC_GMII_LB_EN_MASK		BIT(5)
#define      MVPP2_GMAC_PCS_LB_EN_BIT		6
#define      MVPP2_GMAC_PCS_LB_EN_MASK		BIT(6)
#define      MVPP2_GMAC_SA_LOW_OFFS		7
#define MVPP2_GMAC_CTRL_2_REG			0x8
#define      MVPP2_GMAC_INBAND_AN_MASK		BIT(0)
#define      MVPP2_GMAC_PCS_ENABLE_MASK		BIT(3)
#define      MVPP2_GMAC_PORT_RGMII_MASK		BIT(4)
#define      MVPP2_GMAC_PORT_RESET_MASK		BIT(6)
#define MVPP2_GMAC_AUTONEG_CONFIG		0xc
#define      MVPP2_GMAC_FORCE_LINK_DOWN		BIT(0)
#define      MVPP2_GMAC_FORCE_LINK_PASS		BIT(1)
#define      MVPP2_GMAC_CONFIG_MII_SPEED	BIT(5)
#define      MVPP2_GMAC_CONFIG_GMII_SPEED	BIT(6)
#define      MVPP2_GMAC_AN_SPEED_EN		BIT(7)
#define      MVPP2_GMAC_FC_ADV_EN		BIT(9)
#define      MVPP2_GMAC_CONFIG_FULL_DUPLEX	BIT(12)
#define      MVPP2_GMAC_AN_DUPLEX_EN		BIT(13)
#define MVPP2_GMAC_PORT_FIFO_CFG_1_REG		0x1c
#define      MVPP2_GMAC_TX_FIFO_MIN_TH_OFFS	6
#define      MVPP2_GMAC_TX_FIFO_MIN_TH_ALL_MASK	0x1fc0
#define      MVPP2_GMAC_TX_FIFO_MIN_TH_MASK(v)	(((v) << 6) & \
					MVPP2_GMAC_TX_FIFO_MIN_TH_ALL_MASK)

#define MVPP2_CAUSE_TXQ_SENT_DESC_ALL_MASK	0xff

/* Descriptor ring Macros */
#define MVPP2_QUEUE_NEXT_DESC(q, index) \
	(((index) < (q)->last_desc) ? ((index) + 1) : 0)

/* SMI: 0xc0054 -> offset 0x54 to lms_base */
#define MVPP2_SMI				0x0054
#define     MVPP2_PHY_REG_MASK			0x1f
/* SMI register fields */
#define     MVPP2_SMI_DATA_OFFS			0	/* Data */
#define     MVPP2_SMI_DATA_MASK			(0xffff << MVPP2_SMI_DATA_OFFS)
#define     MVPP2_SMI_DEV_ADDR_OFFS		16	/* PHY device address */
#define     MVPP2_SMI_REG_ADDR_OFFS		21	/* PHY device reg addr*/
#define     MVPP2_SMI_OPCODE_OFFS		26	/* Write/Read opcode */
#define     MVPP2_SMI_OPCODE_READ		(1 << MVPP2_SMI_OPCODE_OFFS)
#define     MVPP2_SMI_READ_VALID		(1 << 27)	/* Read Valid */
#define     MVPP2_SMI_BUSY			(1 << 28)	/* Busy */

#define     MVPP2_PHY_ADDR_MASK			0x1f
#define     MVPP2_PHY_REG_MASK			0x1f

/* Various constants */

/* Coalescing */
#define MVPP2_TXDONE_COAL_PKTS_THRESH	15
#define MVPP2_TXDONE_HRTIMER_PERIOD_NS	1000000UL
#define MVPP2_RX_COAL_PKTS		32
#define MVPP2_RX_COAL_USEC		100

/* The two bytes Marvell header. Either contains a special value used
 * by Marvell switches when a specific hardware mode is enabled (not
 * supported by this driver) or is filled automatically by zeroes on
 * the RX side. Those two bytes being at the front of the Ethernet
 * header, they allow to have the IP header aligned on a 4 bytes
 * boundary automatically: the hardware skips those two bytes on its
 * own.
 */
#define MVPP2_MH_SIZE			2
#define MVPP2_ETH_TYPE_LEN		2
#define MVPP2_PPPOE_HDR_SIZE		8
#define MVPP2_VLAN_TAG_LEN		4

/* Lbtd 802.3 type */
#define MVPP2_IP_LBDT_TYPE		0xfffa

#define MVPP2_CPU_D_CACHE_LINE_SIZE	32
#define MVPP2_TX_CSUM_MAX_SIZE		9800

/* Timeout constants */
#define MVPP2_TX_DISABLE_TIMEOUT_MSEC	1000
#define MVPP2_TX_PENDING_TIMEOUT_MSEC	1000

#define MVPP2_TX_MTU_MAX		0x7ffff

/* Maximum number of T-CONTs of PON port */
#define MVPP2_MAX_TCONT			16

/* Maximum number of supported ports */
#define MVPP2_MAX_PORTS			4

/* Maximum number of TXQs used by single port */
#define MVPP2_MAX_TXQ			8

/* Maximum number of RXQs used by single port */
#define MVPP2_MAX_RXQ			8

/* Default number of TXQs in use */
#define MVPP2_DEFAULT_TXQ		1

/* Dfault number of RXQs in use */
#define MVPP2_DEFAULT_RXQ		1
#define CONFIG_MV_ETH_RXQ		8	/* increment by 8 */

/* Total number of RXQs available to all ports */
#define MVPP2_RXQ_TOTAL_NUM		(MVPP2_MAX_PORTS * MVPP2_MAX_RXQ)

/* Max number of Rx descriptors */
#define MVPP2_MAX_RXD			16

/* Max number of Tx descriptors */
#define MVPP2_MAX_TXD			16

/* Amount of Tx descriptors that can be reserved at once by CPU */
#define MVPP2_CPU_DESC_CHUNK		64

/* Max number of Tx descriptors in each aggregated queue */
#define MVPP2_AGGR_TXQ_SIZE		256

/* Descriptor aligned size */
#define MVPP2_DESC_ALIGNED_SIZE		32

/* Descriptor alignment mask */
#define MVPP2_TX_DESC_ALIGN		(MVPP2_DESC_ALIGNED_SIZE - 1)

/* RX FIFO constants */
#define MVPP2_RX_FIFO_PORT_DATA_SIZE	0x2000
#define MVPP2_RX_FIFO_PORT_ATTR_SIZE	0x80
#define MVPP2_RX_FIFO_PORT_MIN_PKT	0x80

/* RX buffer constants */
#define MVPP2_SKB_SHINFO_SIZE \
	0

#define MVPP2_RX_PKT_SIZE(mtu) \
	ALIGN((mtu) + MVPP2_MH_SIZE + MVPP2_VLAN_TAG_LEN + \
	      ETH_HLEN + ETH_FCS_LEN, MVPP2_CPU_D_CACHE_LINE_SIZE)

#define MVPP2_RX_BUF_SIZE(pkt_size)	((pkt_size) + NET_SKB_PAD)
#define MVPP2_RX_TOTAL_SIZE(buf_size)	((buf_size) + MVPP2_SKB_SHINFO_SIZE)
#define MVPP2_RX_MAX_PKT_SIZE(total_size) \
	((total_size) - NET_SKB_PAD - MVPP2_SKB_SHINFO_SIZE)

#define MVPP2_BIT_TO_BYTE(bit)		((bit) / 8)

/* IPv6 max L3 address size */
#define MVPP2_MAX_L3_ADDR_SIZE		16

/* Port flags */
#define MVPP2_F_LOOPBACK		BIT(0)

/* Marvell tag types */
enum mvpp2_tag_type {
	MVPP2_TAG_TYPE_NONE = 0,
	MVPP2_TAG_TYPE_MH   = 1,
	MVPP2_TAG_TYPE_DSA  = 2,
	MVPP2_TAG_TYPE_EDSA = 3,
	MVPP2_TAG_TYPE_VLAN = 4,
	MVPP2_TAG_TYPE_LAST = 5
};

/* Parser constants */
#define MVPP2_PRS_TCAM_SRAM_SIZE	256
#define MVPP2_PRS_TCAM_WORDS		6
#define MVPP2_PRS_SRAM_WORDS		4
#define MVPP2_PRS_FLOW_ID_SIZE		64
#define MVPP2_PRS_FLOW_ID_MASK		0x3f
#define MVPP2_PRS_TCAM_ENTRY_INVALID	1
#define MVPP2_PRS_TCAM_DSA_TAGGED_BIT	BIT(5)
#define MVPP2_PRS_IPV4_HEAD		0x40
#define MVPP2_PRS_IPV4_HEAD_MASK	0xf0
#define MVPP2_PRS_IPV4_MC		0xe0
#define MVPP2_PRS_IPV4_MC_MASK		0xf0
#define MVPP2_PRS_IPV4_BC_MASK		0xff
#define MVPP2_PRS_IPV4_IHL		0x5
#define MVPP2_PRS_IPV4_IHL_MASK		0xf
#define MVPP2_PRS_IPV6_MC		0xff
#define MVPP2_PRS_IPV6_MC_MASK		0xff
#define MVPP2_PRS_IPV6_HOP_MASK		0xff
#define MVPP2_PRS_TCAM_PROTO_MASK	0xff
#define MVPP2_PRS_TCAM_PROTO_MASK_L	0x3f
#define MVPP2_PRS_DBL_VLANS_MAX		100

/* Tcam structure:
 * - lookup ID - 4 bits
 * - port ID - 1 byte
 * - additional information - 1 byte
 * - header data - 8 bytes
 * The fields are represented by MVPP2_PRS_TCAM_DATA_REG(5)->(0).
 */
#define MVPP2_PRS_AI_BITS			8
#define MVPP2_PRS_PORT_MASK			0xff
#define MVPP2_PRS_LU_MASK			0xf
#define MVPP2_PRS_TCAM_DATA_BYTE(offs)		\
				    (((offs) - ((offs) % 2)) * 2 + ((offs) % 2))
#define MVPP2_PRS_TCAM_DATA_BYTE_EN(offs)	\
					      (((offs) * 2) - ((offs) % 2)  + 2)
#define MVPP2_PRS_TCAM_AI_BYTE			16
#define MVPP2_PRS_TCAM_PORT_BYTE		17
#define MVPP2_PRS_TCAM_LU_BYTE			20
#define MVPP2_PRS_TCAM_EN_OFFS(offs)		((offs) + 2)
#define MVPP2_PRS_TCAM_INV_WORD			5
/* Tcam entries ID */
#define MVPP2_PE_DROP_ALL		0
#define MVPP2_PE_FIRST_FREE_TID		1
#define MVPP2_PE_LAST_FREE_TID		(MVPP2_PRS_TCAM_SRAM_SIZE - 31)
#define MVPP2_PE_IP6_EXT_PROTO_UN	(MVPP2_PRS_TCAM_SRAM_SIZE - 30)
#define MVPP2_PE_MAC_MC_IP6		(MVPP2_PRS_TCAM_SRAM_SIZE - 29)
#define MVPP2_PE_IP6_ADDR_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 28)
#define MVPP2_PE_IP4_ADDR_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 27)
#define MVPP2_PE_LAST_DEFAULT_FLOW	(MVPP2_PRS_TCAM_SRAM_SIZE - 26)
#define MVPP2_PE_FIRST_DEFAULT_FLOW	(MVPP2_PRS_TCAM_SRAM_SIZE - 19)
#define MVPP2_PE_EDSA_TAGGED		(MVPP2_PRS_TCAM_SRAM_SIZE - 18)
#define MVPP2_PE_EDSA_UNTAGGED		(MVPP2_PRS_TCAM_SRAM_SIZE - 17)
#define MVPP2_PE_DSA_TAGGED		(MVPP2_PRS_TCAM_SRAM_SIZE - 16)
#define MVPP2_PE_DSA_UNTAGGED		(MVPP2_PRS_TCAM_SRAM_SIZE - 15)
#define MVPP2_PE_ETYPE_EDSA_TAGGED	(MVPP2_PRS_TCAM_SRAM_SIZE - 14)
#define MVPP2_PE_ETYPE_EDSA_UNTAGGED	(MVPP2_PRS_TCAM_SRAM_SIZE - 13)
#define MVPP2_PE_ETYPE_DSA_TAGGED	(MVPP2_PRS_TCAM_SRAM_SIZE - 12)
#define MVPP2_PE_ETYPE_DSA_UNTAGGED	(MVPP2_PRS_TCAM_SRAM_SIZE - 11)
#define MVPP2_PE_MH_DEFAULT		(MVPP2_PRS_TCAM_SRAM_SIZE - 10)
#define MVPP2_PE_DSA_DEFAULT		(MVPP2_PRS_TCAM_SRAM_SIZE - 9)
#define MVPP2_PE_IP6_PROTO_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 8)
#define MVPP2_PE_IP4_PROTO_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 7)
#define MVPP2_PE_ETH_TYPE_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 6)
#define MVPP2_PE_VLAN_DBL		(MVPP2_PRS_TCAM_SRAM_SIZE - 5)
#define MVPP2_PE_VLAN_NONE		(MVPP2_PRS_TCAM_SRAM_SIZE - 4)
#define MVPP2_PE_MAC_MC_ALL		(MVPP2_PRS_TCAM_SRAM_SIZE - 3)
#define MVPP2_PE_MAC_PROMISCUOUS	(MVPP2_PRS_TCAM_SRAM_SIZE - 2)
#define MVPP2_PE_MAC_NON_PROMISCUOUS	(MVPP2_PRS_TCAM_SRAM_SIZE - 1)

/* Sram structure
 * The fields are represented by MVPP2_PRS_TCAM_DATA_REG(3)->(0).
 */
#define MVPP2_PRS_SRAM_RI_OFFS			0
#define MVPP2_PRS_SRAM_RI_WORD			0
#define MVPP2_PRS_SRAM_RI_CTRL_OFFS		32
#define MVPP2_PRS_SRAM_RI_CTRL_WORD		1
#define MVPP2_PRS_SRAM_RI_CTRL_BITS		32
#define MVPP2_PRS_SRAM_SHIFT_OFFS		64
#define MVPP2_PRS_SRAM_SHIFT_SIGN_BIT		72
#define MVPP2_PRS_SRAM_UDF_OFFS			73
#define MVPP2_PRS_SRAM_UDF_BITS			8
#define MVPP2_PRS_SRAM_UDF_MASK			0xff
#define MVPP2_PRS_SRAM_UDF_SIGN_BIT		81
#define MVPP2_PRS_SRAM_UDF_TYPE_OFFS		82
#define MVPP2_PRS_SRAM_UDF_TYPE_MASK		0x7
#define MVPP2_PRS_SRAM_UDF_TYPE_L3		1
#define MVPP2_PRS_SRAM_UDF_TYPE_L4		4
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_OFFS	85
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_MASK	0x3
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD		1
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_IP4_ADD	2
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_IP6_ADD	3
#define MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS		87
#define MVPP2_PRS_SRAM_OP_SEL_UDF_BITS		2
#define MVPP2_PRS_SRAM_OP_SEL_UDF_MASK		0x3
#define MVPP2_PRS_SRAM_OP_SEL_UDF_ADD		0
#define MVPP2_PRS_SRAM_OP_SEL_UDF_IP4_ADD	2
#define MVPP2_PRS_SRAM_OP_SEL_UDF_IP6_ADD	3
#define MVPP2_PRS_SRAM_OP_SEL_BASE_OFFS		89
#define MVPP2_PRS_SRAM_AI_OFFS			90
#define MVPP2_PRS_SRAM_AI_CTRL_OFFS		98
#define MVPP2_PRS_SRAM_AI_CTRL_BITS		8
#define MVPP2_PRS_SRAM_AI_MASK			0xff
#define MVPP2_PRS_SRAM_NEXT_LU_OFFS		106
#define MVPP2_PRS_SRAM_NEXT_LU_MASK		0xf
#define MVPP2_PRS_SRAM_LU_DONE_BIT		110
#define MVPP2_PRS_SRAM_LU_GEN_BIT		111

/* Sram result info bits assignment */
#define MVPP2_PRS_RI_MAC_ME_MASK		0x1
#define MVPP2_PRS_RI_DSA_MASK			0x2
#define MVPP2_PRS_RI_VLAN_MASK			0xc
#define MVPP2_PRS_RI_VLAN_NONE			~(BIT(2) | BIT(3))
#define MVPP2_PRS_RI_VLAN_SINGLE		BIT(2)
#define MVPP2_PRS_RI_VLAN_DOUBLE		BIT(3)
#define MVPP2_PRS_RI_VLAN_TRIPLE		(BIT(2) | BIT(3))
#define MVPP2_PRS_RI_CPU_CODE_MASK		0x70
#define MVPP2_PRS_RI_CPU_CODE_RX_SPEC		BIT(4)
#define MVPP2_PRS_RI_L2_CAST_MASK		0x600
#define MVPP2_PRS_RI_L2_UCAST			~(BIT(9) | BIT(10))
#define MVPP2_PRS_RI_L2_MCAST			BIT(9)
#define MVPP2_PRS_RI_L2_BCAST			BIT(10)
#define MVPP2_PRS_RI_PPPOE_MASK			0x800
#define MVPP2_PRS_RI_L3_PROTO_MASK		0x7000
#define MVPP2_PRS_RI_L3_UN			~(BIT(12) | BIT(13) | BIT(14))
#define MVPP2_PRS_RI_L3_IP4			BIT(12)
#define MVPP2_PRS_RI_L3_IP4_OPT			BIT(13)
#define MVPP2_PRS_RI_L3_IP4_OTHER		(BIT(12) | BIT(13))
#define MVPP2_PRS_RI_L3_IP6			BIT(14)
#define MVPP2_PRS_RI_L3_IP6_EXT			(BIT(12) | BIT(14))
#define MVPP2_PRS_RI_L3_ARP			(BIT(13) | BIT(14))
#define MVPP2_PRS_RI_L3_ADDR_MASK		0x18000
#define MVPP2_PRS_RI_L3_UCAST			~(BIT(15) | BIT(16))
#define MVPP2_PRS_RI_L3_MCAST			BIT(15)
#define MVPP2_PRS_RI_L3_BCAST			(BIT(15) | BIT(16))
#define MVPP2_PRS_RI_IP_FRAG_MASK		0x20000
#define MVPP2_PRS_RI_UDF3_MASK			0x300000
#define MVPP2_PRS_RI_UDF3_RX_SPECIAL		BIT(21)
#define MVPP2_PRS_RI_L4_PROTO_MASK		0x1c00000
#define MVPP2_PRS_RI_L4_TCP			BIT(22)
#define MVPP2_PRS_RI_L4_UDP			BIT(23)
#define MVPP2_PRS_RI_L4_OTHER			(BIT(22) | BIT(23))
#define MVPP2_PRS_RI_UDF7_MASK			0x60000000
#define MVPP2_PRS_RI_UDF7_IP6_LITE		BIT(29)
#define MVPP2_PRS_RI_DROP_MASK			0x80000000

/* Sram additional info bits assignment */
#define MVPP2_PRS_IPV4_DIP_AI_BIT		BIT(0)
#define MVPP2_PRS_IPV6_NO_EXT_AI_BIT		BIT(0)
#define MVPP2_PRS_IPV6_EXT_AI_BIT		BIT(1)
#define MVPP2_PRS_IPV6_EXT_AH_AI_BIT		BIT(2)
#define MVPP2_PRS_IPV6_EXT_AH_LEN_AI_BIT	BIT(3)
#define MVPP2_PRS_IPV6_EXT_AH_L4_AI_BIT		BIT(4)
#define MVPP2_PRS_SINGLE_VLAN_AI		0
#define MVPP2_PRS_DBL_VLAN_AI_BIT		BIT(7)

/* DSA/EDSA type */
#define MVPP2_PRS_TAGGED		true
#define MVPP2_PRS_UNTAGGED		false
#define MVPP2_PRS_EDSA			true
#define MVPP2_PRS_DSA			false

/* MAC entries, shadow udf */
enum mvpp2_prs_udf {
	MVPP2_PRS_UDF_MAC_DEF,
	MVPP2_PRS_UDF_MAC_RANGE,
	MVPP2_PRS_UDF_L2_DEF,
	MVPP2_PRS_UDF_L2_DEF_COPY,
	MVPP2_PRS_UDF_L2_USER,
};

/* Lookup ID */
enum mvpp2_prs_lookup {
	MVPP2_PRS_LU_MH,
	MVPP2_PRS_LU_MAC,
	MVPP2_PRS_LU_DSA,
	MVPP2_PRS_LU_VLAN,
	MVPP2_PRS_LU_L2,
	MVPP2_PRS_LU_PPPOE,
	MVPP2_PRS_LU_IP4,
	MVPP2_PRS_LU_IP6,
	MVPP2_PRS_LU_FLOWS,
	MVPP2_PRS_LU_LAST,
};

/* L3 cast enum */
enum mvpp2_prs_l3_cast {
	MVPP2_PRS_L3_UNI_CAST,
	MVPP2_PRS_L3_MULTI_CAST,
	MVPP2_PRS_L3_BROAD_CAST
};

/* Classifier constants */
#define MVPP2_CLS_FLOWS_TBL_SIZE	512
#define MVPP2_CLS_FLOWS_TBL_DATA_WORDS	3
#define MVPP2_CLS_LKP_TBL_SIZE		64

/* BM constants */
#define MVPP2_BM_POOLS_NUM		1
#define MVPP2_BM_LONG_BUF_NUM		16
#define MVPP2_BM_SHORT_BUF_NUM		16
#define MVPP2_BM_POOL_SIZE_MAX		(16*1024 - MVPP2_BM_POOL_PTR_ALIGN/4)
#define MVPP2_BM_POOL_PTR_ALIGN		128
#define MVPP2_BM_SWF_LONG_POOL(port)	0

/* BM cookie (32 bits) definition */
#define MVPP2_BM_COOKIE_POOL_OFFS	8
#define MVPP2_BM_COOKIE_CPU_OFFS	24

/* BM short pool packet size
 * These value assure that for SWF the total number
 * of bytes allocated for each buffer will be 512
 */
#define MVPP2_BM_SHORT_PKT_SIZE		MVPP2_RX_MAX_PKT_SIZE(512)

enum mvpp2_bm_type {
	MVPP2_BM_FREE,
	MVPP2_BM_SWF_LONG,
	MVPP2_BM_SWF_SHORT
};

/* Definitions */

/* Shared Packet Processor resources */
struct mvpp2 {
	/* Shared registers' base addresses */
	void __iomem *base;
	void __iomem *lms_base;

	/* List of pointers to port structures */
	struct mvpp2_port **port_list;

	/* Aggregated TXQs */
	struct mvpp2_tx_queue *aggr_txqs;

	/* BM pools */
	struct mvpp2_bm_pool *bm_pools;

	/* PRS shadow table */
	struct mvpp2_prs_shadow *prs_shadow;
	/* PRS auxiliary table for double vlan entries control */
	bool *prs_double_vlans;

	/* Tclk value */
	u32 tclk;

	struct mii_dev *bus;
};

struct mvpp2_pcpu_stats {
	u64	rx_packets;
	u64	rx_bytes;
	u64	tx_packets;
	u64	tx_bytes;
};

struct mvpp2_port {
	u8 id;

	int irq;

	struct mvpp2 *priv;

	/* Per-port registers' base address */
	void __iomem *base;

	struct mvpp2_rx_queue **rxqs;
	struct mvpp2_tx_queue **txqs;

	int pkt_size;

	u32 pending_cause_rx;

	/* Per-CPU port control */
	struct mvpp2_port_pcpu __percpu *pcpu;

	/* Flags */
	unsigned long flags;

	u16 tx_ring_size;
	u16 rx_ring_size;
	struct mvpp2_pcpu_stats __percpu *stats;

	struct phy_device *phy_dev;
	phy_interface_t phy_interface;
	int phy_node;
	int phyaddr;
	int init;
	unsigned int link;
	unsigned int duplex;
	unsigned int speed;

	struct mvpp2_bm_pool *pool_long;
	struct mvpp2_bm_pool *pool_short;

	/* Index of first port's physical RXQ */
	u8 first_rxq;

	u8 dev_addr[ETH_ALEN];
};

/* The mvpp2_tx_desc and mvpp2_rx_desc structures describe the
 * layout of the transmit and reception DMA descriptors, and their
 * layout is therefore defined by the hardware design
 */

#define MVPP2_TXD_L3_OFF_SHIFT		0
#define MVPP2_TXD_IP_HLEN_SHIFT		8
#define MVPP2_TXD_L4_CSUM_FRAG		BIT(13)
#define MVPP2_TXD_L4_CSUM_NOT		BIT(14)
#define MVPP2_TXD_IP_CSUM_DISABLE	BIT(15)
#define MVPP2_TXD_PADDING_DISABLE	BIT(23)
#define MVPP2_TXD_L4_UDP		BIT(24)
#define MVPP2_TXD_L3_IP6		BIT(26)
#define MVPP2_TXD_L_DESC		BIT(28)
#define MVPP2_TXD_F_DESC		BIT(29)

#define MVPP2_RXD_ERR_SUMMARY		BIT(15)
#define MVPP2_RXD_ERR_CODE_MASK		(BIT(13) | BIT(14))
#define MVPP2_RXD_ERR_CRC		0x0
#define MVPP2_RXD_ERR_OVERRUN		BIT(13)
#define MVPP2_RXD_ERR_RESOURCE		(BIT(13) | BIT(14))
#define MVPP2_RXD_BM_POOL_ID_OFFS	16
#define MVPP2_RXD_BM_POOL_ID_MASK	(BIT(16) | BIT(17) | BIT(18))
#define MVPP2_RXD_HWF_SYNC		BIT(21)
#define MVPP2_RXD_L4_CSUM_OK		BIT(22)
#define MVPP2_RXD_IP4_HEADER_ERR	BIT(24)
#define MVPP2_RXD_L4_TCP		BIT(25)
#define MVPP2_RXD_L4_UDP		BIT(26)
#define MVPP2_RXD_L3_IP4		BIT(28)
#define MVPP2_RXD_L3_IP6		BIT(30)
#define MVPP2_RXD_BUF_HDR		BIT(31)

struct mvpp2_tx_desc {
	u32 command;		/* Options used by HW for packet transmitting.*/
	u8  packet_offset;	/* the offset from the buffer beginning	*/
	u8  phys_txq;		/* destination queue ID			*/
	u16 data_size;		/* data size of transmitted packet in bytes */
	u32 buf_phys_addr;	/* physical addr of transmitted buffer	*/
	u32 buf_cookie;		/* cookie for access to TX buffer in tx path */
	u32 reserved1[3];	/* hw_cmd (for future use, BM, PON, PNC) */
	u32 reserved2;		/* reserved (for future use)		*/
};

struct mvpp2_rx_desc {
	u32 status;		/* info about received packet		*/
	u16 reserved1;		/* parser_info (for future use, PnC)	*/
	u16 data_size;		/* size of received packet in bytes	*/
	u32 buf_phys_addr;	/* physical address of the buffer	*/
	u32 buf_cookie;		/* cookie for access to RX buffer in rx path */
	u16 reserved2;		/* gem_port_id (for future use, PON)	*/
	u16 reserved3;		/* csum_l4 (for future use, PnC)	*/
	u8  reserved4;		/* bm_qset (for future use, BM)		*/
	u8  reserved5;
	u16 reserved6;		/* classify_info (for future use, PnC)	*/
	u32 reserved7;		/* flow_id (for future use, PnC) */
	u32 reserved8;
};

/* Per-CPU Tx queue control */
struct mvpp2_txq_pcpu {
	int cpu;

	/* Number of Tx DMA descriptors in the descriptor ring */
	int size;

	/* Number of currently used Tx DMA descriptor in the
	 * descriptor ring
	 */
	int count;

	/* Number of Tx DMA descriptors reserved for each CPU */
	int reserved_num;

	/* Index of last TX DMA descriptor that was inserted */
	int txq_put_index;

	/* Index of the TX DMA descriptor to be cleaned up */
	int txq_get_index;
};

struct mvpp2_tx_queue {
	/* Physical number of this Tx queue */
	u8 id;

	/* Logical number of this Tx queue */
	u8 log_id;

	/* Number of Tx DMA descriptors in the descriptor ring */
	int size;

	/* Number of currently used Tx DMA descriptor in the descriptor ring */
	int count;

	/* Per-CPU control of physical Tx queues */
	struct mvpp2_txq_pcpu __percpu *pcpu;

	u32 done_pkts_coal;

	/* Virtual address of thex Tx DMA descriptors array */
	struct mvpp2_tx_desc *descs;

	/* DMA address of the Tx DMA descriptors array */
	dma_addr_t descs_phys;

	/* Index of the last Tx DMA descriptor */
	int last_desc;

	/* Index of the next Tx DMA descriptor to process */
	int next_desc_to_proc;
};

struct mvpp2_rx_queue {
	/* RX queue number, in the range 0-31 for physical RXQs */
	u8 id;

	/* Num of rx descriptors in the rx descriptor ring */
	int size;

	u32 pkts_coal;
	u32 time_coal;

	/* Virtual address of the RX DMA descriptors array */
	struct mvpp2_rx_desc *descs;

	/* DMA address of the RX DMA descriptors array */
	dma_addr_t descs_phys;

	/* Index of the last RX DMA descriptor */
	int last_desc;

	/* Index of the next RX DMA descriptor to process */
	int next_desc_to_proc;

	/* ID of port to which physical RXQ is mapped */
	int port;

	/* Port's logic RXQ number to which physical RXQ is mapped */
	int logic_rxq;
};

union mvpp2_prs_tcam_entry {
	u32 word[MVPP2_PRS_TCAM_WORDS];
	u8  byte[MVPP2_PRS_TCAM_WORDS * 4];
};

union mvpp2_prs_sram_entry {
	u32 word[MVPP2_PRS_SRAM_WORDS];
	u8  byte[MVPP2_PRS_SRAM_WORDS * 4];
};

struct mvpp2_prs_entry {
	u32 index;
	union mvpp2_prs_tcam_entry tcam;
	union mvpp2_prs_sram_entry sram;
};

struct mvpp2_prs_shadow {
	bool valid;
	bool finish;

	/* Lookup ID */
	int lu;

	/* User defined offset */
	int udf;

	/* Result info */
	u32 ri;
	u32 ri_mask;
};

struct mvpp2_cls_flow_entry {
	u32 index;
	u32 data[MVPP2_CLS_FLOWS_TBL_DATA_WORDS];
};

struct mvpp2_cls_lookup_entry {
	u32 lkpid;
	u32 way;
	u32 data;
};

struct mvpp2_bm_pool {
	/* Pool number in the range 0-7 */
	int id;
	enum mvpp2_bm_type type;

	/* Buffer Pointers Pool External (BPPE) size */
	int size;
	/* Number of buffers for this pool */
	int buf_num;
	/* Pool buffer size */
	int buf_size;
	/* Packet size */
	int pkt_size;

	/* BPPE virtual base address */
	u32 *virt_addr;
	/* BPPE physical base address */
	dma_addr_t phys_addr;

	/* Ports using BM pool */
	u32 port_map;

	/* Occupied buffers indicator */
	int in_use_thresh;
};

struct mvpp2_buff_hdr {
	u32 next_buff_phys_addr;
	u32 next_buff_virt_addr;
	u16 byte_count;
	u16 info;
	u8  reserved1;		/* bm_qset (for future use, BM)		*/
};

/* Buffer header info bits */
#define MVPP2_B_HDR_INFO_MC_ID_MASK	0xfff
#define MVPP2_B_HDR_INFO_MC_ID(info)	((info) & MVPP2_B_HDR_INFO_MC_ID_MASK)
#define MVPP2_B_HDR_INFO_LAST_OFFS	12
#define MVPP2_B_HDR_INFO_LAST_MASK	BIT(12)
#define MVPP2_B_HDR_INFO_IS_LAST(info) \
	   ((info & MVPP2_B_HDR_INFO_LAST_MASK) >> MVPP2_B_HDR_INFO_LAST_OFFS)

/* Static declaractions */

/* Number of RXQs used by single port */
static int rxq_number = MVPP2_DEFAULT_RXQ;
/* Number of TXQs used by single port */
static int txq_number = MVPP2_DEFAULT_TXQ;

#define MVPP2_DRIVER_NAME "mvpp2"
#define MVPP2_DRIVER_VERSION "1.0"

/*
 * U-Boot internal data, mostly uncached buffers for descriptors and data
 */
struct buffer_location {
	struct mvpp2_tx_desc *aggr_tx_descs;
	struct mvpp2_tx_desc *tx_descs;
	struct mvpp2_rx_desc *rx_descs;
	u32 *bm_pool[MVPP2_BM_POOLS_NUM];
	u32 *rx_buffer[MVPP2_BM_LONG_BUF_NUM];
	int first_rxq;
};

/*
 * All 4 interfaces use the same global buffer, since only one interface
 * can be enabled at once
 */
static struct buffer_location buffer_loc;

/*
 * Page table entries are set to 1MB, or multiples of 1MB
 * (not < 1MB). driver uses less bd's so use 1MB bdspace.
 */
#define BD_SPACE	(1 << 20)

/* Utility/helper methods */

static void mvpp2_write(struct mvpp2 *priv, u32 offset, u32 data)
{
	writel(data, priv->base + offset);
}

static u32 mvpp2_read(struct mvpp2 *priv, u32 offset)
{
	return readl(priv->base + offset);
}

static void mvpp2_txq_inc_get(struct mvpp2_txq_pcpu *txq_pcpu)
{
	txq_pcpu->txq_get_index++;
	if (txq_pcpu->txq_get_index == txq_pcpu->size)
		txq_pcpu->txq_get_index = 0;
}

/* Get number of physical egress port */
static inline int mvpp2_egress_port(struct mvpp2_port *port)
{
	return MVPP2_MAX_TCONT + port->id;
}

/* Get number of physical TXQ */
static inline int mvpp2_txq_phys(int port, int txq)
{
	return (MVPP2_MAX_TCONT + port) * MVPP2_MAX_TXQ + txq;
}

/* Parser configuration routines */

/* Update parser tcam and sram hw entries */
static int mvpp2_prs_hw_write(struct mvpp2 *priv, struct mvpp2_prs_entry *pe)
{
	int i;

	if (pe->index > MVPP2_PRS_TCAM_SRAM_SIZE - 1)
		return -EINVAL;

	/* Clear entry invalidation bit */
	pe->tcam.word[MVPP2_PRS_TCAM_INV_WORD] &= ~MVPP2_PRS_TCAM_INV_MASK;

	/* Write tcam index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_TCAM_IDX_REG, pe->index);
	for (i = 0; i < MVPP2_PRS_TCAM_WORDS; i++)
		mvpp2_write(priv, MVPP2_PRS_TCAM_DATA_REG(i), pe->tcam.word[i]);

	/* Write sram index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_SRAM_IDX_REG, pe->index);
	for (i = 0; i < MVPP2_PRS_SRAM_WORDS; i++)
		mvpp2_write(priv, MVPP2_PRS_SRAM_DATA_REG(i), pe->sram.word[i]);

	return 0;
}

/* Read tcam entry from hw */
static int mvpp2_prs_hw_read(struct mvpp2 *priv, struct mvpp2_prs_entry *pe)
{
	int i;

	if (pe->index > MVPP2_PRS_TCAM_SRAM_SIZE - 1)
		return -EINVAL;

	/* Write tcam index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_TCAM_IDX_REG, pe->index);

	pe->tcam.word[MVPP2_PRS_TCAM_INV_WORD] = mvpp2_read(priv,
			      MVPP2_PRS_TCAM_DATA_REG(MVPP2_PRS_TCAM_INV_WORD));
	if (pe->tcam.word[MVPP2_PRS_TCAM_INV_WORD] & MVPP2_PRS_TCAM_INV_MASK)
		return MVPP2_PRS_TCAM_ENTRY_INVALID;

	for (i = 0; i < MVPP2_PRS_TCAM_WORDS; i++)
		pe->tcam.word[i] = mvpp2_read(priv, MVPP2_PRS_TCAM_DATA_REG(i));

	/* Write sram index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_SRAM_IDX_REG, pe->index);
	for (i = 0; i < MVPP2_PRS_SRAM_WORDS; i++)
		pe->sram.word[i] = mvpp2_read(priv, MVPP2_PRS_SRAM_DATA_REG(i));

	return 0;
}

/* Invalidate tcam hw entry */
static void mvpp2_prs_hw_inv(struct mvpp2 *priv, int index)
{
	/* Write index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_TCAM_IDX_REG, index);
	mvpp2_write(priv, MVPP2_PRS_TCAM_DATA_REG(MVPP2_PRS_TCAM_INV_WORD),
		    MVPP2_PRS_TCAM_INV_MASK);
}

/* Enable shadow table entry and set its lookup ID */
static void mvpp2_prs_shadow_set(struct mvpp2 *priv, int index, int lu)
{
	priv->prs_shadow[index].valid = true;
	priv->prs_shadow[index].lu = lu;
}

/* Update ri fields in shadow table entry */
static void mvpp2_prs_shadow_ri_set(struct mvpp2 *priv, int index,
				    unsigned int ri, unsigned int ri_mask)
{
	priv->prs_shadow[index].ri_mask = ri_mask;
	priv->prs_shadow[index].ri = ri;
}

/* Update lookup field in tcam sw entry */
static void mvpp2_prs_tcam_lu_set(struct mvpp2_prs_entry *pe, unsigned int lu)
{
	int enable_off = MVPP2_PRS_TCAM_EN_OFFS(MVPP2_PRS_TCAM_LU_BYTE);

	pe->tcam.byte[MVPP2_PRS_TCAM_LU_BYTE] = lu;
	pe->tcam.byte[enable_off] = MVPP2_PRS_LU_MASK;
}

/* Update mask for single port in tcam sw entry */
static void mvpp2_prs_tcam_port_set(struct mvpp2_prs_entry *pe,
				    unsigned int port, bool add)
{
	int enable_off = MVPP2_PRS_TCAM_EN_OFFS(MVPP2_PRS_TCAM_PORT_BYTE);

	if (add)
		pe->tcam.byte[enable_off] &= ~(1 << port);
	else
		pe->tcam.byte[enable_off] |= 1 << port;
}

/* Update port map in tcam sw entry */
static void mvpp2_prs_tcam_port_map_set(struct mvpp2_prs_entry *pe,
					unsigned int ports)
{
	unsigned char port_mask = MVPP2_PRS_PORT_MASK;
	int enable_off = MVPP2_PRS_TCAM_EN_OFFS(MVPP2_PRS_TCAM_PORT_BYTE);

	pe->tcam.byte[MVPP2_PRS_TCAM_PORT_BYTE] = 0;
	pe->tcam.byte[enable_off] &= ~port_mask;
	pe->tcam.byte[enable_off] |= ~ports & MVPP2_PRS_PORT_MASK;
}

/* Obtain port map from tcam sw entry */
static unsigned int mvpp2_prs_tcam_port_map_get(struct mvpp2_prs_entry *pe)
{
	int enable_off = MVPP2_PRS_TCAM_EN_OFFS(MVPP2_PRS_TCAM_PORT_BYTE);

	return ~(pe->tcam.byte[enable_off]) & MVPP2_PRS_PORT_MASK;
}

/* Set byte of data and its enable bits in tcam sw entry */
static void mvpp2_prs_tcam_data_byte_set(struct mvpp2_prs_entry *pe,
					 unsigned int offs, unsigned char byte,
					 unsigned char enable)
{
	pe->tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE(offs)] = byte;
	pe->tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE_EN(offs)] = enable;
}

/* Get byte of data and its enable bits from tcam sw entry */
static void mvpp2_prs_tcam_data_byte_get(struct mvpp2_prs_entry *pe,
					 unsigned int offs, unsigned char *byte,
					 unsigned char *enable)
{
	*byte = pe->tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE(offs)];
	*enable = pe->tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE_EN(offs)];
}

/* Set ethertype in tcam sw entry */
static void mvpp2_prs_match_etype(struct mvpp2_prs_entry *pe, int offset,
				  unsigned short ethertype)
{
	mvpp2_prs_tcam_data_byte_set(pe, offset + 0, ethertype >> 8, 0xff);
	mvpp2_prs_tcam_data_byte_set(pe, offset + 1, ethertype & 0xff, 0xff);
}

/* Set bits in sram sw entry */
static void mvpp2_prs_sram_bits_set(struct mvpp2_prs_entry *pe, int bit_num,
				    int val)
{
	pe->sram.byte[MVPP2_BIT_TO_BYTE(bit_num)] |= (val << (bit_num % 8));
}

/* Clear bits in sram sw entry */
static void mvpp2_prs_sram_bits_clear(struct mvpp2_prs_entry *pe, int bit_num,
				      int val)
{
	pe->sram.byte[MVPP2_BIT_TO_BYTE(bit_num)] &= ~(val << (bit_num % 8));
}

/* Update ri bits in sram sw entry */
static void mvpp2_prs_sram_ri_update(struct mvpp2_prs_entry *pe,
				     unsigned int bits, unsigned int mask)
{
	unsigned int i;

	for (i = 0; i < MVPP2_PRS_SRAM_RI_CTRL_BITS; i++) {
		int ri_off = MVPP2_PRS_SRAM_RI_OFFS;

		if (!(mask & BIT(i)))
			continue;

		if (bits & BIT(i))
			mvpp2_prs_sram_bits_set(pe, ri_off + i, 1);
		else
			mvpp2_prs_sram_bits_clear(pe, ri_off + i, 1);

		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_RI_CTRL_OFFS + i, 1);
	}
}

/* Update ai bits in sram sw entry */
static void mvpp2_prs_sram_ai_update(struct mvpp2_prs_entry *pe,
				     unsigned int bits, unsigned int mask)
{
	unsigned int i;
	int ai_off = MVPP2_PRS_SRAM_AI_OFFS;

	for (i = 0; i < MVPP2_PRS_SRAM_AI_CTRL_BITS; i++) {

		if (!(mask & BIT(i)))
			continue;

		if (bits & BIT(i))
			mvpp2_prs_sram_bits_set(pe, ai_off + i, 1);
		else
			mvpp2_prs_sram_bits_clear(pe, ai_off + i, 1);

		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_AI_CTRL_OFFS + i, 1);
	}
}

/* Read ai bits from sram sw entry */
static int mvpp2_prs_sram_ai_get(struct mvpp2_prs_entry *pe)
{
	u8 bits;
	int ai_off = MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_AI_OFFS);
	int ai_en_off = ai_off + 1;
	int ai_shift = MVPP2_PRS_SRAM_AI_OFFS % 8;

	bits = (pe->sram.byte[ai_off] >> ai_shift) |
	       (pe->sram.byte[ai_en_off] << (8 - ai_shift));

	return bits;
}

/* In sram sw entry set lookup ID field of the tcam key to be used in the next
 * lookup interation
 */
static void mvpp2_prs_sram_next_lu_set(struct mvpp2_prs_entry *pe,
				       unsigned int lu)
{
	int sram_next_off = MVPP2_PRS_SRAM_NEXT_LU_OFFS;

	mvpp2_prs_sram_bits_clear(pe, sram_next_off,
				  MVPP2_PRS_SRAM_NEXT_LU_MASK);
	mvpp2_prs_sram_bits_set(pe, sram_next_off, lu);
}

/* In the sram sw entry set sign and value of the next lookup offset
 * and the offset value generated to the classifier
 */
static void mvpp2_prs_sram_shift_set(struct mvpp2_prs_entry *pe, int shift,
				     unsigned int op)
{
	/* Set sign */
	if (shift < 0) {
		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_SHIFT_SIGN_BIT, 1);
		shift = 0 - shift;
	} else {
		mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_SHIFT_SIGN_BIT, 1);
	}

	/* Set value */
	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_SHIFT_OFFS)] =
							   (unsigned char)shift;

	/* Reset and set operation */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_OP_SEL_SHIFT_OFFS,
				  MVPP2_PRS_SRAM_OP_SEL_SHIFT_MASK);
	mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_OP_SEL_SHIFT_OFFS, op);

	/* Set base offset as current */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_OP_SEL_BASE_OFFS, 1);
}

/* In the sram sw entry set sign and value of the user defined offset
 * generated to the classifier
 */
static void mvpp2_prs_sram_offset_set(struct mvpp2_prs_entry *pe,
				      unsigned int type, int offset,
				      unsigned int op)
{
	/* Set sign */
	if (offset < 0) {
		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_UDF_SIGN_BIT, 1);
		offset = 0 - offset;
	} else {
		mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_UDF_SIGN_BIT, 1);
	}

	/* Set value */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_UDF_OFFS,
				  MVPP2_PRS_SRAM_UDF_MASK);
	mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_UDF_OFFS, offset);
	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_UDF_OFFS +
					MVPP2_PRS_SRAM_UDF_BITS)] &=
	      ~(MVPP2_PRS_SRAM_UDF_MASK >> (8 - (MVPP2_PRS_SRAM_UDF_OFFS % 8)));
	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_UDF_OFFS +
					MVPP2_PRS_SRAM_UDF_BITS)] |=
				(offset >> (8 - (MVPP2_PRS_SRAM_UDF_OFFS % 8)));

	/* Set offset type */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_UDF_TYPE_OFFS,
				  MVPP2_PRS_SRAM_UDF_TYPE_MASK);
	mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_UDF_TYPE_OFFS, type);

	/* Set offset operation */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_MASK);
	mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS, op);

	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS +
					MVPP2_PRS_SRAM_OP_SEL_UDF_BITS)] &=
					     ~(MVPP2_PRS_SRAM_OP_SEL_UDF_MASK >>
				    (8 - (MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS % 8)));

	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS +
					MVPP2_PRS_SRAM_OP_SEL_UDF_BITS)] |=
			     (op >> (8 - (MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS % 8)));

	/* Set base offset as current */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_OP_SEL_BASE_OFFS, 1);
}

/* Find parser flow entry */
static struct mvpp2_prs_entry *mvpp2_prs_flow_find(struct mvpp2 *priv, int flow)
{
	struct mvpp2_prs_entry *pe;
	int tid;

	pe = kzalloc(sizeof(*pe), GFP_KERNEL);
	if (!pe)
		return NULL;
	mvpp2_prs_tcam_lu_set(pe, MVPP2_PRS_LU_FLOWS);

	/* Go through the all entires with MVPP2_PRS_LU_FLOWS */
	for (tid = MVPP2_PRS_TCAM_SRAM_SIZE - 1; tid >= 0; tid--) {
		u8 bits;

		if (!priv->prs_shadow[tid].valid ||
		    priv->prs_shadow[tid].lu != MVPP2_PRS_LU_FLOWS)
			continue;

		pe->index = tid;
		mvpp2_prs_hw_read(priv, pe);
		bits = mvpp2_prs_sram_ai_get(pe);

		/* Sram store classification lookup ID in AI bits [5:0] */
		if ((bits & MVPP2_PRS_FLOW_ID_MASK) == flow)
			return pe;
	}
	kfree(pe);

	return NULL;
}

/* Return first free tcam index, seeking from start to end */
static int mvpp2_prs_tcam_first_free(struct mvpp2 *priv, unsigned char start,
				     unsigned char end)
{
	int tid;

	if (start > end)
		swap(start, end);

	if (end >= MVPP2_PRS_TCAM_SRAM_SIZE)
		end = MVPP2_PRS_TCAM_SRAM_SIZE - 1;

	for (tid = start; tid <= end; tid++) {
		if (!priv->prs_shadow[tid].valid)
			return tid;
	}

	return -EINVAL;
}

/* Enable/disable dropping all mac da's */
static void mvpp2_prs_mac_drop_all_set(struct mvpp2 *priv, int port, bool add)
{
	struct mvpp2_prs_entry pe;

	if (priv->prs_shadow[MVPP2_PE_DROP_ALL].valid) {
		/* Entry exist - update port only */
		pe.index = MVPP2_PE_DROP_ALL;
		mvpp2_prs_hw_read(priv, &pe);
	} else {
		/* Entry doesn't exist - create new */
		memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
		mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MAC);
		pe.index = MVPP2_PE_DROP_ALL;

		/* Non-promiscuous mode for all ports - DROP unknown packets */
		mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_DROP_MASK,
					 MVPP2_PRS_RI_DROP_MASK);

		mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
		mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);

		/* Update shadow table */
		mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MAC);

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(&pe, 0);
	}

	/* Update port mask */
	mvpp2_prs_tcam_port_set(&pe, port, add);

	mvpp2_prs_hw_write(priv, &pe);
}

/* Set port to promiscuous mode */
static void mvpp2_prs_mac_promisc_set(struct mvpp2 *priv, int port, bool add)
{
	struct mvpp2_prs_entry pe;

	/* Promiscuous mode - Accept unknown packets */

	if (priv->prs_shadow[MVPP2_PE_MAC_PROMISCUOUS].valid) {
		/* Entry exist - update port only */
		pe.index = MVPP2_PE_MAC_PROMISCUOUS;
		mvpp2_prs_hw_read(priv, &pe);
	} else {
		/* Entry doesn't exist - create new */
		memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
		mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MAC);
		pe.index = MVPP2_PE_MAC_PROMISCUOUS;

		/* Continue - set next lookup */
		mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_DSA);

		/* Set result info bits */
		mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L2_UCAST,
					 MVPP2_PRS_RI_L2_CAST_MASK);

		/* Shift to ethertype */
		mvpp2_prs_sram_shift_set(&pe, 2 * ETH_ALEN,
					 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(&pe, 0);

		/* Update shadow table */
		mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MAC);
	}

	/* Update port mask */
	mvpp2_prs_tcam_port_set(&pe, port, add);

	mvpp2_prs_hw_write(priv, &pe);
}

/* Accept multicast */
static void mvpp2_prs_mac_multi_set(struct mvpp2 *priv, int port, int index,
				    bool add)
{
	struct mvpp2_prs_entry pe;
	unsigned char da_mc;

	/* Ethernet multicast address first byte is
	 * 0x01 for IPv4 and 0x33 for IPv6
	 */
	da_mc = (index == MVPP2_PE_MAC_MC_ALL) ? 0x01 : 0x33;

	if (priv->prs_shadow[index].valid) {
		/* Entry exist - update port only */
		pe.index = index;
		mvpp2_prs_hw_read(priv, &pe);
	} else {
		/* Entry doesn't exist - create new */
		memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
		mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MAC);
		pe.index = index;

		/* Continue - set next lookup */
		mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_DSA);

		/* Set result info bits */
		mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L2_MCAST,
					 MVPP2_PRS_RI_L2_CAST_MASK);

		/* Update tcam entry data first byte */
		mvpp2_prs_tcam_data_byte_set(&pe, 0, da_mc, 0xff);

		/* Shift to ethertype */
		mvpp2_prs_sram_shift_set(&pe, 2 * ETH_ALEN,
					 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(&pe, 0);

		/* Update shadow table */
		mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MAC);
	}

	/* Update port mask */
	mvpp2_prs_tcam_port_set(&pe, port, add);

	mvpp2_prs_hw_write(priv, &pe);
}

/* Parser per-port initialization */
static void mvpp2_prs_hw_port_init(struct mvpp2 *priv, int port, int lu_first,
				   int lu_max, int offset)
{
	u32 val;

	/* Set lookup ID */
	val = mvpp2_read(priv, MVPP2_PRS_INIT_LOOKUP_REG);
	val &= ~MVPP2_PRS_PORT_LU_MASK(port);
	val |=  MVPP2_PRS_PORT_LU_VAL(port, lu_first);
	mvpp2_write(priv, MVPP2_PRS_INIT_LOOKUP_REG, val);

	/* Set maximum number of loops for packet received from port */
	val = mvpp2_read(priv, MVPP2_PRS_MAX_LOOP_REG(port));
	val &= ~MVPP2_PRS_MAX_LOOP_MASK(port);
	val |= MVPP2_PRS_MAX_LOOP_VAL(port, lu_max);
	mvpp2_write(priv, MVPP2_PRS_MAX_LOOP_REG(port), val);

	/* Set initial offset for packet header extraction for the first
	 * searching loop
	 */
	val = mvpp2_read(priv, MVPP2_PRS_INIT_OFFS_REG(port));
	val &= ~MVPP2_PRS_INIT_OFF_MASK(port);
	val |= MVPP2_PRS_INIT_OFF_VAL(port, offset);
	mvpp2_write(priv, MVPP2_PRS_INIT_OFFS_REG(port), val);
}

/* Default flow entries initialization for all ports */
static void mvpp2_prs_def_flow_init(struct mvpp2 *priv)
{
	struct mvpp2_prs_entry pe;
	int port;

	for (port = 0; port < MVPP2_MAX_PORTS; port++) {
		memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
		mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_FLOWS);
		pe.index = MVPP2_PE_FIRST_DEFAULT_FLOW - port;

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(&pe, 0);

		/* Set flow ID*/
		mvpp2_prs_sram_ai_update(&pe, port, MVPP2_PRS_FLOW_ID_MASK);
		mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_DONE_BIT, 1);

		/* Update shadow table and hw entry */
		mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_FLOWS);
		mvpp2_prs_hw_write(priv, &pe);
	}
}

/* Set default entry for Marvell Header field */
static void mvpp2_prs_mh_init(struct mvpp2 *priv)
{
	struct mvpp2_prs_entry pe;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));

	pe.index = MVPP2_PE_MH_DEFAULT;
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MH);
	mvpp2_prs_sram_shift_set(&pe, MVPP2_MH_SIZE,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_MAC);

	/* Unmask all ports */
	mvpp2_prs_tcam_port_map_set(&pe, MVPP2_PRS_PORT_MASK);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MH);
	mvpp2_prs_hw_write(priv, &pe);
}

/* Set default entires (place holder) for promiscuous, non-promiscuous and
 * multicast MAC addresses
 */
static void mvpp2_prs_mac_init(struct mvpp2 *priv)
{
	struct mvpp2_prs_entry pe;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));

	/* Non-promiscuous mode for all ports - DROP unknown packets */
	pe.index = MVPP2_PE_MAC_NON_PROMISCUOUS;
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MAC);

	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_DROP_MASK,
				 MVPP2_PRS_RI_DROP_MASK);
	mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);

	/* Unmask all ports */
	mvpp2_prs_tcam_port_map_set(&pe, MVPP2_PRS_PORT_MASK);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MAC);
	mvpp2_prs_hw_write(priv, &pe);

	/* place holders only - no ports */
	mvpp2_prs_mac_drop_all_set(priv, 0, false);
	mvpp2_prs_mac_promisc_set(priv, 0, false);
	mvpp2_prs_mac_multi_set(priv, MVPP2_PE_MAC_MC_ALL, 0, false);
	mvpp2_prs_mac_multi_set(priv, MVPP2_PE_MAC_MC_IP6, 0, false);
}

/* Match basic ethertypes */
static int mvpp2_prs_etype_init(struct mvpp2 *priv)
{
	struct mvpp2_prs_entry pe;
	int tid;

	/* Ethertype: PPPoE */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, PROT_PPP_SES);

	mvpp2_prs_sram_shift_set(&pe, MVPP2_PPPOE_HDR_SIZE,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_PPPOE);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_PPPOE_MASK,
				 MVPP2_PRS_RI_PPPOE_MASK);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = false;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_PPPOE_MASK,
				MVPP2_PRS_RI_PPPOE_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: ARP */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, PROT_ARP);

	/* Generate flow in the next iteration*/
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);
	mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_ARP,
				 MVPP2_PRS_RI_L3_PROTO_MASK);
	/* Set L3 offset */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = true;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_ARP,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: LBTD */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, MVPP2_IP_LBDT_TYPE);

	/* Generate flow in the next iteration*/
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);
	mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_CPU_CODE_RX_SPEC |
				 MVPP2_PRS_RI_UDF3_RX_SPECIAL,
				 MVPP2_PRS_RI_CPU_CODE_MASK |
				 MVPP2_PRS_RI_UDF3_MASK);
	/* Set L3 offset */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = true;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_CPU_CODE_RX_SPEC |
				MVPP2_PRS_RI_UDF3_RX_SPECIAL,
				MVPP2_PRS_RI_CPU_CODE_MASK |
				MVPP2_PRS_RI_UDF3_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: IPv4 without options */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, PROT_IP);
	mvpp2_prs_tcam_data_byte_set(&pe, MVPP2_ETH_TYPE_LEN,
				     MVPP2_PRS_IPV4_HEAD | MVPP2_PRS_IPV4_IHL,
				     MVPP2_PRS_IPV4_HEAD_MASK |
				     MVPP2_PRS_IPV4_IHL_MASK);

	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_IP4);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_IP4,
				 MVPP2_PRS_RI_L3_PROTO_MASK);
	/* Skip eth_type + 4 bytes of IP header */
	mvpp2_prs_sram_shift_set(&pe, MVPP2_ETH_TYPE_LEN + 4,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);
	/* Set L3 offset */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = false;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_IP4,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: IPv4 with options */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	pe.index = tid;

	/* Clear tcam data before updating */
	pe.tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE(MVPP2_ETH_TYPE_LEN)] = 0x0;
	pe.tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE_EN(MVPP2_ETH_TYPE_LEN)] = 0x0;

	mvpp2_prs_tcam_data_byte_set(&pe, MVPP2_ETH_TYPE_LEN,
				     MVPP2_PRS_IPV4_HEAD,
				     MVPP2_PRS_IPV4_HEAD_MASK);

	/* Clear ri before updating */
	pe.sram.word[MVPP2_PRS_SRAM_RI_WORD] = 0x0;
	pe.sram.word[MVPP2_PRS_SRAM_RI_CTRL_WORD] = 0x0;
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_IP4_OPT,
				 MVPP2_PRS_RI_L3_PROTO_MASK);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = false;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_IP4_OPT,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: IPv6 without options */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, PROT_IPV6);

	/* Skip DIP of IPV6 header */
	mvpp2_prs_sram_shift_set(&pe, MVPP2_ETH_TYPE_LEN + 8 +
				 MVPP2_MAX_L3_ADDR_SIZE,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_IP6);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_IP6,
				 MVPP2_PRS_RI_L3_PROTO_MASK);
	/* Set L3 offset */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = false;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_IP6,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Default entry for MVPP2_PRS_LU_L2 - Unknown ethtype */
	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = MVPP2_PE_ETH_TYPE_UN;

	/* Unmask all ports */
	mvpp2_prs_tcam_port_map_set(&pe, MVPP2_PRS_PORT_MASK);

	/* Generate flow in the next iteration*/
	mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_UN,
				 MVPP2_PRS_RI_L3_PROTO_MASK);
	/* Set L3 offset even it's unknown L3 */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = true;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_UN,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	return 0;
}

/* Parser default initialization */
static int mvpp2_prs_default_init(struct udevice *dev,
				  struct mvpp2 *priv)
{
	int err, index, i;

	/* Enable tcam table */
	mvpp2_write(priv, MVPP2_PRS_TCAM_CTRL_REG, MVPP2_PRS_TCAM_EN_MASK);

	/* Clear all tcam and sram entries */
	for (index = 0; index < MVPP2_PRS_TCAM_SRAM_SIZE; index++) {
		mvpp2_write(priv, MVPP2_PRS_TCAM_IDX_REG, index);
		for (i = 0; i < MVPP2_PRS_TCAM_WORDS; i++)
			mvpp2_write(priv, MVPP2_PRS_TCAM_DATA_REG(i), 0);

		mvpp2_write(priv, MVPP2_PRS_SRAM_IDX_REG, index);
		for (i = 0; i < MVPP2_PRS_SRAM_WORDS; i++)
			mvpp2_write(priv, MVPP2_PRS_SRAM_DATA_REG(i), 0);
	}

	/* Invalidate all tcam entries */
	for (index = 0; index < MVPP2_PRS_TCAM_SRAM_SIZE; index++)
		mvpp2_prs_hw_inv(priv, index);

	priv->prs_shadow = devm_kcalloc(dev, MVPP2_PRS_TCAM_SRAM_SIZE,
					sizeof(struct mvpp2_prs_shadow),
					GFP_KERNEL);
	if (!priv->prs_shadow)
		return -ENOMEM;

	/* Always start from lookup = 0 */
	for (index = 0; index < MVPP2_MAX_PORTS; index++)
		mvpp2_prs_hw_port_init(priv, index, MVPP2_PRS_LU_MH,
				       MVPP2_PRS_PORT_LU_MAX, 0);

	mvpp2_prs_def_flow_init(priv);

	mvpp2_prs_mh_init(priv);

	mvpp2_prs_mac_init(priv);

	err = mvpp2_prs_etype_init(priv);
	if (err)
		return err;

	return 0;
}

/* Compare MAC DA with tcam entry data */
static bool mvpp2_prs_mac_range_equals(struct mvpp2_prs_entry *pe,
				       const u8 *da, unsigned char *mask)
{
	unsigned char tcam_byte, tcam_mask;
	int index;

	for (index = 0; index < ETH_ALEN; index++) {
		mvpp2_prs_tcam_data_byte_get(pe, index, &tcam_byte, &tcam_mask);
		if (tcam_mask != mask[index])
			return false;

		if ((tcam_mask & tcam_byte) != (da[index] & mask[index]))
			return false;
	}

	return true;
}

/* Find tcam entry with matched pair <MAC DA, port> */
static struct mvpp2_prs_entry *
mvpp2_prs_mac_da_range_find(struct mvpp2 *priv, int pmap, const u8 *da,
			    unsigned char *mask, int udf_type)
{
	struct mvpp2_prs_entry *pe;
	int tid;

	pe = kzalloc(sizeof(*pe), GFP_KERNEL);
	if (!pe)
		return NULL;
	mvpp2_prs_tcam_lu_set(pe, MVPP2_PRS_LU_MAC);

	/* Go through the all entires with MVPP2_PRS_LU_MAC */
	for (tid = MVPP2_PE_FIRST_FREE_TID;
	     tid <= MVPP2_PE_LAST_FREE_TID; tid++) {
		unsigned int entry_pmap;

		if (!priv->prs_shadow[tid].valid ||
		    (priv->prs_shadow[tid].lu != MVPP2_PRS_LU_MAC) ||
		    (priv->prs_shadow[tid].udf != udf_type))
			continue;

		pe->index = tid;
		mvpp2_prs_hw_read(priv, pe);
		entry_pmap = mvpp2_prs_tcam_port_map_get(pe);

		if (mvpp2_prs_mac_range_equals(pe, da, mask) &&
		    entry_pmap == pmap)
			return pe;
	}
	kfree(pe);

	return NULL;
}

/* Update parser's mac da entry */
static int mvpp2_prs_mac_da_accept(struct mvpp2 *priv, int port,
				   const u8 *da, bool add)
{
	struct mvpp2_prs_entry *pe;
	unsigned int pmap, len, ri;
	unsigned char mask[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	int tid;

	/* Scan TCAM and see if entry with this <MAC DA, port> already exist */
	pe = mvpp2_prs_mac_da_range_find(priv, (1 << port), da, mask,
					 MVPP2_PRS_UDF_MAC_DEF);

	/* No such entry */
	if (!pe) {
		if (!add)
			return 0;

		/* Create new TCAM entry */
		/* Find first range mac entry*/
		for (tid = MVPP2_PE_FIRST_FREE_TID;
		     tid <= MVPP2_PE_LAST_FREE_TID; tid++)
			if (priv->prs_shadow[tid].valid &&
			    (priv->prs_shadow[tid].lu == MVPP2_PRS_LU_MAC) &&
			    (priv->prs_shadow[tid].udf ==
						       MVPP2_PRS_UDF_MAC_RANGE))
				break;

		/* Go through the all entries from first to last */
		tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
						tid - 1);
		if (tid < 0)
			return tid;

		pe = kzalloc(sizeof(*pe), GFP_KERNEL);
		if (!pe)
			return -1;
		mvpp2_prs_tcam_lu_set(pe, MVPP2_PRS_LU_MAC);
		pe->index = tid;

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(pe, 0);
	}

	/* Update port mask */
	mvpp2_prs_tcam_port_set(pe, port, add);

	/* Invalidate the entry if no ports are left enabled */
	pmap = mvpp2_prs_tcam_port_map_get(pe);
	if (pmap == 0) {
		if (add) {
			kfree(pe);
			return -1;
		}
		mvpp2_prs_hw_inv(priv, pe->index);
		priv->prs_shadow[pe->index].valid = false;
		kfree(pe);
		return 0;
	}

	/* Continue - set next lookup */
	mvpp2_prs_sram_next_lu_set(pe, MVPP2_PRS_LU_DSA);

	/* Set match on DA */
	len = ETH_ALEN;
	while (len--)
		mvpp2_prs_tcam_data_byte_set(pe, len, da[len], 0xff);

	/* Set result info bits */
	ri = MVPP2_PRS_RI_L2_UCAST | MVPP2_PRS_RI_MAC_ME_MASK;

	mvpp2_prs_sram_ri_update(pe, ri, MVPP2_PRS_RI_L2_CAST_MASK |
				 MVPP2_PRS_RI_MAC_ME_MASK);
	mvpp2_prs_shadow_ri_set(priv, pe->index, ri, MVPP2_PRS_RI_L2_CAST_MASK |
				MVPP2_PRS_RI_MAC_ME_MASK);

	/* Shift to ethertype */
	mvpp2_prs_sram_shift_set(pe, 2 * ETH_ALEN,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);

	/* Update shadow table and hw entry */
	priv->prs_shadow[pe->index].udf = MVPP2_PRS_UDF_MAC_DEF;
	mvpp2_prs_shadow_set(priv, pe->index, MVPP2_PRS_LU_MAC);
	mvpp2_prs_hw_write(priv, pe);

	kfree(pe);

	return 0;
}

static int mvpp2_prs_update_mac_da(struct mvpp2_port *port, const u8 *da)
{
	int err;

	/* Remove old parser entry */
	err = mvpp2_prs_mac_da_accept(port->priv, port->id, port->dev_addr,
				      false);
	if (err)
		return err;

	/* Add new parser entry */
	err = mvpp2_prs_mac_da_accept(port->priv, port->id, da, true);
	if (err)
		return err;

	/* Set addr in the device */
	memcpy(port->dev_addr, da, ETH_ALEN);

	return 0;
}

/* Set prs flow for the port */
static int mvpp2_prs_def_flow(struct mvpp2_port *port)
{
	struct mvpp2_prs_entry *pe;
	int tid;

	pe = mvpp2_prs_flow_find(port->priv, port->id);

	/* Such entry not exist */
	if (!pe) {
		/* Go through the all entires from last to first */
		tid = mvpp2_prs_tcam_first_free(port->priv,
						MVPP2_PE_LAST_FREE_TID,
					       MVPP2_PE_FIRST_FREE_TID);
		if (tid < 0)
			return tid;

		pe = kzalloc(sizeof(*pe), GFP_KERNEL);
		if (!pe)
			return -ENOMEM;

		mvpp2_prs_tcam_lu_set(pe, MVPP2_PRS_LU_FLOWS);
		pe->index = tid;

		/* Set flow ID*/
		mvpp2_prs_sram_ai_update(pe, port->id, MVPP2_PRS_FLOW_ID_MASK);
		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_LU_DONE_BIT, 1);

		/* Update shadow table */
		mvpp2_prs_shadow_set(port->priv, pe->index, MVPP2_PRS_LU_FLOWS);
	}

	mvpp2_prs_tcam_port_map_set(pe, (1 << port->id));
	mvpp2_prs_hw_write(port->priv, pe);
	kfree(pe);

	return 0;
}

/* Classifier configuration routines */

/* Update classification flow table registers */
static void mvpp2_cls_flow_write(struct mvpp2 *priv,
				 struct mvpp2_cls_flow_entry *fe)
{
	mvpp2_write(priv, MVPP2_CLS_FLOW_INDEX_REG, fe->index);
	mvpp2_write(priv, MVPP2_CLS_FLOW_TBL0_REG,  fe->data[0]);
	mvpp2_write(priv, MVPP2_CLS_FLOW_TBL1_REG,  fe->data[1]);
	mvpp2_write(priv, MVPP2_CLS_FLOW_TBL2_REG,  fe->data[2]);
}

/* Update classification lookup table register */
static void mvpp2_cls_lookup_write(struct mvpp2 *priv,
				   struct mvpp2_cls_lookup_entry *le)
{
	u32 val;

	val = (le->way << MVPP2_CLS_LKP_INDEX_WAY_OFFS) | le->lkpid;
	mvpp2_write(priv, MVPP2_CLS_LKP_INDEX_REG, val);
	mvpp2_write(priv, MVPP2_CLS_LKP_TBL_REG, le->data);
}

/* Classifier default initialization */
static void mvpp2_cls_init(struct mvpp2 *priv)
{
	struct mvpp2_cls_lookup_entry le;
	struct mvpp2_cls_flow_entry fe;
	int index;

	/* Enable classifier */
	mvpp2_write(priv, MVPP2_CLS_MODE_REG, MVPP2_CLS_MODE_ACTIVE_MASK);

	/* Clear classifier flow table */
	memset(&fe.data, 0, MVPP2_CLS_FLOWS_TBL_DATA_WORDS);
	for (index = 0; index < MVPP2_CLS_FLOWS_TBL_SIZE; index++) {
		fe.index = index;
		mvpp2_cls_flow_write(priv, &fe);
	}

	/* Clear classifier lookup table */
	le.data = 0;
	for (index = 0; index < MVPP2_CLS_LKP_TBL_SIZE; index++) {
		le.lkpid = index;
		le.way = 0;
		mvpp2_cls_lookup_write(priv, &le);

		le.way = 1;
		mvpp2_cls_lookup_write(priv, &le);
	}
}

static void mvpp2_cls_port_config(struct mvpp2_port *port)
{
	struct mvpp2_cls_lookup_entry le;
	u32 val;

	/* Set way for the port */
	val = mvpp2_read(port->priv, MVPP2_CLS_PORT_WAY_REG);
	val &= ~MVPP2_CLS_PORT_WAY_MASK(port->id);
	mvpp2_write(port->priv, MVPP2_CLS_PORT_WAY_REG, val);

	/* Pick the entry to be accessed in lookup ID decoding table
	 * according to the way and lkpid.
	 */
	le.lkpid = port->id;
	le.way = 0;
	le.data = 0;

	/* Set initial CPU queue for receiving packets */
	le.data &= ~MVPP2_CLS_LKP_TBL_RXQ_MASK;
	le.data |= port->first_rxq;

	/* Disable classification engines */
	le.data &= ~MVPP2_CLS_LKP_TBL_LOOKUP_EN_MASK;

	/* Update lookup ID table entry */
	mvpp2_cls_lookup_write(port->priv, &le);
}

/* Set CPU queue number for oversize packets */
static void mvpp2_cls_oversize_rxq_set(struct mvpp2_port *port)
{
	u32 val;

	mvpp2_write(port->priv, MVPP2_CLS_OVERSIZE_RXQ_LOW_REG(port->id),
		    port->first_rxq & MVPP2_CLS_OVERSIZE_RXQ_LOW_MASK);

	mvpp2_write(port->priv, MVPP2_CLS_SWFWD_P2HQ_REG(port->id),
		    (port->first_rxq >> MVPP2_CLS_OVERSIZE_RXQ_LOW_BITS));

	val = mvpp2_read(port->priv, MVPP2_CLS_SWFWD_PCTRL_REG);
	val |= MVPP2_CLS_SWFWD_PCTRL_MASK(port->id);
	mvpp2_write(port->priv, MVPP2_CLS_SWFWD_PCTRL_REG, val);
}

/* Buffer Manager configuration routines */

/* Create pool */
static int mvpp2_bm_pool_create(struct udevice *dev,
				struct mvpp2 *priv,
				struct mvpp2_bm_pool *bm_pool, int size)
{
	u32 val;

	bm_pool->virt_addr = buffer_loc.bm_pool[bm_pool->id];
	bm_pool->phys_addr = (dma_addr_t)buffer_loc.bm_pool[bm_pool->id];
	if (!bm_pool->virt_addr)
		return -ENOMEM;

	if (!IS_ALIGNED((u32)bm_pool->virt_addr, MVPP2_BM_POOL_PTR_ALIGN)) {
		dev_err(&pdev->dev, "BM pool %d is not %d bytes aligned\n",
			bm_pool->id, MVPP2_BM_POOL_PTR_ALIGN);
		return -ENOMEM;
	}

	mvpp2_write(priv, MVPP2_BM_POOL_BASE_REG(bm_pool->id),
		    bm_pool->phys_addr);
	mvpp2_write(priv, MVPP2_BM_POOL_SIZE_REG(bm_pool->id), size);

	val = mvpp2_read(priv, MVPP2_BM_POOL_CTRL_REG(bm_pool->id));
	val |= MVPP2_BM_START_MASK;
	mvpp2_write(priv, MVPP2_BM_POOL_CTRL_REG(bm_pool->id), val);

	bm_pool->type = MVPP2_BM_FREE;
	bm_pool->size = size;
	bm_pool->pkt_size = 0;
	bm_pool->buf_num = 0;

	return 0;
}

/* Set pool buffer size */
static void mvpp2_bm_pool_bufsize_set(struct mvpp2 *priv,
				      struct mvpp2_bm_pool *bm_pool,
				      int buf_size)
{
	u32 val;

	bm_pool->buf_size = buf_size;

	val = ALIGN(buf_size, 1 << MVPP2_POOL_BUF_SIZE_OFFSET);
	mvpp2_write(priv, MVPP2_POOL_BUF_SIZE_REG(bm_pool->id), val);
}

/* Free all buffers from the pool */
static void mvpp2_bm_bufs_free(struct udevice *dev, struct mvpp2 *priv,
			       struct mvpp2_bm_pool *bm_pool)
{
	bm_pool->buf_num = 0;
}

/* Cleanup pool */
static int mvpp2_bm_pool_destroy(struct udevice *dev,
				 struct mvpp2 *priv,
				 struct mvpp2_bm_pool *bm_pool)
{
	u32 val;

	mvpp2_bm_bufs_free(dev, priv, bm_pool);
	if (bm_pool->buf_num) {
		dev_err(dev, "cannot free all buffers in pool %d\n", bm_pool->id);
		return 0;
	}

	val = mvpp2_read(priv, MVPP2_BM_POOL_CTRL_REG(bm_pool->id));
	val |= MVPP2_BM_STOP_MASK;
	mvpp2_write(priv, MVPP2_BM_POOL_CTRL_REG(bm_pool->id), val);

	return 0;
}

static int mvpp2_bm_pools_init(struct udevice *dev,
			       struct mvpp2 *priv)
{
	int i, err, size;
	struct mvpp2_bm_pool *bm_pool;

	/* Create all pools with maximum size */
	size = MVPP2_BM_POOL_SIZE_MAX;
	for (i = 0; i < MVPP2_BM_POOLS_NUM; i++) {
		bm_pool = &priv->bm_pools[i];
		bm_pool->id = i;
		err = mvpp2_bm_pool_create(dev, priv, bm_pool, size);
		if (err)
			goto err_unroll_pools;
		mvpp2_bm_pool_bufsize_set(priv, bm_pool, 0);
	}
	return 0;

err_unroll_pools:
	dev_err(&pdev->dev, "failed to create BM pool %d, size %d\n", i, size);
	for (i = i - 1; i >= 0; i--)
		mvpp2_bm_pool_destroy(dev, priv, &priv->bm_pools[i]);
	return err;
}

static int mvpp2_bm_init(struct udevice *dev, struct mvpp2 *priv)
{
	int i, err;

	for (i = 0; i < MVPP2_BM_POOLS_NUM; i++) {
		/* Mask BM all interrupts */
		mvpp2_write(priv, MVPP2_BM_INTR_MASK_REG(i), 0);
		/* Clear BM cause register */
		mvpp2_write(priv, MVPP2_BM_INTR_CAUSE_REG(i), 0);
	}

	/* Allocate and initialize BM pools */
	priv->bm_pools = devm_kcalloc(dev, MVPP2_BM_POOLS_NUM,
				     sizeof(struct mvpp2_bm_pool), GFP_KERNEL);
	if (!priv->bm_pools)
		return -ENOMEM;

	err = mvpp2_bm_pools_init(dev, priv);
	if (err < 0)
		return err;
	return 0;
}

/* Attach long pool to rxq */
static void mvpp2_rxq_long_pool_set(struct mvpp2_port *port,
				    int lrxq, int long_pool)
{
	u32 val;
	int prxq;

	/* Get queue physical ID */
	prxq = port->rxqs[lrxq]->id;

	val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(prxq));
	val &= ~MVPP2_RXQ_POOL_LONG_MASK;
	val |= ((long_pool << MVPP2_RXQ_POOL_LONG_OFFS) &
		    MVPP2_RXQ_POOL_LONG_MASK);

	mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(prxq), val);
}

/* Set pool number in a BM cookie */
static inline u32 mvpp2_bm_cookie_pool_set(u32 cookie, int pool)
{
	u32 bm;

	bm = cookie & ~(0xFF << MVPP2_BM_COOKIE_POOL_OFFS);
	bm |= ((pool & 0xFF) << MVPP2_BM_COOKIE_POOL_OFFS);

	return bm;
}

/* Get pool number from a BM cookie */
static inline int mvpp2_bm_cookie_pool_get(u32 cookie)
{
	return (cookie >> MVPP2_BM_COOKIE_POOL_OFFS) & 0xFF;
}

/* Release buffer to BM */
static inline void mvpp2_bm_pool_put(struct mvpp2_port *port, int pool,
				     u32 buf_phys_addr, u32 buf_virt_addr)
{
	mvpp2_write(port->priv, MVPP2_BM_VIRT_RLS_REG, buf_virt_addr);
	mvpp2_write(port->priv, MVPP2_BM_PHY_RLS_REG(pool), buf_phys_addr);
}

/* Refill BM pool */
static void mvpp2_pool_refill(struct mvpp2_port *port, u32 bm,
			      u32 phys_addr, u32 cookie)
{
	int pool = mvpp2_bm_cookie_pool_get(bm);

	mvpp2_bm_pool_put(port, pool, phys_addr, cookie);
}

/* Allocate buffers for the pool */
static int mvpp2_bm_bufs_add(struct mvpp2_port *port,
			     struct mvpp2_bm_pool *bm_pool, int buf_num)
{
	int i;

	if (buf_num < 0 ||
	    (buf_num + bm_pool->buf_num > bm_pool->size)) {
		netdev_err(port->dev,
			   "cannot allocate %d buffers for pool %d\n",
			   buf_num, bm_pool->id);
		return 0;
	}

	for (i = 0; i < buf_num; i++) {
		mvpp2_bm_pool_put(port, bm_pool->id,
				  (u32)buffer_loc.rx_buffer[i],
				  (u32)buffer_loc.rx_buffer[i]);

	}

	/* Update BM driver with number of buffers added to pool */
	bm_pool->buf_num += i;
	bm_pool->in_use_thresh = bm_pool->buf_num / 4;

	return i;
}

/* Notify the driver that BM pool is being used as specific type and return the
 * pool pointer on success
 */
static struct mvpp2_bm_pool *
mvpp2_bm_pool_use(struct mvpp2_port *port, int pool, enum mvpp2_bm_type type,
		  int pkt_size)
{
	struct mvpp2_bm_pool *new_pool = &port->priv->bm_pools[pool];
	int num;

	if (new_pool->type != MVPP2_BM_FREE && new_pool->type != type) {
		netdev_err(port->dev, "mixing pool types is forbidden\n");
		return NULL;
	}

	if (new_pool->type == MVPP2_BM_FREE)
		new_pool->type = type;

	/* Allocate buffers in case BM pool is used as long pool, but packet
	 * size doesn't match MTU or BM pool hasn't being used yet
	 */
	if (((type == MVPP2_BM_SWF_LONG) && (pkt_size > new_pool->pkt_size)) ||
	    (new_pool->pkt_size == 0)) {
		int pkts_num;

		/* Set default buffer number or free all the buffers in case
		 * the pool is not empty
		 */
		pkts_num = new_pool->buf_num;
		if (pkts_num == 0)
			pkts_num = type == MVPP2_BM_SWF_LONG ?
				   MVPP2_BM_LONG_BUF_NUM :
				   MVPP2_BM_SHORT_BUF_NUM;
		else
			mvpp2_bm_bufs_free(NULL,
					   port->priv, new_pool);

		new_pool->pkt_size = pkt_size;

		/* Allocate buffers for this pool */
		num = mvpp2_bm_bufs_add(port, new_pool, pkts_num);
		if (num != pkts_num) {
			dev_err(dev, "pool %d: %d of %d allocated\n",
				new_pool->id, num, pkts_num);
			return NULL;
		}
	}

	mvpp2_bm_pool_bufsize_set(port->priv, new_pool,
				  MVPP2_RX_BUF_SIZE(new_pool->pkt_size));

	return new_pool;
}

/* Initialize pools for swf */
static int mvpp2_swf_bm_pool_init(struct mvpp2_port *port)
{
	int rxq;

	if (!port->pool_long) {
		port->pool_long =
		       mvpp2_bm_pool_use(port, MVPP2_BM_SWF_LONG_POOL(port->id),
					 MVPP2_BM_SWF_LONG,
					 port->pkt_size);
		if (!port->pool_long)
			return -ENOMEM;

		port->pool_long->port_map |= (1 << port->id);

		for (rxq = 0; rxq < rxq_number; rxq++)
			mvpp2_rxq_long_pool_set(port, rxq, port->pool_long->id);
	}

	return 0;
}

/* Port configuration routines */

static void mvpp2_port_mii_set(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_2_REG);

	switch (port->phy_interface) {
	case PHY_INTERFACE_MODE_SGMII:
		val |= MVPP2_GMAC_INBAND_AN_MASK;
		break;
	case PHY_INTERFACE_MODE_RGMII:
		val |= MVPP2_GMAC_PORT_RGMII_MASK;
	default:
		val &= ~MVPP2_GMAC_PCS_ENABLE_MASK;
	}

	writel(val, port->base + MVPP2_GMAC_CTRL_2_REG);
}

static void mvpp2_port_fc_adv_enable(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_AUTONEG_CONFIG);
	val |= MVPP2_GMAC_FC_ADV_EN;
	writel(val, port->base + MVPP2_GMAC_AUTONEG_CONFIG);
}

static void mvpp2_port_enable(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	val |= MVPP2_GMAC_PORT_EN_MASK;
	val |= MVPP2_GMAC_MIB_CNTR_EN_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);
}

static void mvpp2_port_disable(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	val &= ~(MVPP2_GMAC_PORT_EN_MASK);
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);
}

/* Set IEEE 802.3x Flow Control Xon Packet Transmission Mode */
static void mvpp2_port_periodic_xon_disable(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_1_REG) &
		    ~MVPP2_GMAC_PERIODIC_XON_EN_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_1_REG);
}

/* Configure loopback port */
static void mvpp2_port_loopback_set(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_1_REG);

	if (port->speed == 1000)
		val |= MVPP2_GMAC_GMII_LB_EN_MASK;
	else
		val &= ~MVPP2_GMAC_GMII_LB_EN_MASK;

	if (port->phy_interface == PHY_INTERFACE_MODE_SGMII)
		val |= MVPP2_GMAC_PCS_LB_EN_MASK;
	else
		val &= ~MVPP2_GMAC_PCS_LB_EN_MASK;

	writel(val, port->base + MVPP2_GMAC_CTRL_1_REG);
}

static void mvpp2_port_reset(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_2_REG) &
		    ~MVPP2_GMAC_PORT_RESET_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_2_REG);

	while (readl(port->base + MVPP2_GMAC_CTRL_2_REG) &
	       MVPP2_GMAC_PORT_RESET_MASK)
		continue;
}

/* Change maximum receive size of the port */
static inline void mvpp2_gmac_max_rx_size_set(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	val &= ~MVPP2_GMAC_MAX_RX_SIZE_MASK;
	val |= (((port->pkt_size - MVPP2_MH_SIZE) / 2) <<
		    MVPP2_GMAC_MAX_RX_SIZE_OFFS);
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);
}

/* Set defaults to the MVPP2 port */
static void mvpp2_defaults_set(struct mvpp2_port *port)
{
	int tx_port_num, val, queue, ptxq, lrxq;

	/* Configure port to loopback if needed */
	if (port->flags & MVPP2_F_LOOPBACK)
		mvpp2_port_loopback_set(port);

	/* Update TX FIFO MIN Threshold */
	val = readl(port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);
	val &= ~MVPP2_GMAC_TX_FIFO_MIN_TH_ALL_MASK;
	/* Min. TX threshold must be less than minimal packet length */
	val |= MVPP2_GMAC_TX_FIFO_MIN_TH_MASK(64 - 4 - 2);
	writel(val, port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);

	/* Disable Legacy WRR, Disable EJP, Release from reset */
	tx_port_num = mvpp2_egress_port(port);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG,
		    tx_port_num);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_CMD_1_REG, 0);

	/* Close bandwidth for all queues */
	for (queue = 0; queue < MVPP2_MAX_TXQ; queue++) {
		ptxq = mvpp2_txq_phys(port->id, queue);
		mvpp2_write(port->priv,
			    MVPP2_TXQ_SCHED_TOKEN_CNTR_REG(ptxq), 0);
	}

	/* Set refill period to 1 usec, refill tokens
	 * and bucket size to maximum
	 */
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PERIOD_REG, 0xc8);
	val = mvpp2_read(port->priv, MVPP2_TXP_SCHED_REFILL_REG);
	val &= ~MVPP2_TXP_REFILL_PERIOD_ALL_MASK;
	val |= MVPP2_TXP_REFILL_PERIOD_MASK(1);
	val |= MVPP2_TXP_REFILL_TOKENS_ALL_MASK;
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_REFILL_REG, val);
	val = MVPP2_TXP_TOKEN_SIZE_MAX;
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_TOKEN_SIZE_REG, val);

	/* Set MaximumLowLatencyPacketSize value to 256 */
	mvpp2_write(port->priv, MVPP2_RX_CTRL_REG(port->id),
		    MVPP2_RX_USE_PSEUDO_FOR_CSUM_MASK |
		    MVPP2_RX_LOW_LATENCY_PKT_SIZE(256));

	/* Enable Rx cache snoop */
	for (lrxq = 0; lrxq < rxq_number; lrxq++) {
		queue = port->rxqs[lrxq]->id;
		val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(queue));
		val |= MVPP2_SNOOP_PKT_SIZE_MASK |
			   MVPP2_SNOOP_BUF_HDR_MASK;
		mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(queue), val);
	}
}

/* Enable/disable receiving packets */
static void mvpp2_ingress_enable(struct mvpp2_port *port)
{
	u32 val;
	int lrxq, queue;

	for (lrxq = 0; lrxq < rxq_number; lrxq++) {
		queue = port->rxqs[lrxq]->id;
		val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(queue));
		val &= ~MVPP2_RXQ_DISABLE_MASK;
		mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(queue), val);
	}
}

static void mvpp2_ingress_disable(struct mvpp2_port *port)
{
	u32 val;
	int lrxq, queue;

	for (lrxq = 0; lrxq < rxq_number; lrxq++) {
		queue = port->rxqs[lrxq]->id;
		val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(queue));
		val |= MVPP2_RXQ_DISABLE_MASK;
		mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(queue), val);
	}
}

/* Enable transmit via physical egress queue
 * - HW starts take descriptors from DRAM
 */
static void mvpp2_egress_enable(struct mvpp2_port *port)
{
	u32 qmap;
	int queue;
	int tx_port_num = mvpp2_egress_port(port);

	/* Enable all initialized TXs. */
	qmap = 0;
	for (queue = 0; queue < txq_number; queue++) {
		struct mvpp2_tx_queue *txq = port->txqs[queue];

		if (txq->descs != NULL)
			qmap |= (1 << queue);
	}

	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG, tx_port_num);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_Q_CMD_REG, qmap);
}

/* Disable transmit via physical egress queue
 * - HW doesn't take descriptors from DRAM
 */
static void mvpp2_egress_disable(struct mvpp2_port *port)
{
	u32 reg_data;
	int delay;
	int tx_port_num = mvpp2_egress_port(port);

	/* Issue stop command for active channels only */
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG, tx_port_num);
	reg_data = (mvpp2_read(port->priv, MVPP2_TXP_SCHED_Q_CMD_REG)) &
		    MVPP2_TXP_SCHED_ENQ_MASK;
	if (reg_data != 0)
		mvpp2_write(port->priv, MVPP2_TXP_SCHED_Q_CMD_REG,
			    (reg_data << MVPP2_TXP_SCHED_DISQ_OFFSET));

	/* Wait for all Tx activity to terminate. */
	delay = 0;
	do {
		if (delay >= MVPP2_TX_DISABLE_TIMEOUT_MSEC) {
			netdev_warn(port->dev,
				    "Tx stop timed out, status=0x%08x\n",
				    reg_data);
			break;
		}
		mdelay(1);
		delay++;

		/* Check port TX Command register that all
		 * Tx queues are stopped
		 */
		reg_data = mvpp2_read(port->priv, MVPP2_TXP_SCHED_Q_CMD_REG);
	} while (reg_data & MVPP2_TXP_SCHED_ENQ_MASK);
}

/* Rx descriptors helper methods */

/* Get number of Rx descriptors occupied by received packets */
static inline int
mvpp2_rxq_received(struct mvpp2_port *port, int rxq_id)
{
	u32 val = mvpp2_read(port->priv, MVPP2_RXQ_STATUS_REG(rxq_id));

	return val & MVPP2_RXQ_OCCUPIED_MASK;
}

/* Update Rx queue status with the number of occupied and available
 * Rx descriptor slots.
 */
static inline void
mvpp2_rxq_status_update(struct mvpp2_port *port, int rxq_id,
			int used_count, int free_count)
{
	/* Decrement the number of used descriptors and increment count
	 * increment the number of free descriptors.
	 */
	u32 val = used_count | (free_count << MVPP2_RXQ_NUM_NEW_OFFSET);

	mvpp2_write(port->priv, MVPP2_RXQ_STATUS_UPDATE_REG(rxq_id), val);
}

/* Get pointer to next RX descriptor to be processed by SW */
static inline struct mvpp2_rx_desc *
mvpp2_rxq_next_desc_get(struct mvpp2_rx_queue *rxq)
{
	int rx_desc = rxq->next_desc_to_proc;

	rxq->next_desc_to_proc = MVPP2_QUEUE_NEXT_DESC(rxq, rx_desc);
	prefetch(rxq->descs + rxq->next_desc_to_proc);
	return rxq->descs + rx_desc;
}

/* Set rx queue offset */
static void mvpp2_rxq_offset_set(struct mvpp2_port *port,
				 int prxq, int offset)
{
	u32 val;

	/* Convert offset from bytes to units of 32 bytes */
	offset = offset >> 5;

	val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(prxq));
	val &= ~MVPP2_RXQ_PACKET_OFFSET_MASK;

	/* Offset is in */
	val |= ((offset << MVPP2_RXQ_PACKET_OFFSET_OFFS) &
		    MVPP2_RXQ_PACKET_OFFSET_MASK);

	mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(prxq), val);
}

/* Obtain BM cookie information from descriptor */
static u32 mvpp2_bm_cookie_build(struct mvpp2_rx_desc *rx_desc)
{
	int pool = (rx_desc->status & MVPP2_RXD_BM_POOL_ID_MASK) >>
		   MVPP2_RXD_BM_POOL_ID_OFFS;
	int cpu = smp_processor_id();

	return ((pool & 0xFF) << MVPP2_BM_COOKIE_POOL_OFFS) |
	       ((cpu & 0xFF) << MVPP2_BM_COOKIE_CPU_OFFS);
}

/* Tx descriptors helper methods */

/* Get number of Tx descriptors waiting to be transmitted by HW */
static int mvpp2_txq_pend_desc_num_get(struct mvpp2_port *port,
				       struct mvpp2_tx_queue *txq)
{
	u32 val;

	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);
	val = mvpp2_read(port->priv, MVPP2_TXQ_PENDING_REG);

	return val & MVPP2_TXQ_PENDING_MASK;
}

/* Get pointer to next Tx descriptor to be processed (send) by HW */
static struct mvpp2_tx_desc *
mvpp2_txq_next_desc_get(struct mvpp2_tx_queue *txq)
{
	int tx_desc = txq->next_desc_to_proc;

	txq->next_desc_to_proc = MVPP2_QUEUE_NEXT_DESC(txq, tx_desc);
	return txq->descs + tx_desc;
}

/* Update HW with number of aggregated Tx descriptors to be sent */
static void mvpp2_aggr_txq_pend_desc_add(struct mvpp2_port *port, int pending)
{
	/* aggregated access - relevant TXQ number is written in TX desc */
	mvpp2_write(port->priv, MVPP2_AGGR_TXQ_UPDATE_REG, pending);
}

/* Get number of sent descriptors and decrement counter.
 * The number of sent descriptors is returned.
 * Per-CPU access
 */
static inline int mvpp2_txq_sent_desc_proc(struct mvpp2_port *port,
					   struct mvpp2_tx_queue *txq)
{
	u32 val;

	/* Reading status reg resets transmitted descriptor counter */
	val = mvpp2_read(port->priv, MVPP2_TXQ_SENT_REG(txq->id));

	return (val & MVPP2_TRANSMITTED_COUNT_MASK) >>
		MVPP2_TRANSMITTED_COUNT_OFFSET;
}

static void mvpp2_txq_sent_counter_clear(void *arg)
{
	struct mvpp2_port *port = arg;
	int queue;

	for (queue = 0; queue < txq_number; queue++) {
		int id = port->txqs[queue]->id;

		mvpp2_read(port->priv, MVPP2_TXQ_SENT_REG(id));
	}
}

/* Set max sizes for Tx queues */
static void mvpp2_txp_max_tx_size_set(struct mvpp2_port *port)
{
	u32	val, size, mtu;
	int	txq, tx_port_num;

	mtu = port->pkt_size * 8;
	if (mtu > MVPP2_TXP_MTU_MAX)
		mtu = MVPP2_TXP_MTU_MAX;

	/* WA for wrong Token bucket update: Set MTU value = 3*real MTU value */
	mtu = 3 * mtu;

	/* Indirect access to registers */
	tx_port_num = mvpp2_egress_port(port);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG, tx_port_num);

	/* Set MTU */
	val = mvpp2_read(port->priv, MVPP2_TXP_SCHED_MTU_REG);
	val &= ~MVPP2_TXP_MTU_MAX;
	val |= mtu;
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_MTU_REG, val);

	/* TXP token size and all TXQs token size must be larger that MTU */
	val = mvpp2_read(port->priv, MVPP2_TXP_SCHED_TOKEN_SIZE_REG);
	size = val & MVPP2_TXP_TOKEN_SIZE_MAX;
	if (size < mtu) {
		size = mtu;
		val &= ~MVPP2_TXP_TOKEN_SIZE_MAX;
		val |= size;
		mvpp2_write(port->priv, MVPP2_TXP_SCHED_TOKEN_SIZE_REG, val);
	}

	for (txq = 0; txq < txq_number; txq++) {
		val = mvpp2_read(port->priv,
				 MVPP2_TXQ_SCHED_TOKEN_SIZE_REG(txq));
		size = val & MVPP2_TXQ_TOKEN_SIZE_MAX;

		if (size < mtu) {
			size = mtu;
			val &= ~MVPP2_TXQ_TOKEN_SIZE_MAX;
			val |= size;
			mvpp2_write(port->priv,
				    MVPP2_TXQ_SCHED_TOKEN_SIZE_REG(txq),
				    val);
		}
	}
}

/* Free Tx queue skbuffs */
static void mvpp2_txq_bufs_free(struct mvpp2_port *port,
				struct mvpp2_tx_queue *txq,
				struct mvpp2_txq_pcpu *txq_pcpu, int num)
{
	int i;

	for (i = 0; i < num; i++)
		mvpp2_txq_inc_get(txq_pcpu);
}

static inline struct mvpp2_rx_queue *mvpp2_get_rx_queue(struct mvpp2_port *port,
							u32 cause)
{
	int queue = fls(cause) - 1;

	return port->rxqs[queue];
}

static inline struct mvpp2_tx_queue *mvpp2_get_tx_queue(struct mvpp2_port *port,
							u32 cause)
{
	int queue = fls(cause) - 1;

	return port->txqs[queue];
}

/* Rx/Tx queue initialization/cleanup methods */

/* Allocate and initialize descriptors for aggr TXQ */
static int mvpp2_aggr_txq_init(struct udevice *dev,
			       struct mvpp2_tx_queue *aggr_txq,
			       int desc_num, int cpu,
			       struct mvpp2 *priv)
{
	/* Allocate memory for TX descriptors */
	aggr_txq->descs = buffer_loc.aggr_tx_descs;
	aggr_txq->descs_phys = (dma_addr_t)buffer_loc.aggr_tx_descs;
	if (!aggr_txq->descs)
		return -ENOMEM;

	/* Make sure descriptor address is cache line size aligned  */
	BUG_ON(aggr_txq->descs !=
	       PTR_ALIGN(aggr_txq->descs, MVPP2_CPU_D_CACHE_LINE_SIZE));

	aggr_txq->last_desc = aggr_txq->size - 1;

	/* Aggr TXQ no reset WA */
	aggr_txq->next_desc_to_proc = mvpp2_read(priv,
						 MVPP2_AGGR_TXQ_INDEX_REG(cpu));

	/* Set Tx descriptors queue starting address */
	/* indirect access */
	mvpp2_write(priv, MVPP2_AGGR_TXQ_DESC_ADDR_REG(cpu),
		    aggr_txq->descs_phys);
	mvpp2_write(priv, MVPP2_AGGR_TXQ_DESC_SIZE_REG(cpu), desc_num);

	return 0;
}

/* Create a specified Rx queue */
static int mvpp2_rxq_init(struct mvpp2_port *port,
			  struct mvpp2_rx_queue *rxq)

{
	rxq->size = port->rx_ring_size;

	/* Allocate memory for RX descriptors */
	rxq->descs = buffer_loc.rx_descs;
	rxq->descs_phys = (dma_addr_t)buffer_loc.rx_descs;
	if (!rxq->descs)
		return -ENOMEM;

	BUG_ON(rxq->descs !=
	       PTR_ALIGN(rxq->descs, MVPP2_CPU_D_CACHE_LINE_SIZE));

	rxq->last_desc = rxq->size - 1;

	/* Zero occupied and non-occupied counters - direct access */
	mvpp2_write(port->priv, MVPP2_RXQ_STATUS_REG(rxq->id), 0);

	/* Set Rx descriptors queue starting address - indirect access */
	mvpp2_write(port->priv, MVPP2_RXQ_NUM_REG, rxq->id);
	mvpp2_write(port->priv, MVPP2_RXQ_DESC_ADDR_REG, rxq->descs_phys);
	mvpp2_write(port->priv, MVPP2_RXQ_DESC_SIZE_REG, rxq->size);
	mvpp2_write(port->priv, MVPP2_RXQ_INDEX_REG, 0);

	/* Set Offset */
	mvpp2_rxq_offset_set(port, rxq->id, NET_SKB_PAD);

	/* Add number of descriptors ready for receiving packets */
	mvpp2_rxq_status_update(port, rxq->id, 0, rxq->size);

	return 0;
}

/* Push packets received by the RXQ to BM pool */
static void mvpp2_rxq_drop_pkts(struct mvpp2_port *port,
				struct mvpp2_rx_queue *rxq)
{
	int rx_received, i;

	rx_received = mvpp2_rxq_received(port, rxq->id);
	if (!rx_received)
		return;

	for (i = 0; i < rx_received; i++) {
		struct mvpp2_rx_desc *rx_desc = mvpp2_rxq_next_desc_get(rxq);
		u32 bm = mvpp2_bm_cookie_build(rx_desc);

		mvpp2_pool_refill(port, bm, rx_desc->buf_phys_addr,
				  rx_desc->buf_cookie);
	}
	mvpp2_rxq_status_update(port, rxq->id, rx_received, rx_received);
}

/* Cleanup Rx queue */
static void mvpp2_rxq_deinit(struct mvpp2_port *port,
			     struct mvpp2_rx_queue *rxq)
{
	mvpp2_rxq_drop_pkts(port, rxq);

	rxq->descs             = NULL;
	rxq->last_desc         = 0;
	rxq->next_desc_to_proc = 0;
	rxq->descs_phys        = 0;

	/* Clear Rx descriptors queue starting address and size;
	 * free descriptor number
	 */
	mvpp2_write(port->priv, MVPP2_RXQ_STATUS_REG(rxq->id), 0);
	mvpp2_write(port->priv, MVPP2_RXQ_NUM_REG, rxq->id);
	mvpp2_write(port->priv, MVPP2_RXQ_DESC_ADDR_REG, 0);
	mvpp2_write(port->priv, MVPP2_RXQ_DESC_SIZE_REG, 0);
}

/* Create and initialize a Tx queue */
static int mvpp2_txq_init(struct mvpp2_port *port,
			  struct mvpp2_tx_queue *txq)
{
	u32 val;
	int cpu, desc, desc_per_txq, tx_port_num;
	struct mvpp2_txq_pcpu *txq_pcpu;

	txq->size = port->tx_ring_size;

	/* Allocate memory for Tx descriptors */
	txq->descs = buffer_loc.tx_descs;
	txq->descs_phys = (dma_addr_t)buffer_loc.tx_descs;
	if (!txq->descs)
		return -ENOMEM;

	/* Make sure descriptor address is cache line size aligned  */
	BUG_ON(txq->descs !=
	       PTR_ALIGN(txq->descs, MVPP2_CPU_D_CACHE_LINE_SIZE));

	txq->last_desc = txq->size - 1;

	/* Set Tx descriptors queue starting address - indirect access */
	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);
	mvpp2_write(port->priv, MVPP2_TXQ_DESC_ADDR_REG, txq->descs_phys);
	mvpp2_write(port->priv, MVPP2_TXQ_DESC_SIZE_REG, txq->size &
					     MVPP2_TXQ_DESC_SIZE_MASK);
	mvpp2_write(port->priv, MVPP2_TXQ_INDEX_REG, 0);
	mvpp2_write(port->priv, MVPP2_TXQ_RSVD_CLR_REG,
		    txq->id << MVPP2_TXQ_RSVD_CLR_OFFSET);
	val = mvpp2_read(port->priv, MVPP2_TXQ_PENDING_REG);
	val &= ~MVPP2_TXQ_PENDING_MASK;
	mvpp2_write(port->priv, MVPP2_TXQ_PENDING_REG, val);

	/* Calculate base address in prefetch buffer. We reserve 16 descriptors
	 * for each existing TXQ.
	 * TCONTS for PON port must be continuous from 0 to MVPP2_MAX_TCONT
	 * GBE ports assumed to be continious from 0 to MVPP2_MAX_PORTS
	 */
	desc_per_txq = 16;
	desc = (port->id * MVPP2_MAX_TXQ * desc_per_txq) +
	       (txq->log_id * desc_per_txq);

	mvpp2_write(port->priv, MVPP2_TXQ_PREF_BUF_REG,
		    MVPP2_PREF_BUF_PTR(desc) | MVPP2_PREF_BUF_SIZE_16 |
		    MVPP2_PREF_BUF_THRESH(desc_per_txq/2));

	/* WRR / EJP configuration - indirect access */
	tx_port_num = mvpp2_egress_port(port);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG, tx_port_num);

	val = mvpp2_read(port->priv, MVPP2_TXQ_SCHED_REFILL_REG(txq->log_id));
	val &= ~MVPP2_TXQ_REFILL_PERIOD_ALL_MASK;
	val |= MVPP2_TXQ_REFILL_PERIOD_MASK(1);
	val |= MVPP2_TXQ_REFILL_TOKENS_ALL_MASK;
	mvpp2_write(port->priv, MVPP2_TXQ_SCHED_REFILL_REG(txq->log_id), val);

	val = MVPP2_TXQ_TOKEN_SIZE_MAX;
	mvpp2_write(port->priv, MVPP2_TXQ_SCHED_TOKEN_SIZE_REG(txq->log_id),
		    val);

	for_each_present_cpu(cpu) {
		txq_pcpu = per_cpu_ptr(txq->pcpu, cpu);
		txq_pcpu->size = txq->size;
	}

	return 0;
}

/* Free allocated TXQ resources */
static void mvpp2_txq_deinit(struct mvpp2_port *port,
			     struct mvpp2_tx_queue *txq)
{
	txq->descs             = NULL;
	txq->last_desc         = 0;
	txq->next_desc_to_proc = 0;
	txq->descs_phys        = 0;

	/* Set minimum bandwidth for disabled TXQs */
	mvpp2_write(port->priv, MVPP2_TXQ_SCHED_TOKEN_CNTR_REG(txq->id), 0);

	/* Set Tx descriptors queue starting address and size */
	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);
	mvpp2_write(port->priv, MVPP2_TXQ_DESC_ADDR_REG, 0);
	mvpp2_write(port->priv, MVPP2_TXQ_DESC_SIZE_REG, 0);
}

/* Cleanup Tx ports */
static void mvpp2_txq_clean(struct mvpp2_port *port, struct mvpp2_tx_queue *txq)
{
	struct mvpp2_txq_pcpu *txq_pcpu;
	int delay, pending, cpu;
	u32 val;

	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);
	val = mvpp2_read(port->priv, MVPP2_TXQ_PREF_BUF_REG);
	val |= MVPP2_TXQ_DRAIN_EN_MASK;
	mvpp2_write(port->priv, MVPP2_TXQ_PREF_BUF_REG, val);

	/* The napi queue has been stopped so wait for all packets
	 * to be transmitted.
	 */
	delay = 0;
	do {
		if (delay >= MVPP2_TX_PENDING_TIMEOUT_MSEC) {
			netdev_warn(port->dev,
				    "port %d: cleaning queue %d timed out\n",
				    port->id, txq->log_id);
			break;
		}
		mdelay(1);
		delay++;

		pending = mvpp2_txq_pend_desc_num_get(port, txq);
	} while (pending);

	val &= ~MVPP2_TXQ_DRAIN_EN_MASK;
	mvpp2_write(port->priv, MVPP2_TXQ_PREF_BUF_REG, val);

	for_each_present_cpu(cpu) {
		txq_pcpu = per_cpu_ptr(txq->pcpu, cpu);

		/* Release all packets */
		mvpp2_txq_bufs_free(port, txq, txq_pcpu, txq_pcpu->count);

		/* Reset queue */
		txq_pcpu->count = 0;
		txq_pcpu->txq_put_index = 0;
		txq_pcpu->txq_get_index = 0;
	}
}

/* Cleanup all Tx queues */
static void mvpp2_cleanup_txqs(struct mvpp2_port *port)
{
	struct mvpp2_tx_queue *txq;
	int queue;
	u32 val;

	val = mvpp2_read(port->priv, MVPP2_TX_PORT_FLUSH_REG);

	/* Reset Tx ports and delete Tx queues */
	val |= MVPP2_TX_PORT_FLUSH_MASK(port->id);
	mvpp2_write(port->priv, MVPP2_TX_PORT_FLUSH_REG, val);

	for (queue = 0; queue < txq_number; queue++) {
		txq = port->txqs[queue];
		mvpp2_txq_clean(port, txq);
		mvpp2_txq_deinit(port, txq);
	}

	mvpp2_txq_sent_counter_clear(port);

	val &= ~MVPP2_TX_PORT_FLUSH_MASK(port->id);
	mvpp2_write(port->priv, MVPP2_TX_PORT_FLUSH_REG, val);
}

/* Cleanup all Rx queues */
static void mvpp2_cleanup_rxqs(struct mvpp2_port *port)
{
	int queue;

	for (queue = 0; queue < rxq_number; queue++)
		mvpp2_rxq_deinit(port, port->rxqs[queue]);
}

/* Init all Rx queues for port */
static int mvpp2_setup_rxqs(struct mvpp2_port *port)
{
	int queue, err;

	for (queue = 0; queue < rxq_number; queue++) {
		err = mvpp2_rxq_init(port, port->rxqs[queue]);
		if (err)
			goto err_cleanup;
	}
	return 0;

err_cleanup:
	mvpp2_cleanup_rxqs(port);
	return err;
}

/* Init all tx queues for port */
static int mvpp2_setup_txqs(struct mvpp2_port *port)
{
	struct mvpp2_tx_queue *txq;
	int queue, err;

	for (queue = 0; queue < txq_number; queue++) {
		txq = port->txqs[queue];
		err = mvpp2_txq_init(port, txq);
		if (err)
			goto err_cleanup;
	}

	mvpp2_txq_sent_counter_clear(port);
	return 0;

err_cleanup:
	mvpp2_cleanup_txqs(port);
	return err;
}

/* Adjust link */
static void mvpp2_link_event(struct mvpp2_port *port)
{
	struct phy_device *phydev = port->phy_dev;
	int status_change = 0;
	u32 val;

	if (phydev->link) {
		if ((port->speed != phydev->speed) ||
		    (port->duplex != phydev->duplex)) {
			u32 val;

			val = readl(port->base + MVPP2_GMAC_AUTONEG_CONFIG);
			val &= ~(MVPP2_GMAC_CONFIG_MII_SPEED |
				 MVPP2_GMAC_CONFIG_GMII_SPEED |
				 MVPP2_GMAC_CONFIG_FULL_DUPLEX |
				 MVPP2_GMAC_AN_SPEED_EN |
				 MVPP2_GMAC_AN_DUPLEX_EN);

			if (phydev->duplex)
				val |= MVPP2_GMAC_CONFIG_FULL_DUPLEX;

			if (phydev->speed == SPEED_1000)
				val |= MVPP2_GMAC_CONFIG_GMII_SPEED;
			else if (phydev->speed == SPEED_100)
				val |= MVPP2_GMAC_CONFIG_MII_SPEED;

			writel(val, port->base + MVPP2_GMAC_AUTONEG_CONFIG);

			port->duplex = phydev->duplex;
			port->speed  = phydev->speed;
		}
	}

	if (phydev->link != port->link) {
		if (!phydev->link) {
			port->duplex = -1;
			port->speed = 0;
		}

		port->link = phydev->link;
		status_change = 1;
	}

	if (status_change) {
		if (phydev->link) {
			val = readl(port->base + MVPP2_GMAC_AUTONEG_CONFIG);
			val |= (MVPP2_GMAC_FORCE_LINK_PASS |
				MVPP2_GMAC_FORCE_LINK_DOWN);
			writel(val, port->base + MVPP2_GMAC_AUTONEG_CONFIG);
			mvpp2_egress_enable(port);
			mvpp2_ingress_enable(port);
		} else {
			mvpp2_ingress_disable(port);
			mvpp2_egress_disable(port);
		}
	}
}

/* Main RX/TX processing routines */

/* Display more error info */
static void mvpp2_rx_error(struct mvpp2_port *port,
			   struct mvpp2_rx_desc *rx_desc)
{
	u32 status = rx_desc->status;

	switch (status & MVPP2_RXD_ERR_CODE_MASK) {
	case MVPP2_RXD_ERR_CRC:
		netdev_err(port->dev, "bad rx status %08x (crc error), size=%d\n",
			   status, rx_desc->data_size);
		break;
	case MVPP2_RXD_ERR_OVERRUN:
		netdev_err(port->dev, "bad rx status %08x (overrun error), size=%d\n",
			   status, rx_desc->data_size);
		break;
	case MVPP2_RXD_ERR_RESOURCE:
		netdev_err(port->dev, "bad rx status %08x (resource error), size=%d\n",
			   status, rx_desc->data_size);
		break;
	}
}

/* Reuse skb if possible, or allocate a new skb and add it to BM pool */
static int mvpp2_rx_refill(struct mvpp2_port *port,
			   struct mvpp2_bm_pool *bm_pool,
			   u32 bm, u32 phys_addr)
{
	mvpp2_pool_refill(port, bm, phys_addr, phys_addr);
	return 0;
}

/* Set hw internals when starting port */
static void mvpp2_start_dev(struct mvpp2_port *port)
{
	mvpp2_gmac_max_rx_size_set(port);
	mvpp2_txp_max_tx_size_set(port);

	mvpp2_port_enable(port);
}

/* Set hw internals when stopping port */
static void mvpp2_stop_dev(struct mvpp2_port *port)
{
	/* Stop new packets from arriving to RXQs */
	mvpp2_ingress_disable(port);

	mvpp2_egress_disable(port);
	mvpp2_port_disable(port);
}

static int mvpp2_phy_connect(struct udevice *dev, struct mvpp2_port *port)
{
	struct phy_device *phy_dev;

	if (!port->init || port->link == 0) {
		phy_dev = phy_connect(port->priv->bus, port->phyaddr, dev,
				      port->phy_interface);
		port->phy_dev = phy_dev;
		if (!phy_dev) {
			netdev_err(port->dev, "cannot connect to phy\n");
			return -ENODEV;
		}
		phy_dev->supported &= PHY_GBIT_FEATURES;
		phy_dev->advertising = phy_dev->supported;

		port->phy_dev = phy_dev;
		port->link    = 0;
		port->duplex  = 0;
		port->speed   = 0;

		phy_config(phy_dev);
		phy_startup(phy_dev);
		if (!phy_dev->link) {
			printf("%s: No link\n", phy_dev->dev->name);
			return -1;
		}

		port->init = 1;
	} else {
		mvpp2_egress_enable(port);
		mvpp2_ingress_enable(port);
	}

	return 0;
}

static int mvpp2_open(struct udevice *dev, struct mvpp2_port *port)
{
	unsigned char mac_bcast[ETH_ALEN] = {
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	int err;

	err = mvpp2_prs_mac_da_accept(port->priv, port->id, mac_bcast, true);
	if (err) {
		netdev_err(dev, "mvpp2_prs_mac_da_accept BC failed\n");
		return err;
	}
	err = mvpp2_prs_mac_da_accept(port->priv, port->id,
				      port->dev_addr, true);
	if (err) {
		netdev_err(dev, "mvpp2_prs_mac_da_accept MC failed\n");
		return err;
	}
	err = mvpp2_prs_def_flow(port);
	if (err) {
		netdev_err(dev, "mvpp2_prs_def_flow failed\n");
		return err;
	}

	/* Allocate the Rx/Tx queues */
	err = mvpp2_setup_rxqs(port);
	if (err) {
		netdev_err(port->dev, "cannot allocate Rx queues\n");
		return err;
	}

	err = mvpp2_setup_txqs(port);
	if (err) {
		netdev_err(port->dev, "cannot allocate Tx queues\n");
		return err;
	}

	err = mvpp2_phy_connect(dev, port);
	if (err < 0)
		return err;

	mvpp2_link_event(port);

	mvpp2_start_dev(port);

	return 0;
}

/* No Device ops here in U-Boot */

/* Driver initialization */

static void mvpp2_port_power_up(struct mvpp2_port *port)
{
	mvpp2_port_mii_set(port);
	mvpp2_port_periodic_xon_disable(port);
	mvpp2_port_fc_adv_enable(port);
	mvpp2_port_reset(port);
}

/* Initialize port HW */
static int mvpp2_port_init(struct udevice *dev, struct mvpp2_port *port)
{
	struct mvpp2 *priv = port->priv;
	struct mvpp2_txq_pcpu *txq_pcpu;
	int queue, cpu, err;

	if (port->first_rxq + rxq_number > MVPP2_RXQ_TOTAL_NUM)
		return -EINVAL;

	/* Disable port */
	mvpp2_egress_disable(port);
	mvpp2_port_disable(port);

	port->txqs = devm_kcalloc(dev, txq_number, sizeof(*port->txqs),
				  GFP_KERNEL);
	if (!port->txqs)
		return -ENOMEM;

	/* Associate physical Tx queues to this port and initialize.
	 * The mapping is predefined.
	 */
	for (queue = 0; queue < txq_number; queue++) {
		int queue_phy_id = mvpp2_txq_phys(port->id, queue);
		struct mvpp2_tx_queue *txq;

		txq = devm_kzalloc(dev, sizeof(*txq), GFP_KERNEL);
		if (!txq)
			return -ENOMEM;

		txq->pcpu = devm_kzalloc(dev, sizeof(struct mvpp2_txq_pcpu),
					 GFP_KERNEL);
		if (!txq->pcpu)
			return -ENOMEM;

		txq->id = queue_phy_id;
		txq->log_id = queue;
		txq->done_pkts_coal = MVPP2_TXDONE_COAL_PKTS_THRESH;
		for_each_present_cpu(cpu) {
			txq_pcpu = per_cpu_ptr(txq->pcpu, cpu);
			txq_pcpu->cpu = cpu;
		}

		port->txqs[queue] = txq;
	}

	port->rxqs = devm_kcalloc(dev, rxq_number, sizeof(*port->rxqs),
				  GFP_KERNEL);
	if (!port->rxqs)
		return -ENOMEM;

	/* Allocate and initialize Rx queue for this port */
	for (queue = 0; queue < rxq_number; queue++) {
		struct mvpp2_rx_queue *rxq;

		/* Map physical Rx queue to port's logical Rx queue */
		rxq = devm_kzalloc(dev, sizeof(*rxq), GFP_KERNEL);
		if (!rxq)
			return -ENOMEM;
		/* Map this Rx queue to a physical queue */
		rxq->id = port->first_rxq + queue;
		rxq->port = port->id;
		rxq->logic_rxq = queue;

		port->rxqs[queue] = rxq;
	}

	/* Configure Rx queue group interrupt for this port */
	mvpp2_write(priv, MVPP2_ISR_RXQ_GROUP_REG(port->id), CONFIG_MV_ETH_RXQ);

	/* Create Rx descriptor rings */
	for (queue = 0; queue < rxq_number; queue++) {
		struct mvpp2_rx_queue *rxq = port->rxqs[queue];

		rxq->size = port->rx_ring_size;
		rxq->pkts_coal = MVPP2_RX_COAL_PKTS;
		rxq->time_coal = MVPP2_RX_COAL_USEC;
	}

	mvpp2_ingress_disable(port);

	/* Port default configuration */
	mvpp2_defaults_set(port);

	/* Port's classifier configuration */
	mvpp2_cls_oversize_rxq_set(port);
	mvpp2_cls_port_config(port);

	/* Provide an initial Rx packet size */
	port->pkt_size = MVPP2_RX_PKT_SIZE(PKTSIZE_ALIGN);

	/* Initialize pools for swf */
	err = mvpp2_swf_bm_pool_init(port);
	if (err)
		return err;

	return 0;
}

/* Ports initialization */
static int mvpp2_port_probe(struct udevice *dev,
			    struct mvpp2_port *port,
			    int port_node,
			    struct mvpp2 *priv,
			    int *next_first_rxq)
{
	int phy_node;
	u32 id;
	u32 phyaddr;
	const char *phy_mode_str;
	int phy_mode = -1;
	int priv_common_regs_num = 2;
	int err;

	phy_node = fdtdec_lookup_phandle(gd->fdt_blob, port_node, "phy");
	if (phy_node < 0) {
		dev_err(&pdev->dev, "missing phy\n");
		return -ENODEV;
	}

	phy_mode_str = fdt_getprop(gd->fdt_blob, port_node, "phy-mode", NULL);
	if (phy_mode_str)
		phy_mode = phy_get_interface_by_name(phy_mode_str);
	if (phy_mode == -1) {
		dev_err(&pdev->dev, "incorrect phy mode\n");
		return -EINVAL;
	}

	id = fdtdec_get_int(gd->fdt_blob, port_node, "port-id", -1);
	if (id == -1) {
		dev_err(&pdev->dev, "missing port-id value\n");
		return -EINVAL;
	}

	phyaddr = fdtdec_get_int(gd->fdt_blob, phy_node, "reg", 0);

	port->priv = priv;
	port->id = id;
	port->first_rxq = *next_first_rxq;
	port->phy_node = phy_node;
	port->phy_interface = phy_mode;
	port->phyaddr = phyaddr;

	port->base = (void __iomem *)dev_get_addr_index(dev->parent,
							priv_common_regs_num
							+ id);
	if (IS_ERR(port->base))
		return PTR_ERR(port->base);

	port->tx_ring_size = MVPP2_MAX_TXD;
	port->rx_ring_size = MVPP2_MAX_RXD;

	err = mvpp2_port_init(dev, port);
	if (err < 0) {
		dev_err(&pdev->dev, "failed to init port %d\n", id);
		return err;
	}
	mvpp2_port_power_up(port);

	/* Increment the first Rx queue number to be used by the next port */
	*next_first_rxq += CONFIG_MV_ETH_RXQ;
	priv->port_list[id] = port;
	return 0;
}

/* Initialize decoding windows */
static void mvpp2_conf_mbus_windows(const struct mbus_dram_target_info *dram,
				    struct mvpp2 *priv)
{
	u32 win_enable;
	int i;

	for (i = 0; i < 6; i++) {
		mvpp2_write(priv, MVPP2_WIN_BASE(i), 0);
		mvpp2_write(priv, MVPP2_WIN_SIZE(i), 0);

		if (i < 4)
			mvpp2_write(priv, MVPP2_WIN_REMAP(i), 0);
	}

	win_enable = 0;

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		mvpp2_write(priv, MVPP2_WIN_BASE(i),
			    (cs->base & 0xffff0000) | (cs->mbus_attr << 8) |
			    dram->mbus_dram_target_id);

		mvpp2_write(priv, MVPP2_WIN_SIZE(i),
			    (cs->size - 1) & 0xffff0000);

		win_enable |= (1 << i);
	}

	mvpp2_write(priv, MVPP2_BASE_ADDR_ENABLE, win_enable);
}

/* Initialize Rx FIFO's */
static void mvpp2_rx_fifo_init(struct mvpp2 *priv)
{
	int port;

	for (port = 0; port < MVPP2_MAX_PORTS; port++) {
		mvpp2_write(priv, MVPP2_RX_DATA_FIFO_SIZE_REG(port),
			    MVPP2_RX_FIFO_PORT_DATA_SIZE);
		mvpp2_write(priv, MVPP2_RX_ATTR_FIFO_SIZE_REG(port),
			    MVPP2_RX_FIFO_PORT_ATTR_SIZE);
	}

	mvpp2_write(priv, MVPP2_RX_MIN_PKT_SIZE_REG,
		    MVPP2_RX_FIFO_PORT_MIN_PKT);
	mvpp2_write(priv, MVPP2_RX_FIFO_INIT_REG, 0x1);
}

/* Initialize network controller common part HW */
static int mvpp2_init(struct udevice *dev, struct mvpp2 *priv)
{
	const struct mbus_dram_target_info *dram_target_info;
	int err, i;
	u32 val;

	/* Checks for hardware constraints (U-Boot uses only one rxq) */
	if ((rxq_number > MVPP2_MAX_RXQ) || (txq_number > MVPP2_MAX_TXQ)) {
		dev_err(&pdev->dev, "invalid queue size parameter\n");
		return -EINVAL;
	}

	/* MBUS windows configuration */
	dram_target_info = mvebu_mbus_dram_info();
	if (dram_target_info)
		mvpp2_conf_mbus_windows(dram_target_info, priv);

	/* Disable HW PHY polling */
	val = readl(priv->lms_base + MVPP2_PHY_AN_CFG0_REG);
	val |= MVPP2_PHY_AN_STOP_SMI0_MASK;
	writel(val, priv->lms_base + MVPP2_PHY_AN_CFG0_REG);

	/* Allocate and initialize aggregated TXQs */
	priv->aggr_txqs = devm_kcalloc(dev, num_present_cpus(),
				       sizeof(struct mvpp2_tx_queue),
				       GFP_KERNEL);
	if (!priv->aggr_txqs)
		return -ENOMEM;

	for_each_present_cpu(i) {
		priv->aggr_txqs[i].id = i;
		priv->aggr_txqs[i].size = MVPP2_AGGR_TXQ_SIZE;
		err = mvpp2_aggr_txq_init(dev, &priv->aggr_txqs[i],
					  MVPP2_AGGR_TXQ_SIZE, i, priv);
		if (err < 0)
			return err;
	}

	/* Rx Fifo Init */
	mvpp2_rx_fifo_init(priv);

	/* Reset Rx queue group interrupt configuration */
	for (i = 0; i < MVPP2_MAX_PORTS; i++)
		mvpp2_write(priv, MVPP2_ISR_RXQ_GROUP_REG(i),
			    CONFIG_MV_ETH_RXQ);

	writel(MVPP2_EXT_GLOBAL_CTRL_DEFAULT,
	       priv->lms_base + MVPP2_MNG_EXTENDED_GLOBAL_CTRL_REG);

	/* Allow cache snoop when transmiting packets */
	mvpp2_write(priv, MVPP2_TX_SNOOP_REG, 0x1);

	/* Buffer Manager initialization */
	err = mvpp2_bm_init(dev, priv);
	if (err < 0)
		return err;

	/* Parser default initialization */
	err = mvpp2_prs_default_init(dev, priv);
	if (err < 0)
		return err;

	/* Classifier default initialization */
	mvpp2_cls_init(priv);

	return 0;
}

/* SMI / MDIO functions */

static int smi_wait_ready(struct mvpp2 *priv)
{
	u32 timeout = MVPP2_SMI_TIMEOUT;
	u32 smi_reg;

	/* wait till the SMI is not busy */
	do {
		/* read smi register */
		smi_reg = readl(priv->lms_base + MVPP2_SMI);
		if (timeout-- == 0) {
			printf("Error: SMI busy timeout\n");
			return -EFAULT;
		}
	} while (smi_reg & MVPP2_SMI_BUSY);

	return 0;
}

/*
 * mpp2_mdio_read - miiphy_read callback function.
 *
 * Returns 16bit phy register value, or 0xffff on error
 */
static int mpp2_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mvpp2 *priv = bus->priv;
	u32 smi_reg;
	u32 timeout;

	/* check parameters */
	if (addr > MVPP2_PHY_ADDR_MASK) {
		printf("Error: Invalid PHY address %d\n", addr);
		return -EFAULT;
	}

	if (reg > MVPP2_PHY_REG_MASK) {
		printf("Err: Invalid register offset %d\n", reg);
		return -EFAULT;
	}

	/* wait till the SMI is not busy */
	if (smi_wait_ready(priv) < 0)
		return -EFAULT;

	/* fill the phy address and regiser offset and read opcode */
	smi_reg = (addr << MVPP2_SMI_DEV_ADDR_OFFS)
		| (reg << MVPP2_SMI_REG_ADDR_OFFS)
		| MVPP2_SMI_OPCODE_READ;

	/* write the smi register */
	writel(smi_reg, priv->lms_base + MVPP2_SMI);

	/* wait till read value is ready */
	timeout = MVPP2_SMI_TIMEOUT;

	do {
		/* read smi register */
		smi_reg = readl(priv->lms_base + MVPP2_SMI);
		if (timeout-- == 0) {
			printf("Err: SMI read ready timeout\n");
			return -EFAULT;
		}
	} while (!(smi_reg & MVPP2_SMI_READ_VALID));

	/* Wait for the data to update in the SMI register */
	for (timeout = 0; timeout < MVPP2_SMI_TIMEOUT; timeout++)
		;

	return readl(priv->lms_base + MVPP2_SMI) & MVPP2_SMI_DATA_MASK;
}

/*
 * mpp2_mdio_write - miiphy_write callback function.
 *
 * Returns 0 if write succeed, -EINVAL on bad parameters
 * -ETIME on timeout
 */
static int mpp2_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			   u16 value)
{
	struct mvpp2 *priv = bus->priv;
	u32 smi_reg;

	/* check parameters */
	if (addr > MVPP2_PHY_ADDR_MASK) {
		printf("Error: Invalid PHY address %d\n", addr);
		return -EFAULT;
	}

	if (reg > MVPP2_PHY_REG_MASK) {
		printf("Err: Invalid register offset %d\n", reg);
		return -EFAULT;
	}

	/* wait till the SMI is not busy */
	if (smi_wait_ready(priv) < 0)
		return -EFAULT;

	/* fill the phy addr and reg offset and write opcode and data */
	smi_reg = value << MVPP2_SMI_DATA_OFFS;
	smi_reg |= (addr << MVPP2_SMI_DEV_ADDR_OFFS)
		| (reg << MVPP2_SMI_REG_ADDR_OFFS);
	smi_reg &= ~MVPP2_SMI_OPCODE_READ;

	/* write the smi register */
	writel(smi_reg, priv->lms_base + MVPP2_SMI);

	return 0;
}

static int mvpp2_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct mvpp2_port *port = dev_get_priv(dev);
	struct mvpp2_rx_desc *rx_desc;
	struct mvpp2_bm_pool *bm_pool;
	dma_addr_t phys_addr;
	u32 bm, rx_status;
	int pool, rx_bytes, err;
	int rx_received;
	struct mvpp2_rx_queue *rxq;
	u32 cause_rx_tx, cause_rx, cause_misc;
	u8 *data;

	cause_rx_tx = mvpp2_read(port->priv,
				 MVPP2_ISR_RX_TX_CAUSE_REG(port->id));
	cause_rx_tx &= ~MVPP2_CAUSE_TXQ_OCCUP_DESC_ALL_MASK;
	cause_misc = cause_rx_tx & MVPP2_CAUSE_MISC_SUM_MASK;
	if (!cause_rx_tx && !cause_misc)
		return 0;

	cause_rx = cause_rx_tx & MVPP2_CAUSE_RXQ_OCCUP_DESC_ALL_MASK;

	/* Process RX packets */
	cause_rx |= port->pending_cause_rx;
	rxq = mvpp2_get_rx_queue(port, cause_rx);

	/* Get number of received packets and clamp the to-do */
	rx_received = mvpp2_rxq_received(port, rxq->id);

	/* Return if no packets are received */
	if (!rx_received)
		return 0;

	rx_desc = mvpp2_rxq_next_desc_get(rxq);
	rx_status = rx_desc->status;
	rx_bytes = rx_desc->data_size - MVPP2_MH_SIZE;
	phys_addr = rx_desc->buf_phys_addr;

	bm = mvpp2_bm_cookie_build(rx_desc);
	pool = mvpp2_bm_cookie_pool_get(bm);
	bm_pool = &port->priv->bm_pools[pool];

	/* Check if buffer header is used */
	if (rx_status & MVPP2_RXD_BUF_HDR)
		return 0;

	/* In case of an error, release the requested buffer pointer
	 * to the Buffer Manager. This request process is controlled
	 * by the hardware, and the information about the buffer is
	 * comprised by the RX descriptor.
	 */
	if (rx_status & MVPP2_RXD_ERR_SUMMARY) {
		mvpp2_rx_error(port, rx_desc);
		/* Return the buffer to the pool */
		mvpp2_pool_refill(port, bm, rx_desc->buf_phys_addr,
				  rx_desc->buf_cookie);
		return 0;
	}

	err = mvpp2_rx_refill(port, bm_pool, bm, phys_addr);
	if (err) {
		netdev_err(port->dev, "failed to refill BM pools\n");
		return 0;
	}

	/* Update Rx queue management counters */
	mb();
	mvpp2_rxq_status_update(port, rxq->id, 1, 1);

	/* give packet to stack - skip on first n bytes */
	data = (u8 *)phys_addr + 2 + 32;

	if (rx_bytes <= 0)
		return 0;

	/*
	 * No cache invalidation needed here, since the rx_buffer's are
	 * located in a uncached memory region
	 */
	*packetp = data;

	return rx_bytes;
}

/* Drain Txq */
static void mvpp2_txq_drain(struct mvpp2_port *port, struct mvpp2_tx_queue *txq,
			    int enable)
{
	u32 val;

	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);
	val = mvpp2_read(port->priv, MVPP2_TXQ_PREF_BUF_REG);
	if (enable)
		val |= MVPP2_TXQ_DRAIN_EN_MASK;
	else
		val &= ~MVPP2_TXQ_DRAIN_EN_MASK;
	mvpp2_write(port->priv, MVPP2_TXQ_PREF_BUF_REG, val);
}

static int mvpp2_send(struct udevice *dev, void *packet, int length)
{
	struct mvpp2_port *port = dev_get_priv(dev);
	struct mvpp2_tx_queue *txq, *aggr_txq;
	struct mvpp2_tx_desc *tx_desc;
	int tx_done;
	int timeout;

	txq = port->txqs[0];
	aggr_txq = &port->priv->aggr_txqs[smp_processor_id()];

	/* Get a descriptor for the first part of the packet */
	tx_desc = mvpp2_txq_next_desc_get(aggr_txq);
	tx_desc->phys_txq = txq->id;
	tx_desc->data_size = length;
	tx_desc->packet_offset = (u32)packet & MVPP2_TX_DESC_ALIGN;
	tx_desc->buf_phys_addr = (u32)packet & ~MVPP2_TX_DESC_ALIGN;
	/* First and Last descriptor */
	tx_desc->command = MVPP2_TXD_L4_CSUM_NOT | MVPP2_TXD_IP_CSUM_DISABLE
		| MVPP2_TXD_F_DESC | MVPP2_TXD_L_DESC;

	/* Flush tx data */
	flush_dcache_range((unsigned long)packet,
			   (unsigned long)packet + ALIGN(length, PKTALIGN));

	/* Enable transmit */
	mb();
	mvpp2_aggr_txq_pend_desc_add(port, 1);

	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);

	timeout = 0;
	do {
		if (timeout++ > 10000) {
			printf("timeout: packet not sent from aggregated to phys TXQ\n");
			return 0;
		}
		tx_done = mvpp2_txq_pend_desc_num_get(port, txq);
	} while (tx_done);

	/* Enable TXQ drain */
	mvpp2_txq_drain(port, txq, 1);

	timeout = 0;
	do {
		if (timeout++ > 10000) {
			printf("timeout: packet not sent\n");
			return 0;
		}
		tx_done = mvpp2_txq_sent_desc_proc(port, txq);
	} while (!tx_done);

	/* Disable TXQ drain */
	mvpp2_txq_drain(port, txq, 0);

	return 0;
}

static int mvpp2_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct mvpp2_port *port = dev_get_priv(dev);

	/* Load current MAC address */
	memcpy(port->dev_addr, pdata->enetaddr, ETH_ALEN);

	/* Reconfigure parser accept the original MAC address */
	mvpp2_prs_update_mac_da(port, port->dev_addr);

	mvpp2_port_power_up(port);

	mvpp2_open(dev, port);

	return 0;
}

static void mvpp2_stop(struct udevice *dev)
{
	struct mvpp2_port *port = dev_get_priv(dev);

	mvpp2_stop_dev(port);
	mvpp2_cleanup_rxqs(port);
	mvpp2_cleanup_txqs(port);
}

static int mvpp2_probe(struct udevice *dev)
{
	struct mvpp2_port *port = dev_get_priv(dev);
	struct mvpp2 *priv = dev_get_priv(dev->parent);
	int err;

	/* Initialize network controller */
	err = mvpp2_init(dev, priv);
	if (err < 0) {
		dev_err(&pdev->dev, "failed to initialize controller\n");
		return err;
	}

	return mvpp2_port_probe(dev, port, dev_of_offset(dev), priv,
				&buffer_loc.first_rxq);
}

static const struct eth_ops mvpp2_ops = {
	.start		= mvpp2_start,
	.send		= mvpp2_send,
	.recv		= mvpp2_recv,
	.stop		= mvpp2_stop,
};

static struct driver mvpp2_driver = {
	.name	= "mvpp2",
	.id	= UCLASS_ETH,
	.probe	= mvpp2_probe,
	.ops	= &mvpp2_ops,
	.priv_auto_alloc_size = sizeof(struct mvpp2_port),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

/*
 * Use a MISC device to bind the n instances (child nodes) of the
 * network base controller in UCLASS_ETH.
 */
static int mvpp2_base_probe(struct udevice *dev)
{
	struct mvpp2 *priv = dev_get_priv(dev);
	struct mii_dev *bus;
	void *bd_space;
	u32 size = 0;
	int i;

	/*
	 * U-Boot special buffer handling:
	 *
	 * Allocate buffer area for descs and rx_buffers. This is only
	 * done once for all interfaces. As only one interface can
	 * be active. Make this area DMA-safe by disabling the D-cache
	 */

	/* Align buffer area for descs and rx_buffers to 1MiB */
	bd_space = memalign(1 << MMU_SECTION_SHIFT, BD_SPACE);
	mmu_set_region_dcache_behaviour((u32)bd_space, BD_SPACE, DCACHE_OFF);

	buffer_loc.aggr_tx_descs = (struct mvpp2_tx_desc *)bd_space;
	size += MVPP2_AGGR_TXQ_SIZE * MVPP2_DESC_ALIGNED_SIZE;

	buffer_loc.tx_descs = (struct mvpp2_tx_desc *)((u32)bd_space + size);
	size += MVPP2_MAX_TXD * MVPP2_DESC_ALIGNED_SIZE;

	buffer_loc.rx_descs = (struct mvpp2_rx_desc *)((u32)bd_space + size);
	size += MVPP2_MAX_RXD * MVPP2_DESC_ALIGNED_SIZE;

	for (i = 0; i < MVPP2_BM_POOLS_NUM; i++) {
		buffer_loc.bm_pool[i] = (u32 *)((u32)bd_space + size);
		size += MVPP2_BM_POOL_SIZE_MAX * sizeof(u32);
	}

	for (i = 0; i < MVPP2_BM_LONG_BUF_NUM; i++) {
		buffer_loc.rx_buffer[i] = (u32 *)((u32)bd_space + size);
		size += RX_BUFFER_SIZE;
	}

	/* Save base addresses for later use */
	priv->base = (void *)dev_get_addr_index(dev, 0);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	priv->lms_base = (void *)dev_get_addr_index(dev, 1);
	if (IS_ERR(priv->lms_base))
		return PTR_ERR(priv->lms_base);

	/* Finally create and register the MDIO bus driver */
	bus = mdio_alloc();
	if (!bus) {
		printf("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = mpp2_mdio_read;
	bus->write = mpp2_mdio_write;
	snprintf(bus->name, sizeof(bus->name), dev->name);
	bus->priv = (void *)priv;
	priv->bus = bus;

	return mdio_register(bus);
}

static int mvpp2_base_bind(struct udevice *parent)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(parent);
	struct uclass_driver *drv;
	struct udevice *dev;
	struct eth_pdata *plat;
	char *name;
	int subnode;
	u32 id;

	/* Lookup eth driver */
	drv = lists_uclass_lookup(UCLASS_ETH);
	if (!drv) {
		puts("Cannot find eth driver\n");
		return -ENOENT;
	}

	fdt_for_each_subnode(subnode, blob, node) {
		/* Skip disabled ports */
		if (!fdtdec_get_is_enabled(blob, subnode))
			continue;

		plat = calloc(1, sizeof(*plat));
		if (!plat)
			return -ENOMEM;

		id = fdtdec_get_int(blob, subnode, "port-id", -1);

		name = calloc(1, 16);
		sprintf(name, "mvpp2-%d", id);

		/* Create child device UCLASS_ETH and bind it */
		device_bind(parent, &mvpp2_driver, name, plat, subnode, &dev);
		dev_set_of_offset(dev, subnode);
	}

	return 0;
}

static const struct udevice_id mvpp2_ids[] = {
	{ .compatible = "marvell,armada-375-pp2" },
	{ }
};

U_BOOT_DRIVER(mvpp2_base) = {
	.name	= "mvpp2_base",
	.id	= UCLASS_MISC,
	.of_match = mvpp2_ids,
	.bind	= mvpp2_base_bind,
	.probe	= mvpp2_base_probe,
	.priv_auto_alloc_size = sizeof(struct mvpp2),
};
