// SPDX-License-Identifier: GPL-2.0+

#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6ul_pins.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <linux/libfdt.h>
#include <spl.h>

#if defined(CONFIG_SPL_BUILD)

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	return 0;
}
#endif

#include <asm/arch/mx6-ddr.h>

static struct mx6ul_iomux_grp_regs mx6_grp_ioregs = {
	.grp_addds = 0x00000030,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_b0ds = 0x00000030,
	.grp_ctlds = 0x00000030,
	.grp_b1ds = 0x00000030,
	.grp_ddrpke = 0x00000000,
	.grp_ddrmode = 0x00020000,
	.grp_ddr_type = 0x00080000,
};

static struct mx6ul_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_dqm0 = 0x00000030,
	.dram_dqm1 = 0x00000030,
	.dram_ras = 0x00000030,
	.dram_cas = 0x00000030,
	.dram_odt0 = 0x00000030,
	.dram_odt1 = 0x00000030,
	.dram_sdba2 = 0x00000000,
	.dram_sdclk_0 = 0x00000030,
	.dram_sdqs0 = 0x00000030,
	.dram_sdqs1 = 0x00000030,
	.dram_reset = 0x00000030,
};

static struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0 = 0x00000000,
	.p0_mpdgctrl0 = 0x01380134,
	.p0_mprddlctl = 0x40404244,
	.p0_mpwrdlctl = 0x40405050,
};

static struct mx6_ddr_sysinfo ddr_sysinfo = {
	.dsize		= 0,
	.cs1_mirror	= 0,
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
	.refsel = 1,
	.refr = 3,
};

static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed = 1333,
	.density = 2,
	.width = 16,
	.banks = 8,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1350,
	.trcmin = 4950,
	.trasmin = 3600,
};

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

static void imx6ul_spl_dram_cfg_size(u32 ram_size)
{
	if (ram_size == SZ_256M)
		mem_ddr.rowaddr = 14;
	else
		mem_ddr.rowaddr = 15;

	mx6ul_dram_iocfg(mem_ddr.width, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&ddr_sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

static void imx6ul_spl_dram_cfg(void)
{
	ulong ram_size_test, ram_size = 0;

	for (ram_size = SZ_512M; ram_size >= SZ_256M; ram_size >>= 1) {
		imx6ul_spl_dram_cfg_size(ram_size);
		ram_size_test = get_ram_size((long int *)PHYS_SDRAM, ram_size);
		if (ram_size_test == ram_size)
			break;
	}

	if (ram_size < SZ_256M) {
		puts("ERROR: DRAM size detection failed\n");
		hang();
	}
}

void board_init_f(ulong dummy)
{
	ccgr_init();
	arch_cpu_init();
	board_early_init_f();
	timer_init();
	preloader_console_init();
	imx6ul_spl_dram_cfg();
	memset(__bss_start, 0, __bss_end - __bss_start);
	board_init_r(NULL, 0);
}

void reset_cpu(ulong addr)
{
}
#endif
