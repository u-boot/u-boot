// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#define pr_fmt(fmt) "%s " fmt, KBUILD_MODNAME

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <dm/ofnode.h>
#include <linux/bitmap.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/types.h>
#include <asm/bitops.h>
#include <asm/io.h>

#include <log.h>

#include <soc/qcom/tcs.h>
#include <dt-bindings/soc/qcom,rpmh-rsc.h>

#include "rpmh-internal.h"


#define RSC_DRV_ID			0

#define MAJOR_VER_MASK			0xFF
#define MAJOR_VER_SHIFT			16
#define MINOR_VER_MASK			0xFF
#define MINOR_VER_SHIFT			8

enum {
	RSC_DRV_TCS_OFFSET,
	RSC_DRV_CMD_OFFSET,
/* DRV HW Solver Configuration Information Register */
	DRV_SOLVER_CONFIG,
/* DRV TCS Configuration Information Register */
	DRV_PRNT_CHLD_CONFIG,
/* Offsets for common TCS Registers, one bit per TCS */
	RSC_DRV_IRQ_ENABLE,
	RSC_DRV_IRQ_STATUS,
	RSC_DRV_IRQ_CLEAR,	/* w/o; write 1 to clear */
/*
 * Offsets for per TCS Registers.
 *
 * TCSes start at 0x10 from tcs_base and are stored one after another.
 * Multiply tcs_id by RSC_DRV_TCS_OFFSET to find a given TCS and add one
 * of the below to find a register.
 */
	RSC_DRV_CMD_WAIT_FOR_CMPL,	/* 1 bit per command */
	RSC_DRV_CONTROL,
	RSC_DRV_STATUS,	/* zero if tcs is busy */
	RSC_DRV_CMD_ENABLE,	/* 1 bit per command */
/*
 * Offsets for per command in a TCS.
 *
 * Commands (up to 16) start at 0x30 in a TCS; multiply command index
 * by RSC_DRV_CMD_OFFSET and add one of the below to find a register.
 */
	RSC_DRV_CMD_MSGID,
	RSC_DRV_CMD_ADDR,
	RSC_DRV_CMD_DATA,
	RSC_DRV_CMD_STATUS,
	RSC_DRV_CMD_RESP_DATA,
};

/* DRV HW Solver Configuration Information Register */
#define DRV_HW_SOLVER_MASK		1
#define DRV_HW_SOLVER_SHIFT		24

/* DRV TCS Configuration Information Register */
#define DRV_NUM_TCS_MASK		0x3F
#define DRV_NUM_TCS_SHIFT		6
#define DRV_NCPT_MASK			0x1F
#define DRV_NCPT_SHIFT			27

/* Offsets for CONTROL TCS Registers */
#define RSC_DRV_CTL_TCS_DATA_HI		0x38
#define RSC_DRV_CTL_TCS_DATA_HI_MASK	0xFFFFFF
#define RSC_DRV_CTL_TCS_DATA_HI_VALID	BIT(31)
#define RSC_DRV_CTL_TCS_DATA_LO		0x40
#define RSC_DRV_CTL_TCS_DATA_LO_MASK	0xFFFFFFFF
#define RSC_DRV_CTL_TCS_DATA_SIZE	32

#define TCS_AMC_MODE_ENABLE		BIT(16)
#define TCS_AMC_MODE_TRIGGER		BIT(24)

/* TCS CMD register bit mask */
#define CMD_MSGID_LEN			8
#define CMD_MSGID_RESP_REQ		BIT(8)
#define CMD_MSGID_WRITE			BIT(16)
#define CMD_STATUS_ISSUED		BIT(8)
#define CMD_STATUS_COMPL		BIT(16)

