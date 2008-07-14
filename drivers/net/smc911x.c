/*
 * SMSC LAN9[12]1[567] Network driver
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
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
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <miiphy.h>

#if defined (CONFIG_DRIVER_SMC911X_32_BIT) && \
	defined (CONFIG_DRIVER_SMC911X_16_BIT)
#error "SMC911X: Only one of CONFIG_DRIVER_SMC911X_32_BIT and \
	CONFIG_DRIVER_SMC911X_16_BIT shall be set"
#endif

#if defined (CONFIG_DRIVER_SMC911X_32_BIT)
static inline u32 reg_read(u32 addr)
{
	return *(volatile u32*)addr;
}
static inline void reg_write(u32 addr, u32 val)
{
	*(volatile u32*)addr = val;
}
#elif defined (CONFIG_DRIVER_SMC911X_16_BIT)
static inline u32 reg_read(u32 addr)
{
	volatile u16 *addr_16 = (u16 *)addr;
	return ((*addr_16 & 0x0000ffff) | (*(addr_16 + 1) << 16));
}
static inline void reg_write(u32 addr, u32 val)
{
	*(volatile u16*)addr = (u16)val;
	*(volatile u16*)(addr + 2) = (u16)(val >> 16);
}
#else
#error "SMC911X: undefined bus width"
#endif /* CONFIG_DRIVER_SMC911X_16_BIT */

#define mdelay(n)       udelay((n)*1000)

/* Below are the register offsets and bit definitions
 * of the Lan911x memory space
 */
#define RX_DATA_FIFO		 (CONFIG_DRIVER_SMC911X_BASE + 0x00)

#define TX_DATA_FIFO		 (CONFIG_DRIVER_SMC911X_BASE + 0x20)
#define	TX_CMD_A_INT_ON_COMP			0x80000000
#define	TX_CMD_A_INT_BUF_END_ALGN		0x03000000
#define	TX_CMD_A_INT_4_BYTE_ALGN		0x00000000
#define	TX_CMD_A_INT_16_BYTE_ALGN		0x01000000
#define	TX_CMD_A_INT_32_BYTE_ALGN		0x02000000
#define	TX_CMD_A_INT_DATA_OFFSET		0x001F0000
#define	TX_CMD_A_INT_FIRST_SEG			0x00002000
#define	TX_CMD_A_INT_LAST_SEG			0x00001000
#define	TX_CMD_A_BUF_SIZE			0x000007FF
#define	TX_CMD_B_PKT_TAG			0xFFFF0000
#define	TX_CMD_B_ADD_CRC_DISABLE		0x00002000
#define	TX_CMD_B_DISABLE_PADDING		0x00001000
#define	TX_CMD_B_PKT_BYTE_LENGTH		0x000007FF

#define RX_STATUS_FIFO		(CONFIG_DRIVER_SMC911X_BASE + 0x40)
#define	RX_STS_PKT_LEN				0x3FFF0000
#define	RX_STS_ES				0x00008000
#define	RX_STS_BCST				0x00002000
#define	RX_STS_LEN_ERR				0x00001000
#define	RX_STS_RUNT_ERR				0x00000800
#define	RX_STS_MCAST				0x00000400
#define	RX_STS_TOO_LONG				0x00000080
#define	RX_STS_COLL				0x00000040
#define	RX_STS_ETH_TYPE				0x00000020
#define	RX_STS_WDOG_TMT				0x00000010
#define	RX_STS_MII_ERR				0x00000008
#define	RX_STS_DRIBBLING			0x00000004
#define	RX_STS_CRC_ERR				0x00000002
#define RX_STATUS_FIFO_PEEK	(CONFIG_DRIVER_SMC911X_BASE + 0x44)
#define TX_STATUS_FIFO		(CONFIG_DRIVER_SMC911X_BASE + 0x48)
#define	TX_STS_TAG				0xFFFF0000
#define	TX_STS_ES				0x00008000
#define	TX_STS_LOC				0x00000800
#define	TX_STS_NO_CARR				0x00000400
#define	TX_STS_LATE_COLL			0x00000200
#define	TX_STS_MANY_COLL			0x00000100
#define	TX_STS_COLL_CNT				0x00000078
#define	TX_STS_MANY_DEFER			0x00000004
#define	TX_STS_UNDERRUN				0x00000002
#define	TX_STS_DEFERRED				0x00000001
#define TX_STATUS_FIFO_PEEK	(CONFIG_DRIVER_SMC911X_BASE + 0x4C)
#define ID_REV			(CONFIG_DRIVER_SMC911X_BASE + 0x50)
#define	ID_REV_CHIP_ID				0xFFFF0000  /* RO */
#define	ID_REV_REV_ID				0x0000FFFF  /* RO */

