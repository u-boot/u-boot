/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/periph.h>
#include <asm/arch/grf_rk322x.h>
#include <asm/arch/boot_mode.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int rk_board_late_init(void)
{
	return 0;
}

int board_late_init(void)
{
	setup_boot_mode();

	return rk_board_late_init();
}

int board_init(void)
{
#include <asm/arch/grf_rk322x.h>
	/* Enable early UART2 channel 1 on the RK322x */
#define GRF_BASE	0x11000000
	struct rk322x_grf * const grf = (void *)GRF_BASE;
	enum {
		GPIO1B2_SHIFT		= 4,
		GPIO1B2_MASK		= 3 << GPIO1B2_SHIFT,
		GPIO1B2_GPIO		= 0,
		GPIO1B2_UART21_SIN,

		GPIO1B1_SHIFT		= 2,
		GPIO1B1_MASK		= 3 << GPIO1B1_SHIFT,
		GPIO1B1_GPIO            = 0,
		GPIO1B1_UART1_SOUT,
		GPIO1B1_UART21_SOUT,
	};
	enum {
		CON_IOMUX_UART2SEL_SHIFT= 8,
		CON_IOMUX_UART2SEL_MASK	= 1 << CON_IOMUX_UART2SEL_SHIFT,
		CON_IOMUX_UART2SEL_2	= 0,
		CON_IOMUX_UART2SEL_21,
	};

	rk_clrsetreg(&grf->gpio1b_iomux,
		     GPIO1B1_MASK | GPIO1B2_MASK,
		     GPIO1B2_UART21_SIN << GPIO1B2_SHIFT |
		     GPIO1B1_UART21_SOUT << GPIO1B1_SHIFT);
	/* Set channel C as UART2 input */
	rk_clrsetreg(&grf->con_iomux,
		     CON_IOMUX_UART2SEL_MASK,
		     CON_IOMUX_UART2SEL_21 << CON_IOMUX_UART2SEL_SHIFT);

	/*
	* The integrated macphy is enabled by default, disable it
	* for saving power consuming.
	*/
	rk_clrsetreg(&grf->macphy_con[0],
		     MACPHY_CFG_ENABLE_MASK,
		     0 << MACPHY_CFG_ENABLE_SHIFT);

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = 0x8400000;
	/* Reserve 0x200000 for OPTEE */
	gd->bd->bi_dram[1].start = CONFIG_SYS_SDRAM_BASE
				+ gd->bd->bi_dram[0].size + 0x200000;
	gd->bd->bi_dram[1].size = gd->bd->bi_dram[0].start
				+ gd->ram_size - gd->bd->bi_dram[1].start;

	return 0;
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#if defined(CONFIG_USB_GADGET) && defined(CONFIG_USB_GADGET_DWC2_OTG)
#include <usb.h>
#include <usb/dwc2_udc.h>

static struct dwc2_plat_otg_data rk322x_otg_data = {
	.rx_fifo_sz	= 512,
	.np_tx_fifo_sz	= 16,
	.tx_fifo_sz	= 128,
};

int board_usb_init(int index, enum usb_init_type init)
{
	int node;
	const char *mode;
	bool matched = false;
	const void *blob = gd->fdt_blob;

	/* find the usb_otg node */
	node = fdt_node_offset_by_compatible(blob, -1,
					"rockchip,rk3288-usb");

	while (node > 0) {
		mode = fdt_getprop(blob, node, "dr_mode", NULL);
		if (mode && strcmp(mode, "otg") == 0) {
			matched = true;
			break;
		}

		node = fdt_node_offset_by_compatible(blob, node,
					"rockchip,rk3288-usb");
	}
	if (!matched) {
		debug("Not found usb_otg device\n");
		return -ENODEV;
	}
	rk322x_otg_data.regs_otg = fdtdec_get_addr(blob, node, "reg");

	return dwc2_udc_probe(&rk322x_otg_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}
#endif

#if defined(CONFIG_USB_FUNCTION_FASTBOOT)
int fb_set_reboot_flag(void)
{
	struct rk322x_grf *grf;

	printf("Setting reboot to fastboot flag ...\n");
	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	/* Set boot mode to fastboot */
	writel(BOOT_FASTBOOT, &grf->os_reg[0]);

	return 0;
}
#endif
