/***********************************************************************
 *
 * Copyright (c) 2005 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Description:
 *   Ethernet interface for Tundra TSI108 bridge chip
 *
 ***********************************************************************/

#include <config.h>

#if defined(CONFIG_CMD_NET) && defined(CONFIG_NET_MULTI) \
	&& defined(CONFIG_TSI108_ETH)

#if !defined(CONFIG_TSI108_ETH_NUM_PORTS) || (CONFIG_TSI108_ETH_NUM_PORTS > 2)
#error "CONFIG_TSI108_ETH_NUM_PORTS must be defined as 1 or 2"
#endif

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/cache.h>

#ifdef DEBUG
#define TSI108_ETH_DEBUG 7
#else
#define TSI108_ETH_DEBUG 0
#endif

#if TSI108_ETH_DEBUG > 0
#define debug_lev(lev, fmt, args...) \
if (lev <= TSI108_ETH_DEBUG) \
printf ("%s %d: " fmt, __FUNCTION__, __LINE__, ##args)
#else
#define debug_lev(lev, fmt, args...) do{}while(0)
#endif

#define RX_PRINT_ERRORS
#define TX_PRINT_ERRORS

#define ETH_BASE	(CFG_TSI108_CSR_BASE + 0x6000)

#define ETH_PORT_OFFSET	0x400

#define __REG32(base, offset) (*((volatile u32 *)((char *)(base) + (offset))))

#define reg_MAC_CONFIG_1(base)		__REG32(base, 0x00000000)
#define MAC_CONFIG_1_TX_ENABLE		(0x00000001)
#define MAC_CONFIG_1_SYNC_TX_ENABLE	(0x00000002)
#define MAC_CONFIG_1_RX_ENABLE		(0x00000004)
#define MAC_CONFIG_1_SYNC_RX_ENABLE	(0x00000008)
#define MAC_CONFIG_1_TX_FLOW_CONTROL	(0x00000010)
#define MAC_CONFIG_1_RX_FLOW_CONTROL	(0x00000020)
#define MAC_CONFIG_1_LOOP_BACK		(0x00000100)
#define MAC_CONFIG_1_RESET_TX_FUNCTION	(0x00010000)
#define MAC_CONFIG_1_RESET_RX_FUNCTION	(0x00020000)
#define MAC_CONFIG_1_RESET_TX_MAC	(0x00040000)
#define MAC_CONFIG_1_RESET_RX_MAC	(0x00080000)
#define MAC_CONFIG_1_SIM_RESET		(0x40000000)
#define MAC_CONFIG_1_SOFT_RESET		(0x80000000)

#define reg_MAC_CONFIG_2(base)		__REG32(base, 0x00000004)
#define MAC_CONFIG_2_FULL_DUPLEX	(0x00000001)
#define MAC_CONFIG_2_CRC_ENABLE		(0x00000002)
#define MAC_CONFIG_2_PAD_CRC		(0x00000004)
#define MAC_CONFIG_2_LENGTH_CHECK	(0x00000010)
#define MAC_CONFIG_2_HUGE_FRAME		(0x00000020)
#define MAC_CONFIG_2_INTERFACE_MODE(val)	(((val) & 0x3) << 8)
#define MAC_CONFIG_2_PREAMBLE_LENGTH(val)	(((val) & 0xf) << 12)
#define INTERFACE_MODE_NIBBLE		1	/* 10/100 Mb/s MII) */
#define INTERFACE_MODE_BYTE		2	/* 1000 Mb/s GMII/TBI */

#define reg_MAXIMUM_FRAME_LENGTH(base)		__REG32(base, 0x00000010)

#define reg_MII_MGMT_CONFIG(base)		__REG32(base, 0x00000020)
#define MII_MGMT_CONFIG_MGMT_CLOCK_SELECT(val)	((val) & 0x7)
#define MII_MGMT_CONFIG_NO_PREAMBLE		(0x00000010)
#define MII_MGMT_CONFIG_SCAN_INCREMENT		(0x00000020)
#define MII_MGMT_CONFIG_RESET_MGMT		(0x80000000)

#define reg_MII_MGMT_COMMAND(base)		__REG32(base, 0x00000024)
#define MII_MGMT_COMMAND_READ_CYCLE		(0x00000001)
#define MII_MGMT_COMMAND_SCAN_CYCLE		(0x00000002)

#define reg_MII_MGMT_ADDRESS(base)		__REG32(base, 0x00000028)
#define reg_MII_MGMT_CONTROL(base)		__REG32(base, 0x0000002c)
#define reg_MII_MGMT_STATUS(base)		__REG32(base, 0x00000030)

#define reg_MII_MGMT_INDICATORS(base)		__REG32(base, 0x00000034)
#define MII_MGMT_INDICATORS_BUSY		(0x00000001)
#define MII_MGMT_INDICATORS_SCAN		(0x00000002)
#define MII_MGMT_INDICATORS_NOT_VALID		(0x00000004)

#define reg_INTERFACE_STATUS(base)		__REG32(base, 0x0000003c)
#define INTERFACE_STATUS_LINK_FAIL		(0x00000008)
#define INTERFACE_STATUS_EXCESS_DEFER		(0x00000200)

#define reg_STATION_ADDRESS_1(base)		__REG32(base, 0x00000040)
#define reg_STATION_ADDRESS_2(base)		__REG32(base, 0x00000044)

#define reg_PORT_CONTROL(base)			__REG32(base, 0x00000200)
#define PORT_CONTROL_PRI		(0x00000001)
#define PORT_CONTROL_BPT		(0x00010000)
#define PORT_CONTROL_SPD		(0x00040000)
#define PORT_CONTROL_RBC		(0x00080000)
#define PORT_CONTROL_PRB		(0x00200000)
#define PORT_CONTROL_DIS		(0x00400000)
#define PORT_CONTROL_TBI		(0x00800000)
#define PORT_CONTROL_STE		(0x10000000)
#define PORT_CONTROL_ZOR		(0x20000000)
#define PORT_CONTROL_CLR		(0x40000000)
#define PORT_CONTROL_SRT		(0x80000000)

#define reg_TX_CONFIG(base)		__REG32(base, 0x00000220)
#define TX_CONFIG_START_Q		(0x00000003)
#define TX_CONFIG_EHP			(0x00400000)
#define TX_CONFIG_CHP			(0x00800000)
#define TX_CONFIG_RST			(0x80000000)

#define reg_TX_CONTROL(base)		__REG32(base, 0x00000224)
#define TX_CONTROL_GO			(0x00008000)
#define TX_CONTROL_MP			(0x01000000)
#define TX_CONTROL_EAI			(0x20000000)
#define TX_CONTROL_ABT			(0x40000000)
#define TX_CONTROL_EII			(0x80000000)

#define reg_TX_STATUS(base)		__REG32(base, 0x00000228)
#define TX_STATUS_QUEUE_USABLE		(0x0000000f)
#define TX_STATUS_CURR_Q		(0x00000300)
#define TX_STATUS_ACT			(0x00008000)
#define TX_STATUS_QUEUE_IDLE		(0x000f0000)
#define TX_STATUS_EOQ_PENDING		(0x0f000000)

#define reg_TX_EXTENDED_STATUS(base)		__REG32(base, 0x0000022c)
#define TX_EXTENDED_STATUS_END_OF_QUEUE_CONDITION		(0x0000000f)
#define TX_EXTENDED_STATUS_END_OF_FRAME_CONDITION		(0x00000f00)
#define TX_EXTENDED_STATUS_DESCRIPTOR_INTERRUPT_CONDITION	(0x000f0000)
#define TX_EXTENDED_STATUS_ERROR_FLAG				(0x0f000000)

#define reg_TX_THRESHOLDS(base)			__REG32(base, 0x00000230)

#define reg_TX_DIAGNOSTIC_ADDR(base)           __REG32(base, 0x00000270)
#define TX_DIAGNOSTIC_ADDR_INDEX		(0x0000007f)
#define TX_DIAGNOSTIC_ADDR_DFR			(0x40000000)
#define TX_DIAGNOSTIC_ADDR_AI			(0x80000000)

#define reg_TX_DIAGNOSTIC_DATA(base)		__REG32(base, 0x00000274)

#define reg_TX_ERROR_STATUS(base)		__REG32(base, 0x00000278)
#define TX_ERROR_STATUS				(0x00000278)
#define TX_ERROR_STATUS_QUEUE_0_ERROR_RESPONSE	(0x0000000f)
#define TX_ERROR_STATUS_TEA_ON_QUEUE_0		(0x00000010)
#define TX_ERROR_STATUS_RER_ON_QUEUE_0		(0x00000020)
#define TX_ERROR_STATUS_TER_ON_QUEUE_0		(0x00000040)
#define TX_ERROR_STATUS_DER_ON_QUEUE_0		(0x00000080)
#define TX_ERROR_STATUS_QUEUE_1_ERROR_RESPONSE	(0x00000f00)
#define TX_ERROR_STATUS_TEA_ON_QUEUE_1		(0x00001000)
#define TX_ERROR_STATUS_RER_ON_QUEUE_1		(0x00002000)
#define TX_ERROR_STATUS_TER_ON_QUEUE_1		(0x00004000)
#define TX_ERROR_STATUS_DER_ON_QUEUE_1		(0x00008000)
#define TX_ERROR_STATUS_QUEUE_2_ERROR_RESPONSE	(0x000f0000)
#define TX_ERROR_STATUS_TEA_ON_QUEUE_2		(0x00100000)
#define TX_ERROR_STATUS_RER_ON_QUEUE_2		(0x00200000)
#define TX_ERROR_STATUS_TER_ON_QUEUE_2		(0x00400000)
#define TX_ERROR_STATUS_DER_ON_QUEUE_2		(0x00800000)
#define TX_ERROR_STATUS_QUEUE_3_ERROR_RESPONSE	(0x0f000000)
#define TX_ERROR_STATUS_TEA_ON_QUEUE_3		(0x10000000)
#define TX_ERROR_STATUS_RER_ON_QUEUE_3		(0x20000000)
#define TX_ERROR_STATUS_TER_ON_QUEUE_3		(0x40000000)
#define TX_ERROR_STATUS_DER_ON_QUEUE_3		(0x80000000)

#define reg_TX_QUEUE_0_CONFIG(base)		__REG32(base, 0x00000280)
#define TX_QUEUE_0_CONFIG_OCN_PORT		(0x0000003f)
#define TX_QUEUE_0_CONFIG_BSWP			(0x00000400)
#define TX_QUEUE_0_CONFIG_WSWP			(0x00000800)
#define TX_QUEUE_0_CONFIG_AM			(0x00004000)
#define TX_QUEUE_0_CONFIG_GVI			(0x00008000)
#define TX_QUEUE_0_CONFIG_EEI			(0x00010000)
#define TX_QUEUE_0_CONFIG_ELI			(0x00020000)
#define TX_QUEUE_0_CONFIG_ENI			(0x00040000)
#define TX_QUEUE_0_CONFIG_ESI			(0x00080000)
#define TX_QUEUE_0_CONFIG_EDI			(0x00100000)

#define reg_TX_QUEUE_0_BUF_CONFIG(base)		__REG32(base, 0x00000284)
#define TX_QUEUE_0_BUF_CONFIG_OCN_PORT		(0x0000003f)
#define TX_QUEUE_0_BUF_CONFIG_BURST		(0x00000300)
#define TX_QUEUE_0_BUF_CONFIG_BSWP		(0x00000400)
#define TX_QUEUE_0_BUF_CONFIG_WSWP		(0x00000800)

#define OCN_PORT_HLP			0	/* HLP Interface */
#define OCN_PORT_PCI_X			1	/* PCI-X Interface */
#define OCN_PORT_PROCESSOR_MASTER	2	/* Processor Interface (master) */
#define OCN_PORT_PROCESSOR_SLAVE	3	/* Processor Interface (slave) */
#define OCN_PORT_MEMORY			4	/* Memory Controller */
#define OCN_PORT_DMA			5	/* DMA Controller */
#define OCN_PORT_ETHERNET		6	/* Ethernet Controller */
#define OCN_PORT_PRINT			7	/* Print Engine Interface */

#define reg_TX_QUEUE_0_PTR_LOW(base)		__REG32(base, 0x00000288)

#define reg_TX_QUEUE_0_PTR_HIGH(base)		__REG32(base, 0x0000028c)
#define TX_QUEUE_0_PTR_HIGH_VALID		(0x80000000)

#define reg_RX_CONFIG(base)			__REG32(base, 0x00000320)
#define RX_CONFIG_DEF_Q				(0x00000003)
#define RX_CONFIG_EMF				(0x00000100)
#define RX_CONFIG_EUF				(0x00000200)
#define RX_CONFIG_BFE				(0x00000400)
#define RX_CONFIG_MFE				(0x00000800)
#define RX_CONFIG_UFE				(0x00001000)
#define RX_CONFIG_SE				(0x00002000)
#define RX_CONFIG_ABF				(0x00200000)
#define RX_CONFIG_APE				(0x00400000)
#define RX_CONFIG_CHP				(0x00800000)
#define RX_CONFIG_RST				(0x80000000)

#define reg_RX_CONTROL(base)			__REG32(base, 0x00000324)
#define GE_E0_RX_CONTROL_QUEUE_ENABLES		(0x0000000f)
#define GE_E0_RX_CONTROL_GO			(0x00008000)
#define GE_E0_RX_CONTROL_EAI			(0x20000000)
#define GE_E0_RX_CONTROL_ABT			(0x40000000)
#define GE_E0_RX_CONTROL_EII			(0x80000000)

#define reg_RX_EXTENDED_STATUS(base)		__REG32(base, 0x0000032c)
#define RX_EXTENDED_STATUS			(0x0000032c)
#define RX_EXTENDED_STATUS_EOQ			(0x0000000f)
#define RX_EXTENDED_STATUS_EOQ_0		(0x00000001)
#define RX_EXTENDED_STATUS_EOF			(0x00000f00)
#define RX_EXTENDED_STATUS_DESCRIPTOR_INTERRUPT_CONDITION	(0x000f0000)
#define RX_EXTENDED_STATUS_ERROR_FLAG				(0x0f000000)

#define reg_RX_THRESHOLDS(base)			__REG32(base, 0x00000330)

#define reg_RX_DIAGNOSTIC_ADDR(base)		__REG32(base, 0x00000370)
#define RX_DIAGNOSTIC_ADDR_INDEX		(0x0000007f)
#define RX_DIAGNOSTIC_ADDR_DFR			(0x40000000)
#define RX_DIAGNOSTIC_ADDR_AI			(0x80000000)

#define reg_RX_DIAGNOSTIC_DATA(base)		__REG32(base, 0x00000374)

#define reg_RX_QUEUE_0_CONFIG(base)		__REG32(base, 0x00000380)
#define RX_QUEUE_0_CONFIG_OCN_PORT		(0x0000003f)
#define RX_QUEUE_0_CONFIG_BSWP			(0x00000400)
#define RX_QUEUE_0_CONFIG_WSWP			(0x00000800)
#define RX_QUEUE_0_CONFIG_AM			(0x00004000)
#define RX_QUEUE_0_CONFIG_EEI			(0x00010000)
#define RX_QUEUE_0_CONFIG_ELI			(0x00020000)
#define RX_QUEUE_0_CONFIG_ENI			(0x00040000)
#define RX_QUEUE_0_CONFIG_ESI			(0x00080000)
#define RX_QUEUE_0_CONFIG_EDI			(0x00100000)

#define reg_RX_QUEUE_0_BUF_CONFIG(base)		__REG32(base, 0x00000384)
#define RX_QUEUE_0_BUF_CONFIG_OCN_PORT		(0x0000003f)
#define RX_QUEUE_0_BUF_CONFIG_BURST		(0x00000300)
#define RX_QUEUE_0_BUF_CONFIG_BSWP		(0x00000400)
#define RX_QUEUE_0_BUF_CONFIG_WSWP		(0x00000800)

#define reg_RX_QUEUE_0_PTR_LOW(base)		__REG32(base, 0x00000388)

#define reg_RX_QUEUE_0_PTR_HIGH(base)		__REG32(base, 0x0000038c)
#define RX_QUEUE_0_PTR_HIGH_VALID		(0x80000000)

/*
 *  PHY register definitions
 */
/* the first 15 PHY registers are standard. */
#define PHY_CTRL_REG		0	/* Control Register */
#define PHY_STATUS_REG		1	/* Status Regiser */
#define PHY_ID1_REG		2	/* Phy Id Reg (word 1) */
#define PHY_ID2_REG		3	/* Phy Id Reg (word 2) */
#define PHY_AN_ADV_REG		4	/* Autoneg Advertisement */
#define PHY_LP_ABILITY_REG	5	/* Link Partner Ability (Base Page) */
#define PHY_AUTONEG_EXP_REG	6	/* Autoneg Expansion Reg */
#define PHY_NEXT_PAGE_TX_REG	7	/* Next Page TX */
#define PHY_LP_NEXT_PAGE_REG	8	/* Link Partner Next Page */
#define PHY_1000T_CTRL_REG	9	/* 1000Base-T Control Reg */
#define PHY_1000T_STATUS_REG	10	/* 1000Base-T Status Reg */
#define PHY_EXT_STATUS_REG	11	/* Extended Status Reg */

/*
 * PHY Register bit masks.
 */
#define PHY_CTRL_RESET		(1 << 15)
#define PHY_CTRL_LOOPBACK	(1 << 14)
#define PHY_CTRL_SPEED0		(1 << 13)
#define PHY_CTRL_AN_EN		(1 << 12)
#define PHY_CTRL_PWR_DN		(1 << 11)
#define PHY_CTRL_ISOLATE	(1 << 10)
#define PHY_CTRL_RESTART_AN	(1 << 9)
#define PHY_CTRL_FULL_DUPLEX	(1 << 8)
#define PHY_CTRL_CT_EN		(1 << 7)
#define PHY_CTRL_SPEED1		(1 << 6)

#define PHY_STAT_100BASE_T4	(1 << 15)
#define PHY_STAT_100BASE_X_FD	(1 << 14)
#define PHY_STAT_100BASE_X_HD	(1 << 13)
#define PHY_STAT_10BASE_T_FD	(1 << 12)
#define PHY_STAT_10BASE_T_HD	(1 << 11)
#define PHY_STAT_100BASE_T2_FD	(1 << 10)
#define PHY_STAT_100BASE_T2_HD	(1 << 9)
#define PHY_STAT_EXT_STAT	(1 << 8)
#define PHY_STAT_RESERVED	(1 << 7)
#define PHY_STAT_MFPS		(1 << 6)	/* Management Frames Preamble Suppression */
#define PHY_STAT_AN_COMPLETE	(1 << 5)
#define PHY_STAT_REM_FAULT	(1 << 4)
#define PHY_STAT_AN_CAP		(1 << 3)
#define PHY_STAT_LINK_UP	(1 << 2)
#define PHY_STAT_JABBER		(1 << 1)
#define PHY_STAT_EXT_CAP	(1 << 0)

#define TBI_CONTROL_2					0x11
#define TBI_CONTROL_2_ENABLE_COMMA_DETECT		0x0001
#define TBI_CONTROL_2_ENABLE_WRAP			0x0002
#define TBI_CONTROL_2_G_MII_MODE			0x0010
#define TBI_CONTROL_2_RECEIVE_CLOCK_SELECT		0x0020
#define TBI_CONTROL_2_AUTO_NEGOTIATION_SENSE		0x0100
#define TBI_CONTROL_2_DISABLE_TRANSMIT_RUNNING_DISPARITY	0x1000
#define TBI_CONTROL_2_DISABLE_RECEIVE_RUNNING_DISPARITY		0x2000
#define TBI_CONTROL_2_SHORTCUT_LINK_TIMER			0x4000
#define TBI_CONTROL_2_SOFT_RESET				0x8000

/* marvel specific */
#define MV1111_EXT_CTRL1_REG	16	/* PHY Specific Control Reg */
#define MV1111_SPEC_STAT_REG	17	/* PHY Specific Status Reg */
#define MV1111_EXT_CTRL2_REG	20	/* Extended PHY Specific Control Reg */

/*
 * MARVELL 88E1111 PHY register bit masks
 */
/* PHY Specific Status Register (MV1111_EXT_CTRL1_REG) */

#define SPEC_STAT_SPEED_MASK	(3 << 14)
#define SPEC_STAT_FULL_DUP	(1 << 13)
#define SPEC_STAT_PAGE_RCVD	(1 << 12)
#define SPEC_STAT_RESOLVED	(1 << 11)	/* Speed and Duplex Resolved */
#define SPEC_STAT_LINK_UP	(1 << 10)
#define SPEC_STAT_CABLE_LEN_MASK	(7 << 7)/* Cable Length (100/1000 modes only) */
#define SPEC_STAT_MDIX		(1 << 6)
#define SPEC_STAT_POLARITY	(1 << 1)
#define SPEC_STAT_JABBER	(1 << 0)

#define SPEED_1000		(2 << 14)
#define SPEED_100		(1 << 14)
#define SPEED_10		(0 << 14)

#define TBI_ADDR	0x1E	/* Ten Bit Interface address */

/* negotiated link parameters */
#define LINK_SPEED_UNKNOWN	0
#define LINK_SPEED_10		1
#define LINK_SPEED_100		2
#define LINK_SPEED_1000		3

#define LINK_DUPLEX_UNKNOWN	0
#define LINK_DUPLEX_HALF	1
#define LINK_DUPLEX_FULL	2

static unsigned int phy_address[] = { 8, 9 };

#define vuint32 volatile u32

/* TX/RX buffer descriptors. MUST be cache line aligned in memory. (32 byte)
 * This structure is accessed by the ethernet DMA engine which means it
 * MUST be in LITTLE ENDIAN format */
struct dma_descriptor {
	vuint32 start_addr0;	/* buffer address, least significant bytes. */
	vuint32 start_addr1;	/* buffer address, most significant bytes. */
	vuint32 next_descr_addr0;/* next descriptor address, least significant bytes.  Must be 64-bit aligned. */
	vuint32 next_descr_addr1;/* next descriptor address, most significant bytes. */
	vuint32 vlan_byte_count;/* VLAN tag(top 2 bytes) and byte countt (bottom 2 bytes). */
	vuint32 config_status;	/* Configuration/Status. */
	vuint32 reserved1;	/* reserved to make the descriptor cache line aligned. */
	vuint32 reserved2;	/* reserved to make the descriptor cache line aligned. */
};

/* last next descriptor address flag */
#define DMA_DESCR_LAST		(1 << 31)

/* TX DMA descriptor config status bits */
#define DMA_DESCR_TX_EOF	(1 <<  0)	/* end of frame */
#define DMA_DESCR_TX_SOF	(1 <<  1)	/* start of frame */
#define DMA_DESCR_TX_PFVLAN	(1 <<  2)
#define DMA_DESCR_TX_HUGE	(1 <<  3)
#define DMA_DESCR_TX_PAD	(1 <<  4)
#define DMA_DESCR_TX_CRC	(1 <<  5)
#define DMA_DESCR_TX_DESCR_INT	(1 << 14)
#define DMA_DESCR_TX_RETRY_COUNT	0x000F0000
#define DMA_DESCR_TX_ONE_COLLISION	(1 << 20)
#define DMA_DESCR_TX_LATE_COLLISION	(1 << 24)
#define DMA_DESCR_TX_UNDERRUN		(1 << 25)
#define DMA_DESCR_TX_RETRY_LIMIT	(1 << 26)
#define DMA_DESCR_TX_OK			(1 << 30)
#define DMA_DESCR_TX_OWNER		(1 << 31)

/* RX DMA descriptor status bits */
#define DMA_DESCR_RX_EOF		(1 <<  0)
#define DMA_DESCR_RX_SOF		(1 <<  1)
#define DMA_DESCR_RX_VTF		(1 <<  2)
#define DMA_DESCR_RX_FRAME_IS_TYPE	(1 <<  3)
#define DMA_DESCR_RX_SHORT_FRAME	(1 <<  4)
#define DMA_DESCR_RX_HASH_MATCH		(1 <<  7)
#define DMA_DESCR_RX_BAD_FRAME		(1 <<  8)
#define DMA_DESCR_RX_OVERRUN		(1 <<  9)
#define DMA_DESCR_RX_MAX_FRAME_LEN	(1 << 11)
#define DMA_DESCR_RX_CRC_ERROR		(1 << 12)
#define DMA_DESCR_RX_DESCR_INT		(1 << 13)
#define DMA_DESCR_RX_OWNER		(1 << 15)

#define RX_BUFFER_SIZE	PKTSIZE
#define NUM_RX_DESC	PKTBUFSRX

static struct dma_descriptor tx_descriptor __attribute__ ((aligned(32)));

static struct dma_descriptor rx_descr_array[NUM_RX_DESC]
	__attribute__ ((aligned(32)));

static struct dma_descriptor *rx_descr_current;

static int tsi108_eth_probe (struct eth_device *dev, bd_t * bis);
static int tsi108_eth_send (struct eth_device *dev,
			   volatile void *packet, int length);
static int tsi108_eth_recv (struct eth_device *dev);
static void tsi108_eth_halt (struct eth_device *dev);
static unsigned int read_phy (unsigned int base,
			     unsigned int phy_addr, unsigned int phy_reg);
static void write_phy (unsigned int base,
		      unsigned int phy_addr,
		      unsigned int phy_reg, unsigned int phy_data);

#if TSI108_ETH_DEBUG > 100
/*
 * print phy debug infomation
 */
static void dump_phy_regs (unsigned int phy_addr)
{
	int i;

	printf ("PHY %d registers\n", phy_addr);
	for (i = 0; i <= 30; i++) {
		printf ("%2d  0x%04x\n", i, read_phy (ETH_BASE, phy_addr, i));
	}
	printf ("\n");

}
#else
#define dump_phy_regs(base) do{}while(0)
#endif

#if TSI108_ETH_DEBUG > 100
/*
 * print debug infomation
 */
static void tx_diag_regs (unsigned int base)
{
	int i;
	unsigned long dummy;

	printf ("TX diagnostics registers\n");
	reg_TX_DIAGNOSTIC_ADDR(base) = 0x00 | TX_DIAGNOSTIC_ADDR_AI;
	udelay (1000);
	dummy = reg_TX_DIAGNOSTIC_DATA(base);
	for (i = 0x00; i <= 0x05; i++) {
		udelay (1000);
		printf ("0x%02x  0x%08x\n", i, reg_TX_DIAGNOSTIC_DATA(base));
	}
	reg_TX_DIAGNOSTIC_ADDR(base) = 0x40 | TX_DIAGNOSTIC_ADDR_AI;
	udelay (1000);
	dummy = reg_TX_DIAGNOSTIC_DATA(base);
	for (i = 0x40; i <= 0x47; i++) {
		udelay (1000);
		printf ("0x%02x  0x%08x\n", i, reg_TX_DIAGNOSTIC_DATA(base));
	}
	printf ("\n");

}
#else
#define tx_diag_regs(base) do{}while(0)
#endif

#if TSI108_ETH_DEBUG > 100
/*
 * print debug infomation
 */
static void rx_diag_regs (unsigned int base)
{
	int i;
	unsigned long dummy;

	printf ("RX diagnostics registers\n");
	reg_RX_DIAGNOSTIC_ADDR(base) = 0x00 | RX_DIAGNOSTIC_ADDR_AI;
	udelay (1000);
	dummy = reg_RX_DIAGNOSTIC_DATA(base);
	for (i = 0x00; i <= 0x05; i++) {
		udelay (1000);
		printf ("0x%02x  0x%08x\n", i, reg_RX_DIAGNOSTIC_DATA(base));
	}
	reg_RX_DIAGNOSTIC_ADDR(base) = 0x40 | RX_DIAGNOSTIC_ADDR_AI;
	udelay (1000);
	dummy = reg_RX_DIAGNOSTIC_DATA(base);
	for (i = 0x08; i <= 0x0a; i++) {
		udelay (1000);
		printf ("0x%02x  0x%08x\n", i, reg_RX_DIAGNOSTIC_DATA(base));
	}
	printf ("\n");

}
#else
#define rx_diag_regs(base) do{}while(0)
#endif

#if TSI108_ETH_DEBUG > 100
/*
 * print debug infomation
 */
static void debug_mii_regs (unsigned int base)
{
	printf ("MII_MGMT_CONFIG     0x%08x\n", reg_MII_MGMT_CONFIG(base));
	printf ("MII_MGMT_COMMAND    0x%08x\n", reg_MII_MGMT_COMMAND(base));
	printf ("MII_MGMT_ADDRESS    0x%08x\n", reg_MII_MGMT_ADDRESS(base));
	printf ("MII_MGMT_CONTROL    0x%08x\n", reg_MII_MGMT_CONTROL(base));
	printf ("MII_MGMT_STATUS     0x%08x\n", reg_MII_MGMT_STATUS(base));
	printf ("MII_MGMT_INDICATORS 0x%08x\n", reg_MII_MGMT_INDICATORS(base));
	printf ("\n");

}
#else
#define debug_mii_regs(base) do{}while(0)
#endif

/*
 * Wait until the phy bus is non-busy
 */
static void phy_wait (unsigned int base, unsigned int condition)
{
	int timeout;

	timeout = 0;
	while (reg_MII_MGMT_INDICATORS(base) & condition) {
		udelay (10);
		if (++timeout > 10000) {
			printf ("ERROR: timeout waiting for phy bus (%d)\n",
			       condition);
			break;
		}
	}
}

/*
 * read phy register
 */
static unsigned int read_phy (unsigned int base,
			     unsigned int phy_addr, unsigned int phy_reg)
{
	unsigned int value;

	phy_wait (base, MII_MGMT_INDICATORS_BUSY);

	reg_MII_MGMT_ADDRESS(base) = (phy_addr << 8) | phy_reg;

	/* Ensure that the Read Cycle bit is cleared prior to next read cycle */
	reg_MII_MGMT_COMMAND(base) = 0;

	/* start the read */
	reg_MII_MGMT_COMMAND(base) = MII_MGMT_COMMAND_READ_CYCLE;

	/* wait for the read to complete */
	phy_wait (base,
		 MII_MGMT_INDICATORS_NOT_VALID | MII_MGMT_INDICATORS_BUSY);

	value = reg_MII_MGMT_STATUS(base);

	reg_MII_MGMT_COMMAND(base) = 0;

	return value;
}

/*
 * write phy register
 */
static void write_phy (unsigned int base,
		      unsigned int phy_addr,
		      unsigned int phy_reg, unsigned int phy_data)
{
	phy_wait (base, MII_MGMT_INDICATORS_BUSY);

	reg_MII_MGMT_ADDRESS(base) = (phy_addr << 8) | phy_reg;

	/* Ensure that the Read Cycle bit is cleared prior to next cycle */
	reg_MII_MGMT_COMMAND(base) = 0;

	/* start the write */
	reg_MII_MGMT_CONTROL(base) = phy_data;
}

/*
 * configure the marvell 88e1111 phy
 */
static int marvell_88e_phy_config (struct eth_device *dev, int *speed,
				  int *duplex)
{
	unsigned long base;
	unsigned long phy_addr;
	unsigned int phy_status;
	unsigned int phy_spec_status;
	int timeout;
	int phy_speed;
	int phy_duplex;
	unsigned int value;

	phy_speed = LINK_SPEED_UNKNOWN;
	phy_duplex = LINK_DUPLEX_UNKNOWN;

	base = dev->iobase;
	phy_addr = (unsigned long)dev->priv;

	/* Take the PHY out of reset. */
	write_phy (ETH_BASE, phy_addr, PHY_CTRL_REG, PHY_CTRL_RESET);

	/* Wait for the reset process to complete. */
	udelay (10);
	timeout = 0;
	while ((phy_status =
		read_phy (ETH_BASE, phy_addr, PHY_CTRL_REG)) & PHY_CTRL_RESET) {
		udelay (10);
		if (++timeout > 10000) {
			printf ("ERROR: timeout waiting for phy reset\n");
			break;
		}
	}

	/* TBI Configuration. */
	write_phy (base, TBI_ADDR, TBI_CONTROL_2, TBI_CONTROL_2_G_MII_MODE |
		  TBI_CONTROL_2_RECEIVE_CLOCK_SELECT);
	/* Wait for the link to be established. */
	timeout = 0;
	do {
		udelay (20000);
		phy_status = read_phy (ETH_BASE, phy_addr, PHY_STATUS_REG);
		if (++timeout > 100) {
			debug_lev(1, "ERROR: unable to establish link!!!\n");
			break;
		}
	} while ((phy_status & PHY_STAT_LINK_UP) == 0);

	if ((phy_status & PHY_STAT_LINK_UP) == 0)
		return 0;

	value = 0;
	phy_spec_status = read_phy (ETH_BASE, phy_addr, MV1111_SPEC_STAT_REG);
	if (phy_spec_status & SPEC_STAT_RESOLVED) {
		switch (phy_spec_status & SPEC_STAT_SPEED_MASK) {
		case SPEED_1000:
			phy_speed = LINK_SPEED_1000;
			value |= PHY_CTRL_SPEED1;
			break;
		case SPEED_100:
			phy_speed = LINK_SPEED_100;
			value |= PHY_CTRL_SPEED0;
			break;
		case SPEED_10:
			phy_speed = LINK_SPEED_10;
			break;
		}
		if (phy_spec_status & SPEC_STAT_FULL_DUP) {
			phy_duplex = LINK_DUPLEX_FULL;
			value |= PHY_CTRL_FULL_DUPLEX;
		} else
			phy_duplex = LINK_DUPLEX_HALF;
	}
	/* set TBI speed */
	write_phy (base, TBI_ADDR, PHY_CTRL_REG, value);
	write_phy (base, TBI_ADDR, PHY_AN_ADV_REG, 0x0060);

#if TSI108_ETH_DEBUG > 0
	printf ("%s link is up", dev->name);
	phy_spec_status = read_phy (ETH_BASE, phy_addr, MV1111_SPEC_STAT_REG);
	if (phy_spec_status & SPEC_STAT_RESOLVED) {
		switch (phy_speed) {
		case LINK_SPEED_1000:
			printf (", 1000 Mbps");
			break;
		case LINK_SPEED_100:
			printf (", 100 Mbps");
			break;
		case LINK_SPEED_10:
			printf (", 10 Mbps");
			break;
		}
		if (phy_duplex == LINK_DUPLEX_FULL)
			printf (", Full duplex");
		else
			printf (", Half duplex");
	}
	printf ("\n");
#endif

	dump_phy_regs (TBI_ADDR);
	if (speed)
		*speed = phy_speed;
	if (duplex)
		*duplex = phy_duplex;

	return 1;
}

/*
 * External interface
 *
 * register the tsi108 ethernet controllers with the multi-ethernet system
 */
int tsi108_eth_initialize (bd_t * bis)
{
	struct eth_device *dev;
	int index;

	for (index = 0; index < CONFIG_TSI108_ETH_NUM_PORTS; index++) {
		dev = (struct eth_device *)malloc(sizeof(struct eth_device));

		sprintf (dev->name, "TSI108_eth%d", index);

		dev->iobase = ETH_BASE + (index * ETH_PORT_OFFSET);
		dev->priv = (void *)(phy_address[index]);
		dev->init = tsi108_eth_probe;
		dev->halt = tsi108_eth_halt;
		dev->send = tsi108_eth_send;
		dev->recv = tsi108_eth_recv;

		eth_register(dev);
	}
	return index;
}

/*
 * probe for and initialize a single ethernet interface
 */
static int tsi108_eth_probe (struct eth_device *dev, bd_t * bis)
{
	unsigned long base;
	unsigned long value;
	int index;
	struct dma_descriptor *tx_descr;
	struct dma_descriptor *rx_descr;
	int speed;
	int duplex;

	base = dev->iobase;

	reg_PORT_CONTROL(base) = PORT_CONTROL_STE | PORT_CONTROL_BPT;

	/* Bring DMA/FIFO out of reset. */
	reg_TX_CONFIG(base) = 0x00000000;
	reg_RX_CONFIG(base) = 0x00000000;

	reg_TX_THRESHOLDS(base) = (192 << 16) | 192;
	reg_RX_THRESHOLDS(base) = (192 << 16) | 112;

	/* Bring MAC out of reset. */
	reg_MAC_CONFIG_1(base) = 0x00000000;

	/* DMA MAC configuration. */
	reg_MAC_CONFIG_1(base) =
	    MAC_CONFIG_1_RX_ENABLE | MAC_CONFIG_1_TX_ENABLE;

	reg_MII_MGMT_CONFIG(base) = MII_MGMT_CONFIG_NO_PREAMBLE;
	reg_MAXIMUM_FRAME_LENGTH(base) = RX_BUFFER_SIZE;

	/* Note: Early tsi108 manual did not have correct byte order
	 * for the station address.*/
	reg_STATION_ADDRESS_1(base) = (dev->enetaddr[5] << 24) |
	    (dev->enetaddr[4] << 16) |
	    (dev->enetaddr[3] << 8) | (dev->enetaddr[2] << 0);

	reg_STATION_ADDRESS_2(base) = (dev->enetaddr[1] << 24) |
	    (dev->enetaddr[0] << 16);

	if (marvell_88e_phy_config(dev, &speed, &duplex) == 0)
		return 0;

	value =
	    MAC_CONFIG_2_PREAMBLE_LENGTH(7) | MAC_CONFIG_2_PAD_CRC |
	    MAC_CONFIG_2_CRC_ENABLE;
	if (speed == LINK_SPEED_1000)
		value |= MAC_CONFIG_2_INTERFACE_MODE(INTERFACE_MODE_BYTE);
	else {
		value |= MAC_CONFIG_2_INTERFACE_MODE(INTERFACE_MODE_NIBBLE);
		reg_PORT_CONTROL(base) |= PORT_CONTROL_SPD;
	}
	if (duplex == LINK_DUPLEX_FULL) {
		value |= MAC_CONFIG_2_FULL_DUPLEX;
		reg_PORT_CONTROL(base) &= ~PORT_CONTROL_BPT;
	} else
		reg_PORT_CONTROL(base) |= PORT_CONTROL_BPT;
	reg_MAC_CONFIG_2(base) = value;

	reg_RX_CONFIG(base) = RX_CONFIG_SE;
	reg_RX_QUEUE_0_CONFIG(base) = OCN_PORT_MEMORY;
	reg_RX_QUEUE_0_BUF_CONFIG(base) = OCN_PORT_MEMORY;

	/* initialize the RX DMA descriptors */
	rx_descr = &rx_descr_array[0];
	rx_descr_current = rx_descr;
	for (index = 0; index < NUM_RX_DESC; index++) {
		/* make sure the receive buffers are not in cache */
		invalidate_dcache_range((unsigned long)NetRxPackets[index],
					(unsigned long)NetRxPackets[index] +
					RX_BUFFER_SIZE);
		rx_descr->start_addr0 =
		    cpu_to_le32((vuint32) NetRxPackets[index]);
		rx_descr->start_addr1 = 0;
		rx_descr->next_descr_addr0 =
		    cpu_to_le32((vuint32) (rx_descr + 1));
		rx_descr->next_descr_addr1 = 0;
		rx_descr->vlan_byte_count = 0;
		rx_descr->config_status = cpu_to_le32((RX_BUFFER_SIZE << 16) |
						      DMA_DESCR_RX_OWNER);
		rx_descr++;
	}
	rx_descr--;
	rx_descr->next_descr_addr0 = 0;
	rx_descr->next_descr_addr1 = cpu_to_le32(DMA_DESCR_LAST);
	/* Push the descriptors to RAM so the ethernet DMA can see them */
	invalidate_dcache_range((unsigned long)rx_descr_array,
				(unsigned long)rx_descr_array +
				sizeof(rx_descr_array));

	/* enable RX queue */
	reg_RX_CONTROL(base) = TX_CONTROL_GO | 0x01;
	reg_RX_QUEUE_0_PTR_LOW(base) = (u32) rx_descr_current;
	/* enable receive DMA */
	reg_RX_QUEUE_0_PTR_HIGH(base) = RX_QUEUE_0_PTR_HIGH_VALID;

	reg_TX_QUEUE_0_CONFIG(base) = OCN_PORT_MEMORY;
	reg_TX_QUEUE_0_BUF_CONFIG(base) = OCN_PORT_MEMORY;

	/* initialize the TX DMA descriptor */
	tx_descr = &tx_descriptor;

	tx_descr->start_addr0 = 0;
	tx_descr->start_addr1 = 0;
	tx_descr->next_descr_addr0 = 0;
	tx_descr->next_descr_addr1 = cpu_to_le32(DMA_DESCR_LAST);
	tx_descr->vlan_byte_count = 0;
	tx_descr->config_status = cpu_to_le32(DMA_DESCR_TX_OK |
					      DMA_DESCR_TX_SOF |
					      DMA_DESCR_TX_EOF);
	/* enable TX queue */
	reg_TX_CONTROL(base) = TX_CONTROL_GO | 0x01;

	return 1;
}

/*
 * send a packet
 */
static int tsi108_eth_send (struct eth_device *dev,
			   volatile void *packet, int length)
{
	unsigned long base;
	int timeout;
	struct dma_descriptor *tx_descr;
	unsigned long status;

	base = dev->iobase;
	tx_descr = &tx_descriptor;

	/* Wait until the last packet has been transmitted. */
	timeout = 0;
	do {
		/* make sure we see the changes made by the DMA engine */
		invalidate_dcache_range((unsigned long)tx_descr,
					(unsigned long)tx_descr +
					sizeof(struct dma_descriptor));

		if (timeout != 0)
			udelay (15);
		if (++timeout > 10000) {
			tx_diag_regs(base);
			debug_lev(1,
				  "ERROR: timeout waiting for last transmit packet to be sent\n");
			return 0;
		}
	} while (tx_descr->config_status & cpu_to_le32(DMA_DESCR_TX_OWNER));

	status = le32_to_cpu(tx_descr->config_status);
	if ((status & DMA_DESCR_TX_OK) == 0) {
#ifdef TX_PRINT_ERRORS
		printf ("TX packet error: 0x%08x\n    %s%s%s%s\n", status,
		       status & DMA_DESCR_TX_OK ? "tx error, " : "",
		       status & DMA_DESCR_TX_RETRY_LIMIT ?
		       "retry limit reached, " : "",
		       status & DMA_DESCR_TX_UNDERRUN ? "underrun, " : "",
		       status & DMA_DESCR_TX_LATE_COLLISION ? "late collision, "
		       : "");
#endif
	}

	debug_lev (9, "sending packet %d\n", length);
	tx_descr->start_addr0 = cpu_to_le32((vuint32) packet);
	tx_descr->start_addr1 = 0;
	tx_descr->next_descr_addr0 = 0;
	tx_descr->next_descr_addr1 = cpu_to_le32(DMA_DESCR_LAST);
	tx_descr->vlan_byte_count = cpu_to_le32(length);
	tx_descr->config_status = cpu_to_le32(DMA_DESCR_TX_OWNER |
					      DMA_DESCR_TX_CRC |
					      DMA_DESCR_TX_PAD |
					      DMA_DESCR_TX_SOF |
					      DMA_DESCR_TX_EOF);

	invalidate_dcache_range((unsigned long)tx_descr,
				(unsigned long)tx_descr +
				sizeof(struct dma_descriptor));

	invalidate_dcache_range((unsigned long)packet,
				(unsigned long)packet + length);

	reg_TX_QUEUE_0_PTR_LOW(base) = (u32) tx_descr;
	reg_TX_QUEUE_0_PTR_HIGH(base) = TX_QUEUE_0_PTR_HIGH_VALID;

	return length;
}

/*
 * Check for received packets and send them up the protocal stack
 */
static int tsi108_eth_recv (struct eth_device *dev)
{
	struct dma_descriptor *rx_descr;
	unsigned long base;
	int length = 0;
	unsigned long status;
	volatile uchar *buffer;

	base = dev->iobase;

	/* make sure we see the changes made by the DMA engine */
	invalidate_dcache_range ((unsigned long)rx_descr_array,
				(unsigned long)rx_descr_array +
				sizeof(rx_descr_array));

	/* process all of the received packets */
	rx_descr = rx_descr_current;
	while ((rx_descr->config_status & cpu_to_le32(DMA_DESCR_RX_OWNER)) == 0) {
		/* check for error */
		status = le32_to_cpu(rx_descr->config_status);
		if (status & DMA_DESCR_RX_BAD_FRAME) {
#ifdef RX_PRINT_ERRORS
			printf ("RX packet error: 0x%08x\n    %s%s%s%s%s%s\n",
			       status,
			       status & DMA_DESCR_RX_FRAME_IS_TYPE ? "too big, "
			       : "",
			       status & DMA_DESCR_RX_SHORT_FRAME ? "too short, "
			       : "",
			       status & DMA_DESCR_RX_BAD_FRAME ? "bad frame, " :
			       "",
			       status & DMA_DESCR_RX_OVERRUN ? "overrun, " : "",
			       status & DMA_DESCR_RX_MAX_FRAME_LEN ?
			       "max length, " : "",
			       status & DMA_DESCR_RX_CRC_ERROR ? "CRC error, " :
			       "");
#endif
		} else {
			length =
			    le32_to_cpu(rx_descr->vlan_byte_count) & 0xFFFF;

			/*** process packet ***/
			buffer =
			    (volatile uchar
			     *)(le32_to_cpu (rx_descr->start_addr0));
			NetReceive (buffer, length);

			invalidate_dcache_range ((unsigned long)buffer,
						(unsigned long)buffer +
						RX_BUFFER_SIZE);
		}
		/* Give this buffer back to the DMA engine */
		rx_descr->vlan_byte_count = 0;
		rx_descr->config_status = cpu_to_le32 ((RX_BUFFER_SIZE << 16) |
						      DMA_DESCR_RX_OWNER);
		/* move descriptor pointer forward */
		rx_descr =
		    (struct dma_descriptor
		     *)(le32_to_cpu (rx_descr->next_descr_addr0));
		if (rx_descr == 0)
			rx_descr = &rx_descr_array[0];
	}
	/* remember where we are for next time */
	rx_descr_current = rx_descr;

	/* If the DMA engine has reached the end of the queue
	 * start over at the begining */
	if (reg_RX_EXTENDED_STATUS(base) & RX_EXTENDED_STATUS_EOQ_0) {

		reg_RX_EXTENDED_STATUS(base) = RX_EXTENDED_STATUS_EOQ_0;
		reg_RX_QUEUE_0_PTR_LOW(base) = (u32) & rx_descr_array[0];
		reg_RX_QUEUE_0_PTR_HIGH(base) = RX_QUEUE_0_PTR_HIGH_VALID;
	}

	return length;
}

/*
 * disable an ethernet interface
 */
static void tsi108_eth_halt (struct eth_device *dev)
{
	unsigned long base;

	base = dev->iobase;

	/* Put DMA/FIFO into reset state. */
	reg_TX_CONFIG(base) = TX_CONFIG_RST;
	reg_RX_CONFIG(base) = RX_CONFIG_RST;

	/* Put MAC into reset state. */
	reg_MAC_CONFIG_1(base) = MAC_CONFIG_1_SOFT_RESET;
}

#endif