#define INT_CFG			(CONFIG_DRIVER_SMC911X_BASE + 0x54)
#define	INT_CFG_INT_DEAS			0xFF000000  /* R/W */
#define	INT_CFG_INT_DEAS_CLR			0x00004000
#define	INT_CFG_INT_DEAS_STS			0x00002000
#define	INT_CFG_IRQ_INT				0x00001000  /* RO */
#define	INT_CFG_IRQ_EN				0x00000100  /* R/W */
#define	INT_CFG_IRQ_POL				0x00000010  /* R/W Not Affected by SW Reset */
#define	INT_CFG_IRQ_TYPE			0x00000001  /* R/W Not Affected by SW Reset */

#define INT_STS			(CONFIG_DRIVER_SMC911X_BASE + 0x58)
#define	INT_STS_SW_INT				0x80000000  /* R/WC */
#define	INT_STS_TXSTOP_INT			0x02000000  /* R/WC */
#define	INT_STS_RXSTOP_INT			0x01000000  /* R/WC */
#define	INT_STS_RXDFH_INT			0x00800000  /* R/WC */
#define	INT_STS_RXDF_INT			0x00400000  /* R/WC */
#define	INT_STS_TX_IOC				0x00200000  /* R/WC */
#define	INT_STS_RXD_INT				0x00100000  /* R/WC */
#define	INT_STS_GPT_INT				0x00080000  /* R/WC */
#define	INT_STS_PHY_INT				0x00040000  /* RO */
#define	INT_STS_PME_INT				0x00020000  /* R/WC */
#define	INT_STS_TXSO				0x00010000  /* R/WC */
#define	INT_STS_RWT				0x00008000  /* R/WC */
#define	INT_STS_RXE				0x00004000  /* R/WC */
#define	INT_STS_TXE				0x00002000  /* R/WC */
/*#define	INT_STS_ERX		0x00001000*/  /* R/WC */
#define	INT_STS_TDFU				0x00000800  /* R/WC */
#define	INT_STS_TDFO				0x00000400  /* R/WC */
#define	INT_STS_TDFA				0x00000200  /* R/WC */
#define	INT_STS_TSFF				0x00000100  /* R/WC */
#define	INT_STS_TSFL				0x00000080  /* R/WC */
/*#define	INT_STS_RXDF		0x00000040*/  /* R/WC */
#define	INT_STS_RDFO				0x00000040  /* R/WC */
#define	INT_STS_RDFL				0x00000020  /* R/WC */
#define	INT_STS_RSFF				0x00000010  /* R/WC */
#define	INT_STS_RSFL				0x00000008  /* R/WC */
#define	INT_STS_GPIO2_INT			0x00000004  /* R/WC */
#define	INT_STS_GPIO1_INT			0x00000002  /* R/WC */
#define	INT_STS_GPIO0_INT			0x00000001  /* R/WC */
#define INT_EN			(CONFIG_DRIVER_SMC911X_BASE + 0x5C)
#define	INT_EN_SW_INT_EN			0x80000000  /* R/W */
#define	INT_EN_TXSTOP_INT_EN			0x02000000  /* R/W */
#define	INT_EN_RXSTOP_INT_EN			0x01000000  /* R/W */
#define	INT_EN_RXDFH_INT_EN			0x00800000  /* R/W */
/*#define	INT_EN_RXDF_INT_EN		0x00400000*/  /* R/W */
#define	INT_EN_TIOC_INT_EN			0x00200000  /* R/W */
#define	INT_EN_RXD_INT_EN			0x00100000  /* R/W */
#define	INT_EN_GPT_INT_EN			0x00080000  /* R/W */
#define	INT_EN_PHY_INT_EN			0x00040000  /* R/W */
#define	INT_EN_PME_INT_EN			0x00020000  /* R/W */
#define	INT_EN_TXSO_EN				0x00010000  /* R/W */
#define	INT_EN_RWT_EN				0x00008000  /* R/W */
#define	INT_EN_RXE_EN				0x00004000  /* R/W */
#define	INT_EN_TXE_EN				0x00002000  /* R/W */
/*#define	INT_EN_ERX_EN			0x00001000*/  /* R/W */
#define	INT_EN_TDFU_EN				0x00000800  /* R/W */
#define	INT_EN_TDFO_EN				0x00000400  /* R/W */
#define	INT_EN_TDFA_EN				0x00000200  /* R/W */
#define	INT_EN_TSFF_EN				0x00000100  /* R/W */
#define	INT_EN_TSFL_EN				0x00000080  /* R/W */
/*#define	INT_EN_RXDF_EN			0x00000040*/  /* R/W */
#define	INT_EN_RDFO_EN				0x00000040  /* R/W */
#define	INT_EN_RDFL_EN				0x00000020  /* R/W */
#define	INT_EN_RSFF_EN				0x00000010  /* R/W */
#define	INT_EN_RSFL_EN				0x00000008  /* R/W */
#define	INT_EN_GPIO2_INT			0x00000004  /* R/W */
#define	INT_EN_GPIO1_INT			0x00000002  /* R/W */
#define	INT_EN_GPIO0_INT			0x00000001  /* R/W */

