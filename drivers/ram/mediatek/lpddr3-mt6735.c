// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek LPDDR3 driver for MT6735 SoC
*/

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <ram.h>
#include <power/regulator.h>
#include <asm/io.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#define WALKING_PATTERN             0x12345678
#define WALKING_STEP                0x4000000
#define CONA_DUAL_CH_EN      BIT(0)
#define CONA_32BIT_EN        BIT(1)
#define CONA_DUAL_RANK_EN       BIT(17)
#define COL_ADDR_BITS_SHIFT     4
#define COL_ADDR_BITS_MASK      3 << COL_ADDR_BITS_SHIFT
#define ROW_ADDR_BITS_SHIFT     12
#define ROW_ADDR_BITS_MASK      3 << ROW_ADDR_BITS_SHIFT

enum {
    PLL_MODE_1  = 1,
    PLL_MODE_2  = 2,
    PLL_MODE_3  = 3,
};
enum {
     DDR533   = 533,
     DDR800   = 800,
     DDR900   = 900,
     DDR938   = 938,
     DDR1066  = 1066,
     DDR1280  = 1280,
     DDR1313  = 1313,
     DDR1333  = 1333,
     DDR1466  = 1466,
     DDR1600  = 1600,
};

struct mtk_lpddr3_priv {
	fdt_addr_t emi;
	fdt_addr_t ddrphy_base;
	fdt_addr_t dramc0_base;
    fdt_addr_t dramc_nao_base;
	struct clk phy;
	struct clk phy_mux;
	struct clk mem;
	struct clk mem_mux;
    unsigned int emi_cona;
    unsigned int drvctl0;
    unsigned int drvctl1;
    unsigned int actim;
    unsigned int gddr3ctl;
    unsigned int conf1;
    unsigned int ddr2ctl;
    unsigned int test2_1;
    unsigned int test2_2;
    unsigned int test2_3;
    unsigned int conf2;
    unsigned int pd_ctrl;
    unsigned int padctl3;
    unsigned int padctl4;
    unsigned int dqodly;
    unsigned int addr_output_dly;
    unsigned int clk_output_dly;
    unsigned int actim1;
    unsigned int misctl0;
    unsigned int actim05t;
    unsigned int modereg1;
    unsigned int modereg2;
    unsigned int modereg3;
    unsigned int modereg5;
    unsigned int modereg10;
    unsigned int modereg63;
    unsigned int dqsgctl;
    unsigned int dqsctl1;
    unsigned int dqidly1;
    unsigned int dqidly2;
    unsigned int dqidly3;
    unsigned int dqidly4;
    unsigned int dqidly5;
    unsigned int dqidly6;
    unsigned int dqidly7;
    unsigned int dqidly8;
    unsigned int dqodly1;
    unsigned int dqodly2;
    unsigned int dqodly3;
    unsigned int dqodly4;
    unsigned int r0deldly;
    unsigned int r1deldly;
    unsigned int r0dqsien;
    unsigned int r1dqsien;
    unsigned int spcmd;
};

unsigned int drvn;
unsigned int drvp;

unsigned int DRAMC_READ_REG(struct udevice *dev, unsigned int offset)
{
    struct mtk_lpddr3_priv *priv = dev_get_priv(dev);

    return (readl(priv->dramc0_base + offset) | readl(priv->ddrphy_base + offset) | readl(priv->dramc_nao_base + offset));
}

void DRAMC_WRITE_REG(struct udevice *dev, unsigned int val, unsigned int offset)
{
    struct mtk_lpddr3_priv *priv = dev_get_priv(dev);

    writel(val, priv->dramc0_base + offset);
    writel(val, priv->ddrphy_base + offset);
    writel(val, priv->dramc_nao_base + offset);
}

void DRAMC_WRITE_SET(struct udevice *dev, unsigned int val, unsigned int offset)
{
    struct mtk_lpddr3_priv *priv = dev_get_priv(dev);
    unsigned int temp;

    temp = readl(priv->dramc0_base + offset);
    writel((temp | val), priv->dramc0_base + offset);
    temp = readl(priv->ddrphy_base + offset);
    writel((temp | val), priv->ddrphy_base + offset);
    temp = readl(priv->dramc_nao_base + offset);
    writel((temp | val), priv->dramc_nao_base + offset);
}

void DRAMC_WRITE_CLEAR(struct udevice *dev, unsigned int val, unsigned int offset)
{
    struct mtk_lpddr3_priv *priv = dev_get_priv(dev);
    unsigned int temp;

    temp = readl(priv->dramc0_base + offset);
    writel((temp & ~(val)), priv->dramc0_base + offset);
    temp = readl(priv->ddrphy_base + offset);
    writel((temp & ~(val)), priv->ddrphy_base + offset);
    temp = readl(priv->dramc_nao_base + offset);
    writel((temp & ~(val)), priv->dramc_nao_base + offset);
}

void ett_rextdn_sw_calibration(struct udevice *dev)
{
    unsigned int tmp;
    unsigned int drvn;
    unsigned int drvp;

    tmp = (DRAMC_READ_REG(dev, 0x0644) |0x00000200);
    DRAMC_WRITE_REG(dev, tmp, 0x0644);

    udelay(1);

    for(drvp = 0 ; drvp <=15; drvp ++)
    {
        tmp = (DRAMC_READ_REG(dev, 0x00c0) & 0xffff0fff)|(drvp << 12);
        DRAMC_WRITE_REG(dev, tmp, 0x00c0);

        udelay(1);
        if ((DRAMC_READ_REG(dev, 0x3dc) >> 31)  == 1)
        {
            break;
        }
    }

    tmp = (DRAMC_READ_REG(dev, 0x0644) & 0xfffffdff);
    DRAMC_WRITE_REG(dev, tmp, 0x0644);

    if (drvp == 16)
    {
        drvp = 10;
    }

    tmp = (DRAMC_READ_REG(dev, 0x0644) | 0x00000100);
    DRAMC_WRITE_REG(dev, tmp, 0x0644);

    udelay(1);

    for(drvn = 0 ; drvn <=15; drvn ++)
    {
        tmp = ((DRAMC_READ_REG(dev, 0x00c0) & 0xfffff0ff) | (drvn<<8));
        DRAMC_WRITE_REG(dev, tmp, 0x00c0);
        udelay(1);

        if ((DRAMC_READ_REG(dev, 0x3dc) >> 30)  == 1)
        {
            if (drvn > 0)
                drvn--;

            break;
        }
    }

    tmp = (DRAMC_READ_REG(dev, 0x0644) & 0xfffffeff);
    DRAMC_WRITE_REG(dev, tmp, 0x0644);

    if (drvn == 16)
    {
        drvn = 10;
    }
}


