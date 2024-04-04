// SPDX-License-Identifier: GPL-2.0
/* ICSSG Ethernet driver
 *
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com
 */

#include <phy.h>
#include "icssg_prueth.h"
#include "icssg_switch_map.h"
#include "icss_mii_rt.h"
#include <dm/device_compat.h>
#include <linux/iopoll.h>

/* TX IPG Values to be set for 100M and 1G link speeds.  These values are
 * in ocp_clk cycles. So need change if ocp_clk is changed for a specific
 * h/w design.
 */

/* SR2.0 IPG is in rgmii_clk (125MHz) clock cycles + 1 */
#define MII_RT_TX_IPG_100M      0x17
#define MII_RT_TX_IPG_1G        0xb

#define	ICSSG_QUEUES_MAX		64
#define	ICSSG_QUEUE_OFFSET		0xd00
#define	ICSSG_QUEUE_PEEK_OFFSET		0xe00
#define	ICSSG_QUEUE_CNT_OFFSET		0xe40
#define	ICSSG_QUEUE_RESET_OFFSET	0xf40

#define	ICSSG_NUM_TX_QUEUES	8

#define	RECYCLE_Q_SLICE0	16
#define	RECYCLE_Q_SLICE1	17

#define	ICSSG_NUM_OTHER_QUEUES	5	/* port, host and special queues */

#define	PORT_HI_Q_SLICE0	32
#define	PORT_LO_Q_SLICE0	33
#define	HOST_HI_Q_SLICE0	34
#define	HOST_LO_Q_SLICE0	35
#define	HOST_SPL_Q_SLICE0	40	/* Special Queue */

#define	PORT_HI_Q_SLICE1	36
#define	PORT_LO_Q_SLICE1	37
#define	HOST_HI_Q_SLICE1	38
#define	HOST_LO_Q_SLICE1	39
#define	HOST_SPL_Q_SLICE1	41	/* Special Queue */

#define MII_RXCFG_DEFAULT	(PRUSS_MII_RT_RXCFG_RX_ENABLE | \
				 PRUSS_MII_RT_RXCFG_RX_DATA_RDY_MODE_DIS | \
				 PRUSS_MII_RT_RXCFG_RX_L2_EN | \
				 PRUSS_MII_RT_RXCFG_RX_L2_EOF_SCLR_DIS)

#define MII_TXCFG_DEFAULT	(PRUSS_MII_RT_TXCFG_TX_ENABLE | \
				 PRUSS_MII_RT_TXCFG_TX_AUTO_PREAMBLE | \
				 PRUSS_MII_RT_TXCFG_TX_32_MODE_EN | \
				 PRUSS_MII_RT_TXCFG_TX_IPG_WIRE_CLK_EN)

#define ICSSG_CFG_DEFAULT	(ICSSG_CFG_TX_L1_EN | \
				 ICSSG_CFG_TX_L2_EN | ICSSG_CFG_RX_L2_G_EN | \
				 ICSSG_CFG_TX_PRU_EN | /* SR2.0 only */ \
				 ICSSG_CFG_SGMII_MODE)

#define FDB_GEN_CFG1		0x60
#define SMEM_VLAN_OFFSET	8
#define SMEM_VLAN_OFFSET_MASK	GENMASK(25, 8)

#define FDB_GEN_CFG2		0x64
#define FDB_VLAN_EN		BIT(6)
#define FDB_HOST_EN		BIT(2)
#define FDB_PRU1_EN		BIT(1)
#define FDB_PRU0_EN		BIT(0)
#define FDB_EN_ALL		(FDB_PRU0_EN | FDB_PRU1_EN | \
				 FDB_HOST_EN | FDB_VLAN_EN)

struct map {
	int queue;
	u32 pd_addr_start;
	u32 flags;
	bool special;
};

struct map hwq_map[2][ICSSG_NUM_OTHER_QUEUES] = {
	{
		{ PORT_HI_Q_SLICE0, PORT_DESC0_HI, 0x200000, 0 },
		{ PORT_LO_Q_SLICE0, PORT_DESC0_LO, 0, 0 },
		{ HOST_HI_Q_SLICE0, HOST_DESC0_HI, 0x200000, 0 },
		{ HOST_LO_Q_SLICE0, HOST_DESC0_LO, 0, 0 },
		{ HOST_SPL_Q_SLICE0, HOST_SPPD0, 0x400000, 1 },
	},
	{
		{ PORT_HI_Q_SLICE1, PORT_DESC1_HI, 0xa00000, 0 },
		{ PORT_LO_Q_SLICE1, PORT_DESC1_LO, 0x800000, 0 },
		{ HOST_HI_Q_SLICE1, HOST_DESC1_HI, 0xa00000, 0 },
		{ HOST_LO_Q_SLICE1, HOST_DESC1_LO, 0x800000, 0 },
		{ HOST_SPL_Q_SLICE1, HOST_SPPD1, 0xc00000, 1 },
	},
};