#define BYTE_TEST		(CONFIG_DRIVER_SMC911X_BASE + 0x64)
#define FIFO_INT		(CONFIG_DRIVER_SMC911X_BASE + 0x68)
#define	FIFO_INT_TX_AVAIL_LEVEL			0xFF000000  /* R/W */
#define	FIFO_INT_TX_STS_LEVEL			0x00FF0000  /* R/W */
#define	FIFO_INT_RX_AVAIL_LEVEL			0x0000FF00  /* R/W */
#define	FIFO_INT_RX_STS_LEVEL			0x000000FF  /* R/W */

#define RX_CFG			(CONFIG_DRIVER_SMC911X_BASE + 0x6C)
#define	RX_CFG_RX_END_ALGN			0xC0000000  /* R/W */
#define		RX_CFG_RX_END_ALGN4		0x00000000  /* R/W */
#define		RX_CFG_RX_END_ALGN16		0x40000000  /* R/W */
#define		RX_CFG_RX_END_ALGN32		0x80000000  /* R/W */
#define	RX_CFG_RX_DMA_CNT			0x0FFF0000  /* R/W */
#define	RX_CFG_RX_DUMP				0x00008000  /* R/W */
#define	RX_CFG_RXDOFF				0x00001F00  /* R/W */
/*#define	RX_CFG_RXBAD			0x00000001*/  /* R/W */

#define TX_CFG			(CONFIG_DRIVER_SMC911X_BASE + 0x70)
/*#define	TX_CFG_TX_DMA_LVL		0xE0000000*/	 /* R/W */
/*#define	TX_CFG_TX_DMA_CNT		0x0FFF0000*/	 /* R/W Self Clearing */
#define	TX_CFG_TXS_DUMP				0x00008000  /* Self Clearing */
#define	TX_CFG_TXD_DUMP				0x00004000  /* Self Clearing */
#define	TX_CFG_TXSAO				0x00000004  /* R/W */
#define	TX_CFG_TX_ON				0x00000002  /* R/W */
#define	TX_CFG_STOP_TX				0x00000001  /* Self Clearing */

