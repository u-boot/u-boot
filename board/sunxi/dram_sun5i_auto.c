/* DRAM parameters for auto dram configuration on sun5i and sun7i */

#include <common.h>
#include <asm/arch/dram.h>

static struct dram_para dram_para = {
	.clock = CONFIG_DRAM_CLK,
	.type = 3,
	.rank_num = 1,
	.density = 0,
	.io_width = 0,
	.bus_width = 0,
	.cas = 9,
	.zq = CONFIG_DRAM_ZQ,
	.odt_en = 0,
	.size = 0,
	.tpr0 = 0x42d899b7,
	.tpr1 = 0xa090,
	.tpr2 = 0x22a00,
	.tpr3 = 0,
	.tpr4 = 0,
	.tpr5 = 0,
	.emr1 = CONFIG_DRAM_EMR1,
	.emr2 = 0x10,
	.emr3 = 0,
};

unsigned long sunxi_dram_init(void)
{
	return dramc_init(&dram_para);
}
