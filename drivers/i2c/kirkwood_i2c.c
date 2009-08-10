/*
 * Driver for the i2c controller on the Marvell line of host bridges
 * (e.g, gt642[46]0, mv643[46]0, mv644[46]0, Orion SoC family),
 * and Kirkwood family.
 *
 * Based on:
 * Author: Mark A. Greer <mgreer@mvista.com>
 *
 * 2005 (c) MontaVista, Software, Inc.  This file is licensed under
 * the terms of the GNU General Public License version 2.  This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 *
 * ported from Linux to u-boot
 * (C) Copyright 2009
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 */
#include <common.h>
#include <i2c.h>
#include <asm/arch/kirkwood.h>
#include <asm/errno.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static unsigned int i2c_bus_num __attribute__ ((section (".data"))) = 0;
#if defined(CONFIG_I2C_MUX)
static unsigned int i2c_bus_num_mux __attribute__ ((section ("data"))) = 0;
#endif

/* Register defines */
#define	KW_I2C_REG_SLAVE_ADDR			0x00
#define	KW_I2C_REG_DATA				0x04
#define	KW_I2C_REG_CONTROL			0x08
#define	KW_I2C_REG_STATUS			0x0c
#define	KW_I2C_REG_BAUD				0x0c
#define	KW_I2C_REG_EXT_SLAVE_ADDR		0x10
#define	KW_I2C_REG_SOFT_RESET			0x1c

#define	KW_I2C_REG_CONTROL_ACK			0x00000004
#define	KW_I2C_REG_CONTROL_IFLG			0x00000008
#define	KW_I2C_REG_CONTROL_STOP			0x00000010
#define	KW_I2C_REG_CONTROL_START		0x00000020
#define	KW_I2C_REG_CONTROL_TWSIEN		0x00000040
#define	KW_I2C_REG_CONTROL_INTEN		0x00000080

/* Ctlr status values */
#define	KW_I2C_STATUS_BUS_ERR			0x00
#define	KW_I2C_STATUS_MAST_START		0x08
#define	KW_I2C_STATUS_MAST_REPEAT_START		0x10
#define	KW_I2C_STATUS_MAST_WR_ADDR_ACK		0x18
#define	KW_I2C_STATUS_MAST_WR_ADDR_NO_ACK	0x20
#define	KW_I2C_STATUS_MAST_WR_ACK		0x28
#define	KW_I2C_STATUS_MAST_WR_NO_ACK		0x30
#define	KW_I2C_STATUS_MAST_LOST_ARB		0x38
#define	KW_I2C_STATUS_MAST_RD_ADDR_ACK		0x40
#define	KW_I2C_STATUS_MAST_RD_ADDR_NO_ACK	0x48
#define	KW_I2C_STATUS_MAST_RD_DATA_ACK		0x50
#define	KW_I2C_STATUS_MAST_RD_DATA_NO_ACK	0x58
#define	KW_I2C_STATUS_MAST_WR_ADDR_2_ACK	0xd0
#define	KW_I2C_STATUS_MAST_WR_ADDR_2_NO_ACK	0xd8
#define	KW_I2C_STATUS_MAST_RD_ADDR_2_ACK	0xe0
#define	KW_I2C_STATUS_MAST_RD_ADDR_2_NO_ACK	0xe8
#define	KW_I2C_STATUS_NO_STATUS			0xf8

/* Driver states */
enum {
	KW_I2C_STATE_INVALID,
	KW_I2C_STATE_IDLE,
	KW_I2C_STATE_WAITING_FOR_START_COND,
	KW_I2C_STATE_WAITING_FOR_ADDR_1_ACK,
	KW_I2C_STATE_WAITING_FOR_ADDR_2_ACK,
	KW_I2C_STATE_WAITING_FOR_SLAVE_ACK,
	KW_I2C_STATE_WAITING_FOR_SLAVE_DATA,
};

/* Driver actions */
enum {
	KW_I2C_ACTION_INVALID,
	KW_I2C_ACTION_CONTINUE,
	KW_I2C_ACTION_SEND_START,
	KW_I2C_ACTION_SEND_ADDR_1,
	KW_I2C_ACTION_SEND_ADDR_2,
	KW_I2C_ACTION_SEND_DATA,
	KW_I2C_ACTION_RCV_DATA,
	KW_I2C_ACTION_RCV_DATA_STOP,
	KW_I2C_ACTION_SEND_STOP,
};

/* defines to get compatible with Linux driver */
#define IRQ_NONE	0x0
#define IRQ_HANDLED	0x01

