/*
 * TNETV107X: Asynchronous EMIF Configuration
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/mux.h>

#define ASYNC_EMIF_BASE			TNETV107X_ASYNC_EMIF_CNTRL_BASE
#define ASYNC_EMIF_CONFIG(cs)		(ASYNC_EMIF_BASE+0x10+(cs)*4)
#define ASYNC_EMIF_ONENAND_CONTROL	(ASYNC_EMIF_BASE+0x5c)
#define ASYNC_EMIF_NAND_CONTROL		(ASYNC_EMIF_BASE+0x60)
#define ASYNC_EMIF_WAITCYCLE_CONFIG	(ASYNC_EMIF_BASE+0x4)

#define CONFIG_SELECT_STROBE(v)		((v) ? 1 << 31 : 0)
#define CONFIG_EXTEND_WAIT(v)		((v) ? 1 << 30 : 0)
#define CONFIG_WR_SETUP(v)		(((v) & 0x0f) << 26)
#define CONFIG_WR_STROBE(v)		(((v) & 0x3f) << 20)
#define CONFIG_WR_HOLD(v)		(((v) & 0x07) << 17)
#define CONFIG_RD_SETUP(v)		(((v) & 0x0f) << 13)
#define CONFIG_RD_STROBE(v)		(((v) & 0x3f) << 7)
#define CONFIG_RD_HOLD(v)		(((v) & 0x07) << 4)
#define CONFIG_TURN_AROUND(v)		(((v) & 0x03) << 2)
#define CONFIG_WIDTH(v)			(((v) & 0x03) << 0)

#define NUM_CS				4

#define set_config_field(reg, field, val)			\
	do {							\
		if (val != -1) {				\
			reg &= ~CONFIG_##field(0xffffffff);	\
			reg |=	CONFIG_##field(val);		\
		}						\
	} while (0)

void configure_async_emif(int cs, struct async_emif_config *cfg)
{
	unsigned long tmp;

	if (cfg->mode == ASYNC_EMIF_MODE_NAND) {
		tmp = __raw_readl(ASYNC_EMIF_NAND_CONTROL);
		tmp |= (1 << cs);
		__raw_writel(tmp, ASYNC_EMIF_NAND_CONTROL);

	} else if (cfg->mode == ASYNC_EMIF_MODE_ONENAND) {
		tmp = __raw_readl(ASYNC_EMIF_ONENAND_CONTROL);
		tmp |= (1 << cs);
		__raw_writel(tmp, ASYNC_EMIF_ONENAND_CONTROL);
	}

	tmp = __raw_readl(ASYNC_EMIF_CONFIG(cs));

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

	__raw_writel(tmp, ASYNC_EMIF_CONFIG(cs));
}

void init_async_emif(int num_cs, struct async_emif_config *config)
{
	int cs;

	clk_enable(TNETV107X_LPSC_AEMIF);

	for (cs = 0; cs < num_cs; cs++)
		configure_async_emif(cs, config + cs);
}
