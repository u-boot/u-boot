/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>

#define ZYNQ_SILICON_VER_MASK	0xF000
#define ZYNQ_SILICON_VER_SHIFT	12

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region zynqmp_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x70000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0xf8000000UL,
		.phys = 0xf8000000UL,
		.size = 0x07e00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x400000000UL,
		.phys = 0x400000000UL,
		.size = 0x200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x600000000UL,
		.phys = 0x600000000UL,
		.size = 0x800000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xe00000000UL,
		.phys = 0xe00000000UL,
		.size = 0xf200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};
struct mm_region *mem_map = zynqmp_mem_map;

u64 get_page_table_size(void)
{
	return 0x14000;
}

static unsigned int zynqmp_get_silicon_version_secure(void)
{
	u32 ver;

	ver = readl(&csu_base->version);
	ver &= ZYNQMP_SILICON_VER_MASK;
	ver >>= ZYNQMP_SILICON_VER_SHIFT;

	return ver;
}

unsigned int zynqmp_get_silicon_version(void)
{
	if (current_el() == 3)
		return zynqmp_get_silicon_version_secure();

	gd->cpu_clk = get_tbclk();

	switch (gd->cpu_clk) {
	case 0 ... 1000000:
		return ZYNQMP_CSU_VERSION_VELOCE;
	case 50000000:
		return ZYNQMP_CSU_VERSION_QEMU;
	case 4000000:
		return ZYNQMP_CSU_VERSION_EP108;
	}

	return ZYNQMP_CSU_VERSION_SILICON;
}

#define ZYNQMP_MMIO_READ	0xC2000014
#define ZYNQMP_MMIO_WRITE	0xC2000013

#ifndef CONFIG_SPL_BUILD
int invoke_smc(u32 pm_api_id, u32 arg0, u32 arg1, u32 arg2, u32 arg3,
	       u32 *ret_payload)
{
	/*
	 * Added SIP service call Function Identifier
	 * Make sure to stay in x0 register
	 */
	struct pt_regs regs;

	regs.regs[0] = pm_api_id;
	regs.regs[1] = ((u64)arg1 << 32) | arg0;
	regs.regs[2] = ((u64)arg3 << 32) | arg2;

	smc_call(&regs);

	if (ret_payload != NULL) {
		ret_payload[0] = (u32)regs.regs[0];
		ret_payload[1] = upper_32_bits(regs.regs[0]);
		ret_payload[2] = (u32)regs.regs[1];
		ret_payload[3] = upper_32_bits(regs.regs[1]);
		ret_payload[4] = (u32)regs.regs[2];
	}

	return regs.regs[0];
}

#define ZYNQMP_SIP_SVC_GET_API_VERSION		0xC2000001

#define ZYNQMP_PM_VERSION_MAJOR		0
#define ZYNQMP_PM_VERSION_MINOR		3
#define ZYNQMP_PM_VERSION_MAJOR_SHIFT	16
#define ZYNQMP_PM_VERSION_MINOR_MASK	0xFFFF

#define ZYNQMP_PM_VERSION	\
	((ZYNQMP_PM_VERSION_MAJOR << ZYNQMP_PM_VERSION_MAJOR_SHIFT) | \
				 ZYNQMP_PM_VERSION_MINOR)

#if defined(CONFIG_CLK_ZYNQMP)
void zynqmp_pmufw_version(void)
{
	int ret;
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u32 pm_api_version;

	ret = invoke_smc(ZYNQMP_SIP_SVC_GET_API_VERSION, 0, 0, 0, 0,
			 ret_payload);
	pm_api_version = ret_payload[1];

	if (ret)
		panic("PMUFW is not found - Please load it!\n");

	printf("PMUFW:\tv%d.%d\n",
	       pm_api_version >> ZYNQMP_PM_VERSION_MAJOR_SHIFT,
	       pm_api_version & ZYNQMP_PM_VERSION_MINOR_MASK);

	if (pm_api_version != ZYNQMP_PM_VERSION)
		panic("PMUFW version error. Expected: v%d.%d\n",
		      ZYNQMP_PM_VERSION_MAJOR, ZYNQMP_PM_VERSION_MINOR);
}
#endif

int zynqmp_mmio_write(const u32 address,
		      const u32 mask,
		      const u32 value)
{
	return invoke_smc(ZYNQMP_MMIO_WRITE, address, mask, value, 0, NULL);
}

int zynqmp_mmio_read(const u32 address, u32 *value)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u32 ret;

	if (!value)
		return -EINVAL;

	ret = invoke_smc(ZYNQMP_MMIO_READ, address, 0, 0, 0, ret_payload);
	*value = ret_payload[1];

	return ret;
}
#else
int zynqmp_mmio_write(const u32 address,
		      const u32 mask,
		      const u32 value)
{
	u32 data;
	u32 value_local = value;

	zynqmp_mmio_read(address, &data);
	data &= ~mask;
	value_local &= mask;
	value_local |= data;
	writel(value_local, (ulong)address);
	return 0;
}

int zynqmp_mmio_read(const u32 address, u32 *value)
{
	*value = readl((ulong)address);
	return 0;
}
#endif
