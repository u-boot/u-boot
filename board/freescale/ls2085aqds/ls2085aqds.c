/*
 * Copyright 2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fsl_ddr.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <libfdt.h>
#include <fsl_debug_server.h>
#include <fsl-mc/fsl_mc.h>
#include <environment.h>
#include <i2c.h>
#include <asm/arch-fsl-lsch3/soc.h>

#include "../common/qixis.h"
#include "ls2085aqds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

unsigned long long get_qixis_addr(void)
{
	unsigned long long addr;

	if (gd->flags & GD_FLG_RELOC)
		addr = QIXIS_BASE_PHYS;
	else
		addr = QIXIS_BASE_PHYS_EARLY;

	/*
	 * IFC address under 256MB is mapped to 0x30000000, any address above
	 * is mapped to 0x5_10000000 up to 4GB.
	 */
	addr = addr  > 0x10000000 ? addr + 0x500000000ULL : addr + 0x30000000;

	return addr;
}

int checkboard(void)
{
	char buf[64];
	u8 sw;
	static const char *const freq[] = {"100", "125", "156.25",
					    "100 separate SSCG"};
	int clock;

	sw = QIXIS_READ(arch);
	printf("Board: %s, ", CONFIG_IDENT_STRING);
	printf("Board Arch: V%d, ", sw >> 4);
	printf("Board version: %c, boot from ", (sw & 0xf) + 'A' - 1);

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x8)
		puts("PromJet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else if (sw == 0x15)
		printf("IFCCard\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);

	printf("FPGA: v%d (%s), build %d",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());
	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));

	/*
	 * Display the actual SERDES reference clocks as configured by the
	 * dip switches on the board.  Note that the SWx registers could
	 * technically be set to force the reference clocks to match the
	 * values that the SERDES expects (or vice versa).  For now, however,
	 * we just display both values and hope the user notices when they
	 * don't match.
	 */
	puts("SERDES1 Reference : ");
	sw = QIXIS_READ(brdcfg[2]);
	clock = (sw >> 6) & 3;
	printf("Clock1 = %sMHz ", freq[clock]);
	clock = (sw >> 4) & 3;
	printf("Clock2 = %sMHz", freq[clock]);

	puts("\nSERDES2 Reference : ");
	clock = (sw >> 2) & 3;
	printf("Clock1 = %sMHz ", freq[clock]);
	clock = (sw >> 0) & 3;
	printf("Clock2 = %sMHz\n", freq[clock]);

	return 0;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch (sysclk_conf & 0x0F) {
	case QIXIS_SYSCLK_83:
		return 83333333;
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	case QIXIS_SYSCLK_150:
		return 150000000;
	case QIXIS_SYSCLK_160:
		return 160000000;
	case QIXIS_SYSCLK_166:
		return 166666666;
	}
	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}
	return 66666666;
}

int select_i2c_ch_pca9547(u8 ch)
{
	int ret;

	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}

int board_init(void)
{
	init_final_memctl_regs();

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);

	return 0;
}

int board_early_init_f(void)
{
	fsl_lsch3_early_init_f();
	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
	print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
	print_ddr_info(0);
	if (gd->bd->bi_dram[2].size) {
		puts("\nDP-DDR ");
		print_size(gd->bd->bi_dram[2].size, "");
		print_ddr_info(CONFIG_DP_DDR_CTRL);
	}
}

int dram_init(void)
{
	gd->ram_size = initdram(0);

	return 0;
}

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
#ifdef CONFIG_FSL_DEBUG_SERVER
	debug_server_init();
#endif

	return 0;
}
#endif

unsigned long get_dram_size_to_hide(void)
{
	unsigned long dram_to_hide = 0;

/* Carve the Debug Server private DRAM block from the end of DRAM */
#ifdef CONFIG_FSL_DEBUG_SERVER
	dram_to_hide += debug_server_get_dram_block_size();
#endif

/* Carve the MC private DRAM block from the end of DRAM */
#ifdef CONFIG_FSL_MC_ENET
	dram_to_hide += mc_get_dram_block_size();
#endif

	return dram_to_hide;
}

#ifdef CONFIG_FSL_MC_ENET
void fdt_fixup_board_enet(void *fdt)
{
	int offset;

	offset = fdt_path_offset(fdt, "/fsl-mc");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/fsl,dprc@0");

	if (offset < 0) {
		printf("%s: ERROR: fsl-mc node not found in device tree (error %d)\n",
		       __func__, offset);
		return;
	}

	if (get_mc_boot_status() == 0)
		fdt_status_okay(fdt, offset);
	else
		fdt_status_fail(fdt, offset);
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	/* limit the memory size to bank 1 until Linux can handle 40-bit PA */
	base = getenv_bootm_low();
	size = getenv_bootm_size();
	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_FSL_MC_ENET
	fdt_fixup_board_enet(blob);
	fsl_mc_ldpaa_exit(bd);
#endif

	return 0;
}
#endif

void qixis_dump_switch(void)
{
	int i, nr_of_cfgsw;

	QIXIS_WRITE(cms[0], 0x00);
	nr_of_cfgsw = QIXIS_READ(cms[1]);

	puts("DIP switch settings dump:\n");
	for (i = 1; i <= nr_of_cfgsw; i++) {
		QIXIS_WRITE(cms[0], i);
		printf("SW%d = (0x%02x)\n", i, QIXIS_READ(cms[1]));
	}
}