/*
 * Here's a high level overview of how all the registers in RPMH work
 * together:
 *
 * - The main rpmh-rsc address is the base of a register space that can
 *   be used to find overall configuration of the hardware
 *   (DRV_PRNT_CHLD_CONFIG). Also found within the rpmh-rsc register
 *   space are all the TCS blocks. The offset of the TCS blocks is
 *   specified in the device tree by "qcom,tcs-offset" and used to
 *   compute tcs_base.
 * - TCS blocks come one after another. Type, count, and order are
 *   specified by the device tree as "qcom,tcs-config".
 * - Each TCS block has some registers, then space for up to 16 commands.
 *   Note that though address space is reserved for 16 commands, fewer
 *   might be present. See ncpt (num cmds per TCS).
 *
 * Here's a picture:
 *
 *  +---------------------------------------------------+
 *  |RSC                                                |
 *  | ctrl                                              |
 *  |                                                   |
 *  | Drvs:                                             |
 *  | +-----------------------------------------------+ |
 *  | |DRV0                                           | |
 *  | | ctrl/config                                   | |
 *  | | IRQ                                           | |
 *  | |                                               | |
 *  | | TCSes:                                        | |
 *  | | +------------------------------------------+  | |
 *  | | |TCS0  |  |  |  |  |  |  |  |  |  |  |  |  |  | |
 *  | | | ctrl | 0| 1| 2| 3| 4| 5| .| .| .| .|14|15|  | |
 *  | | |      |  |  |  |  |  |  |  |  |  |  |  |  |  | |
 *  | | +------------------------------------------+  | |
 *  | | +------------------------------------------+  | |
 *  | | |TCS1  |  |  |  |  |  |  |  |  |  |  |  |  |  | |
 *  | | | ctrl | 0| 1| 2| 3| 4| 5| .| .| .| .|14|15|  | |
 *  | | |      |  |  |  |  |  |  |  |  |  |  |  |  |  | |
 *  | | +------------------------------------------+  | |
 *  | | +------------------------------------------+  | |
 *  | | |TCS2  |  |  |  |  |  |  |  |  |  |  |  |  |  | |
 *  | | | ctrl | 0| 1| 2| 3| 4| 5| .| .| .| .|14|15|  | |
 *  | | |      |  |  |  |  |  |  |  |  |  |  |  |  |  | |
 *  | | +------------------------------------------+  | |
 *  | |                    ......                     | |
 *  | +-----------------------------------------------+ |
 *  | +-----------------------------------------------+ |
 *  | |DRV1                                           | |
 *  | | (same as DRV0)                                | |
 *  | +-----------------------------------------------+ |
 *  |                      ......                       |
 *  +---------------------------------------------------+
 */

static u32 rpmh_rsc_reg_offset_ver_2_7[] = {
	[RSC_DRV_TCS_OFFSET]		= 672,
	[RSC_DRV_CMD_OFFSET]		= 20,
	[DRV_SOLVER_CONFIG]		= 0x04,
	[DRV_PRNT_CHLD_CONFIG]		= 0x0C,
	[RSC_DRV_IRQ_ENABLE]		= 0x00,
	[RSC_DRV_IRQ_STATUS]		= 0x04,
	[RSC_DRV_IRQ_CLEAR]		= 0x08,
	[RSC_DRV_CMD_WAIT_FOR_CMPL]	= 0x10,
	[RSC_DRV_CONTROL]		= 0x14,
	[RSC_DRV_STATUS]		= 0x18,
	[RSC_DRV_CMD_ENABLE]		= 0x1C,
	[RSC_DRV_CMD_MSGID]		= 0x30,
	[RSC_DRV_CMD_ADDR]		= 0x34,
	[RSC_DRV_CMD_DATA]		= 0x38,
	[RSC_DRV_CMD_STATUS]		= 0x3C,
	[RSC_DRV_CMD_RESP_DATA]		= 0x40,
};

static u32 rpmh_rsc_reg_offset_ver_3_0[] = {
	[RSC_DRV_TCS_OFFSET]		= 672,
	[RSC_DRV_CMD_OFFSET]		= 24,
	[DRV_SOLVER_CONFIG]		= 0x04,
	[DRV_PRNT_CHLD_CONFIG]		= 0x0C,
	[RSC_DRV_IRQ_ENABLE]		= 0x00,
	[RSC_DRV_IRQ_STATUS]		= 0x04,
	[RSC_DRV_IRQ_CLEAR]		= 0x08,
	[RSC_DRV_CMD_WAIT_FOR_CMPL]	= 0x20,
	[RSC_DRV_CONTROL]		= 0x24,
	[RSC_DRV_STATUS]		= 0x28,
	[RSC_DRV_CMD_ENABLE]		= 0x2C,
	[RSC_DRV_CMD_MSGID]		= 0x34,
	[RSC_DRV_CMD_ADDR]		= 0x38,
	[RSC_DRV_CMD_DATA]		= 0x3C,
	[RSC_DRV_CMD_STATUS]		= 0x40,
	[RSC_DRV_CMD_RESP_DATA]		= 0x44,
};