#define HW_CFG			(CONFIG_DRIVER_SMC911X_BASE + 0x74)
#define	HW_CFG_TTM				0x00200000  /* R/W */
#define	HW_CFG_SF				0x00100000  /* R/W */
#define	HW_CFG_TX_FIF_SZ			0x000F0000  /* R/W */
#define	HW_CFG_TR				0x00003000  /* R/W */
#define	HW_CFG_PHY_CLK_SEL			0x00000060  /* R/W */
#define	HW_CFG_PHY_CLK_SEL_INT_PHY		0x00000000 /* R/W */
#define	HW_CFG_PHY_CLK_SEL_EXT_PHY		0x00000020 /* R/W */
#define	HW_CFG_PHY_CLK_SEL_CLK_DIS		0x00000040 /* R/W */
#define	HW_CFG_SMI_SEL				0x00000010  /* R/W */
#define	HW_CFG_EXT_PHY_DET			0x00000008  /* RO */
#define	HW_CFG_EXT_PHY_EN			0x00000004  /* R/W */
#define	HW_CFG_32_16_BIT_MODE			0x00000004  /* RO */
#define	HW_CFG_SRST_TO				0x00000002  /* RO */
#define	HW_CFG_SRST				0x00000001  /* Self Clearing */

#define RX_DP_CTRL		(CONFIG_DRIVER_SMC911X_BASE + 0x78)
#define	RX_DP_CTRL_RX_FFWD			0x80000000  /* R/W */
#define	RX_DP_CTRL_FFWD_BUSY			0x80000000  /* RO */

#define RX_FIFO_INF		(CONFIG_DRIVER_SMC911X_BASE + 0x7C)
#define	 RX_FIFO_INF_RXSUSED			0x00FF0000  /* RO */
#define	 RX_FIFO_INF_RXDUSED			0x0000FFFF  /* RO */

#define TX_FIFO_INF		(CONFIG_DRIVER_SMC911X_BASE + 0x80)
#define	TX_FIFO_INF_TSUSED			0x00FF0000  /* RO */
#define	TX_FIFO_INF_TDFREE			0x0000FFFF  /* RO */

#define PMT_CTRL		(CONFIG_DRIVER_SMC911X_BASE + 0x84)
#define	PMT_CTRL_PM_MODE			0x00003000  /* Self Clearing */
#define	PMT_CTRL_PHY_RST			0x00000400  /* Self Clearing */
#define	PMT_CTRL_WOL_EN				0x00000200  /* R/W */
#define	PMT_CTRL_ED_EN				0x00000100  /* R/W */
#define	PMT_CTRL_PME_TYPE			0x00000040  /* R/W Not Affected by SW Reset */
#define	PMT_CTRL_WUPS				0x00000030  /* R/WC */
#define	PMT_CTRL_WUPS_NOWAKE			0x00000000  /* R/WC */
#define	PMT_CTRL_WUPS_ED			0x00000010  /* R/WC */
#define	PMT_CTRL_WUPS_WOL			0x00000020  /* R/WC */
#define	PMT_CTRL_WUPS_MULTI			0x00000030  /* R/WC */
#define	PMT_CTRL_PME_IND			0x00000008  /* R/W */
#define	PMT_CTRL_PME_POL			0x00000004  /* R/W */
#define	PMT_CTRL_PME_EN				0x00000002  /* R/W Not Affected by SW Reset */
#define	PMT_CTRL_READY				0x00000001  /* RO */

#define GPIO_CFG		(CONFIG_DRIVER_SMC911X_BASE + 0x88)
#define	GPIO_CFG_LED3_EN			0x40000000  /* R/W */
#define	GPIO_CFG_LED2_EN			0x20000000  /* R/W */
#define	GPIO_CFG_LED1_EN			0x10000000  /* R/W */
#define	GPIO_CFG_GPIO2_INT_POL			0x04000000  /* R/W */
#define	GPIO_CFG_GPIO1_INT_POL			0x02000000  /* R/W */
#define	GPIO_CFG_GPIO0_INT_POL			0x01000000  /* R/W */
#define	GPIO_CFG_EEPR_EN			0x00700000  /* R/W */
#define	GPIO_CFG_GPIOBUF2			0x00040000  /* R/W */
#define	GPIO_CFG_GPIOBUF1			0x00020000  /* R/W */
#define	GPIO_CFG_GPIOBUF0			0x00010000  /* R/W */
#define	GPIO_CFG_GPIODIR2			0x00000400  /* R/W */
#define	GPIO_CFG_GPIODIR1			0x00000200  /* R/W */
#define	GPIO_CFG_GPIODIR0			0x00000100  /* R/W */
#define	GPIO_CFG_GPIOD4				0x00000010  /* R/W */
#define	GPIO_CFG_GPIOD3				0x00000008  /* R/W */
#define	GPIO_CFG_GPIOD2				0x00000004  /* R/W */
#define	GPIO_CFG_GPIOD1				0x00000002  /* R/W */
#define	GPIO_CFG_GPIOD0				0x00000001  /* R/W */