#ifdef CONFIG_SPL_BUILD
static int mtk_lpddr3_memtest(void)
{
	int step;
	u32 start, test;
	writel(WALKING_PATTERN, (unsigned long)CONFIG_SYS_SDRAM_BASE);

    if (readl((unsigned long)CONFIG_SYS_SDRAM_BASE) != WALKING_PATTERN) {
        printf("SDRAM_BASE return 0x%lx = 0x%x\n", (unsigned long)CONFIG_SYS_SDRAM_BASE, readl((unsigned long)CONFIG_SYS_SDRAM_BASE));
        return -EINVAL;
    }
    
	for (step = 0; step < 5; step++) {
		writel(~WALKING_PATTERN, (unsigned long)CONFIG_SYS_SDRAM_BASE + (WALKING_STEP << step));

		start = readl(CONFIG_SYS_SDRAM_BASE);
		test = readl((unsigned long)CONFIG_SYS_SDRAM_BASE + (WALKING_STEP << step));
		if ((test != ~WALKING_PATTERN) || test == start)
			break;
	}
    
	return 0;
}

void mtk_mempll_init(struct udevice *dev, int type, int pll_mode)
{
    struct mtk_lpddr3_priv *priv = dev_get_priv(dev);
    unsigned int temp;
    
    temp = readl(priv->dramc0_base + 0x007c);
    writel(temp | 0x00000001, priv->dramc0_base + 0x007c);
    temp = readl(priv->dramc0_base + 0x007c);
    writel(temp | 0x00000001, priv->ddrphy_base + 0x007c);
    
    if (pll_mode == PLL_MODE_3)
    {
        writel(0x00000020, priv->ddrphy_base + (0x0640));
    }
    else if (pll_mode== PLL_MODE_2)
    {
        writel(0x00000022, priv->ddrphy_base + (0x0640));
    }
    else
    {
        writel(0x00000007, priv->ddrphy_base + (0x0640));
    }
    
    writel((0x0 | 0x0 | 0x0 | 0x0), priv->ddrphy_base + (0x05c0));
    writel((0x0 | 0x0 | 0x0 | 0x0), priv->ddrphy_base + (0x05c4));
    writel((0x10000000| 0x1000000 | 0x100000| 0x10000 | 0x8000| 0x4000| 0x2000| 0x1000 | 0x800 | 0x400 | 0x200 | 0x100 | 0x10 | 0x1), priv->ddrphy_base + (0x05c8));
    writel((0x10000000| 0x1000000 | 0x100000| 0x10000 | 0x1000| 0x100 | 0x10| 0x1), priv->ddrphy_base + (0x05c8));
    
    writel((((readl(priv->ddrphy_base + (0x0600))) & (~0xc000)) | 0x00000000), priv->ddrphy_base + (0x0600));
    writel((((readl(priv->ddrphy_base + (0x0600))) & (~0x1000)) | (0x1000)), priv->ddrphy_base + (0x0600));
    writel((((readl(priv->ddrphy_base + (0x0604))) & (~(0x0000007f <<25))) | (0x00000052 << 25)), priv->ddrphy_base + (0x0604));
    writel((((readl(priv->ddrphy_base + (0x0604))) & (~0x40000)) | (0x00000001 << 18)), priv->ddrphy_base + (0x0604));
    writel((((readl(priv->ddrphy_base + (0x0608))) & (~0xc00)) | 0x00000000), priv->ddrphy_base + (0x0608));
    writel((((readl(priv->ddrphy_base + (0x060c))) & (~0xc000000)) | (0x00000003 << 26)), priv->ddrphy_base + (0x060c));
    
    if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
    {
        writel((((readl(priv->ddrphy_base + (0x060c))) & (~0x200)) | (0x200)), priv->ddrphy_base + (0x060c));
    }
    
    writel((((readl(priv->ddrphy_base + (0x0610))) & (~0xc00)) | 0x00000000), priv->ddrphy_base + (0x0610));
    writel((((readl(priv->ddrphy_base + (0x0614))) & (~0xc000000)) | (0x00000003 << 26)), priv->ddrphy_base + (0x0614));
    
    if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
    {
        writel((((readl(priv->ddrphy_base + (0x0614))) & (~0x200)) | (0x200)), priv->ddrphy_base + (0x0614));
    }
    
    writel((((readl(priv->ddrphy_base + (0x0618))) & (~0xc00)) | 0x00000000), priv->ddrphy_base + (0x0618));
    writel((((readl(priv->ddrphy_base + (0x061c))) & (~0xc000000)) | (0x00000003 << 26)), priv->ddrphy_base + (0x061c));
    
    if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
    {
        writel((((readl(priv->ddrphy_base + (0x061c))) & (~0x200)) | (0x200)), priv->ddrphy_base + (0x061c));
    }
    
    if (type == DDR1333)
    {
        writel((((readl(priv->ddrphy_base + (0x060c))) & (~0x20000000)) | 0x00000000), priv->ddrphy_base + (0x060c));
        writel((((readl(priv->ddrphy_base + (0x0614))) & (~0x20000000)) | 0x00000000), priv->ddrphy_base + (0x0614));
        writel((((readl(priv->ddrphy_base + (0x061c))) & (~0x20000000)) | 0x00000000), priv->ddrphy_base + (0x061c));
        writel((((readl(priv->ddrphy_base + (0x0624))) & (~(0x7fffffff << 1))) | (0x4da21535 << 1)), priv->ddrphy_base + (0x0624));
        
        if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
        {
            writel((((readl(priv->ddrphy_base + (0x0608))) & (~0x1fc)) | (0x0000000d << 2)), priv->ddrphy_base + (0x0608));
            writel((((readl(priv->ddrphy_base + (0x0610))) & (~0x1fc)) | (0x0000000d << 2)), priv->ddrphy_base + (0x0610));
            writel((((readl(priv->ddrphy_base + (0x0618))) & (~0x1fc)) | (0x0000000d << 2)), priv->ddrphy_base + (0x0618));
        }
        else
        {
            writel((((readl(priv->ddrphy_base + (0x0608))) & (~0x1fc)) | (0x00000034 << 2)), priv->ddrphy_base + (0x0608));
            writel((((readl(priv->ddrphy_base + (0x0610))) & (~0x1fc)) | (0x00000034 << 2)), priv->ddrphy_base + (0x0610));
            writel((((readl(priv->ddrphy_base + (0x0618))) & (~0x1fc)) | (0x00000034 << 2)), priv->ddrphy_base + (0x0618));
        }
    }
    else if (type == DDR938)
    {
        writel((((readl(priv->ddrphy_base + (0x060c))) & (~0x20000000)) | 0x00000000), priv->ddrphy_base + (0x060c));
        writel((((readl(priv->ddrphy_base + (0x0614))) & (~0x20000000)) | 0x00000000), priv->ddrphy_base + (0x0614));
        writel((((readl(priv->ddrphy_base + (0x061c))) & (~0x20000000)) | 0x00000000), priv->ddrphy_base + (0x061c));
        writel((((readl(priv->ddrphy_base + (0x0624))) & (~(0x7fffffff << 1))) | (0x38e3f9f0 << 1)), priv->ddrphy_base + (0x0624));
        
        if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
        {
            writel((((readl(priv->ddrphy_base + (0x0608))) & (~0x1fc)) | (0x0000000d << 2)), (priv->ddrphy_base + (0x0608)));
            writel((((readl(priv->ddrphy_base + (0x0610))) & (~0x1fc)) | (0x0000000d << 2)), (priv->ddrphy_base + (0x0610)));
            writel((((readl(priv->ddrphy_base + (0x0618))) & (~0x1fc)) | (0x0000000d << 2)), (priv->ddrphy_base + (0x0618)));
        }
        else
        {
            writel((((readl(priv->ddrphy_base + (0x0608))) & (~0x1fc)) | (0x00000034 << 2)), priv->ddrphy_base + (0x0608));
            writel((((readl(priv->ddrphy_base + (0x0610))) & (~0x1fc)) | (0x00000034 << 2)), priv->ddrphy_base + (0x0610));
            writel((((readl(priv->ddrphy_base + (0x0618))) & (~0x1fc)) | (0x00000034 << 2)), priv->ddrphy_base + (0x0618));
        }
    }
    else if (type == DDR1466)
    {
        writel(((((readl(priv->ddrphy_base + (0x060c))) & (~0x20000000)) | 0x00000000)), priv->ddrphy_base + (0x060c));
        writel(((((readl(priv->ddrphy_base + (0x0614))) & (~0x20000000)) | 0x00000000)), priv->ddrphy_base + (0x0614));
        writel(((((readl(priv->ddrphy_base + (0x061c))) & (~0x20000000)) | 0x00000000)), priv->ddrphy_base + (0x061c));
        writel((((readl(priv->ddrphy_base + (0x0624))) & (~(0x7fffffff << 1))) | (0x52902d02 << 1)), priv->ddrphy_base + (0x0624));
        if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
        {
            writel((((readl(priv->ddrphy_base + (0x0608))) & (~0x1fc)) | (0x0000000e << 2)), priv->ddrphy_base + (0x0608));
            writel((((readl(priv->ddrphy_base + (0x0610))) & (~0x1fc)) | (0x0000000e << 2)), priv->ddrphy_base + (0x0610));
            writel((((readl(priv->ddrphy_base + (0x0618))) & (~0x1fc)) | (0x0000000e << 2)), priv->ddrphy_base + (0x0618));
        }
        else
        {
            writel((((readl(priv->ddrphy_base + (0x0608))) & (~0x1fc)) | (0x00000038 << 2)), priv->ddrphy_base + (0x0608));
            writel((((readl(priv->ddrphy_base + (0x0610))) & (~0x1fc)) | (0x00000038 << 2)), priv->ddrphy_base + (0x0610));
            writel((((readl(priv->ddrphy_base + (0x0618))) & (~0x1fc)) | (0x00000038 << 2)), priv->ddrphy_base + (0x0618));
        }
    }
    else
    {
        writel((((readl(priv->ddrphy_base + (0x0624))) & (~(0x7fffffff << 1))) | (0x4c68b439 << 1)), priv->ddrphy_base + (0x0624));
        
        if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
        {
            writel((((readl(priv->ddrphy_base + (0x0608))) & (~0x1fc)) | (0x0000000b << 2)), priv->ddrphy_base + (0x0608));
            writel((((readl(priv->ddrphy_base + (0x0610))) & (~0x1fc)) | (0x0000000b << 2)), priv->ddrphy_base + (0x0610));
            writel((((readl(priv->ddrphy_base + (0x0618))) & (~0x1fc)) | (0x0000000b << 2)), priv->ddrphy_base + (0x0618));
        }
        else
        {
            writel((((readl(priv->ddrphy_base + (0x0608))) & (~0x1fc)) | (0x0000002c << 2)), priv->ddrphy_base + (0x0608));
            writel((((readl(priv->ddrphy_base + (0x0610))) & (~0x1fc)) | (0x0000002c << 2)), priv->ddrphy_base + (0x0610));
            writel((((readl(priv->ddrphy_base + (0x0618))) & (~0x1fc)) | (0x0000002c << 2)), priv->ddrphy_base + (0x0618));
        }
    }
    
    if (pll_mode == PLL_MODE_2)
    {
        writel((0x00000022  | 0x20), priv->ddrphy_base + (0x0640));
    }
    else if (pll_mode == PLL_MODE_3)
    {
        writel((0x00000020 | 0x20), priv->ddrphy_base + (0x0640));
    }
    else
    {
        writel(0x00000007, priv->ddrphy_base + (0x0640));
    }
    
    udelay(2);
    
    writel((((readl(priv->ddrphy_base + (0x0604))) & (~0x4000)) | (0x4000)), priv->ddrphy_base + (0x0604));
    udelay(2);
    
    writel((((readl(priv->ddrphy_base + (0x0604))) & (~0x8000)) | (0x8000)), priv->ddrphy_base + (0x0604));
    udelay(1000);
    
    writel((((readl(priv->ddrphy_base + (0x0600))) & (~0x4)) | (0x4)), priv->ddrphy_base + (0x0600));
    udelay(20);
    
    writel((((readl(priv->ddrphy_base + (0x0604))) & (~0x1000000)) | (0x1000000)), priv->ddrphy_base + (0x0604));
    
    udelay(1);
    
    if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
    {
        writel((((readl(priv->ddrphy_base + (0x060c))) & (~0x40000)) | (0x00000001 << 18)), priv->ddrphy_base + (0x060c));
        writel((((readl(priv->ddrphy_base + (0x0614))) & (~0x40000)) | (0x00000001 << 18)), priv->ddrphy_base + (0x0614));
        writel((((readl(priv->ddrphy_base + (0x061c))) & (~0x40000)) | (0x00000001 << 18)), priv->ddrphy_base + (0x061c));
    }
    else
    {
        writel((((readl(priv->ddrphy_base + (0x060c))) & (~0x40000)) | (0x00000001 << 18)), priv->ddrphy_base + (0x060c));
        writel((((readl(priv->ddrphy_base + (0x0614))) & (~0x40000)) | (0x00000000 << 18)), priv->ddrphy_base + (0x0614));
        writel((((readl(priv->ddrphy_base + (0x061c))) & (~0x40000)) | (0x00000000 << 18)), priv->ddrphy_base + (0x061c));
    }
    
    udelay(23);
    
    if (pll_mode == PLL_MODE_2)
    {
        writel((0x00000022  | 0x20 | 0x10), priv->ddrphy_base + (0x0640));
    }
    else if (pll_mode == PLL_MODE_3)
    {
        writel((0x00000020 | 0x20 | 0x10), priv->ddrphy_base + (0x0640));
    }
    else
    {
        writel((0x00000007 | 0x20 | 0x10), priv->ddrphy_base + (0x0640));
    }
    
    if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
    {
        writel(0x40101000, priv->ddrphy_base + (0x05cc));
    }
    else
    {
        writel(0x40101000, priv->ddrphy_base + (0x05cc));
    }
    
    if ((pll_mode == PLL_MODE_3) || (pll_mode == PLL_MODE_2))
    {
        writel(0x0000f010, priv->ddrphy_base + (0x05c8));
    }
    else
    {
        writel(0x0000fc10, priv->ddrphy_base + (0x05c8));
    }
    
    writel(0x063c0000, priv->ddrphy_base + (0x05c0));
}

