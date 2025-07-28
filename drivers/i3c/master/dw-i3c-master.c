// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Synopsys, Inc. and/or its affiliates.
 *
 * Author: Vitor Soares <vitor.soares@synopsys.com>
 */

#include <asm/io.h>
#include <dm.h>
#include <dw-i3c.h>
#include <i2c.h>
#include <log.h>
#include <malloc.h>
#include <pci.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/iopoll.h>
#include <linux/i3c/master.h>

#ifdef CONFIG_SANDBOX
#define cpu_relax() do {} while (0)
#endif

static u8 even_parity(u8 p)
{
	p ^= p >> 4;
	p &= 0xf;

	return (0x9669 >> p) & 1;
}

ulong msecs_to_jiffies(ulong msec)
{
	ulong hz = CONFIG_SYS_HZ;

	return (msec + (1000 / hz) - 1) / (1000 / hz);
}

static bool dw_i3c_master_supports_ccc_cmd(struct i3c_master_controller *m,
					   const struct i3c_ccc_cmd *cmd)
{
	if (cmd->ndests > 1)
		return false;

	switch (cmd->id) {
	case I3C_CCC_ENEC(true):
	case I3C_CCC_ENEC(false):
	case I3C_CCC_DISEC(true):
	case I3C_CCC_DISEC(false):
	case I3C_CCC_ENTAS(0, true):
	case I3C_CCC_ENTAS(0, false):
	case I3C_CCC_RSTDAA(true):
	case I3C_CCC_RSTDAA(false):
	case I3C_CCC_ENTDAA:
	case I3C_CCC_SETMWL(true):
	case I3C_CCC_SETMWL(false):
	case I3C_CCC_SETMRL(true):
	case I3C_CCC_SETMRL(false):
	case I3C_CCC_ENTHDR(0):
	case I3C_CCC_SETDASA:
	case I3C_CCC_SETNEWDA:
	case I3C_CCC_GETMWL:
	case I3C_CCC_GETMRL:
	case I3C_CCC_GETPID:
	case I3C_CCC_GETBCR:
	case I3C_CCC_GETDCR:
	case I3C_CCC_GETSTATUS:
	case I3C_CCC_GETMXDS:
	case I3C_CCC_GETHDRCAP:
		return true;
	default:
		return false;
	}
}

static inline struct dw_i3c_master *
to_dw_i3c_master(struct i3c_master_controller *master)
{
	return container_of(master, struct dw_i3c_master, base);
}

static void dw_i3c_master_disable(struct dw_i3c_master *master)
{
	writel(readl(master->regs + DEVICE_CTRL) & ~DEV_CTRL_ENABLE,
	       master->regs + DEVICE_CTRL);
}

static void dw_i3c_master_enable(struct dw_i3c_master *master)
{
	writel(readl(master->regs + DEVICE_CTRL) | DEV_CTRL_ENABLE,
	       master->regs + DEVICE_CTRL);
}

static int dw_i3c_master_get_addr_pos(struct dw_i3c_master *master, u8 addr)
{
	int pos;

	for (pos = 0; pos < master->maxdevs; pos++) {
		if (addr == master->addrs[pos])
			return pos;
	}

	return -EINVAL;
}

static int dw_i3c_master_get_free_pos(struct dw_i3c_master *master)
{
	if (!(master->free_pos & GENMASK(master->maxdevs - 1, 0)))
		return -ENOSPC;

	return ffs(master->free_pos) - 1;
}

static void dw_i3c_master_wr_tx_fifo(struct dw_i3c_master *master,
				     const u8 *bytes, int nbytes)
{
	writesl(master->regs + RX_TX_DATA_PORT, bytes, nbytes / 4);
	if (nbytes & 3) {
		u32 tmp = 0;

		memcpy(&tmp, bytes + (nbytes & ~3), nbytes & 3);
		writesl(master->regs + RX_TX_DATA_PORT, &tmp, 1);
	}
}

static void dw_i3c_master_read_rx_fifo(struct dw_i3c_master *master,
				       u8 *bytes, int nbytes)
{
	readsl(master->regs + RX_TX_DATA_PORT, bytes, nbytes / 4);
	if (nbytes & 3) {
		u32 tmp;

		readsl(master->regs + RX_TX_DATA_PORT, &tmp, 1);
		memcpy(bytes + (nbytes & ~3), &tmp, nbytes & 3);
	}
}

static struct dw_i3c_xfer *
dw_i3c_master_alloc_xfer(struct dw_i3c_master *master, unsigned int ncmds)
{
	struct dw_i3c_xfer *xfer;

	xfer = kzalloc(STRUCT_SZ(struct dw_i3c_xfer, ncmds), GFP_KERNEL);
	if (!xfer)
		return NULL;

	INIT_LIST_HEAD(&xfer->node);
	xfer->ncmds = ncmds;
	xfer->ret = -ETIMEDOUT;

	return xfer;
}

