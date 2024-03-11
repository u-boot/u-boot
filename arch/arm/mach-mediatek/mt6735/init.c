// SPDX-License-Identifier: GPL-2.0

#include <clk.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdtdec.h>
#include <init.h>
#include <ram.h>
#include <dt-bindings/clock/mt6735-clk.h>
#include <linux/io.h>
#include <asm/arch/misc.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <configs/mt6735.h>
#include "pll.h"

#define VER_BASE                0x08000000
#define VER_SIZE                0x10
#define EMI_BASE                0x10203000
#define APHW_CODE               0x00
#define APHW_SUBCODE            0x04
#define APHW_VER                0x08
#define APSW_VER                0x0c

#define WDOG_RESTART		    0x10212008
#define WDOG_RESTART_KEY	    0x1971
#define WDOG_SWRST		        0x10212014
#define WDOG_SWRST_KEY		    0x1209

#define CPUX_BASE		        0x10200670

/* cpux mcusys wrapper */
#define CPUX_CON_REG		    0x0
#define CPUX_IDX_REG		    0x4

/* cpux */
#define CPUX_IDX_GLOBAL_CTRL	0x0
#define CPUX_ENABLE		        BIT(0)
#define CPUX_CLK_DIV_MASK	    GENMASK(10, 8)
#define CPUX_CLK_DIV1		    BIT(8)
#define CPUX_CLK_DIV2		    BIT(9)
#define CPUX_CLK_DIV4		    BIT(10)
#define CPUX_IDX_GLOBAL_IRQ	    0x30

DECLARE_GLOBAL_DATA_PTR;

int timer_init(void)
{
	u32 val;

	/* Initialize CPUX timers */
	/* Set DIV2 to achieve 13MHz clock */
	writel(CPUX_IDX_GLOBAL_CTRL, CPUX_BASE + CPUX_IDX_REG);
	val = readl(CPUX_BASE + CPUX_CON_REG);

	val &= ~CPUX_CLK_DIV_MASK;
	val |= CPUX_CLK_DIV2;

	writel(CPUX_IDX_GLOBAL_CTRL, CPUX_BASE + CPUX_IDX_REG);
	writel(val, CPUX_BASE + CPUX_CON_REG);

	/* Enable all CPUX timers */
	writel(CPUX_IDX_GLOBAL_CTRL, CPUX_BASE + CPUX_IDX_REG);
	val = readl(CPUX_BASE + CPUX_CON_REG);

	writel(CPUX_IDX_GLOBAL_CTRL, CPUX_BASE + CPUX_IDX_REG);
	writel(val | CPUX_ENABLE, CPUX_BASE + CPUX_CON_REG);

	return 0;
}

int mtk_pll_early_init(void)
{
    unsigned long pll_rates[] = {
        [CLK_APMIXED_ARMPLL] = 819000000,
        [CLK_APMIXED_MAINPLL] = 1092000000,
        [CLK_APMIXED_UNIVPLL] = 1248000000,
        [CLK_APMIXED_MMPLL] = 450000000,
        [CLK_APMIXED_MSDCPLL] = 800000000,
        [CLK_APMIXED_TVDPLL] = 297000000,
        [CLK_APMIXED_VENCPLL] = 300000000,
        [CLK_APMIXED_APLL1] = 90316800,
        [CLK_APMIXED_APLL2] = 90316800,
    };
    struct udevice *dev;
    int ret, i;

    ret = uclass_get_device_by_driver(UCLASS_CLK,
            DM_DRIVER_GET(mtk_clk_apmixedsys), &dev);
    if (ret)
        return ret;

    /* configure default rate then enable apmixedsys */
    for (i = 0; i < ARRAY_SIZE(pll_rates); i++) {
        struct clk clk = { .id = i, .dev = dev };

        ret = clk_set_rate(&clk, pll_rates[i]);
        if (ret)
            return ret;

        ret = clk_enable(&clk);
        if (ret)
            return ret;
    }
    
    /* setup mcu bus */
    ret = uclass_get_device_by_driver(UCLASS_SYSCON,
            DM_DRIVER_GET(mtk_mcucfg), &dev);
    if (ret) {
        return ret;
    }
    
    ret = uclass_get_device_by_driver(UCLASS_CLK,
            DM_DRIVER_GET(mtk_clk_topckgen), &dev);
    if (ret) {
        return ret;
    }
    
    ret = uclass_get_device_by_driver(UCLASS_CLK,
            DM_DRIVER_GET(mtk_clk_infracfg), &dev);
    if (ret) {
        return ret;
    }
    
    ret = uclass_get_device_by_driver(UCLASS_CLK,
            DM_DRIVER_GET(mtk_clk_pericfg), &dev);
    if (ret) {
        return ret;
    }
    
    return 0;
}

int mtk_soc_early_init(void)
{
	struct udevice *dev;
	int ret;

	ret = timer_init();
	if (ret)
		return ret;
    
    mt_pll_init();
    
    ret = mtk_pll_early_init();
    if (ret) {
        return ret;
    }
    
    ret = uclass_get_device_by_driver(UCLASS_PINCTRL,
            DM_DRIVER_GET(mt6735_pinctrl), &dev);
    if (ret) {
        return ret;
    }
    
    ret = uclass_get_device_by_driver(UCLASS_PWRAP,
            DM_DRIVER_GET(mtk_pwrap), &dev);
    if (ret) {
        return ret;
    }
    
    ret = uclass_get_device_by_driver(UCLASS_PMIC,
            DM_DRIVER_GET(pmic_mt6328), &dev);
    if (ret) {
        return ret;
    }

    ret = uclass_get_device_by_driver(UCLASS_REGULATOR,
            DM_DRIVER_GET(mt6328_buck), &dev);
    if (ret) {
        return ret;
    }
    
    ret = uclass_get_device_by_driver(UCLASS_REGULATOR,
            DM_DRIVER_GET(mt6328_ldo), &dev);
    if (ret) {
        return ret;
    }
    
    ret = uclass_get_device_by_driver(UCLASS_REGULATOR,
            DM_DRIVER_GET(mt6328_fixed), &dev);
    if (ret) {
        return ret;
    }
    
    dram_init();
    
	return 0;
}

int dram_init(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int ret;
    
	ret = uclass_first_device_err(UCLASS_RAM, &dev);
	if (ret)
		return ret;

	ret = ram_get_info(dev, &ram);
	if (ret)
		return ret;
    gd->ram_base = ram.base;
    gd->ram_size = ram.size;
    printf("RAM init base = 0x%lx, size = 0x%llx\n", gd->ram_base, gd->ram_size);
    
	return 0;
}

#ifdef CONFIG_SPL_BUILD
void reset_cpu(void)
{
	writel(WDOG_RESTART_KEY, WDOG_RESTART);
	writel(WDOG_SWRST_KEY, WDOG_SWRST);
	hang();
}
#endif

static struct mm_region mt6735_mem_map[] = {
	{
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0xc0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE,
	}, {
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		0,
	}
};
struct mm_region *mem_map = mt6735_mem_map;

int print_cpuinfo(void)
{
	void __iomem *chipid;
	u32 hwcode, swver;

	chipid = ioremap(VER_BASE, VER_SIZE);
	hwcode = readl(chipid + APHW_CODE);
	swver = readl(chipid + APSW_VER);

	printf("CPU:   MediaTek MT%04x E%d\n", hwcode, (swver & 0xf) + 1);

	return 0;
}