#define GPT_CFG			(CONFIG_DRIVER_SMC911X_BASE + 0x8C)
#define	GPT_CFG_TIMER_EN			0x20000000  /* R/W */
#define	GPT_CFG_GPT_LOAD			0x0000FFFF  /* R/W */

#define GPT_CNT			(CONFIG_DRIVER_SMC911X_BASE + 0x90)
#define	GPT_CNT_GPT_CNT				0x0000FFFF  /* RO */

#define ENDIAN			(CONFIG_DRIVER_SMC911X_BASE + 0x98)
#define FREE_RUN		(CONFIG_DRIVER_SMC911X_BASE + 0x9C)
#define RX_DROP			(CONFIG_DRIVER_SMC911X_BASE + 0xA0)
#define MAC_CSR_CMD		(CONFIG_DRIVER_SMC911X_BASE + 0xA4)
#define	 MAC_CSR_CMD_CSR_BUSY			0x80000000  /* Self Clearing */
#define	 MAC_CSR_CMD_R_NOT_W			0x40000000  /* R/W */
#define	 MAC_CSR_CMD_CSR_ADDR			0x000000FF  /* R/W */

#define MAC_CSR_DATA		(CONFIG_DRIVER_SMC911X_BASE + 0xA8)
#define AFC_CFG			(CONFIG_DRIVER_SMC911X_BASE + 0xAC)
#define		AFC_CFG_AFC_HI			0x00FF0000  /* R/W */
#define		AFC_CFG_AFC_LO			0x0000FF00  /* R/W */
#define		AFC_CFG_BACK_DUR		0x000000F0  /* R/W */
#define		AFC_CFG_FCMULT			0x00000008  /* R/W */
#define		AFC_CFG_FCBRD			0x00000004  /* R/W */
#define		AFC_CFG_FCADD			0x00000002  /* R/W */
#define		AFC_CFG_FCANY			0x00000001  /* R/W */

#define E2P_CMD			(CONFIG_DRIVER_SMC911X_BASE + 0xB0)
#define		E2P_CMD_EPC_BUSY		0x80000000  /* Self Clearing */
#define		E2P_CMD_EPC_CMD			0x70000000  /* R/W */
#define		E2P_CMD_EPC_CMD_READ		0x00000000  /* R/W */
#define		E2P_CMD_EPC_CMD_EWDS		0x10000000  /* R/W */
#define		E2P_CMD_EPC_CMD_EWEN		0x20000000  /* R/W */
#define		E2P_CMD_EPC_CMD_WRITE		0x30000000  /* R/W */
#define		E2P_CMD_EPC_CMD_WRAL		0x40000000  /* R/W */
#define		E2P_CMD_EPC_CMD_ERASE		0x50000000  /* R/W */
#define		E2P_CMD_EPC_CMD_ERAL		0x60000000  /* R/W */
#define		E2P_CMD_EPC_CMD_RELOAD		0x70000000  /* R/W */
#define		E2P_CMD_EPC_TIMEOUT		0x00000200  /* RO */
#define		E2P_CMD_MAC_ADDR_LOADED		0x00000100  /* RO */
#define		E2P_CMD_EPC_ADDR		0x000000FF  /* R/W */

#define E2P_DATA		(CONFIG_DRIVER_SMC911X_BASE + 0xB4)
#define	E2P_DATA_EEPROM_DATA			0x000000FF  /* R/W */
/* end of LAN register offsets and bit definitions */

/* MAC Control and Status registers */
#define MAC_CR			0x01  /* R/W */