static int init_lpddr3(struct udevice *dev)
{
	struct mtk_lpddr3_priv *priv = dev_get_priv(dev);
    unsigned int temp;

    writel(0x004210000, priv->emi + 0x28);

    temp = readl(priv->emi + 0x0040);
    writel(temp | 0xcc00, priv->emi + 0x0040);
    
    ett_rextdn_sw_calibration(dev);
    writel(priv->emi_cona, priv->emi + 0x000);
    writel(0x7f07704a, priv->emi + 0x100);
    writel(0xa0a070db, priv->emi + 0x108);
    writel(0xa0a07042, priv->emi + 0x110);
    writel(0x7007047, priv->emi + 0x118);
    writel(0x2030604b, priv->emi + 0x120);
    writel(0xa0a07046, priv->emi + 0x128);
    writel(0xa0a07046, priv->emi + 0x134);
    writel(0xd1e293a, priv->emi + 0x008);
    writel(0x9190819, priv->emi + 0x010);
    writel(0x2b2b282e, priv->emi + 0x030);
    writel(0x3657587a, priv->emi + 0x018);
    writel(0xffff0848, priv->emi + 0x020);
    writel(0, priv->emi + 0x038);
    writel(0x34220e17, priv->emi + 0x078);
    writel(0xcccccccc, priv->emi + 0x0d0);
    writel(0xcccccccc, priv->emi + 0x0d8);
    writel(0x20027, priv->emi + 0x0e8);
    writel(0x38460000, priv->emi + 0x0f0);
    writel(0, priv->emi + 0x0f8);
    writel(0x20406188, priv->emi + 0x140);
    writel(0x20406188, priv->emi + 0x144);
    writel(0x9719595e, priv->emi + 0x148);
    writel(0x9719595e, priv->emi + 0x14c);
    writel(0x64f3fc79, priv->emi + 0x150);
    writel(0x64f3fc79, priv->emi + 0x154);
    writel(0xff01ff00, priv->emi + 0x158);
    writel(0x421000, priv->emi + 0x028);
    writel(0x6ff, priv->emi + 0x060);
    writel(0, priv->dramc0_base + 0x00c);
    writel(0, priv->ddrphy_base + 0x00c);
    writel(0x00000000 | ((9<<28) + (9<<24) + (9<<12) + (9<<8)), priv->dramc0_base + 0x0b4);
    writel(0x00000000 | ((9<<28) + (9<<24) + (9<<12) + (9<<8)), priv->ddrphy_base + 0x0b4);
    writel(0x00000000 | ((9<<28) + (9<<24) + (9<<12) + (9<<8)), priv->dramc0_base + 0x0b8);
    writel(0x00000000 | ((9<<28) + (9<<24) + (9<<12) + (9<<8)), priv->ddrphy_base + 0x0b8);
    
    if ((9>1) && (9>1))
    {
       writel(0x00000000 | ((9<<28) + (9<<24) + ((9-1)<<12) + ((9-1)<<8)), priv->dramc0_base + 0x00bc);
       writel(0x00000000 | ((9<<28) + (9<<24) + ((9-1)<<12) + ((9-1)<<8)), priv->ddrphy_base + 0x00bc);
    }
    else
    {
        writel(0x00000000 | ((9<<28) + (9<<24) + (9<<12) + (9<<8)), priv->dramc0_base + 0x00bc);
        writel(0x00000000 | ((9<<28) + (9<<24) + (9<<12) + (9<<8)), priv->ddrphy_base + 0x00bc);
    }
    
    temp = readl(priv->dramc0_base + 0x644);
    writel(temp & 0xfffffffe, priv->dramc0_base + 0x644);
    temp = readl(priv->ddrphy_base + 0x644);
    writel(temp & 0xfffffffe, priv->ddrphy_base + 0x644);
    writel(0x1110d, priv->dramc0_base + 0x048);
    writel(0x1110d, priv->ddrphy_base + 0x048);
    writel(0x00500900, priv->dramc0_base + 0x00d8);
    writel(0x00500900, priv->ddrphy_base + 0x00d8);
    writel(0x00000000, priv->dramc0_base + 0x00e4);
    writel(0x00000000, priv->ddrphy_base + 0x00e4);
    writel(1, priv->dramc0_base + 0x08c);
    writel(1, priv->ddrphy_base + 0x08c);
    writel(0, priv->dramc0_base + 0x090);
    writel(0, priv->ddrphy_base + 0x090);
    writel(0x80000000, priv->dramc0_base + 0x094);
    writel(0x80000000, priv->ddrphy_base + 0x094);
    writel(0x83004004, priv->dramc0_base + 0x0dc);
    writel(0x83004004, priv->ddrphy_base + 0x0dc);
    writel(0x1c004004, priv->dramc0_base + 0x0e0);
    writel(0x1c004004, priv->ddrphy_base + 0x0e0);
    writel(0xaa080033, priv->dramc0_base + 0x124);
    writel(0xaa080033, priv->ddrphy_base + 0x124);
    writel(0xc0000000, priv->dramc0_base + 0x0f0);
    writel(0xc0000000, priv->ddrphy_base + 0x0f0);
    writel(priv->gddr3ctl | 0xf00000, priv->dramc0_base + 0x0f4);
    writel(priv->gddr3ctl | 0xf00000, priv->ddrphy_base + 0x0f4);
    writel(0x80, priv->dramc0_base + 0x168);
    writel(0x80, priv->ddrphy_base + 0x168);
    writel(0x700900, priv->dramc0_base + 0x0d8);
    writel(0x700900, priv->ddrphy_base + 0x0d8);
    writel(0xf1200f01, priv->dramc0_base + 0x028);
    writel(0xf1200f01, priv->ddrphy_base + 0x028);
    writel(0x2001ebff, priv->dramc0_base + 0x1e0);
    writel(0x2001ebff, priv->ddrphy_base + 0x1e0);
    writel(priv->actim1, priv->dramc0_base + 0x1e8);
    writel(priv->actim1, priv->ddrphy_base + 0x1e8);
    writel(0xf0f0f0f0, priv->dramc0_base + 0x158);
    writel(0xf0f0f0f0, priv->ddrphy_base + 0x158);
    writel(0x111100, priv->dramc0_base + 0x400);
    writel(0x111100, priv->ddrphy_base + 0x400);
    writel(2, priv->dramc0_base + 0x404);
    writel(2, priv->ddrphy_base + 0x404);
    writel(0x222222, priv->dramc0_base + 0x408);
    writel(0x222222, priv->ddrphy_base + 0x408);
    writel(0x33330000, priv->dramc0_base + 0x40c);
    writel(0x33330000, priv->ddrphy_base + 0x40c);
    writel(0x33330000, priv->dramc0_base + 0x410);
    writel(0x33330000, priv->ddrphy_base + 0x410);
    writel(0xb052311, priv->dramc0_base + 0x110);
    writel(0xb052311, priv->ddrphy_base + 0x110);
    writel(5, priv->dramc0_base + 0x0e4);
    writel(5, priv->ddrphy_base + 0x0e4);
    udelay(200);
    writel(priv->modereg63, priv->dramc0_base + 0x088);
    writel(priv->modereg63, priv->ddrphy_base + 0x088);
    writel(1, priv->dramc0_base + 0x1e4);
    writel(1, priv->ddrphy_base + 0x1e4);
    udelay(10);
    writel(0, priv->dramc0_base + 0x1e4);
    writel(0, priv->ddrphy_base + 0x1e4);
    temp = readl(priv->dramc0_base + 0x110);
    writel((temp & (~0x7)), priv->dramc0_base + 0x110);
    temp = readl(priv->ddrphy_base + 0x110);
    writel((temp & (~0x7)), priv->ddrphy_base + 0x110);
    writel(priv->modereg10, priv->dramc0_base + 0x088);
    writel(priv->modereg10, priv->ddrphy_base + 0x088);
    writel(1, priv->dramc0_base + 0x1e4);
    writel(1, priv->ddrphy_base + 0x1e4);
    udelay(1);
    writel(0, priv->dramc0_base + 0x1e4);
    writel(0, priv->ddrphy_base + 0x1e4);
    
    if ((readl(priv->emi + 0x0000) & 0x20000) != 0) {

      temp = readl(priv->dramc0_base + 0x110);
      writel((temp | 8), priv->dramc0_base + 0x110);
      temp = readl(priv->ddrphy_base + 0x110);
      writel((temp | 8), priv->ddrphy_base + 0x110);
      writel(priv->modereg10, priv->dramc0_base + 0x088);
      writel(priv->modereg10, priv->ddrphy_base + 0x088);
      writel(1, priv->dramc0_base + 0x1e4);
      writel(1, priv->ddrphy_base + 0x1e4);
      udelay(1);
      writel(0, priv->dramc0_base + 0x1e4);
      writel(0, priv->ddrphy_base + 0x1e4);
      temp = readl(priv->dramc0_base + 0x110);
      writel((temp & (~0x8)), priv->dramc0_base + 0x110);
      temp = readl(priv->ddrphy_base + 0x110);
      writel((temp & (~0x8)), priv->ddrphy_base + 0x110);
      writel((temp | 1), priv->dramc0_base + 0x110);
      temp = readl(priv->ddrphy_base + 0x110);
      writel((temp | 1), priv->ddrphy_base + 0x110);
    }
    
    writel(priv->modereg1, priv->dramc0_base + 0x088);
    writel(priv->modereg10, priv->ddrphy_base + 0x088);
    writel(1, priv->dramc0_base + 0x1e4);
    writel(1, priv->ddrphy_base + 0x1e4);
    udelay(1);
    writel(0, priv->dramc0_base + 0x1e4);
    writel(0, priv->ddrphy_base + 0x1e4);
    writel(0x80002, priv->dramc0_base + 0x088);
    writel(0x80002, priv->ddrphy_base + 0x088);
    writel(1, priv->dramc0_base + 0x1e4);
    writel(1, priv->ddrphy_base + 0x1e4);
    udelay(1);
    
    writel(0x00001100, priv->dramc0_base + 0x01e4);
    writel(0x00001100, priv->ddrphy_base + 0x01e4);

    if ((readl(priv->emi + 0x0000) & 0x20000) != 0)  {
        writel(0x00112391, priv->dramc0_base + 0x0110);
        writel(0x00112391, priv->ddrphy_base + 0x0110);
    } else {
        writel(0x00112390, priv->dramc0_base + 0x0110);
        writel(0x00112390, priv->ddrphy_base + 0x0110);
    }

    writel(0x00000001, priv->dramc0_base + 0x00e4);
    writel(0x00000001, priv->ddrphy_base + 0x00e4);
    writel(0x00000001, priv->dramc0_base + 0x01ec);
    writel(0x00000001, priv->ddrphy_base + 0x01ec);
    writel(0x00000a56, priv->dramc0_base + 0x0084);
    writel(0x00000a56, priv->ddrphy_base + 0x0084);
    writel(priv->actim, priv->dramc0_base + 0x0000);
    writel(priv->actim, priv->ddrphy_base + 0x0000);
    writel(priv->conf1, priv->dramc0_base + 0x0004);
    writel(priv->conf1, priv->ddrphy_base + 0x0004);
    writel(priv->conf2, priv->dramc0_base + 0x0008);
    writel(priv->conf2, priv->ddrphy_base + 0x0008);
    writel(priv->test2_3, priv->dramc0_base + 0x0044);
    writel(priv->test2_3, priv->ddrphy_base + 0x0044);
    writel(priv->ddr2ctl, priv->dramc0_base + 0x007c);
    writel(priv->ddr2ctl, priv->ddrphy_base + 0x007c);
    writel(priv->misctl0, priv->dramc0_base + 0x00fc);
    writel(priv->misctl0, priv->ddrphy_base + 0x00fc);
    writel(priv->actim1, priv->dramc0_base + 0x01e8);
    writel(priv->actim1, priv->ddrphy_base + 0x01e8);
    writel(priv->actim05t, priv->dramc0_base + 0x01f8);
    writel(priv->actim05t, priv->ddrphy_base + 0x01f8);
    writel((priv->conf2 | 0x10000000), priv->dramc0_base + 0x0008);
    writel((priv->conf2 | 0x10000000), priv->ddrphy_base + 0x0008);
    writel(priv->pd_ctrl, priv->dramc0_base + 0x01dc);
    writel(priv->pd_ctrl, priv->ddrphy_base + 0x01dc);
    writel(priv->conf2, priv->dramc0_base + 0x0008);
    writel(priv->conf2, priv->ddrphy_base + 0x0008);
    writel(0x00000000, priv->dramc0_base + 0x0010);
    writel(0x00000000, priv->ddrphy_base + 0x0010);
    writel(0xedcb000f, priv->dramc0_base + 0x00f8);
    writel(0xedcb000f, priv->ddrphy_base + 0x00f8);
    writel(0x00000000, priv->dramc0_base + 0x0020);
    writel(0x00000000, priv->ddrphy_base + 0x0020);
    writel((((readl(priv->dramc0_base + 0x007c)) & 0xFFFFFF8F) | ((8 & 0x07) <<4)), priv->dramc0_base + 0x007c);
    writel((((readl(priv->ddrphy_base + 0x007c)) & 0xFFFFFF8F) | ((8 & 0x07) <<4)), priv->ddrphy_base + 0x007c);
    writel((((readl(priv->dramc0_base + 0x00e4)) & 0xFFFFFFEF) | (((8 >> 3) & 0x01) << 4)), priv->dramc0_base + 0x00e4);
    writel((((readl(priv->ddrphy_base + 0x00e4)) & 0xFFFFFFEF) | (((8 >> 3) & 0x01) << 4)), priv->ddrphy_base + 0x00e4);
    
    writel(0x4050003, priv->dramc0_base + 0x0210);
    writel(0x4050003, priv->ddrphy_base + 0x0210);
    writel(0x60305, priv->dramc0_base + 0x0214);
    writel(0x60305, priv->ddrphy_base + 0x0214);
    writel(0x1010405, priv->dramc0_base + 0x0218);
    writel(0x1010405, priv->ddrphy_base + 0x0218);
    writel(0x5030004, priv->dramc0_base + 0x021c);
    writel(0x5030004, priv->ddrphy_base + 0x021c);
    writel(0x4040001, priv->dramc0_base + 0x0220);
    writel(0x4040001, priv->ddrphy_base + 0x0220);
    writel(0x1040304, priv->dramc0_base + 0x0224);
    writel(0x1040304, priv->ddrphy_base + 0x0224);
    writel(0x1030404, priv->dramc0_base + 0x0228);
    writel(0x1030404, priv->ddrphy_base + 0x0228);
    writel(0x4040004, priv->dramc0_base + 0x022c);
    writel(0x4040004, priv->ddrphy_base + 0x022c);
    writel(0x1A1C1A1D, priv->dramc0_base + 0x0018);
    writel(0x1A1C1A1D, priv->ddrphy_base + 0x0018);
    writel(0x1A1C1A1D, priv->dramc0_base + 0x001c);
    writel(0x1A1C1A1D, priv->ddrphy_base + 0x001c);
    
    temp = readl(priv->emi + 0x0060);
    writel((temp | (1<<10)), priv->emi + 0x0060);
   
    DRAMC_WRITE_SET(dev, 0x10000000, 0xE0);
    DRAMC_WRITE_SET(dev, priv->spcmd, 0x1E4);
    DRAMC_WRITE_REG(dev, priv->test2_1, 0x3C);
    DRAMC_WRITE_REG(dev, priv->test2_2, 0x40);
    DRAMC_WRITE_REG(dev, priv->dqsctl1, 0xE0);
    DRAMC_WRITE_REG(dev, priv->dqsgctl, 0x124);
    DRAMC_WRITE_REG(dev, priv->r0dqsien, 0x94);
    DRAMC_WRITE_REG(dev, 0xc0064181, 0x7C);
    DRAMC_WRITE_REG(dev, priv->padctl4, 0xE4);
    DRAMC_WRITE_REG(dev, priv->dqidly1, 0x210);
    DRAMC_WRITE_REG(dev, priv->dqidly2, 0x214);
    DRAMC_WRITE_REG(dev, priv->dqidly3, 0x218);
    DRAMC_WRITE_REG(dev, priv->dqidly4, 0x21c);
    DRAMC_WRITE_REG(dev, priv->dqidly5, 0x220);
    DRAMC_WRITE_REG(dev, priv->dqidly6, 0x224);
    DRAMC_WRITE_REG(dev, priv->dqidly7, 0x228);
    DRAMC_WRITE_REG(dev, priv->dqidly8, 0x22c);
    DRAMC_WRITE_REG(dev, priv->r0deldly, 0x18);

    temp = readl(priv->dramc0_base + 0x110);
    writel((temp | 8), priv->dramc0_base + 0x110);
    temp = readl(priv->ddrphy_base + 0x110);
    writel((temp | 8), priv->ddrphy_base + 0x110);

    DRAMC_WRITE_REG(dev, 0xc0064181, 0x7C);
    DRAMC_WRITE_REG(dev, priv->padctl4, 0xE4);
    DRAMC_WRITE_REG(dev, priv->dqidly1, 0x210);
    DRAMC_WRITE_REG(dev, priv->dqidly2, 0x214);
    DRAMC_WRITE_REG(dev, priv->dqidly3, 0x218);
    DRAMC_WRITE_REG(dev, priv->dqidly4, 0x21c);
    DRAMC_WRITE_REG(dev, priv->dqidly5, 0x220);
    DRAMC_WRITE_REG(dev, priv->dqidly6, 0x224);
    DRAMC_WRITE_REG(dev, priv->dqidly7, 0x228);
    DRAMC_WRITE_REG(dev, priv->dqidly8, 0x22c);
    DRAMC_WRITE_REG(dev, priv->r0deldly, 0x18);
    DRAMC_WRITE_REG(dev, 0x4, 0x118);
    DRAMC_WRITE_REG(dev, priv->dqsgctl, 0x124);
    DRAMC_WRITE_REG(dev, priv->r1dqsien, 0x98);
    DRAMC_WRITE_REG(dev, priv->r1deldly, 0x1C);
    temp = readl(priv->dramc0_base + 0x110);
    writel((temp & (~0x8)), priv->dramc0_base + 0x110);
    temp = readl(priv->ddrphy_base + 0x110);
    writel((temp & (~0x8)), priv->ddrphy_base + 0x110);

    DRAMC_WRITE_REG(dev, priv->dqodly1, 0x200);
    DRAMC_WRITE_REG(dev, priv->dqodly2, 0x204);
    DRAMC_WRITE_REG(dev, priv->dqodly3, 0x208);
    DRAMC_WRITE_REG(dev, priv->dqodly4, 0x20c);
    DRAMC_WRITE_REG(dev, priv->padctl3, 0x14);
    DRAMC_WRITE_REG(dev, priv->dqsctl1, 0xE0);
    DRAMC_WRITE_REG(dev, priv->dqsgctl, 0x124);
    DRAMC_WRITE_REG(dev, priv->r0dqsien, 0x94);

    if ((readl(priv->emi + 0x0000) & 0x20000) != 0)
    {
        unsigned int val1, val2;
        val1 = ((readl(priv->dramc0_base + 0xe0)) & 0x07000000);
        val1 = (((val1 >> 24)+1) << 16);
        val2 = (((readl(priv->dramc0_base + 0x1c4)) & 0xFFF0FFFF) | val1);
        writel(val2, priv->dramc0_base + 0x1c4);
    }
    
    temp = readl(priv->dramc0_base + 0x01c0);
    writel((temp | (0x80000000)), priv->dramc0_base + 0x01c0);
    temp = readl(priv->ddrphy_base + 0x01c0);
    writel((temp | (0x80000000)), priv->ddrphy_base + 0x01c0);
    writel((((readl(priv->dramc0_base + 0x00e4)) & 0xFFFF8FFF) | ((3 & 0x07) <<12)), priv->dramc0_base + 0x00e4);
    writel((((readl(priv->ddrphy_base + 0x00e4)) & 0xFFFF8FFF) | ((3 & 0x07) <<12)), priv->ddrphy_base + 0x00e4);

    temp = readl(priv->dramc0_base + 0x00e4);
    writel((temp | (0x00000200)), priv->dramc0_base + 0x00e4);
    temp = readl(priv->ddrphy_base + 0x00e4);
    writel((temp | (0x00000200)), priv->ddrphy_base + 0x00e4);

    temp = readl(priv->dramc0_base + 0x01ec);
    writel((temp | 0x4f10), priv->dramc0_base + 0x01ec);
    temp = readl(priv->ddrphy_base + 0x01ec);
    writel((temp | 0x4f10), priv->ddrphy_base + 0x01ec);

    temp = readl(priv->dramc0_base + 0x00fc);
    writel((temp & 0xf9ffffff), priv->dramc0_base + 0x00fc);
    temp = readl(priv->ddrphy_base + 0x00fc);
    writel((temp & 0xf9ffffff), priv->ddrphy_base + 0x00fc);

    temp = readl(priv->ddrphy_base + 0x0640);
    writel((temp | 0xffe88000<<0), priv->ddrphy_base + 0x0640);
    temp = readl(priv->dramc0_base + 0x01dc);
    writel((temp | 0x02000000), priv->dramc0_base + 0x01dc);
    temp = readl(priv->ddrphy_base + 0x01dc);
    writel((temp | 0x02000000), priv->ddrphy_base + 0x01dc);
    
    return mtk_lpddr3_memtest();

}
#endif

