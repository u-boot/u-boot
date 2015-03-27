/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>
#include "t102xrdb.h"
#ifdef CONFIG_T1024RDB
#include "cpld.h"
#endif
#include "../common/sleep.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_T1023RDB
enum {
	GPIO1_SD_SEL    = 0x00020000, /* GPIO1_14, 0: EMMC, 1:SD/MMC */
	GPIO1_EMMC_SEL,
	GPIO1_VBANK0,
	GPIO1_VBANK4    = 0x00008000, /* GPIO1_16/20/22,  100:vBank4 */
	GPIO1_VBANK_MASK = 0x00008a00,
	GPIO1_DIR_OUTPUT = 0x00028a00,
	GPIO1_GET_VAL,
};
#endif

int checkboard(void)
{
	struct cpu_type *cpu = gd->arch.cpu;
	static const char *freq[3] = {"100.00MHZ", "125.00MHz", "156.25MHZ"};
	ccsr_gur_t __iomem *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_be32(&gur->rcwsr[4]) & FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;

	printf("Board: %sRDB, ", cpu->name);
#ifdef CONFIG_T1024RDB
	printf("Board rev: 0x%02x CPLD ver: 0x%02x, ",
	       CPLD_READ(hw_ver), CPLD_READ(sw_ver));
#endif
	printf("boot from ");

#ifdef CONFIG_SDCARD
	puts("SD/MMC\n");
#elif CONFIG_SPIFLASH
	puts("SPI\n");
#elif defined(CONFIG_T1024RDB)
	u8 reg;

	reg = CPLD_READ(flash_csr);

	if (reg & CPLD_BOOT_SEL) {
		puts("NAND\n");
	} else {
		reg = ((reg & CPLD_LBMAP_MASK) >> CPLD_LBMAP_SHIFT);
		printf("NOR vBank%d\n", reg);
	}
#elif defined(CONFIG_T1023RDB)
#ifdef CONFIG_NAND
	puts("NAND\n");
#else
	printf("NOR vBank%d\n", (t1023rdb_gpio_ctrl(GPIO1_GET_VAL) &
	       GPIO1_VBANK4) >> 15 ? 4 : 0);
#endif
#endif

	puts("SERDES Reference Clocks:\n");
	if (srds_s1 == 0x95)
		printf("SD1_CLK1=%s, SD1_CLK2=%s\n", freq[2], freq[0]);
	else
		printf("SD1_CLK1=%s, SD1_CLK2=%s\n", freq[0], freq[1]);

	return 0;
}

#ifdef CONFIG_T1024RDB
static void board_mux_lane(void)
{
	ccsr_gur_t __iomem *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_prtcl_s1;
	u8 reg = CPLD_READ(misc_ctl_status);

	srds_prtcl_s1 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_prtcl_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;

	if (srds_prtcl_s1 == 0x95) {
		/* Route Lane B to PCIE */
		CPLD_WRITE(misc_ctl_status, reg & ~CPLD_PCIE_SGMII_MUX);
	} else {
		/* Route Lane B to SGMII */
		CPLD_WRITE(misc_ctl_status, reg | CPLD_PCIE_SGMII_MUX);
	}
	CPLD_WRITE(boot_override, CPLD_OVERRIDE_MUX_EN);
}
#endif

int board_early_init_f(void)
{
#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot())
		fsl_dp_disable_console();
#endif

	return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_SYS_FLASH_BASE
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);
	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();
	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash + promjet */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);
#endif

	set_liodns();
#ifdef CONFIG_SYS_DPAA_QBMAN
	setup_portals();
#endif
#ifdef CONFIG_T1024RDB
	board_mux_lane();
#endif

	return 0;
}

unsigned long get_board_sys_clk(void)
{
	return CONFIG_SYS_CLK_FREQ;
}

unsigned long get_board_ddr_clk(void)
{
	return CONFIG_DDR_CLK_FREQ;
}

int misc_init_r(void)
{
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);
	fdt_fixup_dr_usb(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif

	return 0;
}


#ifdef CONFIG_T1023RDB
static u32 t1023rdb_gpio_ctrl(u32 ctrl_type)
{
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
	u32 gpioval;

	setbits_be32(&pgpio->gpdir, GPIO1_DIR_OUTPUT);
	gpioval = in_be32(&pgpio->gpdat);

	switch (ctrl_type) {
	case GPIO1_SD_SEL:
		gpioval |= GPIO1_SD_SEL;
		break;
	case GPIO1_EMMC_SEL:
		gpioval &= ~GPIO1_SD_SEL;
		break;
	case GPIO1_VBANK0:
		gpioval &= ~GPIO1_VBANK_MASK;
		break;
	case GPIO1_VBANK4:
		gpioval &= ~GPIO1_VBANK_MASK;
		gpioval |= GPIO1_VBANK4;
		break;
	case GPIO1_GET_VAL:
		return gpioval;
	default:
		break;
	}
	out_be32(&pgpio->gpdat, gpioval);

	return 0;
}

static int gpio_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
		    char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;
	if (!strcmp(argv[1], "vbank0"))
		t1023rdb_gpio_ctrl(GPIO1_VBANK0);
	else if (!strcmp(argv[1], "vbank4"))
		t1023rdb_gpio_ctrl(GPIO1_VBANK4);
	else if (!strcmp(argv[1], "sd"))
		t1023rdb_gpio_ctrl(GPIO1_SD_SEL);
	else if (!strcmp(argv[1], "EMMC"))
		t1023rdb_gpio_ctrl(GPIO1_EMMC_SEL);
	else
		return CMD_RET_USAGE;
	return 0;
}

U_BOOT_CMD(
	gpio, 2, 0, gpio_cmd,
	"for vbank0/vbank4/SD/eMMC switch control in runtime",
	"command (e.g. gpio vbank4)"
);
#endif
