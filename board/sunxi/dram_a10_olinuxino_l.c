/* this file is generated, don't edit it yourself */

#include <common.h>
#include <asm/arch/dram.h>

static struct dram_para dram_para = {
	.clock = 480,
	.type = 3,
	.rank_num = 1,
	.density = 4096,
	.io_width = 16,
	.bus_width = 16,
	.cas = 6,
	.zq = 123,
	.odt_en = 0,
	.size = 512,
	.tpr0 = 0x30926692,
	.tpr1 = 0x1090,
	.tpr2 = 0x1a0c8,
	.tpr3 = 0,
	.tpr4 = 0,
	.tpr5 = 0,
	.emr1 = 0x4,
	.emr2 = 0,
	.emr3 = 0,
};

unsigned long sunxi_dram_init(void)
{
	return dramc_init(&dram_para);
}