static int mtk_lpddr3_probe(struct udevice *dev)
{
	struct mtk_lpddr3_priv *priv = dev_get_priv(dev);
    int ret;
	priv->emi = dev_read_addr_index(dev, 0);
	if (priv->emi == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->ddrphy_base = dev_read_addr_index(dev, 1);
	if (priv->ddrphy_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->dramc0_base = dev_read_addr_index(dev, 2);
	if (priv->dramc0_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->dramc_nao_base = dev_read_addr_index(dev, 3);
	if (priv->dramc0_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &priv->phy);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 1, &priv->phy_mux);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 2, &priv->mem);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 3, &priv->mem_mux);
	if (ret)
		return ret;

#ifdef CONFIG_SPL_BUILD

    /* derive platform specific register values from DT for calibration */

    ofnode_reg_read(dev_ofnode(dev), "emi_cona", &priv->emi_cona);
    ofnode_reg_read(dev_ofnode(dev), "drvctrl0", &priv->drvctl0);
    ofnode_reg_read(dev_ofnode(dev), "drvctrl1", &priv->drvctl1);
    ofnode_reg_read(dev_ofnode(dev), "actim", &priv->actim);
    ofnode_reg_read(dev_ofnode(dev), "gddr3ctl", &priv->gddr3ctl);
    ofnode_reg_read(dev_ofnode(dev), "conf1", &priv->conf1);
    ofnode_reg_read(dev_ofnode(dev), "ddr2ctl", &priv->ddr2ctl);
    ofnode_reg_read(dev_ofnode(dev), "test2_1", &priv->test2_1);
    ofnode_reg_read(dev_ofnode(dev), "test2_2", &priv->test2_2);
    ofnode_reg_read(dev_ofnode(dev), "test2_3", &priv->test2_3);
    ofnode_reg_read(dev_ofnode(dev), "conf2", &priv->conf2);
    ofnode_reg_read(dev_ofnode(dev), "pd_ctrl", &priv->pd_ctrl);
    ofnode_reg_read(dev_ofnode(dev), "padctl3", &priv->padctl3);
    ofnode_reg_read(dev_ofnode(dev), "padctl4", &priv->padctl4);
    ofnode_reg_read(dev_ofnode(dev), "actim1", &priv->actim1);
    ofnode_reg_read(dev_ofnode(dev), "misctl0", &priv->misctl0);
    ofnode_reg_read(dev_ofnode(dev), "actim05t", &priv->actim05t);
    ofnode_reg_read(dev_ofnode(dev), "modereg1", &priv->modereg1);
    ofnode_reg_read(dev_ofnode(dev), "modereg2", &priv->modereg2);
    ofnode_reg_read(dev_ofnode(dev), "modereg3", &priv->modereg3);
    ofnode_reg_read(dev_ofnode(dev), "modereg5", &priv->modereg5);
    ofnode_reg_read(dev_ofnode(dev), "modereg10", &priv->modereg10);
    ofnode_reg_read(dev_ofnode(dev), "modereg63", &priv->modereg63);
    ofnode_reg_read(dev_ofnode(dev), "dqsctl1", &priv->dqsctl1);
    ofnode_reg_read(dev_ofnode(dev), "dqsgctl", &priv->dqsgctl);
    ofnode_reg_read(dev_ofnode(dev), "dqidly1", &priv->dqidly1);
    ofnode_reg_read(dev_ofnode(dev), "dqidly2", &priv->dqidly2);
    ofnode_reg_read(dev_ofnode(dev), "dqidly3", &priv->dqidly3);
    ofnode_reg_read(dev_ofnode(dev), "dqidly4", &priv->dqidly4);
    ofnode_reg_read(dev_ofnode(dev), "dqidly5", &priv->dqidly5);
    ofnode_reg_read(dev_ofnode(dev), "dqidly6", &priv->dqidly6);
    ofnode_reg_read(dev_ofnode(dev), "dqidly7", &priv->dqidly7);
    ofnode_reg_read(dev_ofnode(dev), "dqidly8", &priv->dqidly8);
    ofnode_reg_read(dev_ofnode(dev), "dqodly1", &priv->dqodly1);
    ofnode_reg_read(dev_ofnode(dev), "dqodly2", &priv->dqodly2);
    ofnode_reg_read(dev_ofnode(dev), "dqodly3", &priv->dqodly3);
    ofnode_reg_read(dev_ofnode(dev), "dqodly4", &priv->dqodly4);
    ofnode_reg_read(dev_ofnode(dev), "r0deldly", &priv->r0deldly);
    ofnode_reg_read(dev_ofnode(dev), "r1deldly", &priv->r1deldly);
    ofnode_reg_read(dev_ofnode(dev), "r0dqsien", &priv->r0dqsien);
    ofnode_reg_read(dev_ofnode(dev), "r1dqsien", &priv->r1dqsien);
    ofnode_reg_read(dev_ofnode(dev), "spcmd", &priv->spcmd);

    mtk_mempll_init(dev, DDR1333, PLL_MODE_1);
    
	ret = init_lpddr3(dev);
	if (ret)
		return ret;
#endif

	return 0;
}