static void icssg_config_mii_init(struct prueth_priv *priv, int slice)
{
	struct prueth *prueth = priv->prueth;
	struct regmap *mii_rt = prueth->mii_rt;
	u32 txcfg_reg, pcnt_reg;
	u32 txcfg;

	txcfg_reg = (slice == ICSS_MII0) ? PRUSS_MII_RT_TXCFG0 :
				       PRUSS_MII_RT_TXCFG1;
	pcnt_reg = (slice == ICSS_MII0) ? PRUSS_MII_RT_RX_PCNT0 :
				       PRUSS_MII_RT_RX_PCNT1;

	txcfg = MII_TXCFG_DEFAULT;

	if (prueth->phy_interface == PHY_INTERFACE_MODE_MII && slice == ICSS_MII0)
		txcfg |= PRUSS_MII_RT_TXCFG_TX_MUX_SEL;
	else if (prueth->phy_interface != PHY_INTERFACE_MODE_MII && slice == ICSS_MII1)
		txcfg |= PRUSS_MII_RT_TXCFG_TX_MUX_SEL;

	regmap_write(mii_rt, txcfg_reg, txcfg);
	regmap_write(mii_rt, pcnt_reg, 0x1);
}

static void icssg_miig_queues_init(struct prueth_priv *priv, int slice)
{
	struct prueth *prueth = priv->prueth;
	void __iomem *smem = (void __iomem *)prueth->shram.pa;
	struct regmap *miig_rt = prueth->miig_rt;
	int queue = 0, i, j;
	u8 pd[ICSSG_SPECIAL_PD_SIZE];
	u32 *pdword;

	/* reset hwqueues */
	if (slice)
		queue = ICSSG_NUM_TX_QUEUES;

	for (i = 0; i < ICSSG_NUM_TX_QUEUES; i++) {
		regmap_write(miig_rt, ICSSG_QUEUE_RESET_OFFSET, queue);
		queue++;
	}

	queue = slice ? RECYCLE_Q_SLICE1 : RECYCLE_Q_SLICE0;
	regmap_write(miig_rt, ICSSG_QUEUE_RESET_OFFSET, queue);

	for (i = 0; i < ICSSG_NUM_OTHER_QUEUES; i++) {
		regmap_write(miig_rt, ICSSG_QUEUE_RESET_OFFSET,
			     hwq_map[slice][i].queue);
	}

	/* initialize packet descriptors in SMEM */
	/* push pakcet descriptors to hwqueues */

	pdword = (u32 *)pd;
	for (j = 0; j < ICSSG_NUM_OTHER_QUEUES; j++) {
		struct map *mp;
		int pd_size, num_pds;
		u32 pdaddr;

		mp = &hwq_map[slice][j];
		if (mp->special) {
			pd_size = ICSSG_SPECIAL_PD_SIZE;
			num_pds = ICSSG_NUM_SPECIAL_PDS;
		} else	{
			pd_size = ICSSG_NORMAL_PD_SIZE;
			num_pds = ICSSG_NUM_NORMAL_PDS;
		}

		for (i = 0; i < num_pds; i++) {
			memset(pd, 0, pd_size);

			pdword[0] &= cpu_to_le32(ICSSG_FLAG_MASK);
			pdword[0] |= cpu_to_le32(mp->flags);
			pdaddr = mp->pd_addr_start + i * pd_size;

			memcpy_toio(smem + pdaddr, pd, pd_size);
			queue = mp->queue;
			regmap_write(miig_rt, ICSSG_QUEUE_OFFSET + 4 * queue,
				     pdaddr);
		}
	}
}

void icssg_config_ipg(struct prueth_priv *priv, int speed, int mii)
{
	struct prueth *prueth = priv->prueth;

	switch (speed) {
	case SPEED_1000:
		icssg_mii_update_ipg(prueth->mii_rt, mii, MII_RT_TX_IPG_1G);
		break;
	case SPEED_100:
		icssg_mii_update_ipg(prueth->mii_rt, mii, MII_RT_TX_IPG_100M);
		break;
	default:
		/* Other links speeds not supported */
		pr_err("Unsupported link speed\n");
		return;
	}
}

