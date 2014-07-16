/*
 * TNETV107X-EVM: Board initialization
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <miiphy.h>
#include <linux/mtd/nand.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/ti-common/davinci_nand.h>
#include <asm/arch/mux.h>

DECLARE_GLOBAL_DATA_PTR;

static struct async_emif_config async_emif_config[ASYNC_EMIF_NUM_CS] = {
	{			/* CS0 */
		.mode		= ASYNC_EMIF_MODE_NAND,
		.wr_setup	= 5,
		.wr_strobe	= 5,
		.wr_hold	= 2,
		.rd_setup	= 5,
		.rd_strobe	= 5,
		.rd_hold	= 2,
		.turn_around	= 5,
		.width		= ASYNC_EMIF_8,
	},
	{			/* CS1 */
		.mode		= ASYNC_EMIF_MODE_NOR,
		.wr_setup	= 2,
		.wr_strobe	= 27,
		.wr_hold	= 4,
		.rd_setup	= 2,
		.rd_strobe	= 27,
		.rd_hold	= 4,
		.turn_around	= 2,
		.width		= ASYNC_EMIF_PRESERVE,
	},
	{			/* CS2 */
		.mode		= ASYNC_EMIF_MODE_NOR,
		.wr_setup	= 2,
		.wr_strobe	= 27,
		.wr_hold	= 4,
		.rd_setup	= 2,
		.rd_strobe	= 27,
		.rd_hold	= 4,
		.turn_around	= 2,
		.width		= ASYNC_EMIF_PRESERVE,
	},
	{			/* CS3 */
		.mode		= ASYNC_EMIF_MODE_NOR,
		.wr_setup	= 1,
		.wr_strobe	= 90,
		.wr_hold	= 3,
		.rd_setup	= 1,
		.rd_strobe	= 26,
		.rd_hold	= 3,
		.turn_around	= 1,
		.width		= ASYNC_EMIF_8,
	},
};

static struct pll_init_data pll_config[] = {
	{
		.pll			= ETH_PLL,
		.internal_osc		= 1,
		.pll_freq		= 500000000,
		.div_freq = {
			5000000, 50000000, 125000000, 250000000, 25000000,
		},
	},
};

static const short sdio1_pins[] = {
	TNETV107X_PIN_SDIO1_CLK_1,	TNETV107X_PIN_SDIO1_CMD_1,
	TNETV107X_PIN_SDIO1_DATA0_1,	TNETV107X_PIN_SDIO1_DATA1_1,
	TNETV107X_PIN_SDIO1_DATA2_1,	TNETV107X_PIN_SDIO1_DATA3_1,
	-1
};

static const short uart1_pins[] = {
	TNETV107X_PIN_UART1_RD, TNETV107X_PIN_UART1_TD, -1
};

static const short ssp_pins[] = {
	TNETV107X_PIN_SSP0_0, TNETV107X_PIN_SSP0_1, TNETV107X_PIN_SSP0_2,
	TNETV107X_PIN_SSP1_0, TNETV107X_PIN_SSP1_1, TNETV107X_PIN_SSP1_2,
	TNETV107X_PIN_SSP1_3, -1
};

int board_init(void)
{
#ifndef CONFIG_USE_IRQ
	__raw_writel(0, INTC_GLB_EN);		/* Global disable       */
	__raw_writel(0, INTC_HINT_EN);		/* Disable host ints    */
	__raw_writel(0, INTC_EN_CLR0 + 0);	/* Clear enable         */
	__raw_writel(0, INTC_EN_CLR0 + 4);	/* Clear enable         */
	__raw_writel(0, INTC_EN_CLR0 + 8);	/* Clear enable         */
#endif

	gd->bd->bi_arch_number = MACH_TYPE_TNETV107X;
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	init_plls(ARRAY_SIZE(pll_config), pll_config);

	init_async_emif(ARRAY_SIZE(async_emif_config), async_emif_config);

	mux_select_pin(TNETV107X_PIN_ASR_CS3);
	mux_select_pins(sdio1_pins);
	mux_select_pins(uart1_pins);
	mux_select_pins(ssp_pins);

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifdef CONFIG_NAND_DAVINCI
int board_nand_init(struct nand_chip *nand)
{
	davinci_nand_init(nand);

	return 0;
}
#endif