static int mtk_lpddr3_get_info(struct udevice *dev, struct ram_info *info)
{
    struct mtk_lpddr3_priv *priv = dev_get_priv(dev);
    unsigned int cona, conh, mem_max, bit_counter = 0;
    unsigned long size;
    int ret;
    info->base = CONFIG_SYS_SDRAM_BASE;
    
    ret = ofnode_reg_read(dev_ofnode(dev), "max-size", &mem_max);
    if(ret)
        return ret;
    
    cona = readl(priv->emi + 0x0000);
    conh = readl(priv->emi + 0x0038);
    
    if (cona & CONA_DUAL_CH_EN)
        bit_counter++;

    /* check if 32bit, 32 = 2^5*/
    if (cona & CONA_32BIT_EN)
        bit_counter += 5;
    else
        bit_counter += 4;

    /* check column address */
    /* 00 is 9 bits, 01 is 10 bits, 10 is 11 bits */
    bit_counter += ((cona & COL_ADDR_BITS_MASK) >> COL_ADDR_BITS_SHIFT) +
               9;

    /* check if row address */
    /* 00 is 13 bits, 01 is 14 bits, 10 is 15bits, 11 is 16 bits */
    bit_counter += ((cona & ROW_ADDR_BITS_MASK) >> ROW_ADDR_BITS_SHIFT) +
               13;

    /* check if dual rank */
    if (cona & CONA_DUAL_RANK_EN)
        bit_counter++;

    /* add bank address bit, LPDDR3 is 8 banks =2^3 */
    bit_counter += 3;
    /* transform bits to bytes */
    size = ((size_t)1 << (bit_counter - 3));
    
    if (conh == 0x30000) {
        printf("Detected Single rank 748. Using CONH Value for Size detection\n");
        info->size = SZ_512M + SZ_256M;
    } else if (conh == 0x60000) {
        printf("Detected Dual rank 1.5G. Using CONH Value for size detection\n");
        info->size = SZ_1G + SZ_512M;
    } else {
        if (size > mem_max)
        {
            size = mem_max;
        }
        info->size = size;
    }
    
	return 0;
}

static struct ram_ops mtk_ddr3_ops = {
	.get_info = mtk_lpddr3_get_info,
};

static const struct udevice_id mtk_ddr3_ids[] = {
	{ .compatible = "mediatek,mt6735-dramc" },
	{ }
};

U_BOOT_DRIVER(mediatek_lpddr3) = {
	.name     = "mediatek_lpddr3",
	.id       = UCLASS_RAM,
	.of_match = mtk_ddr3_ids,
	.ops      = &mtk_ddr3_ops,
	.probe    = mtk_lpddr3_probe,
	.priv_auto	= sizeof(struct mtk_lpddr3_priv),
};
