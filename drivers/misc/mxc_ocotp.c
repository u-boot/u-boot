/*
 * (C) Copyright 2013 ADVANSEE
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
 *
 * Based on Dirk Behme's
 * https://github.com/dirkbehme/u-boot-imx6/blob/28b17e9/drivers/misc/imx_otp.c,
 * which is based on Freescale's
 * http://git.freescale.com/git/cgit.cgi/imx/uboot-imx.git/tree/drivers/misc/imx_otp.c?h=imx_v2009.08_1.1.0&id=9aa74e6,
 * which is:
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fuse.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>

#define BO_CTRL_WR_UNLOCK		16
#define BM_CTRL_WR_UNLOCK		0xffff0000
#define BV_CTRL_WR_UNLOCK_KEY		0x3e77
#define BM_CTRL_ERROR			0x00000200
#define BM_CTRL_BUSY			0x00000100
#define BO_CTRL_ADDR			0
#ifdef CONFIG_MX7
#define BM_CTRL_ADDR                    0x0000000f
#define BM_CTRL_RELOAD                  0x00000400
#else
#define BM_CTRL_ADDR			0x0000007f
#endif

#ifdef CONFIG_MX7
#define BO_TIMING_FSOURCE               12
#define BM_TIMING_FSOURCE               0x0007f000
#define BV_TIMING_FSOURCE_NS            1001
#define BO_TIMING_PROG                  0
#define BM_TIMING_PROG                  0x00000fff
#define BV_TIMING_PROG_US               10
#else
#define BO_TIMING_STROBE_READ		16
#define BM_TIMING_STROBE_READ		0x003f0000
#define BV_TIMING_STROBE_READ_NS	37
#define BO_TIMING_RELAX			12
#define BM_TIMING_RELAX			0x0000f000
#define BV_TIMING_RELAX_NS		17
#define BO_TIMING_STROBE_PROG		0
#define BM_TIMING_STROBE_PROG		0x00000fff
#define BV_TIMING_STROBE_PROG_US	10
#endif

#define BM_READ_CTRL_READ_FUSE		0x00000001

#define BF(value, field)		(((value) << BO_##field) & BM_##field)

#define WRITE_POSTAMBLE_US		2

static void wait_busy(struct ocotp_regs *regs, unsigned int delay_us)
{
	while (readl(&regs->ctrl) & BM_CTRL_BUSY)
		udelay(delay_us);
}

static void clear_error(struct ocotp_regs *regs)
{
	writel(BM_CTRL_ERROR, &regs->ctrl_clr);
}

static int prepare_access(struct ocotp_regs **regs, u32 bank, u32 word,
				int assert, const char *caller)
{
	*regs = (struct ocotp_regs *)OCOTP_BASE_ADDR;

	if (bank >= ARRAY_SIZE((*regs)->bank) ||
			word >= ARRAY_SIZE((*regs)->bank[0].fuse_regs) >> 2 ||
			!assert) {
		printf("mxc_ocotp %s(): Invalid argument\n", caller);
		return -EINVAL;
	}

	enable_ocotp_clk(1);

	wait_busy(*regs, 1);
	clear_error(*regs);

	return 0;
}

static int finish_access(struct ocotp_regs *regs, const char *caller)
{
	u32 err;

	err = !!(readl(&regs->ctrl) & BM_CTRL_ERROR);
	clear_error(regs);

	if (err) {
		printf("mxc_ocotp %s(): Access protect error\n", caller);
		return -EIO;
	}

	return 0;
}

static int prepare_read(struct ocotp_regs **regs, u32 bank, u32 word, u32 *val,
			const char *caller)
{
	return prepare_access(regs, bank, word, val != NULL, caller);
}

int fuse_read(u32 bank, u32 word, u32 *val)
{
	struct ocotp_regs *regs;
	int ret;

	ret = prepare_read(&regs, bank, word, val, __func__);
	if (ret)
		return ret;

	*val = readl(&regs->bank[bank].fuse_regs[word << 2]);

	return finish_access(regs, __func__);
}

#ifdef CONFIG_MX7
static void set_timing(struct ocotp_regs *regs)
{
	u32 ipg_clk;
	u32 fsource, prog;
	u32 timing;

	ipg_clk = mxc_get_clock(MXC_IPG_CLK);

	fsource = DIV_ROUND_UP((ipg_clk / 1000) * BV_TIMING_FSOURCE_NS,
			+       1000000) + 1;
	prog = DIV_ROUND_CLOSEST(ipg_clk * BV_TIMING_PROG_US, 1000000) + 1;

	timing = BF(fsource, TIMING_FSOURCE) | BF(prog, TIMING_PROG);

	clrsetbits_le32(&regs->timing, BM_TIMING_FSOURCE | BM_TIMING_PROG,
			timing);
}
#else
static void set_timing(struct ocotp_regs *regs)
{
	u32 ipg_clk;
	u32 relax, strobe_read, strobe_prog;
	u32 timing;

	ipg_clk = mxc_get_clock(MXC_IPG_CLK);

	relax = DIV_ROUND_UP(ipg_clk * BV_TIMING_RELAX_NS, 1000000000) - 1;
	strobe_read = DIV_ROUND_UP(ipg_clk * BV_TIMING_STROBE_READ_NS,
					1000000000) + 2 * (relax + 1) - 1;
	strobe_prog = DIV_ROUND_CLOSEST(ipg_clk * BV_TIMING_STROBE_PROG_US,
						1000000) + 2 * (relax + 1) - 1;

	timing = BF(strobe_read, TIMING_STROBE_READ) |
			BF(relax, TIMING_RELAX) |
			BF(strobe_prog, TIMING_STROBE_PROG);

	clrsetbits_le32(&regs->timing, BM_TIMING_STROBE_READ | BM_TIMING_RELAX |
			BM_TIMING_STROBE_PROG, timing);
}
#endif

static void setup_direct_access(struct ocotp_regs *regs, u32 bank, u32 word,
				int write)
{
	u32 wr_unlock = write ? BV_CTRL_WR_UNLOCK_KEY : 0;
#ifdef CONFIG_MX7
	u32 addr = bank;
#else
	u32 addr = bank << 3 | word;
#endif

	set_timing(regs);
	clrsetbits_le32(&regs->ctrl, BM_CTRL_WR_UNLOCK | BM_CTRL_ADDR,
			BF(wr_unlock, CTRL_WR_UNLOCK) |
			BF(addr, CTRL_ADDR));
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	struct ocotp_regs *regs;
	int ret;

	ret = prepare_read(&regs, bank, word, val, __func__);
	if (ret)
		return ret;

	setup_direct_access(regs, bank, word, false);
	writel(BM_READ_CTRL_READ_FUSE, &regs->read_ctrl);
	wait_busy(regs, 1);
#ifdef CONFIG_MX7
	*val = readl((&regs->read_fuse_data0) + (word << 2));
#else
	*val = readl(&regs->read_fuse_data);
#endif

	return finish_access(regs, __func__);
}

static int prepare_write(struct ocotp_regs **regs, u32 bank, u32 word,
				const char *caller)
{
	return prepare_access(regs, bank, word, true, caller);
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	struct ocotp_regs *regs;
	int ret;

	ret = prepare_write(&regs, bank, word, __func__);
	if (ret)
		return ret;

	setup_direct_access(regs, bank, word, true);
#ifdef CONFIG_MX7
	switch (word) {
	case 0:
		writel(0, &regs->data1);
		writel(0, &regs->data2);
		writel(0, &regs->data3);
		writel(val, &regs->data0);
		break;
	case 1:
		writel(val, &regs->data1);
		writel(0, &regs->data2);
		writel(0, &regs->data3);
		writel(0, &regs->data0);
		break;
	case 2:
		writel(0, &regs->data1);
		writel(val, &regs->data2);
		writel(0, &regs->data3);
		writel(0, &regs->data0);
		break;
	case 3:
		writel(0, &regs->data1);
		writel(0, &regs->data2);
		writel(val, &regs->data3);
		writel(0, &regs->data0);
		break;
	}
	wait_busy(regs, BV_TIMING_PROG_US);
#else
	writel(val, &regs->data);
	wait_busy(regs, BV_TIMING_STROBE_PROG_US);
#endif
	udelay(WRITE_POSTAMBLE_US);

	return finish_access(regs, __func__);
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	struct ocotp_regs *regs;
	int ret;

	ret = prepare_write(&regs, bank, word, __func__);
	if (ret)
		return ret;

	writel(val, &regs->bank[bank].fuse_regs[word << 2]);

	return finish_access(regs, __func__);
}