static void emac_r30_cmd_init(struct prueth_priv *priv)
{
	struct prueth *prueth = priv->prueth;
	struct icssg_r30_cmd *p;
	int i;

	p = (struct icssg_r30_cmd *)(prueth->dram[priv->port_id].pa + MGR_R30_CMD_OFFSET);

	for (i = 0; i < 4; i++)
		writel(EMAC_NONE, &p->cmd[i]);
}

static int emac_r30_is_done(struct prueth_priv *priv)
{
	struct prueth *prueth = priv->prueth;
	const struct icssg_r30_cmd *p;
	int i;
	u32 cmd;

	p = (const struct icssg_r30_cmd *)(prueth->dram[priv->port_id].pa + MGR_R30_CMD_OFFSET);

	for (i = 0; i < 4; i++) {
		cmd = readl(&p->cmd[i]);
		if (cmd != EMAC_NONE)
			return 0;
	}

	return 1;
}

static int prueth_emac_buffer_setup(struct prueth_priv *priv)
{
	struct prueth *prueth = priv->prueth;
	struct icssg_buffer_pool_cfg *bpool_cfg;
	struct icssg_rxq_ctx *rxq_ctx;
	int slice = priv->port_id;
	u32 addr;
	int i;

	/* Layout to have 64KB aligned buffer pool
	 * |BPOOL0|BPOOL1|RX_CTX0|RX_CTX1|
	 */

	addr = lower_32_bits(prueth->sram_pa);
	if (slice)
		addr += PRUETH_NUM_BUF_POOLS * PRUETH_EMAC_BUF_POOL_SIZE;

	if (addr % SZ_64K) {
		dev_warn(prueth->dev, "buffer pool needs to be 64KB aligned\n");
		return -EINVAL;
	}

	bpool_cfg = (struct icssg_buffer_pool_cfg *)(prueth->dram[priv->port_id].pa + BUFFER_POOL_0_ADDR_OFFSET);
	/* workaround for f/w bug. bpool 0 needs to be initilalized */
	bpool_cfg[0].addr = cpu_to_le32(addr);
	bpool_cfg[0].len = 0;

	for (i = PRUETH_EMAC_BUF_POOL_START;
	     i < (PRUETH_EMAC_BUF_POOL_START + PRUETH_NUM_BUF_POOLS);
	     i++) {
		bpool_cfg[i].addr = cpu_to_le32(addr);
		bpool_cfg[i].len = cpu_to_le32(PRUETH_EMAC_BUF_POOL_SIZE);
		addr += PRUETH_EMAC_BUF_POOL_SIZE;
	}

	if (!slice)
		addr += PRUETH_NUM_BUF_POOLS * PRUETH_EMAC_BUF_POOL_SIZE;
	else
		addr += PRUETH_EMAC_RX_CTX_BUF_SIZE * 2;

	rxq_ctx = (struct icssg_rxq_ctx *)(prueth->dram[priv->port_id].pa + HOST_RX_Q_PRE_CONTEXT_OFFSET);

	for (i = 0; i < 3; i++)
		rxq_ctx->start[i] = cpu_to_le32(addr);

	addr += PRUETH_EMAC_RX_CTX_BUF_SIZE;
	rxq_ctx->end = cpu_to_le32(addr);

	/* Express RX buffer queue */
	rxq_ctx = (struct icssg_rxq_ctx *)(prueth->dram[priv->port_id].pa + HOST_RX_Q_EXP_CONTEXT_OFFSET);
	for (i = 0; i < 3; i++)
		rxq_ctx->start[i] = cpu_to_le32(addr);

	addr += PRUETH_EMAC_RX_CTX_BUF_SIZE;
	rxq_ctx->end = cpu_to_le32(addr);

	return 0;
}

static void icssg_init_emac_mode(struct prueth *prueth)
{
	u8 mac[6] = { 0 };

	regmap_update_bits(prueth->miig_rt, FDB_GEN_CFG1, SMEM_VLAN_OFFSET_MASK, 0);
	regmap_write(prueth->miig_rt, FDB_GEN_CFG2, 0);
	/* Clear host MAC address */
	icssg_class_set_host_mac_addr(prueth->miig_rt, mac);
}