/* MAC_CR - MAC Control Register */
#define MAC_CR_RXALL			0x80000000
/* TODO: delete this bit? It is not described in the data sheet. */
#define MAC_CR_HBDIS			0x10000000
#define MAC_CR_RCVOWN			0x00800000
#define MAC_CR_LOOPBK			0x00200000
#define MAC_CR_FDPX			0x00100000
#define MAC_CR_MCPAS			0x00080000
#define MAC_CR_PRMS			0x00040000
#define MAC_CR_INVFILT			0x00020000
#define MAC_CR_PASSBAD			0x00010000
#define MAC_CR_HFILT			0x00008000
#define MAC_CR_HPFILT			0x00002000
#define MAC_CR_LCOLL			0x00001000
#define MAC_CR_BCAST			0x00000800
#define MAC_CR_DISRTY			0x00000400
#define MAC_CR_PADSTR			0x00000100
#define MAC_CR_BOLMT_MASK		0x000000C0
#define MAC_CR_DFCHK			0x00000020
#define MAC_CR_TXEN			0x00000008
#define MAC_CR_RXEN			0x00000004

#define ADDRH			0x02	  /* R/W mask 0x0000FFFFUL */
#define ADDRL			0x03	  /* R/W mask 0xFFFFFFFFUL */
#define HASHH			0x04	  /* R/W */
#define HASHL			0x05	  /* R/W */

#define MII_ACC			0x06	  /* R/W */
#define MII_ACC_PHY_ADDR		0x0000F800
#define MII_ACC_MIIRINDA		0x000007C0
#define MII_ACC_MII_WRITE		0x00000002
#define MII_ACC_MII_BUSY		0x00000001

#define MII_DATA		0x07	  /* R/W mask 0x0000FFFFUL */

#define FLOW			0x08	  /* R/W */
#define FLOW_FCPT			0xFFFF0000
#define FLOW_FCPASS			0x00000004
#define FLOW_FCEN			0x00000002
#define FLOW_FCBSY			0x00000001

#define VLAN1			0x09	  /* R/W mask 0x0000FFFFUL */
#define VLAN1_VTI1			0x0000ffff

#define VLAN2			0x0A	  /* R/W mask 0x0000FFFFUL */
#define VLAN2_VTI2			0x0000ffff

#define WUFF			0x0B	  /* WO */

#define WUCSR			0x0C	  /* R/W */
#define WUCSR_GUE			0x00000200
#define WUCSR_WUFR			0x00000040
#define WUCSR_MPR			0x00000020
#define WUCSR_WAKE_EN			0x00000004
#define WUCSR_MPEN			0x00000002

/* Chip ID values */
#define CHIP_9115	0x115
#define CHIP_9116	0x116
#define CHIP_9117	0x117
#define CHIP_9118	0x118
#define CHIP_9215	0x115a
#define CHIP_9216	0x116a
#define CHIP_9217	0x117a
#define CHIP_9218	0x118a

struct chip_id {
	u16 id;
	char *name;
};

static const struct chip_id chip_ids[] =  {
	{ CHIP_9115, "LAN9115" },
	{ CHIP_9116, "LAN9116" },
	{ CHIP_9117, "LAN9117" },
	{ CHIP_9118, "LAN9118" },
	{ CHIP_9215, "LAN9215" },
	{ CHIP_9216, "LAN9216" },
	{ CHIP_9217, "LAN9217" },
	{ CHIP_9218, "LAN9218" },
	{ 0, NULL },
};

#define DRIVERNAME "smc911x"

