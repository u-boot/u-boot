// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <common.h>
#include <dm.h>
#include <linux/delay.h>
#include <linux/io.h>

#ifdef CONFIG_MISC_INIT_R

#define FU540_OTP_BASE_ADDR			0x10070000

struct fu540_otp_regs {
	u32 pa;     /* Address input */
	u32 paio;   /* Program address input */
	u32 pas;    /* Program redundancy cell selection input */
	u32 pce;    /* OTP Macro enable input */
	u32 pclk;   /* Clock input */
	u32 pdin;   /* Write data input */
	u32 pdout;  /* Read data output */
	u32 pdstb;  /* Deep standby mode enable input (active low) */
	u32 pprog;  /* Program mode enable input */
	u32 ptc;    /* Test column enable input */
	u32 ptm;    /* Test mode enable input */
	u32 ptm_rep;/* Repair function test mode enable input */
	u32 ptr;    /* Test row enable input */
	u32 ptrim;  /* Repair function enable input */
	u32 pwe;    /* Write enable input (defines program cycle) */
} __packed;

#define BYTES_PER_FUSE				4
#define NUM_FUSES				0x1000

static int fu540_otp_read(int offset, void *buf, int size)
{
	struct fu540_otp_regs *regs = (void __iomem *)FU540_OTP_BASE_ADDR;
	unsigned int i;
	int fuseidx = offset / BYTES_PER_FUSE;
	int fusecount = size / BYTES_PER_FUSE;
	u32 fusebuf[fusecount];

	/* check bounds */
	if (offset < 0 || size < 0)
		return -EINVAL;
	if (fuseidx >= NUM_FUSES)
		return -EINVAL;
	if ((fuseidx + fusecount) > NUM_FUSES)
		return -EINVAL;

	/* init OTP */
	writel(0x01, &regs->pdstb); /* wake up from stand-by */
	writel(0x01, &regs->ptrim); /* enable repair function */
	writel(0x01, &regs->pce);   /* enable input */

	/* read all requested fuses */
	for (i = 0; i < fusecount; i++, fuseidx++) {
		writel(fuseidx, &regs->pa);

		/* cycle clock to read */
		writel(0x01, &regs->pclk);
		mdelay(1);
		writel(0x00, &regs->pclk);
		mdelay(1);

		/* read the value */
		fusebuf[i] = readl(&regs->pdout);
	}

	/* shut down */
	writel(0, &regs->pce);
	writel(0, &regs->ptrim);
	writel(0, &regs->pdstb);

	/* copy out */
	memcpy(buf, fusebuf, size);

	return 0;
}

static u32 fu540_read_serialnum(void)
{
	int ret;
	u32 serial[2] = {0};

	for (int i = 0xfe * 4; i > 0; i -= 8) {
		ret = fu540_otp_read(i, serial, sizeof(serial));
		if (ret) {
			printf("%s: error reading from OTP\n", __func__);
			break;
		}
		if (serial[0] == ~serial[1])
			return serial[0];
	}

	return 0;
}

static void fu540_setup_macaddr(u32 serialnum)
{
	/* Default MAC address */
	unsigned char mac[6] = { 0x70, 0xb3, 0xd5, 0x92, 0xf0, 0x00 };

	/*
	 * We derive our board MAC address by ORing last three bytes
	 * of board serial number to above default MAC address.
	 *
	 * This logic of deriving board MAC address is taken from
	 * SiFive FSBL and is kept unchanged.
	 */
	mac[5] |= (serialnum >>  0) & 0xff;
	mac[4] |= (serialnum >>  8) & 0xff;
	mac[3] |= (serialnum >> 16) & 0xff;

	/* Update environment variable */
	eth_env_set_enetaddr("ethaddr", mac);
}

int misc_init_r(void)
{
	u32 serial_num;
	char buf[9] = {0};

	/* Set ethaddr environment variable from board serial number */
	if (!env_get("serial#")) {
		serial_num = fu540_read_serialnum();
		if (!serial_num) {
			WARN(true, "Board serial number should not be 0 !!\n");
			return 0;
		}
		snprintf(buf, sizeof(buf), "%08x", serial_num);
		env_set("serial#", buf);
		fu540_setup_macaddr(serial_num);
	}
	return 0;
}

#endif

int board_init(void)
{
	/* For now nothing to do here. */

	return 0;
}
