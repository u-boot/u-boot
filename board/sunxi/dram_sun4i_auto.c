#include <common.h>
#include <asm/arch/dram.h>

static struct dram_para dram_para = {
	.clock = CONFIG_DRAM_CLK,
	.type = 3,
	.rank_num = 1,
	.density = 0,
	.io_width = 0,
	.bus_width = 0,
	.zq = CONFIG_DRAM_ZQ,
	.odt_en = 0,
	.size = 0,
#ifdef CONFIG_DRAM_TIMINGS_VENDOR_MAGIC
	.cas = 6,
	.tpr0 = 0x30926692,
	.tpr1 = 0x1090,
	.tpr2 = 0x1a0c8,
	.emr2 = 0,
#else
#	include "dram_timings_sun4i.h"
#endif
	.tpr3 = 0,
	.tpr4 = 0,
	.tpr5 = 0,
	.emr1 = CONFIG_DRAM_EMR1,
	.emr3 = 0,
};

unsigned long sunxi_dram_init(void)
{
	return dramc_init(&dram_para);
}