static void dw_i3c_master_free_xfer(struct dw_i3c_xfer *xfer)
{
	kfree(xfer);
}

static void dw_i3c_master_start_xfer_locked(struct dw_i3c_master *master)
{
	struct dw_i3c_xfer *xfer = master->xferqueue.cur;
	unsigned int i;
	u32 thld_ctrl;

	if (!xfer)
		return;

	for (i = 0; i < xfer->ncmds; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		dw_i3c_master_wr_tx_fifo(master, cmd->tx_buf, cmd->tx_len);
	}

	thld_ctrl = readl(master->regs + QUEUE_THLD_CTRL);
	thld_ctrl &= ~QUEUE_THLD_CTRL_RESP_BUF_MASK;
	thld_ctrl |= QUEUE_THLD_CTRL_RESP_BUF(xfer->ncmds);
	writel(thld_ctrl, master->regs + QUEUE_THLD_CTRL);

	for (i = 0; i < xfer->ncmds; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		writel(cmd->cmd_hi, master->regs + COMMAND_QUEUE_PORT);
		writel(cmd->cmd_lo, master->regs + COMMAND_QUEUE_PORT);
	}
}

static void dw_i3c_master_enqueue_xfer(struct dw_i3c_master *master,
				       struct dw_i3c_xfer *xfer)
{
	unsigned long flags;

	spin_lock_irqsave(&master->xferqueue.lock, flags);

	if (master->xferqueue.cur) {
		list_add_tail(&xfer->node, &master->xferqueue.list);
	} else {
		master->xferqueue.cur = xfer;
		dw_i3c_master_start_xfer_locked(master);
	}

	spin_unlock_irqrestore(&master->xferqueue.lock, flags);
}

static void dw_i3c_master_dequeue_xfer_locked(struct dw_i3c_master *master,
					      struct dw_i3c_xfer *xfer)
{
	if (master->xferqueue.cur == xfer) {
		u32 status;

		master->xferqueue.cur = NULL;

		writel(RESET_CTRL_RX_FIFO | RESET_CTRL_TX_FIFO |
		       RESET_CTRL_RESP_QUEUE | RESET_CTRL_CMD_QUEUE,
		       master->regs + RESET_CTRL);

		readl_poll_timeout_atomic(master->regs + RESET_CTRL, status,
					  !status, 10, 1000000);
	} else {
		list_del_init(&xfer->node);
	}
}

static int dw_i3c_status_poll_timeout(struct dw_i3c_master *master)
{
	u32 status;
	unsigned long base, limit;

	base = get_timer(0);
	limit = CONFIG_SYS_HZ * 5000 / 1000;
	do {
		status = readl(master->regs + INTR_STATUS);
		if (status)
			return 0;

		cpu_relax();
	} while (get_timer(base) < limit);

	return -ETIMEDOUT;
}

static void dw_i3c_master_dequeue_xfer(struct dw_i3c_master *master,
				       struct dw_i3c_xfer *xfer)
{
	unsigned long flags;

	spin_lock_irqsave(&master->xferqueue.lock, flags);
	dw_i3c_master_dequeue_xfer_locked(master, xfer);
	spin_unlock_irqrestore(&master->xferqueue.lock, flags);
}

static void dw_i3c_master_end_xfer_locked(struct dw_i3c_master *master, u32 isr)
{
	struct dw_i3c_xfer *xfer = master->xferqueue.cur;
	int i, ret = 0;
	u32 nresp;

	if (!xfer)
		return;

	nresp = readl(master->regs + QUEUE_STATUS_LEVEL);
	nresp = QUEUE_STATUS_LEVEL_RESP(nresp);

	for (i = 0; i < nresp; i++) {
		struct dw_i3c_cmd *cmd;
		u32 resp;

		resp = readl(master->regs + RESPONSE_QUEUE_PORT);

		cmd = &xfer->cmds[RESPONSE_PORT_TID(resp)];
		cmd->rx_len = RESPONSE_PORT_DATA_LEN(resp);
		cmd->error = RESPONSE_PORT_ERR_STATUS(resp);
		if (cmd->rx_len && !cmd->error)
			dw_i3c_master_read_rx_fifo(master, cmd->rx_buf,
						   cmd->rx_len);
	}

	for (i = 0; i < nresp; i++) {
		switch (xfer->cmds[i].error) {
		case RESPONSE_NO_ERROR:
			break;
		case RESPONSE_ERROR_PARITY:
		case RESPONSE_ERROR_IBA_NACK:
		case RESPONSE_ERROR_TRANSF_ABORT:
		case RESPONSE_ERROR_CRC:
		case RESPONSE_ERROR_FRAME:
			ret = -EIO;
			break;
		case RESPONSE_ERROR_OVER_UNDER_FLOW:
			ret = -ENOSPC;
			break;
		case RESPONSE_ERROR_I2C_W_NACK_ERR:
		case RESPONSE_ERROR_ADDRESS_NACK:
		default:
			ret = -EINVAL;
			break;
		}
	}

	xfer->ret = ret;

	if (ret < 0) {
		dw_i3c_master_dequeue_xfer_locked(master, xfer);
		writel(readl(master->regs + DEVICE_CTRL) | DEV_CTRL_RESUME,
		       master->regs + DEVICE_CTRL);
	}

	xfer = list_first_entry_or_null(&master->xferqueue.list,
					struct dw_i3c_xfer,
					node);
	if (xfer)
		list_del_init(&xfer->node);

	master->xferqueue.cur = xfer;
	dw_i3c_master_start_xfer_locked(master);
}