static inline void __iomem *
tcs_reg_addr(const struct rsc_drv *drv, int reg, int tcs_id)
{
	return drv->tcs_base + drv->regs[RSC_DRV_TCS_OFFSET] * tcs_id + reg;
}

static inline void __iomem *
tcs_cmd_addr(const struct rsc_drv *drv, int reg, int tcs_id, int cmd_id)
{
	return tcs_reg_addr(drv, reg, tcs_id) + drv->regs[RSC_DRV_CMD_OFFSET] * cmd_id;
}

static u32 read_tcs_cmd(const struct rsc_drv *drv, int reg, int tcs_id,
			int cmd_id)
{
	return readl_relaxed(tcs_cmd_addr(drv, reg, tcs_id, cmd_id));
}

static u32 read_tcs_reg(const struct rsc_drv *drv, int reg, int tcs_id)
{
	return readl_relaxed(tcs_reg_addr(drv, reg, tcs_id));
}

static void write_tcs_cmd(const struct rsc_drv *drv, int reg, int tcs_id,
			  int cmd_id, u32 data)
{
	writel_relaxed(data, tcs_cmd_addr(drv, reg, tcs_id, cmd_id));
}

static void write_tcs_reg(const struct rsc_drv *drv, int reg, int tcs_id,
			  u32 data)
{
	writel_relaxed(data, tcs_reg_addr(drv, reg, tcs_id));
}

static void write_tcs_reg_sync(const struct rsc_drv *drv, int reg, int tcs_id,
			       u32 data)
{
	int i;

	writel(data, tcs_reg_addr(drv, reg, tcs_id));

	/*
	 * Wait until we read back the same value.  Use a counter rather than
	 * ktime for timeout since this may be called after timekeeping stops.
	 */
	for (i = 0; i < USEC_PER_SEC; i++) {
		if (readl(tcs_reg_addr(drv, reg, tcs_id)) == data)
			return;
		udelay(1);
	}
	pr_err("%s: error writing %#x to %d:%#x\n", drv->name,
	       data, tcs_id, reg);
}

/**
 * get_tcs_for_msg() - Get the tcs_group used to send the given message.
 * @drv: The RSC controller.
 * @msg: The message we want to send.
 *
 * This is normally pretty straightforward except if we are trying to send
 * an ACTIVE_ONLY message but don't have any active_only TCSes.
 *
 * Return: A pointer to a tcs_group or an ERR_PTR.
 */
static struct tcs_group *get_tcs_for_msg(struct rsc_drv *drv,
					 const struct tcs_request *msg)
{
	/*
	 * U-Boot: since we're single threaded and running synchronously we can
	 * just always used the first active TCS.
	 */
	if (msg->state != RPMH_ACTIVE_ONLY_STATE) {
		log_err("WARN: only ACTIVE_ONLY state supported\n");
		return ERR_PTR(-EINVAL);
	}

	return &drv->tcs[ACTIVE_TCS];
}

/**
 * __tcs_buffer_write() - Write to TCS hardware from a request; don't trigger.
 * @drv:    The controller.
 * @tcs_id: The global ID of this TCS.
 * @cmd_id: The index within the TCS to start writing.
 * @msg:    The message we want to send, which will contain several addr/data
 *          pairs to program (but few enough that they all fit in one TCS).
 *
 * This is used for all types of transfers (active, sleep, and wake).
 */