u32 smc911x_get_mac_csr(u8 reg)
{
	while (reg_read(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY)
		;
	reg_write(MAC_CSR_CMD, MAC_CSR_CMD_CSR_BUSY | MAC_CSR_CMD_R_NOT_W | reg);
	while (reg_read(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY)
		;

	return reg_read(MAC_CSR_DATA);
}

void smc911x_set_mac_csr(u8 reg, u32 data)
{
	while (reg_read(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY)
		;
	reg_write(MAC_CSR_DATA, data);
	reg_write(MAC_CSR_CMD, MAC_CSR_CMD_CSR_BUSY | reg);
	while (reg_read(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY)
		;
}

static int smx911x_handle_mac_address(bd_t *bd)
{
	unsigned long addrh, addrl;
	unsigned char *m = bd->bi_enetaddr;

	/* if the environment has a valid mac address then use it */
	if ((m[0] | m[1] | m[2] | m[3] | m[4] | m[5])) {
		addrl = m[0] | m[1] << 8 | m[2] << 16 | m[3] << 24;
		addrh = m[4] | m[5] << 8;
		smc911x_set_mac_csr(ADDRH, addrh);
		smc911x_set_mac_csr(ADDRL, addrl);
	} else {
		/* if not, try to get one from the eeprom */
		addrh = smc911x_get_mac_csr(ADDRH);
		addrl = smc911x_get_mac_csr(ADDRL);

		m[0] = (addrl       ) & 0xff;
		m[1] = (addrl >>  8 ) & 0xff;
		m[2] = (addrl >> 16 ) & 0xff;
		m[3] = (addrl >> 24 ) & 0xff;
		m[4] = (addrh       ) & 0xff;
		m[5] = (addrh >>  8 ) & 0xff;

		/* we get 0xff when there is no eeprom connected */
		if ((m[0] & m[1] & m[2] & m[3] & m[4] & m[5]) == 0xff) {
			printf(DRIVERNAME ": no valid mac address in environment "
				"and no eeprom found\n");
			return -1;
		}
	}

	printf(DRIVERNAME ": MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
		m[0], m[1], m[2], m[3], m[4], m[5]);

	return 0;
}

static int smc911x_miiphy_read(u8 phy, u8 reg, u16 *val)
{
	while (smc911x_get_mac_csr(MII_ACC) & MII_ACC_MII_BUSY)
		;

	smc911x_set_mac_csr(MII_ACC, phy << 11 | reg << 6 | MII_ACC_MII_BUSY);

	while (smc911x_get_mac_csr(MII_ACC) & MII_ACC_MII_BUSY)
		;

	*val = smc911x_get_mac_csr(MII_DATA);

	return 0;
}

static int smc911x_miiphy_write(u8 phy, u8 reg, u16  val)
{
	while (smc911x_get_mac_csr(MII_ACC) & MII_ACC_MII_BUSY)
		;

	smc911x_set_mac_csr(MII_DATA, val);
	smc911x_set_mac_csr(MII_ACC,
		phy << 11 | reg << 6 | MII_ACC_MII_BUSY | MII_ACC_MII_WRITE);

	while (smc911x_get_mac_csr(MII_ACC) & MII_ACC_MII_BUSY)
		;
	return 0;
}

static int smc911x_phy_reset(void)
{
	u32 reg;

	reg = reg_read(PMT_CTRL);
	reg &= ~0xfffff030;
	reg |= PMT_CTRL_PHY_RST;
	reg_write(PMT_CTRL, reg);

	mdelay(100);

	return 0;
}

static void smc911x_phy_configure(void)
{
	int timeout;
	u16 status;

	smc911x_phy_reset();

	smc911x_miiphy_write(1, PHY_BMCR, PHY_BMCR_RESET);
	mdelay(1);
	smc911x_miiphy_write(1, PHY_ANAR, 0x01e1);
	smc911x_miiphy_write(1, PHY_BMCR, PHY_BMCR_AUTON | PHY_BMCR_RST_NEG);

	timeout = 5000;
	do {
		mdelay(1);
		if ((timeout--) == 0)
			goto err_out;

		if (smc911x_miiphy_read(1, PHY_BMSR, &status) != 0)
			goto err_out;
	} while (!(status & PHY_BMSR_LS));

	printf(DRIVERNAME ": phy initialized\n");

	return;

err_out:
	printf(DRIVERNAME ": autonegotiation timed out\n");
}

static void smc911x_reset(void)
{
	int timeout;

	/* Take out of PM setting first */
	if (reg_read(PMT_CTRL) & PMT_CTRL_READY) {
		/* Write to the bytetest will take out of powerdown */
		reg_write(BYTE_TEST, 0x0);

		timeout = 10;

		while (timeout-- && !(reg_read(PMT_CTRL) & PMT_CTRL_READY))
			udelay(10);
		if (!timeout) {
			printf(DRIVERNAME
				": timeout waiting for PM restore\n");
			return;
		}
	}

	/* Disable interrupts */
	reg_write(INT_EN, 0);

	reg_write(HW_CFG, HW_CFG_SRST);

	timeout = 1000;
	while (timeout-- && reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY)
		udelay(10);

	if (!timeout) {
		printf(DRIVERNAME ": reset timeout\n");
		return;
	}

	/* Reset the FIFO level and flow control settings */
	smc911x_set_mac_csr(FLOW, FLOW_FCPT | FLOW_FCEN);
	reg_write(AFC_CFG, 0x0050287F);

	/* Set to LED outputs */
	reg_write(GPIO_CFG, 0x70070000);
}

static void smc911x_enable(void)
{
	/* Enable TX */
	reg_write(HW_CFG, 8 << 16 | HW_CFG_SF);

	reg_write(GPT_CFG, GPT_CFG_TIMER_EN | 10000);

	reg_write(TX_CFG, TX_CFG_TX_ON);

	/* no padding to start of packets */
	reg_write(RX_CFG, 0);

	smc911x_set_mac_csr(MAC_CR, MAC_CR_TXEN | MAC_CR_RXEN | MAC_CR_HBDIS);

}

int eth_init(bd_t *bd)
{
	unsigned long val, i;

	printf(DRIVERNAME ": initializing\n");

	val = reg_read(BYTE_TEST);
	if (val != 0x87654321) {
		printf(DRIVERNAME ": Invalid chip endian 0x%08lx\n", val);
		goto err_out;
	}

	val = reg_read(ID_REV) >> 16;
	for (i = 0; chip_ids[i].id != 0; i++) {
		if (chip_ids[i].id == val) break;
	}
	if (!chip_ids[i].id) {
		printf(DRIVERNAME ": Unknown chip ID %04lx\n", val);
		goto err_out;
	}

	printf(DRIVERNAME ": detected %s controller\n", chip_ids[i].name);

	smc911x_reset();

	/* Configure the PHY, initialize the link state */
	smc911x_phy_configure();

	if (smx911x_handle_mac_address(bd))
		goto err_out;

	/* Turn on Tx + Rx */
	smc911x_enable();

	return 0;

err_out:
	return -1;
}

int eth_send(volatile void *packet, int length)
{
	u32 *data = (u32*)packet;
	u32 tmplen;
	u32 status;

	reg_write(TX_DATA_FIFO, TX_CMD_A_INT_FIRST_SEG | TX_CMD_A_INT_LAST_SEG | length);
	reg_write(TX_DATA_FIFO, length);

	tmplen = (length + 3) / 4;

	while (tmplen--)
		reg_write(TX_DATA_FIFO, *data++);

	/* wait for transmission */
	while (!((reg_read(TX_FIFO_INF) & TX_FIFO_INF_TSUSED) >> 16));

	/* get status. Ignore 'no carrier' error, it has no meaning for
	 * full duplex operation
	 */
	status = reg_read(TX_STATUS_FIFO) & (TX_STS_LOC | TX_STS_LATE_COLL |
		TX_STS_MANY_COLL | TX_STS_MANY_DEFER | TX_STS_UNDERRUN);

	if (!status)
		return 0;

	printf(DRIVERNAME ": failed to send packet: %s%s%s%s%s\n",
		status & TX_STS_LOC ? "TX_STS_LOC " : "",
		status & TX_STS_LATE_COLL ? "TX_STS_LATE_COLL " : "",
		status & TX_STS_MANY_COLL ? "TX_STS_MANY_COLL " : "",
		status & TX_STS_MANY_DEFER ? "TX_STS_MANY_DEFER " : "",
		status & TX_STS_UNDERRUN ? "TX_STS_UNDERRUN" : "");

	return -1;
}

void eth_halt(void)
{
	smc911x_reset();
}

int eth_rx(void)
{
	u32 *data = (u32 *)NetRxPackets[0];
	u32 pktlen, tmplen;
	u32 status;

	if ((reg_read(RX_FIFO_INF) & RX_FIFO_INF_RXSUSED) >> 16) {
		status = reg_read(RX_STATUS_FIFO);
		pktlen = (status & RX_STS_PKT_LEN) >> 16;

		reg_write(RX_CFG, 0);

		tmplen = (pktlen + 2+ 3) / 4;
		while (tmplen--)
			*data++ = reg_read(RX_DATA_FIFO);

		if (status & RX_STS_ES)
			printf(DRIVERNAME
				": dropped bad packet. Status: 0x%08x\n",
				status);
		else
			NetReceive(NetRxPackets[0], pktlen);
	}

	return 0;
}
