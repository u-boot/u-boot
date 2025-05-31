// SPDX-License-Identifier: GPL-2.0+

#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6ull_pins.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <spl.h>
#include <asm/arch/mx6-ddr.h>

#include "spl_mtypes.h"

#define UART_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | \
		       PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
		       PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

static const iomux_v3_cfg_t uart4_pads[] = {
	MX6_PAD_UART4_TX_DATA__UART4_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART4_RX_DATA__UART4_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

static void ddr_cfg_write(const struct dram_timing_info *dram_timing_info)
{
	int i;
	const struct dram_cfg_param *ddrc_cfg = dram_timing_info->ddrc_cfg;
	const int ddrc_cfg_num = dram_timing_info->ddrc_cfg_num;
	struct mmdc_p_regs *mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;

	clrbits_le32(&mmdc0->mdctl, 1 << 31);	/* clear SDE_0 */
	clrbits_le32(&mmdc0->mdctl, 1 << 30);	/* clear SDE_1 */

	for (i = 0; i < ddrc_cfg_num; i++) {
		debug("Writing 0x%x to register 0x%x\n", ddrc_cfg->val,
		      ddrc_cfg->reg);
		writel(ddrc_cfg->val, ddrc_cfg->reg);
		ddrc_cfg++;
	}
}

static const struct dram_timing_info *board_dram_timing[] = {
#if defined(CONFIG_M2_MEMORY)
	&bsh_dram_timing_512mb,
#endif
	&bsh_dram_timing_256mb,
	&bsh_dram_timing_128mb,
};

static void spl_dram_init(void)
{
	/* Configure memory to maximum supported size for detection */
	ddr_cfg_write(board_dram_timing[0]);

	/* Detect memory physically present */
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE, board_dram_timing[0]->dram_size);

	if (board_dram_timing[0]->dram_size == gd->ram_size)
		return;

	for (size_t index = 1; index < ARRAY_SIZE(board_dram_timing); index++) {
		if (board_dram_timing[index]->dram_size == gd->ram_size) {
			udelay(1);
			ddr_cfg_write(board_dram_timing[index]);
			break;
		}
	}
}

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0xFFFFFFFF, &ccm->CCGR0);
	writel(0xFFFFFFFF, &ccm->CCGR1);
	writel(0xFFFFFFFF, &ccm->CCGR2);
	writel(0xFFFFFFFF, &ccm->CCGR3);
	writel(0xFFFFFFFF, &ccm->CCGR4);
	writel(0xFFFFFFFF, &ccm->CCGR5);
	writel(0xFFFFFFFF, &ccm->CCGR6);
}

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* DDR initialization */
	spl_dram_init();

	arch_cpu_init();
	timer_init();
	setup_iomux_uart();
	preloader_console_init();
}

void reset_cpu(void)
{
}