static void __tcs_buffer_write(struct rsc_drv *drv, int tcs_id, int cmd_id,
			       const struct tcs_request *msg)
{
	u32 msgid;
	u32 cmd_msgid = CMD_MSGID_LEN;
	u32 cmd_enable = 0;
	u32 cmd_complete = 0;
	struct tcs_cmd *cmd;
	int i, j;

	if (!msg->is_read)
		cmd_msgid |= CMD_MSGID_WRITE;

	if (msg->wait_for_compl)
		cmd_msgid |= CMD_MSGID_RESP_REQ;

	cmd_complete = read_tcs_reg(drv, drv->regs[RSC_DRV_CMD_WAIT_FOR_CMPL], tcs_id);

	for (i = 0, j = cmd_id; i < msg->num_cmds; i++, j++) {
		cmd = &msg->cmds[i];
		cmd_enable |= BIT(j);
		/* U-Boot: always wait for completion */
		cmd_complete |= (!!msg->wait_for_compl) << j;
		msgid = cmd_msgid;

		write_tcs_cmd(drv, drv->regs[RSC_DRV_CMD_MSGID], tcs_id, j, msgid);
		write_tcs_cmd(drv, drv->regs[RSC_DRV_CMD_ADDR], tcs_id, j, cmd->addr);
		if (!msg->is_read)
			write_tcs_cmd(drv, drv->regs[RSC_DRV_CMD_DATA], tcs_id, j, cmd->data);
		debug("tcs(%d): [%s] cmd_id: %d: msgid: %#x addr: %#x data: %#x complete: %#x\n",
		      tcs_id, msg->state == RPMH_ACTIVE_ONLY_STATE ? "active" : "?", j, msgid,
		      cmd->addr, cmd->data, cmd_complete);
	}

	cmd_enable |= read_tcs_reg(drv, drv->regs[RSC_DRV_CMD_ENABLE], tcs_id);
	write_tcs_reg(drv, drv->regs[RSC_DRV_CMD_ENABLE], tcs_id, cmd_enable);
	/* U-Boot: Tell the DRV to wait for completion (?) so we can poll on DRV_STATUS */
	/* This register applies to the entire TCS group not per command */
	write_tcs_reg(drv, drv->regs[RSC_DRV_CMD_WAIT_FOR_CMPL], tcs_id, cmd_complete);
}

/**
 * __tcs_set_trigger() - Start xfer on a TCS or unset trigger on a borrowed TCS
 * @drv:     The controller.
 * @tcs_id:  The global ID of this TCS.
 * @trigger: If true then untrigger/retrigger. If false then just untrigger.
 *
 * In the normal case we only ever call with "trigger=true" to start a
 * transfer. That will un-trigger/disable the TCS from the last transfer
 * then trigger/enable for this transfer.
 *
 * If we borrowed a wake TCS for an active-only transfer we'll also call
 * this function with "trigger=false" to just do the un-trigger/disable
 * before using the TCS for wake purposes again.
 *
 * Note that the AP is only in charge of triggering active-only transfers.
 * The AP never triggers sleep/wake values using this function.
 */
static void __tcs_set_trigger(struct rsc_drv *drv, int tcs_id, bool trigger)
{
	u32 enable;
	u32 reg = drv->regs[RSC_DRV_CONTROL];

	/*
	 * HW req: Clear the DRV_CONTROL and enable TCS again
	 * While clearing ensure that the AMC mode trigger is cleared
	 * and then the mode enable is cleared.
	 */
	enable = read_tcs_reg(drv, reg, tcs_id);
	enable &= ~TCS_AMC_MODE_TRIGGER;
	write_tcs_reg_sync(drv, reg, tcs_id, enable);
	enable &= ~TCS_AMC_MODE_ENABLE;
	write_tcs_reg_sync(drv, reg, tcs_id, enable);

	if (trigger) {
		/* Enable the AMC mode on the TCS and then trigger the TCS */
		enable = TCS_AMC_MODE_ENABLE;
		write_tcs_reg_sync(drv, reg, tcs_id, enable);
		enable |= TCS_AMC_MODE_TRIGGER;
		write_tcs_reg(drv, reg, tcs_id, enable);
	}
}

/**
 * rpmh_rsc_send_data() - Write / trigger active-only message.
 * @drv: The controller.
 * @msg: The data to be sent.
 *
 * NOTES:
 * - This is only used for "ACTIVE_ONLY" since the limitations of this
 *   function don't make sense for sleep/wake cases.
 * - To do the transfer, we will grab a whole TCS for ourselves--we don't
 *   try to share. If there are none available we'll wait indefinitely
 *   for a free one.
 * - This function will not wait for the commands to be finished, only for
 *   data to be programmed into the RPMh. See rpmh_tx_done() which will
 *   be called when the transfer is fully complete.
 * - This function must be called with interrupts enabled. If the hardware
 *   is busy doing someone else's transfer we need that transfer to fully
 *   finish so that we can have the hardware, and to fully finish it needs
 *   the interrupt handler to run. If the interrupts is set to run on the
 *   active CPU this can never happen if interrupts are disabled.
 *
 * Return: 0 on success, -EINVAL on error.
 */