#define I2C_M_TEN	0x01
#define I2C_M_RD	0x02
#define	I2C_M_REV_DIR_ADDR	0x04;

struct i2c_msg {
	u32	addr;
	u32	flags;
	u8	*buf;
	u32	len;
};

struct kirkwood_i2c_data {
	int			irq;
	u32			state;
	u32			action;
	u32			aborting;
	u32			cntl_bits;
	void			*reg_base;
	u32			reg_base_p;
	u32			reg_size;
	u32			addr1;
	u32			addr2;
	u32			bytes_left;
	u32			byte_posn;
	u32			block;
	int			rc;
	u32			freq_m;
	u32			freq_n;
	struct i2c_msg		*msg;
};

static struct kirkwood_i2c_data __drv_data __attribute__ ((section (".data")));
static struct kirkwood_i2c_data *drv_data = &__drv_data;
static struct i2c_msg __i2c_msg __attribute__ ((section (".data")));
static struct i2c_msg *kirkwood_i2c_msg = &__i2c_msg;

/*
 *****************************************************************************
 *
 *	Finite State Machine & Interrupt Routines
 *
 *****************************************************************************
 */

static inline int abs(int n)
{
	 if(n >= 0)
		return n;
	else
		return n * -1;
}

static void kirkwood_calculate_speed(int speed)
{
	int	calcspeed;
	int	diff;
	int	best_diff = CONFIG_SYS_TCLK;
	int	best_speed = 0;
	int	m, n;
	int	tmp[8] = {2, 4, 8, 16, 32, 64, 128, 256};

	for (n = 0; n < 8; n++) {
		for (m = 0; m < 16; m++) {
			calcspeed = CONFIG_SYS_TCLK / (10 * (m + 1) * tmp[n]);
			diff = abs((speed - calcspeed));
			if ( diff < best_diff) {
				best_diff = diff;
				best_speed = calcspeed;
				drv_data->freq_m = m;
				drv_data->freq_n = n;
			}
		}
	}
}