int icssg_config(struct prueth_priv *priv)
{
	struct prueth *prueth = priv->prueth;
	void *config = (void *)(prueth->dram[priv->port_id].pa + ICSSG_CONFIG_OFFSET);
	u8 *cfg_byte_ptr = config;
	struct icssg_flow_cfg *flow_cfg;
	u32 mask;
	int ret;

	int slice = priv->port_id;

	icssg_init_emac_mode(prueth);

	memset_io(config, 0, TAS_GATE_MASK_LIST0);
	icssg_miig_queues_init(priv, slice);

	prueth->speed = SPEED_1000;
	prueth->duplex = DUPLEX_FULL;
	if (!phy_interface_is_rgmii(priv->phydev)) {
		prueth->speed = SPEED_100;
		prueth->duplex = DUPLEX_FULL;
	}

	regmap_update_bits(prueth->miig_rt, ICSSG_CFG_OFFSET,
			   ICSSG_CFG_DEFAULT, ICSSG_CFG_DEFAULT);
	icssg_miig_set_interface_mode(prueth->miig_rt, ICSS_MII0, prueth->phy_interface);
	icssg_miig_set_interface_mode(prueth->miig_rt, ICSS_MII1, prueth->phy_interface);
	icssg_config_mii_init(priv, slice);

	icssg_config_ipg(priv, SPEED_1000, slice);
	icssg_update_rgmii_cfg(prueth->miig_rt, SPEED_1000, true, slice, priv);

	/* set GPI mode */
	pruss_cfg_gpimode(prueth->pruss, slice, PRUSS_GPI_MODE_MII);

	/* enable XFR shift for PRU and RTU */
	mask = PRUSS_SPP_XFER_SHIFT_EN | PRUSS_SPP_RTU_XFR_SHIFT_EN;
	pruss_cfg_update(prueth->pruss, PRUSS_CFG_SPP, mask, mask);

	flow_cfg = config + PSI_L_REGULAR_FLOW_ID_BASE_OFFSET;
	flow_cfg->rx_base_flow = prueth->dma_rx.id;
	flow_cfg->mgm_base_flow = 0;
	*(cfg_byte_ptr + SPL_PKT_DEFAULT_PRIORITY) = 0;
	*(cfg_byte_ptr + QUEUE_NUM_UNTAGGED) = 0x0;

	ret = prueth_emac_buffer_setup(priv);

	if (ret)
		return ret;

	emac_r30_cmd_init(priv);
	return 0;
}

/* commands to program ICSSG R30 registers */
static struct icssg_r30_cmd emac_r32_bitmask[] = {
	{{0xffff0004, 0xffff0100, 0xffff0004, EMAC_NONE}},	/* EMAC_PORT_DISABLE */
	{{0xfffb0040, 0xfeff0200, 0xfeff0200, EMAC_NONE}},	/* EMAC_PORT_BLOCK */
	{{0xffbb0000, 0xfcff0000, 0xdcfb0000, EMAC_NONE}},	/* EMAC_PORT_FORWARD */
	{{0xffbb0000, 0xfcff0000, 0xfcff2000, EMAC_NONE}},	/* EMAC_PORT_FORWARD_WO_LEARNING */
	{{0xffff0001, EMAC_NONE,  EMAC_NONE, EMAC_NONE}},	/* ACCEPT ALL */
	{{0xfffe0002, EMAC_NONE,  EMAC_NONE, EMAC_NONE}},	/* ACCEPT TAGGED */
	{{0xfffc0000, EMAC_NONE,  EMAC_NONE, EMAC_NONE}},	/* ACCEPT UNTAGGED and PRIO */
	{{EMAC_NONE,  0xffff0020, EMAC_NONE, EMAC_NONE}},	/* TAS Trigger List change */
	{{EMAC_NONE,  0xdfff1000, EMAC_NONE, EMAC_NONE}},	/* TAS set state ENABLE*/
	{{EMAC_NONE,  0xefff2000, EMAC_NONE, EMAC_NONE}},	/* TAS set state RESET*/
	{{EMAC_NONE,  0xcfff0000, EMAC_NONE, EMAC_NONE}},	/* TAS set state DISABLE*/
	{{EMAC_NONE,  EMAC_NONE,  0xffff0400, EMAC_NONE}},	/* UC flooding ENABLE*/
	{{EMAC_NONE,  EMAC_NONE,  0xfbff0000, EMAC_NONE}},	/* UC flooding DISABLE*/
	{{EMAC_NONE,  EMAC_NONE,  0xffff0800, EMAC_NONE}},	/* MC flooding ENABLE*/
	{{EMAC_NONE,  EMAC_NONE,  0xf7ff0000, EMAC_NONE}},	/* MC flooding DISABLE*/
	{{EMAC_NONE,  0xffff4000, EMAC_NONE, EMAC_NONE}},	/* Preemption on Tx ENABLE*/
	{{EMAC_NONE,  0xbfff0000, EMAC_NONE, EMAC_NONE}}	/* Preemption on Tx DISABLE*/
};