int rpmh_rsc_send_data(struct rsc_drv *drv, const struct tcs_request *msg)
{
	struct tcs_group *tcs;
	int tcs_id, i = 0;
	u32 val;

	tcs = get_tcs_for_msg(drv, msg);
	if (IS_ERR(tcs))
		return PTR_ERR(tcs);

	/* U-Boot is single-threaded, always use the first TCS as we'll never conflict */
	tcs_id = tcs->offset;
	if (!read_tcs_reg(drv, drv->regs[RSC_DRV_STATUS], tcs_id)) {
		pr_err("%s: TCS %d is busy!\n", __func__, tcs_id);
		return -EBUSY;
	}

	tcs->req[tcs_id - tcs->offset] = msg;
	generic_set_bit(tcs_id, drv->tcs_in_use);

	/*
	 * These two can be done after the lock is released because:
	 * - We marked "tcs_in_use" under lock.
	 * - Once "tcs_in_use" has been marked nobody else could be writing
	 *   to these registers until the interrupt goes off.
	 * - The interrupt can't go off until we trigger w/ the last line
	 *   of __tcs_set_trigger() below.
	 */
	__tcs_buffer_write(drv, tcs_id, 0, msg);
	__tcs_set_trigger(drv, tcs_id, true);

	/* U-Boot: Now wait for the TCS to be cleared, indicating that we're done. */
	for (i = 0; i < USEC_PER_SEC; i++) {
		val = read_tcs_cmd(drv, drv->regs[RSC_DRV_CMD_STATUS], tcs_id, 0);
		if (val & CMD_STATUS_COMPL)
			break;
		udelay(1);
	}

	/* U-Boot: read the response now we know it's available */
	if (msg->is_read) {
		msg->cmds[0].data = read_tcs_cmd(drv, drv->regs[RSC_DRV_CMD_RESP_DATA], tcs_id, 0);
		log_debug("data response: %#x\n", msg->cmds[0].data);
	}

	__tcs_set_trigger(drv, tcs_id, false);

	/* Reclaim the TCS */
	write_tcs_reg(drv, drv->regs[RSC_DRV_CMD_ENABLE], tcs_id, 0);
	writel_relaxed(BIT(tcs_id), drv->tcs_base + drv->regs[RSC_DRV_IRQ_CLEAR]);
	generic_clear_bit(tcs_id, drv->tcs_in_use);

	if (i == USEC_PER_SEC) {
		log_err("%s: error writing %#x to %d:%#x\n", drv->name,
			msg->cmds[0].addr, tcs_id, drv->regs[RSC_DRV_CMD_ADDR]);
		return -EINVAL;
	}

	return 0;
}