static int dw_i3c_clk_cfg(struct dw_i3c_master *master)
{
	unsigned long core_rate, core_period;
	u32 scl_timing;
	u8 hcnt, lcnt;

	core_rate = clk_get_rate(&master->core_clk);
	if (!core_rate)
		return -EINVAL;

	core_period = DIV_ROUND_UP(1000000000, core_rate);

	hcnt = DIV_ROUND_UP(I3C_BUS_THIGH_MAX_NS, core_period) - 1;
	if (hcnt < SCL_I3C_TIMING_CNT_MIN)
		hcnt = SCL_I3C_TIMING_CNT_MIN;

	/* set back to THIGH_MAX_NS, after disable spike filter */
	if (!master->first_broadcast) {
		lcnt = SCL_I3C_TIMING_LCNT(readl(master->regs + SCL_I3C_OD_TIMING));
		scl_timing = SCL_I3C_TIMING_HCNT(hcnt) | lcnt;
		writel(scl_timing, master->regs + SCL_I3C_OD_TIMING);
		return 0;
	}

	lcnt = DIV_ROUND_UP(core_rate, master->base.bus.scl_rate.i3c) - hcnt;
	if (lcnt < SCL_I3C_TIMING_CNT_MIN)
		lcnt = SCL_I3C_TIMING_CNT_MIN;

	scl_timing = SCL_I3C_TIMING_HCNT(hcnt) | SCL_I3C_TIMING_LCNT(lcnt);
	writel(scl_timing, master->regs + SCL_I3C_PP_TIMING);

	/*
	 * In pure i3c mode, MST_FREE represents tCAS. In shared mode, this
	 * will be set up by dw_i2c_clk_cfg as tLOW.
	 */
	if (master->base.bus.mode == I3C_BUS_MODE_PURE)
		writel(BUS_I3C_MST_FREE(lcnt), master->regs + BUS_FREE_TIMING);

	lcnt = max_t(u8,
		     DIV_ROUND_UP(I3C_BUS_TLOW_OD_MIN_NS, core_period), lcnt);

	/* first broadcast thigh to 200ns, to disable spike filter */
	hcnt = DIV_ROUND_UP(I3C_BUS_THIGH_INIT_OD_MIN_NS, core_period);

	scl_timing = SCL_I3C_TIMING_HCNT(hcnt) | SCL_I3C_TIMING_LCNT(lcnt);
	writel(scl_timing, master->regs + SCL_I3C_OD_TIMING);

	lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR1_SCL_RATE) - hcnt;
	scl_timing = SCL_EXT_LCNT_1(lcnt);
	lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR2_SCL_RATE) - hcnt;
	scl_timing |= SCL_EXT_LCNT_2(lcnt);
	lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR3_SCL_RATE) - hcnt;
	scl_timing |= SCL_EXT_LCNT_3(lcnt);
	lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR4_SCL_RATE) - hcnt;
	scl_timing |= SCL_EXT_LCNT_4(lcnt);
	writel(scl_timing, master->regs + SCL_EXT_LCNT_TIMING);

	return 0;
}

