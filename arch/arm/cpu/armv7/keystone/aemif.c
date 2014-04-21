/*
 * Keystone2: Asynchronous EMIF Configuration
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/emif_defs.h>

#define AEMIF_CFG_SELECT_STROBE(v)	((v) ? 1 << 31 : 0)
#define AEMIF_CFG_EXTEND_WAIT(v)	((v) ? 1 << 30 : 0)
#define AEMIF_CFG_WR_SETUP(v)		(((v) & 0x0f) << 26)
#define AEMIF_CFG_WR_STROBE(v)		(((v) & 0x3f) << 20)
#define AEMIF_CFG_WR_HOLD(v)		(((v) & 0x07) << 17)
#define AEMIF_CFG_RD_SETUP(v)		(((v) & 0x0f) << 13)
#define AEMIF_CFG_RD_STROBE(v)		(((v) & 0x3f) << 7)
#define AEMIF_CFG_RD_HOLD(v)		(((v) & 0x07) << 4)
#define AEMIF_CFG_TURN_AROUND(v)	(((v) & 0x03) << 2)
#define AEMIF_CFG_WIDTH(v)		(((v) & 0x03) << 0)

#define set_config_field(reg, field, val)			\
	do {							\
		if (val != -1) {				\
			reg &= ~AEMIF_CFG_##field(0xffffffff);	\
			reg |=	AEMIF_CFG_##field(val);		\
		}						\
	} while (0)

void configure_async_emif(int cs, struct async_emif_config *cfg)
{
	unsigned long tmp;

	if (cfg->mode == ASYNC_EMIF_MODE_NAND) {
		tmp = __raw_readl(&davinci_emif_regs->nandfcr);
		tmp |= (1 << cs);
		__raw_writel(tmp, &davinci_emif_regs->nandfcr);

	} else if (cfg->mode == ASYNC_EMIF_MODE_ONENAND) {
		tmp = __raw_readl(&davinci_emif_regs->one_nand_cr);
		tmp |= (1 << cs);
		__raw_writel(tmp, &davinci_emif_regs->one_nand_cr);
	}

	tmp = __raw_readl(&davinci_emif_regs->abncr[cs]);

	set_config_field(tmp, SELECT_STROBE,	cfg->select_strobe);
	set_config_field(tmp, EXTEND_WAIT,	cfg->extend_wait);
	set_config_field(tmp, WR_SETUP,		cfg->wr_setup);
	set_config_field(tmp, WR_STROBE,	cfg->wr_strobe);
	set_config_field(tmp, WR_HOLD,		cfg->wr_hold);
	set_config_field(tmp, RD_SETUP,		cfg->rd_setup);
	set_config_field(tmp, RD_STROBE,	cfg->rd_strobe);
	set_config_field(tmp, RD_HOLD,		cfg->rd_hold);
	set_config_field(tmp, TURN_AROUND,	cfg->turn_around);
	set_config_field(tmp, WIDTH,		cfg->width);

	__raw_writel(tmp, &davinci_emif_regs->abncr[cs]);
}

void init_async_emif(int num_cs, struct async_emif_config *config)
{
	int cs;

	for (cs = 0; cs < num_cs; cs++)
		configure_async_emif(cs, config + cs);
}