int emac_set_port_state(struct prueth_priv *priv,
			enum icssg_port_state_cmd cmd)
{
	struct prueth *prueth = priv->prueth;
	struct icssg_r30_cmd *p;
	int ret = -ETIMEDOUT;
	int timeout = 10;
	int i;

	p = (struct icssg_r30_cmd *)(prueth->dram[priv->port_id].pa + MGR_R30_CMD_OFFSET);

	if (cmd >= ICSSG_EMAC_PORT_MAX_COMMANDS) {
		dev_err(prueth->dev, "invalid port command\n");
		return -EINVAL;
	}

	for (i = 0; i < 4; i++)
		writel(emac_r32_bitmask[cmd].cmd[i], &p->cmd[i]);

	/* wait for done */
	while (timeout) {
		if (emac_r30_is_done(priv)) {
			ret = 0;
			break;
		}

		udelay(2000);
		timeout--;
	}

	if (ret == -ETIMEDOUT)
		dev_err(prueth->dev, "timeout waiting for command done\n");

	return ret;
}

int icssg_send_fdb_msg(struct prueth_priv *priv, struct mgmt_cmd *cmd,
		       struct mgmt_cmd_rsp *rsp)
{
	struct prueth *prueth = priv->prueth;
	int slice = priv->port_id;
	int ret, addr;

	addr = icssg_queue_pop(prueth, slice == 0 ?
			       ICSSG_CMD_POP_SLICE0 : ICSSG_CMD_POP_SLICE1);
	if (addr < 0)
		return addr;

	/* First 4 bytes have FW owned buffer linking info which should
	 * not be touched
	 */
	memcpy_toio((void __iomem *)prueth->shram.pa + addr + 4, cmd, sizeof(*cmd));
	icssg_queue_push(prueth, slice == 0 ?
			 ICSSG_CMD_PUSH_SLICE0 : ICSSG_CMD_PUSH_SLICE1, addr);
	ret = read_poll_timeout(icssg_queue_pop, addr, addr >= 0,
				2000, 20000000, prueth, slice == 0 ?
				ICSSG_RSP_POP_SLICE0 : ICSSG_RSP_POP_SLICE1);

	if (ret) {
		dev_err(prueth->dev, "Timedout sending HWQ message\n");
		return ret;
	}

	memcpy_fromio(rsp, (void __iomem *)prueth->shram.pa + addr, sizeof(*rsp));
	/* Return buffer back for to pool */
	icssg_queue_push(prueth, slice == 0 ?
			 ICSSG_RSP_PUSH_SLICE0 : ICSSG_RSP_PUSH_SLICE1, addr);

	return 0;
}

int emac_fdb_flow_id_updated(struct prueth_priv *priv)
{
	struct mgmt_cmd_rsp fdb_cmd_rsp = { 0 };
	struct prueth *prueth = priv->prueth;
	struct mgmt_cmd fdb_cmd = { 0 };
	int slice = priv->port_id;
	int ret = 0;

	fdb_cmd.header = ICSSG_FW_MGMT_CMD_HEADER;
	fdb_cmd.type   = ICSSG_FW_MGMT_FDB_CMD_TYPE_RX_FLOW;
	fdb_cmd.seqnum = ++(prueth->icssg_hwcmdseq);
	fdb_cmd.param  = 0;

	fdb_cmd.param |= (slice << 4);
	fdb_cmd.cmd_args[0] = 0;

	ret = icssg_send_fdb_msg(priv, &fdb_cmd, &fdb_cmd_rsp);
	if (ret)
		return ret;

	if (fdb_cmd.seqnum != fdb_cmd_rsp.seqnum) {
		dev_err(prueth->dev, "seqnum doesn't match, cmd.seqnum %d != rsp.seqnum %d\n",
			fdb_cmd.seqnum, fdb_cmd_rsp.seqnum);
		return -EINVAL;
	}

	if (fdb_cmd_rsp.status == 1)
		return 0;

	return -EINVAL;
}