static int dw_i2c_clk_cfg(struct dw_i3c_master *master)
{
	unsigned long core_rate, core_period;
	u16 hcnt, lcnt;
	u32 scl_timing;

	core_rate = clk_get_rate(&master->core_clk);
	if (!core_rate)
		return -EINVAL;

	core_period = DIV_ROUND_UP(1000000000, core_rate);

	lcnt = DIV_ROUND_UP(I3C_BUS_I2C_FMP_TLOW_MIN_NS, core_period);
	hcnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_PLUS_SCL_RATE) - lcnt;
	scl_timing = SCL_I2C_FMP_TIMING_HCNT(hcnt) |
		     SCL_I2C_FMP_TIMING_LCNT(lcnt);
	writel(scl_timing, master->regs + SCL_I2C_FMP_TIMING);

	lcnt = DIV_ROUND_UP(I3C_BUS_I2C_FM_TLOW_MIN_NS, core_period);
	hcnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_SCL_RATE) - lcnt;
	scl_timing = SCL_I2C_FM_TIMING_HCNT(hcnt) |
		     SCL_I2C_FM_TIMING_LCNT(lcnt);
	writel(scl_timing, master->regs + SCL_I2C_FM_TIMING);

	writel(BUS_I3C_MST_FREE(lcnt), master->regs + BUS_FREE_TIMING);
	writel(readl(master->regs + DEVICE_CTRL) | DEV_CTRL_I2C_SLAVE_PRESENT,
	       master->regs + DEVICE_CTRL);

	return 0;
}

static int dw_i3c_master_bus_init(struct i3c_master_controller *m)
{
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	struct i3c_bus *bus = i3c_master_get_bus(m);
	struct i3c_device_info info = { };
	u32 thld_ctrl;
	int ret;

	/* first broadcast to disable spike filter */
	master->first_broadcast = true;

	switch (bus->mode) {
	case I3C_BUS_MODE_MIXED_FAST:
	case I3C_BUS_MODE_MIXED_LIMITED:
		ret = dw_i2c_clk_cfg(master);
		if (ret)
			return ret;
		fallthrough;
	case I3C_BUS_MODE_PURE:
		ret = dw_i3c_clk_cfg(master);
		if (ret)
			return ret;
		break;
	default:
		return -EINVAL;
	}

	thld_ctrl = readl(master->regs + QUEUE_THLD_CTRL);
	thld_ctrl &= ~QUEUE_THLD_CTRL_RESP_BUF_MASK;
	writel(thld_ctrl, master->regs + QUEUE_THLD_CTRL);

	thld_ctrl = readl(master->regs + DATA_BUFFER_THLD_CTRL);
	thld_ctrl &= ~DATA_BUFFER_THLD_CTRL_RX_BUF;
	writel(thld_ctrl, master->regs + DATA_BUFFER_THLD_CTRL);

	writel(INTR_ALL, master->regs + INTR_STATUS);
	writel(INTR_MASTER_MASK, master->regs + INTR_STATUS_EN);
	writel(INTR_MASTER_MASK, master->regs + INTR_SIGNAL_EN);

	ret = i3c_master_get_free_addr(m, 0);
	if (ret < 0)
		return ret;

	writel(DEV_ADDR_DYNAMIC_ADDR_VALID | DEV_ADDR_DYNAMIC(ret),
	       master->regs + DEVICE_ADDR);

	memset(&info, 0, sizeof(info));
	info.dyn_addr = ret;

	ret = i3c_master_set_info(&master->base, &info);
	if (ret)
		return ret;

	writel(IBI_REQ_REJECT_ALL, master->regs + IBI_SIR_REQ_REJECT);
	writel(IBI_REQ_REJECT_ALL, master->regs + IBI_MR_REQ_REJECT);

	/* For now don't support Hot-Join */
	writel(readl(master->regs + DEVICE_CTRL) | DEV_CTRL_HOT_JOIN_NACK,
	       master->regs + DEVICE_CTRL);

	dw_i3c_master_enable(master);

	return 0;
}

static void dw_i3c_master_bus_cleanup(struct i3c_master_controller *m)
{
	struct dw_i3c_master *master = to_dw_i3c_master(m);

	dw_i3c_master_disable(master);
}

static void dw_i3c_master_irq_handler(struct dw_i3c_master *master)
{
	u32 status, en;

	status = readl(master->regs + INTR_STATUS);
	en = readl(master->regs + INTR_STATUS_EN);

	if (!(status & readl(master->regs + INTR_STATUS_EN))) {
		pr_err("Failed to get completion status, status=%d, status_en=%d\n", status, en);
		writel(INTR_ALL, master->regs + INTR_STATUS);
	}

	spin_lock(&master->xferqueue.lock);
	dw_i3c_master_end_xfer_locked(master, status);

	if (status & INTR_TRANSFER_ERR_STAT)
		writel(INTR_TRANSFER_ERR_STAT, master->regs + INTR_STATUS);

	/* set back to THIGH_MAX_NS, after disable spike filter */
	if (master->first_broadcast) {
		master->first_broadcast = false;
		int ret = dw_i3c_clk_cfg(master);

		if (ret)
			pr_err("Failed to set clk cfg\n");
	}

	spin_unlock(&master->xferqueue.lock);
}