/* Reset hardware and initialize FSM */
static void
kirkwood_i2c_hw_init(int speed, int slaveadd)
{
	drv_data->state = KW_I2C_STATE_IDLE;

	kirkwood_calculate_speed(speed);
	writel(0, CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_SOFT_RESET);
	writel((((drv_data->freq_m & 0xf) << 3) | (drv_data->freq_n & 0x7)),
		CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_BAUD);
	writel(slaveadd, CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_SLAVE_ADDR);
	writel(0, CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_EXT_SLAVE_ADDR);
	writel(KW_I2C_REG_CONTROL_TWSIEN | KW_I2C_REG_CONTROL_STOP,
		CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
}

static void
kirkwood_i2c_fsm(u32 status)
{
	/*
	 * If state is idle, then this is likely the remnants of an old
	 * operation that driver has given up on or the user has killed.
	 * If so, issue the stop condition and go to idle.
	 */
	if (drv_data->state == KW_I2C_STATE_IDLE) {
		drv_data->action = KW_I2C_ACTION_SEND_STOP;
		return;
	}

	/* The status from the ctlr [mostly] tells us what to do next */
	switch (status) {
	/* Start condition interrupt */
	case KW_I2C_STATUS_MAST_START: /* 0x08 */
	case KW_I2C_STATUS_MAST_REPEAT_START: /* 0x10 */
		drv_data->action = KW_I2C_ACTION_SEND_ADDR_1;
		drv_data->state = KW_I2C_STATE_WAITING_FOR_ADDR_1_ACK;
		break;

	/* Performing a write */
	case KW_I2C_STATUS_MAST_WR_ADDR_ACK: /* 0x18 */
		if (drv_data->msg->flags & I2C_M_TEN) {
			drv_data->action = KW_I2C_ACTION_SEND_ADDR_2;
			drv_data->state =
				KW_I2C_STATE_WAITING_FOR_ADDR_2_ACK;
			break;
		}
		/* FALLTHRU */
	case KW_I2C_STATUS_MAST_WR_ADDR_2_ACK: /* 0xd0 */
	case KW_I2C_STATUS_MAST_WR_ACK: /* 0x28 */
		if ((drv_data->bytes_left == 0)
				|| (drv_data->aborting
					&& (drv_data->byte_posn != 0))) {
			drv_data->action = KW_I2C_ACTION_SEND_STOP;
			drv_data->state = KW_I2C_STATE_IDLE;
		} else {
			drv_data->action = KW_I2C_ACTION_SEND_DATA;
			drv_data->state =
				KW_I2C_STATE_WAITING_FOR_SLAVE_ACK;
			drv_data->bytes_left--;
		}
		break;

	/* Performing a read */
	case KW_I2C_STATUS_MAST_RD_ADDR_ACK: /* 40 */
		if (drv_data->msg->flags & I2C_M_TEN) {
			drv_data->action = KW_I2C_ACTION_SEND_ADDR_2;
			drv_data->state =
				KW_I2C_STATE_WAITING_FOR_ADDR_2_ACK;
			break;
		}
		/* FALLTHRU */
	case KW_I2C_STATUS_MAST_RD_ADDR_2_ACK: /* 0xe0 */
		if (drv_data->bytes_left == 0) {
			drv_data->action = KW_I2C_ACTION_SEND_STOP;
			drv_data->state = KW_I2C_STATE_IDLE;
			break;
		}
		/* FALLTHRU */
	case KW_I2C_STATUS_MAST_RD_DATA_ACK: /* 0x50 */
		if (status != KW_I2C_STATUS_MAST_RD_DATA_ACK)
			drv_data->action = KW_I2C_ACTION_CONTINUE;
		else {
			drv_data->action = KW_I2C_ACTION_RCV_DATA;
			drv_data->bytes_left--;
		}
		drv_data->state = KW_I2C_STATE_WAITING_FOR_SLAVE_DATA;

		if ((drv_data->bytes_left == 1) || drv_data->aborting)
			drv_data->cntl_bits &= ~KW_I2C_REG_CONTROL_ACK;
		break;

	case KW_I2C_STATUS_MAST_RD_DATA_NO_ACK: /* 0x58 */
		drv_data->action = KW_I2C_ACTION_RCV_DATA_STOP;
		drv_data->state = KW_I2C_STATE_IDLE;
		break;

	case KW_I2C_STATUS_MAST_WR_ADDR_NO_ACK: /* 0x20 */
	case KW_I2C_STATUS_MAST_WR_NO_ACK: /* 30 */
	case KW_I2C_STATUS_MAST_RD_ADDR_NO_ACK: /* 48 */
		/* Doesn't seem to be a device at other end */
		drv_data->action = KW_I2C_ACTION_SEND_STOP;
		drv_data->state = KW_I2C_STATE_IDLE;
		drv_data->rc = -ENODEV;
		break;

	default:
		printf("kirkwood_i2c_fsm: Ctlr Error -- state: 0x%x, "
			"status: 0x%x, addr: 0x%x, flags: 0x%x\n",
			 drv_data->state, status, drv_data->msg->addr,
			 drv_data->msg->flags);
		drv_data->action = KW_I2C_ACTION_SEND_STOP;
		kirkwood_i2c_hw_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
		drv_data->rc = -EIO;
	}
}

static void
kirkwood_i2c_do_action(void)
{
	switch(drv_data->action) {
	case KW_I2C_ACTION_CONTINUE:
		writel(drv_data->cntl_bits,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
		break;

	case KW_I2C_ACTION_SEND_START:
		writel(drv_data->cntl_bits | KW_I2C_REG_CONTROL_START,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
		break;

	case KW_I2C_ACTION_SEND_ADDR_1:
		writel(drv_data->addr1,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_DATA);
		writel(drv_data->cntl_bits,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
		break;

	case KW_I2C_ACTION_SEND_ADDR_2:
		writel(drv_data->addr2,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_DATA);
		writel(drv_data->cntl_bits,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
		break;

	case KW_I2C_ACTION_SEND_DATA:
		writel(drv_data->msg->buf[drv_data->byte_posn++],
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_DATA);
		writel(drv_data->cntl_bits,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
		break;

	case KW_I2C_ACTION_RCV_DATA:
		drv_data->msg->buf[drv_data->byte_posn++] =
			readl(CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_DATA);
		writel(drv_data->cntl_bits,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
		break;

	case KW_I2C_ACTION_RCV_DATA_STOP:
		drv_data->msg->buf[drv_data->byte_posn++] =
			readl(CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_DATA);
		drv_data->cntl_bits &= ~KW_I2C_REG_CONTROL_INTEN;
		writel(drv_data->cntl_bits | KW_I2C_REG_CONTROL_STOP,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
		drv_data->block = 0;
		break;

	case KW_I2C_ACTION_INVALID:
	default:
		printf("kirkwood_i2c_do_action: Invalid action: %d\n",
			drv_data->action);
		drv_data->rc = -EIO;
		/* FALLTHRU */
	case KW_I2C_ACTION_SEND_STOP:
		drv_data->cntl_bits &= ~KW_I2C_REG_CONTROL_INTEN;
		writel(drv_data->cntl_bits | KW_I2C_REG_CONTROL_STOP,
			CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
		drv_data->block = 0;
		break;
	}
}

static	int
kirkwood_i2c_intr(void)
{
	u32		status;
	u32		ctrl;
	int		rc = IRQ_NONE;

	ctrl = readl(CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
	while ((ctrl & KW_I2C_REG_CONTROL_IFLG) &&
		(drv_data->rc == 0)) {
		status = readl(CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_STATUS);
		kirkwood_i2c_fsm(status);
		kirkwood_i2c_do_action();
		rc = IRQ_HANDLED;
		ctrl = readl(CONFIG_I2C_KW_REG_BASE + KW_I2C_REG_CONTROL);
		udelay(1000);
	}
	return rc;
}

static void
kirkwood_i2c_doio(struct i2c_msg *msg)
{
	int	ret;

	while ((drv_data->rc == 0) && (drv_data->state != KW_I2C_STATE_IDLE)) {
		/* poll Status register */
		ret = kirkwood_i2c_intr();
		if (ret == IRQ_NONE)
			udelay(10);
	}
}

static void
kirkwood_i2c_prepare_for_io(struct i2c_msg *msg)
{
	u32	dir = 0;

	drv_data->msg = msg;
	drv_data->byte_posn = 0;
	drv_data->bytes_left = msg->len;
	drv_data->aborting = 0;
	drv_data->rc = 0;
	/* in u-boot we use no IRQs */
	drv_data->cntl_bits = KW_I2C_REG_CONTROL_ACK | KW_I2C_REG_CONTROL_TWSIEN;

	if (msg->flags & I2C_M_RD)
		dir = 1;
	if (msg->flags & I2C_M_TEN) {
		drv_data->addr1 = 0xf0 | (((u32)msg->addr & 0x300) >> 7) | dir;
		drv_data->addr2 = (u32)msg->addr & 0xff;
	} else {
		drv_data->addr1 = ((u32)msg->addr & 0x7f) << 1 | dir;
		drv_data->addr2 = 0;
	}
	/* OK, no start it (from kirkwood_i2c_execute_msg())*/
	drv_data->action = KW_I2C_ACTION_SEND_START;
	drv_data->state = KW_I2C_STATE_WAITING_FOR_START_COND;
	drv_data->block = 1;
	kirkwood_i2c_do_action();
}

void
i2c_init(int speed, int slaveadd)
{
	kirkwood_i2c_hw_init(speed, slaveadd);
}

int
i2c_read(u8 dev, uint addr, int alen, u8 *data, int length)
{
	kirkwood_i2c_msg->buf = data;
	kirkwood_i2c_msg->len = length;
	kirkwood_i2c_msg->addr = dev;
	kirkwood_i2c_msg->flags = I2C_M_RD;

	kirkwood_i2c_prepare_for_io(kirkwood_i2c_msg);
	kirkwood_i2c_doio(kirkwood_i2c_msg);
	return drv_data->rc;
}

int
i2c_write(u8 dev, uint addr, int alen, u8 *data, int length)
{
	kirkwood_i2c_msg->buf = data;
	kirkwood_i2c_msg->len = length;
	kirkwood_i2c_msg->addr = dev;
	kirkwood_i2c_msg->flags = 0;

	kirkwood_i2c_prepare_for_io(kirkwood_i2c_msg);
	kirkwood_i2c_doio(kirkwood_i2c_msg);
	return drv_data->rc;
}

int
i2c_probe(uchar chip)
{
	return i2c_read(chip, 0, 0, NULL, 0);
}

int i2c_set_bus_num(unsigned int bus)
{
#if defined(CONFIG_I2C_MUX)
	if (bus < CONFIG_SYS_MAX_I2C_BUS) {
		i2c_bus_num = bus;
	} else {
		int	ret;

		ret = i2x_mux_select_mux(bus);
		if (ret)
			return ret;
		i2c_bus_num = 0;
	}
	i2c_bus_num_mux = bus;
#else
	if (bus > 0) {
		return -1;
	}

	i2c_bus_num = bus;
#endif
	return 0;
}

unsigned int i2c_get_bus_num(void)
{
#if defined(CONFIG_I2C_MUX)
	return i2c_bus_num_mux;
#else
	return i2c_bus_num;
#endif
}
