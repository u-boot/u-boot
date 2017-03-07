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
#include <hwconfig.h>
#include <fdt_support.h>
#include <libfdt.h>
#include <fsl-mc/fsl_mc.h>
#include <environment.h>
#include <efi_loader.h>
#include <i2c.h>
#include <asm/arch/mmu.h>
#include <asm/arch/soc.h>
#include <asm/arch/ppa.h>
#include <fsl_sec.h>

#include "../common/qixis.h"
#include "ls2080ardb_qixis.h"
#include "../common/vid.h"

#define PIN_MUX_SEL_SDHC	0x00
#define PIN_MUX_SEL_DSPI	0x0a

#define SET_SDHC_MUX_SEL(reg, value)	((reg & 0xf0) | value)
DECLARE_GLOBAL_DATA_PTR;

enum {
	MUX_TYPE_SDHC,
	MUX_TYPE_DSPI,
};

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
	u8 sw;
	char buf[15];

	cpu_name(buf);
	printf("Board: %s-RDB, ", buf);

	sw = QIXIS_READ(arch);
	printf("Board Arch: V%d, ", sw >> 4);
	printf("Board version: %c, boot from ", (sw & 0xf) + 'A');

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x9)
		puts("NAND\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);

	printf("FPGA: v%d.%d\n", QIXIS_READ(scver), QIXIS_READ(tagdata));

	puts("SERDES1 Reference : ");
	printf("Clock1 = 156.25MHz ");
	printf("Clock2 = 156.25MHz");

	puts("\nSERDES2 Reference : ");
	printf("Clock1 = 100MHz ");
	printf("Clock2 = 100MHz\n");

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

int i2c_multiplexer_select_vid_channel(u8 channel)
{
	return select_i2c_ch_pca9547(channel);
}

int config_board_mux(int ctrl_type)
{
	u8 reg5;

	reg5 = QIXIS_READ(brdcfg[5]);

	switch (ctrl_type) {
	case MUX_TYPE_SDHC:
		reg5 = SET_SDHC_MUX_SEL(reg5, PIN_MUX_SEL_SDHC);
		break;
	case MUX_TYPE_DSPI:
		reg5 = SET_SDHC_MUX_SEL(reg5, PIN_MUX_SEL_DSPI);
		break;
	default:
		printf("Wrong mux interface type\n");
		return -1;
	}

	QIXIS_WRITE(brdcfg[5], reg5);

	return 0;
}

int board_init(void)
{
	char *env_hwconfig;
	u32 __iomem *dcfg_ccsr = (u32 __iomem *)DCFG_BASE;
#ifdef CONFIG_FSL_MC_ENET
	u32 __iomem *irq_ccsr = (u32 __iomem *)ISC_BASE;
#endif
	u32 val;

	init_final_memctl_regs();

	val = in_le32(dcfg_ccsr + DCFG_RCWSR13 / 4);

	env_hwconfig = getenv("hwconfig");

	if (hwconfig_f("dspi", env_hwconfig) &&
	    DCFG_RCWSR13_DSPI == (val & (u32)(0xf << 8)))
		config_board_mux(MUX_TYPE_DSPI);
	else
		config_board_mux(MUX_TYPE_SDHC);

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);

	QIXIS_WRITE(rst_ctl, QIXIS_RST_CTL_RESET_EN);

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif

#ifdef CONFIG_FSL_MC_ENET
	/* invert AQR405 IRQ pins polarity */
	out_le32(irq_ccsr + IRQCR_OFFSET / 4, AQR405_IRQ_MASK);
#endif
#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

	return 0;
}

int board_early_init_f(void)
{
	fsl_lsch3_early_init_f();
	return 0;
}

int misc_init_r(void)
{
	if (hwconfig("sdhc"))
		config_board_mux(MUX_TYPE_SDHC);

	if (adjust_vdd(0))
		printf("Warning: Adjusting core voltage failed.\n");

	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
	print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
	print_ddr_info(0);
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	if (soc_has_dp_ddr() && gd->bd->bi_dram[2].size) {
		puts("\nDP-DDR ");
		print_size(gd->bd->bi_dram[2].size, "");
		print_ddr_info(CONFIG_DP_DDR_CTRL);
	}
#endif
}

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	return 0;
}
#endif

#ifdef CONFIG_FSL_MC_ENET
void fdt_fixup_board_enet(void *fdt)
{
	int offset;

	offset = fdt_path_offset(fdt, "/soc/fsl-mc");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/fsl-mc");

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

void board_quiesce_devices(void)
{
	fsl_mc_ldpaa_exit(gd->bd);
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	ft_cpu_setup(blob, bd);

	/* fixup DT for the two GPP DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;
	base[1] = gd->bd->bi_dram[1].start;
	size[1] = gd->bd->bi_dram[1].size;

#ifdef CONFIG_RESV_RAM
	/* reduce size if reserved memory is within this bank */
	if (gd->arch.resv_ram >= base[0] &&
	    gd->arch.resv_ram < base[0] + size[0])
		size[0] = gd->arch.resv_ram - base[0];
	else if (gd->arch.resv_ram >= base[1] &&
		 gd->arch.resv_ram < base[1] + size[1])
		size[1] = gd->arch.resv_ram - base[1];
#endif

	fdt_fixup_memory_banks(blob, base, size, 2);

	fsl_fdt_fixup_dr_usb(blob, bd);

#ifdef CONFIG_FSL_MC_ENET
	fdt_fixup_board_enet(blob);
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

/*
 * Board rev C and earlier has duplicated I2C addresses for 2nd controller.
 * Both slots has 0x54, resulting 2nd slot unusable.
 */
void update_spd_address(unsigned int ctrl_num,
			unsigned int slot,
			unsigned int *addr)
{
	u8 sw;

	sw = QIXIS_READ(arch);
	if ((sw & 0xf) < 0x3) {
		if (ctrl_num == 1 && slot == 0)
			*addr = SPD_EEPROM_ADDRESS4;
		else if (ctrl_num == 1 && slot == 1)
			*addr = SPD_EEPROM_ADDRESS3;
	}
}