static int dw_i3c_ccc_set(struct dw_i3c_master *master,
			  struct i3c_ccc_cmd *ccc)
{
	struct dw_i3c_xfer *xfer;
	struct dw_i3c_cmd *cmd;
	int ret, pos = 0;

	if (ccc->id & I3C_CCC_DIRECT) {
		pos = dw_i3c_master_get_addr_pos(master, ccc->dests[0].addr);
		if (pos < 0)
			return pos;
	}

	xfer = dw_i3c_master_alloc_xfer(master, 1);
	if (!xfer)
		return -ENOMEM;

	cmd = xfer->cmds;
	cmd->tx_buf = ccc->dests[0].payload.data;
	cmd->tx_len = ccc->dests[0].payload.len;

	cmd->cmd_hi = COMMAND_PORT_ARG_DATA_LEN(ccc->dests[0].payload.len) |
		      COMMAND_PORT_TRANSFER_ARG;

	cmd->cmd_lo = COMMAND_PORT_CP |
		      COMMAND_PORT_DEV_INDEX(pos) |
		      COMMAND_PORT_CMD(ccc->id) |
		      COMMAND_PORT_TOC |
		      COMMAND_PORT_ROC;

	dw_i3c_master_enqueue_xfer(master, xfer);

	if (dw_i3c_status_poll_timeout(master) > POLL_SUCCESS)
		dw_i3c_master_dequeue_xfer(master, xfer);
	else
		dw_i3c_master_irq_handler(master);

	ret = xfer->ret;

	if (xfer->cmds[0].error == RESPONSE_ERROR_IBA_NACK)
		ccc->err = I3C_ERROR_M2;

	dw_i3c_master_free_xfer(xfer);

	return ret;
}

static int dw_i3c_ccc_get(struct dw_i3c_master *master, struct i3c_ccc_cmd *ccc)
{
	struct dw_i3c_xfer *xfer;
	struct dw_i3c_cmd *cmd;
	int ret, pos;

	pos = dw_i3c_master_get_addr_pos(master, ccc->dests[0].addr);
	if (pos < 0)
		return pos;

	xfer = dw_i3c_master_alloc_xfer(master, 1);
	if (!xfer)
		return -ENOMEM;

	cmd = xfer->cmds;
	cmd->rx_buf = ccc->dests[0].payload.data;
	cmd->rx_len = ccc->dests[0].payload.len;

	cmd->cmd_hi = COMMAND_PORT_ARG_DATA_LEN(ccc->dests[0].payload.len) |
		      COMMAND_PORT_TRANSFER_ARG;

	cmd->cmd_lo = COMMAND_PORT_READ_TRANSFER |
		      COMMAND_PORT_CP |
		      COMMAND_PORT_DEV_INDEX(pos) |
		      COMMAND_PORT_CMD(ccc->id) |
		      COMMAND_PORT_TOC |
		      COMMAND_PORT_ROC;

	dw_i3c_master_enqueue_xfer(master, xfer);

	if (dw_i3c_status_poll_timeout(master) > POLL_SUCCESS)
		dw_i3c_master_dequeue_xfer(master, xfer);
	else
		dw_i3c_master_irq_handler(master);

	ret = xfer->ret;

	if (xfer->cmds[0].error == RESPONSE_ERROR_IBA_NACK)
		ccc->err = I3C_ERROR_M2;
	dw_i3c_master_free_xfer(xfer);

	return ret;
}

static int dw_i3c_master_send_ccc_cmd(struct i3c_master_controller *m,
				      struct i3c_ccc_cmd *ccc)
{
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	int ret = 0;

	if (ccc->id == I3C_CCC_ENTDAA)
		return -EINVAL;

	if (ccc->rnw)
		ret = dw_i3c_ccc_get(master, ccc);
	else
		ret = dw_i3c_ccc_set(master, ccc);

	return ret;
}