static int rpmh_probe_tcs_config(struct udevice *dev, struct rsc_drv *drv)
{
	struct tcs_type_config {
		u32 type;
		u32 n;
	} tcs_cfg[TCS_TYPE_NR] = { { 0 } };
	ofnode dn = dev_ofnode(dev);
	u32 config, max_tcs, ncpt, offset;
	int i, ret, n, st = 0;
	struct tcs_group *tcs;

	ret = ofnode_read_u32(dn, "qcom,tcs-offset", &offset);
	if (ret)
		return ret;
	drv->tcs_base = drv->base + offset;

	config = readl_relaxed(drv->base + drv->regs[DRV_PRNT_CHLD_CONFIG]);

	max_tcs = config;
	max_tcs &= DRV_NUM_TCS_MASK << (DRV_NUM_TCS_SHIFT * drv->id);
	max_tcs = max_tcs >> (DRV_NUM_TCS_SHIFT * drv->id);

	ncpt = config & (DRV_NCPT_MASK << DRV_NCPT_SHIFT);
	ncpt = ncpt >> DRV_NCPT_SHIFT;

	n = ofnode_read_u32_array(dn, "qcom,tcs-config", (u32 *)tcs_cfg, 2 * TCS_TYPE_NR);
	if (n < 0) {
		log_err("RPMh: %s: error reading qcom,tcs-config %d\n", dev->name, n);
		return n;
	}

	for (i = 0; i < TCS_TYPE_NR; i++) {
		if (tcs_cfg[i].n > MAX_TCS_PER_TYPE)
			return -EINVAL;
	}

	for (i = 0; i < TCS_TYPE_NR; i++) {
		tcs = &drv->tcs[tcs_cfg[i].type];
		if (tcs->drv)
			return -EINVAL;
		tcs->drv = drv;
		tcs->type = tcs_cfg[i].type;
		tcs->num_tcs = tcs_cfg[i].n;
		tcs->ncpt = ncpt;

		if (!tcs->num_tcs || tcs->type == CONTROL_TCS)
			continue;

		if (st + tcs->num_tcs > max_tcs ||
		    st + tcs->num_tcs >= BITS_PER_BYTE * sizeof(tcs->mask))
			return -EINVAL;

		tcs->mask = ((1 << tcs->num_tcs) - 1) << st;
		tcs->offset = st;
		st += tcs->num_tcs;
	}

	drv->num_tcs = st;

	return 0;
}

static int rpmh_rsc_probe(struct udevice *dev)
{
	ofnode dn = dev_ofnode(dev);
	struct rsc_drv *drv;
	char drv_id[10] = {0};
	int ret;
	u32 rsc_id;

	drv = dev_get_priv(dev);

	ret = ofnode_read_u32(dn, "qcom,drv-id", &drv->id);
	if (ret)
		return ret;

	drv->name = ofnode_get_property(dn, "label", NULL);
	if (!drv->name)
		drv->name = dev->name;

	snprintf(drv_id, ARRAY_SIZE(drv_id), "drv-%d", drv->id);
	drv->base = (void __iomem *)dev_read_addr_name(dev, drv_id);
	if (IS_ERR(drv->base))
		return PTR_ERR(drv->base);

	rsc_id = readl_relaxed(drv->base + RSC_DRV_ID);
	drv->ver.major = rsc_id & (MAJOR_VER_MASK << MAJOR_VER_SHIFT);
	drv->ver.major >>= MAJOR_VER_SHIFT;
	drv->ver.minor = rsc_id & (MINOR_VER_MASK << MINOR_VER_SHIFT);
	drv->ver.minor >>= MINOR_VER_SHIFT;

	if (drv->ver.major == 3)
		drv->regs = rpmh_rsc_reg_offset_ver_3_0;
	else
		drv->regs = rpmh_rsc_reg_offset_ver_2_7;

	ret = rpmh_probe_tcs_config(dev, drv);
	if (ret)
		return ret;

	spin_lock_init(&drv->lock);
	init_waitqueue_head(&drv->tcs_wait);
	bitmap_zero(drv->tcs_in_use, MAX_TCS_NR);

	/* Enable the active TCS to send requests immediately */
	writel_relaxed(drv->tcs[ACTIVE_TCS].mask,
		       drv->tcs_base + drv->regs[RSC_DRV_IRQ_ENABLE]);

	spin_lock_init(&drv->client.cache_lock);
	INIT_LIST_HEAD(&drv->client.cache);
	INIT_LIST_HEAD(&drv->client.batch_cache);

	dev_set_drvdata(dev, drv);
	drv->dev = dev;

	log_debug("RPMh: %s: v%d.%d\n", dev->name, drv->ver.major, drv->ver.minor);

	return ret;
}

static const struct udevice_id qcom_rpmh_ids[] = {
	{ .compatible = "qcom,rpmh-rsc" },
	{ }
};

U_BOOT_DRIVER(qcom_rpmh_rsc) = {
	.name		= "qcom_rpmh_rsc",
	.id		= UCLASS_MISC,
	.priv_auto	= sizeof(struct rsc_drv),
	.probe		= rpmh_rsc_probe,
	.of_match	= qcom_rpmh_ids,
	/* rpmh is under CLUSTER_PD which we don't support, so skip trying to enable PDs */
	.flags		= DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

MODULE_DESCRIPTION("Qualcomm Technologies, Inc. RPMh Driver");
MODULE_LICENSE("GPL v2");