static int dw_i3c_master_daa(struct i3c_master_controller *m)
{
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	struct dw_i3c_xfer *xfer;
	struct dw_i3c_cmd *cmd;
	u32 olddevs, newdevs;
	u8 p, last_addr = 0;
	int ret, pos;

	olddevs = ~(master->free_pos);

	/* Prepare DAT before launching DAA. */
	for (pos = 0; pos < master->maxdevs; pos++) {
		if (olddevs & BIT(pos))
			continue;

		ret = i3c_master_get_free_addr(m, last_addr + 1);
		if (ret < 0)
			return -ENOSPC;

		master->addrs[pos] = ret;
		p = even_parity(ret);
		last_addr = ret;
		ret |= (p << 7);

		writel(DEV_ADDR_TABLE_DYNAMIC_ADDR(ret),
		       master->regs +
		       DEV_ADDR_TABLE_LOC(master->datstartaddr, pos));
	}

	xfer = dw_i3c_master_alloc_xfer(master, 1);
	if (!xfer)
		return -ENOMEM;

	pos = dw_i3c_master_get_free_pos(master);
	cmd = &xfer->cmds[0];
	cmd->cmd_hi = 0x1;
	cmd->cmd_lo = COMMAND_PORT_DEV_COUNT(master->maxdevs - pos) |
		      COMMAND_PORT_DEV_INDEX(pos) |
		      COMMAND_PORT_CMD(I3C_CCC_ENTDAA) |
		      COMMAND_PORT_ADDR_ASSGN_CMD |
		      COMMAND_PORT_TOC |
		      COMMAND_PORT_ROC;

	dw_i3c_master_enqueue_xfer(master, xfer);

	if (dw_i3c_status_poll_timeout(master) > POLL_SUCCESS)
		dw_i3c_master_dequeue_xfer(master, xfer);
	else
		dw_i3c_master_irq_handler(master);

	newdevs = GENMASK(master->maxdevs - cmd->rx_len - 1, 0);
	newdevs &= ~olddevs;

	for (pos = 0; pos < master->maxdevs; pos++) {
		if (newdevs & BIT(pos)) {
			i3c_master_add_i3c_dev_locked(m, master->addrs[pos]);
			master->i3cdev[pos] = m->this;
			master->num_i3cdevs++;
		}
	}

	dw_i3c_master_free_xfer(xfer);

	return 0;
}

static int dw_i3c_master_priv_xfers(struct i3c_dev_desc *dev,
				    struct i3c_priv_xfer *i3c_xfers,
				    u32 i3c_nxfers)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	unsigned int nrxwords = 0, ntxwords = 0;
	struct dw_i3c_xfer *xfer;
	int i, ret = 0;

	if (!i3c_nxfers)
		return 0;

	if (i3c_nxfers > master->caps.cmdfifodepth)
		return -EOPNOTSUPP;

	for (i = 0; i < i3c_nxfers; i++) {
		if (i3c_xfers[i].rnw)
			nrxwords += DIV_ROUND_UP(i3c_xfers[i].len, 4);
		else
			ntxwords += DIV_ROUND_UP(i3c_xfers[i].len, 4);
	}

	if (ntxwords > master->caps.datafifodepth ||
	    nrxwords > master->caps.datafifodepth)
		return -EOPNOTSUPP;

	xfer = dw_i3c_master_alloc_xfer(master, i3c_nxfers);
	if (!xfer)
		return -ENOMEM;

	for (i = 0; i < i3c_nxfers; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		cmd->cmd_hi = COMMAND_PORT_ARG_DATA_LEN(i3c_xfers[i].len) |
			COMMAND_PORT_TRANSFER_ARG;

		if (i3c_xfers[i].rnw) {
			cmd->rx_buf = i3c_xfers[i].data.in;
			cmd->rx_len = i3c_xfers[i].len;
			cmd->cmd_lo = COMMAND_PORT_READ_TRANSFER |
				      COMMAND_PORT_SPEED(dev->info.max_read_ds);

		} else {
			cmd->tx_buf = i3c_xfers[i].data.out;
			cmd->tx_len = i3c_xfers[i].len;
			cmd->cmd_lo =
				COMMAND_PORT_SPEED(dev->info.max_write_ds);
		}

		cmd->cmd_lo |= COMMAND_PORT_TID(i) |
			       COMMAND_PORT_DEV_INDEX(data->index) |
			       COMMAND_PORT_ROC;

		if (i == (i3c_nxfers - 1))
			cmd->cmd_lo |= COMMAND_PORT_TOC;
	}

	dw_i3c_master_enqueue_xfer(master, xfer);

	if (dw_i3c_status_poll_timeout(master) > POLL_SUCCESS)
		dw_i3c_master_dequeue_xfer(master, xfer);
	else
		dw_i3c_master_irq_handler(master);

	ret = xfer->ret;

	dw_i3c_master_free_xfer(xfer);

	return ret;
}

static int dw_i3c_master_reattach_i3c_dev(struct i3c_dev_desc *dev,
					  u8 old_dyn_addr)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	int pos;

	pos = dw_i3c_master_get_free_pos(master);

	if (data->index > pos && pos > 0) {
		writel(0,
		       master->regs +
		       DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

		master->addrs[data->index] = 0;
		master->free_pos |= BIT(data->index);

		data->index = pos;
		master->addrs[pos] = dev->info.dyn_addr;
		master->free_pos &= ~BIT(pos);
	}

	writel(DEV_ADDR_TABLE_DYNAMIC_ADDR(dev->info.dyn_addr),
	       master->regs +
	       DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	master->addrs[data->index] = dev->info.dyn_addr;

	return 0;
}

static int dw_i3c_master_attach_i3c_dev(struct i3c_dev_desc *dev)
{
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	struct dw_i3c_i2c_dev_data *data;
	int pos;

	pos = dw_i3c_master_get_free_pos(master);
	if (pos < 0)
		return pos;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->index = pos;
	master->addrs[pos] = dev->info.dyn_addr ? : dev->info.static_addr;
	master->free_pos &= ~BIT(pos);
	i3c_dev_set_master_data(dev, data);

	writel(DEV_ADDR_TABLE_DYNAMIC_ADDR(master->addrs[pos]),
	       master->regs +
	       DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	return 0;
}

static void dw_i3c_master_detach_i3c_dev(struct i3c_dev_desc *dev)
{
	struct dw_i3c_i2c_dev_data *data = i3c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i3c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);

	writel(0,
	       master->regs +
	       DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	i3c_dev_set_master_data(dev, NULL);
	master->addrs[data->index] = 0;
	master->free_pos |= BIT(data->index);
	kfree(data);
}

static int dw_i3c_master_i2c_xfers(struct i2c_dev_desc *dev,
				   const struct i2c_msg *i2c_xfers,
				   int i2c_nxfers)
{
	struct dw_i3c_i2c_dev_data *data = i2c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i2c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	unsigned int nrxwords = 0, ntxwords = 0;
	struct dw_i3c_xfer *xfer;
	int i, ret = 0;

	if (!i2c_nxfers)
		return 0;

	if (i2c_nxfers > master->caps.cmdfifodepth)
		return -EOPNOTSUPP;

	for (i = 0; i < i2c_nxfers; i++) {
		if (i2c_xfers[i].flags & I2C_M_RD)
			nrxwords += DIV_ROUND_UP(i2c_xfers[i].len, 4);
		else
			ntxwords += DIV_ROUND_UP(i2c_xfers[i].len, 4);
	}

	if (ntxwords > master->caps.datafifodepth ||
	    nrxwords > master->caps.datafifodepth)
		return -EOPNOTSUPP;

	xfer = dw_i3c_master_alloc_xfer(master, i2c_nxfers);
	if (!xfer)
		return -ENOMEM;

	for (i = 0; i < i2c_nxfers; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		cmd->cmd_hi = COMMAND_PORT_ARG_DATA_LEN(i2c_xfers[i].len) |
			COMMAND_PORT_TRANSFER_ARG;

		cmd->cmd_lo = COMMAND_PORT_TID(i) |
			      COMMAND_PORT_DEV_INDEX(data->index) |
			      COMMAND_PORT_ROC;

		if (i2c_xfers[i].flags & I2C_M_RD) {
			cmd->cmd_lo |= COMMAND_PORT_READ_TRANSFER;
			cmd->rx_buf = i2c_xfers[i].buf;
			cmd->rx_len = i2c_xfers[i].len;
		} else {
			cmd->tx_buf = i2c_xfers[i].buf;
			cmd->tx_len = i2c_xfers[i].len;
		}

		if (i == (i2c_nxfers - 1))
			cmd->cmd_lo |= COMMAND_PORT_TOC;
	}

	dw_i3c_master_enqueue_xfer(master, xfer);

	if (dw_i3c_status_poll_timeout(master) > POLL_SUCCESS)
		dw_i3c_master_dequeue_xfer(master, xfer);
	else
		dw_i3c_master_irq_handler(master);

	ret = xfer->ret;
	dw_i3c_master_free_xfer(xfer);

	return ret;
}

static int dw_i3c_master_attach_i2c_dev(struct i2c_dev_desc *dev)
{
	struct i3c_master_controller *m = i2c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);
	struct dw_i3c_i2c_dev_data *data;
	int pos;

	pos = dw_i3c_master_get_free_pos(master);
	if (pos < 0)
		return pos;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->index = pos;
	master->addrs[pos] = dev->addr;
	master->free_pos &= ~BIT(pos);
	i2c_dev_set_master_data(dev, data);

	writel(DEV_ADDR_TABLE_LEGACY_I2C_DEV |
	       DEV_ADDR_TABLE_STATIC_ADDR(dev->addr),
	       master->regs +
	       DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	return 0;
}

static void dw_i3c_master_detach_i2c_dev(struct i2c_dev_desc *dev)
{
	struct dw_i3c_i2c_dev_data *data = i2c_dev_get_master_data(dev);
	struct i3c_master_controller *m = i2c_dev_get_master(dev);
	struct dw_i3c_master *master = to_dw_i3c_master(m);

	writel(0,
	       master->regs +
	       DEV_ADDR_TABLE_LOC(master->datstartaddr, data->index));

	i2c_dev_set_master_data(dev, NULL);
	master->addrs[data->index] = 0;
	master->free_pos |= BIT(data->index);
	kfree(data);
}

static const struct i3c_master_controller_ops dw_mipi_i3c_ops = {
	.bus_init = dw_i3c_master_bus_init,
	.bus_cleanup = dw_i3c_master_bus_cleanup,
	.attach_i3c_dev = dw_i3c_master_attach_i3c_dev,
	.reattach_i3c_dev = dw_i3c_master_reattach_i3c_dev,
	.detach_i3c_dev = dw_i3c_master_detach_i3c_dev,
	.do_daa = dw_i3c_master_daa,
	.supports_ccc_cmd = dw_i3c_master_supports_ccc_cmd,
	.send_ccc_cmd = dw_i3c_master_send_ccc_cmd,
	.priv_xfers = dw_i3c_master_priv_xfers,
	.attach_i2c_dev = dw_i3c_master_attach_i2c_dev,
	.detach_i2c_dev = dw_i3c_master_detach_i2c_dev,
	.i2c_xfers = dw_i3c_master_i2c_xfers,
};

static int dw_i3c_probe(struct udevice *dev)
{
	struct dw_i3c_master *master = dev_get_priv(dev);
	int ret;

	if (!master->regs)
		master->regs = dev_read_addr_ptr(dev);

	ret = clk_get_by_index(dev, 0, &master->core_clk);
	if (ret) {
		dev_err(dev, "Can't get clock: %d\n", ret);
		return ret;
	}

	ret = reset_get_bulk(dev, &master->resets);
	if (ret) {
		dev_err(dev, "Can't get reset: %d\n", ret);
		return ret;
	}

	reset_deassert_bulk(&master->resets);

	spin_lock_init(&master->xferqueue.lock);
	INIT_LIST_HEAD(&master->xferqueue.list);

	writel(INTR_ALL, master->regs + INTR_STATUS);

	/* Information regarding the FIFOs/QUEUEs depth */
	ret = readl(master->regs + QUEUE_STATUS_LEVEL);
	master->caps.cmdfifodepth = QUEUE_STATUS_LEVEL_CMD(ret);

	ret = readl(master->regs + DATA_BUFFER_STATUS_LEVEL);
	master->caps.datafifodepth = DATA_BUFFER_STATUS_LEVEL_TX(ret);

	ret = readl(master->regs + DEVICE_ADDR_TABLE_POINTER);
	master->datstartaddr = ret;
	master->maxdevs = ret >> 16;
	master->free_pos = GENMASK(master->maxdevs - 1, 0);

	ret = i3c_master_register(&master->base, dev,
				  &dw_mipi_i3c_ops, false);
	if (ret)
		goto err_assert_rst;

	return 0;

err_assert_rst:
	reset_assert_bulk(&master->resets);

	return ret;
}

static int dw_i3c_master_priv_read(struct udevice *dev, u32 dev_number,
				   u8 *buf, u32 buf_size)
{
	struct dw_i3c_master *master = dev_get_priv(dev);
	struct i3c_dev_desc *i3cdev = master->i3cdev[dev_number];
	struct i3c_priv_xfer i3c_xfers;

	i3c_xfers.data.in = buf;
	i3c_xfers.len = buf_size;
	i3c_xfers.rnw = I3C_MSG_READ;

	return dw_i3c_master_priv_xfers(i3cdev, &i3c_xfers, 1);
}

static int dw_i3c_master_priv_write(struct udevice *dev, u32 dev_number,
				    u8 *buf, u32 buf_size)
{
	struct dw_i3c_master *master = dev_get_priv(dev);
	struct i3c_dev_desc *i3cdev = master->i3cdev[dev_number];
	struct i3c_priv_xfer i3c_xfers;

	i3c_xfers.data.out = buf;
	i3c_xfers.len = buf_size;
	i3c_xfers.rnw = I3C_MSG_WRITE;

	return dw_i3c_master_priv_xfers(i3cdev, &i3c_xfers, 1);
}

static const struct dm_i3c_ops dw_i3c_ops = {
	.i3c_xfers = dw_i3c_master_priv_xfers,
	.read = dw_i3c_master_priv_read,
	.write = dw_i3c_master_priv_write,
};

static const struct udevice_id dw_i3c_ids[] = {
	{ .compatible = "snps,dw-i3c-master-1.00a" },
	{ }
};

U_BOOT_DRIVER(dw_i3c_driver) = {
	.name = "dw-i3c-master",
	.id = UCLASS_I3C,
	.of_match = dw_i3c_ids,
	.probe = dw_i3c_probe,
	.priv_auto = sizeof(struct dw_i3c_master),
	.ops    = &dw_i3c_ops,
};
