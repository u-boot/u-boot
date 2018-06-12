// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2018 NXP
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#ifdef CONFIG_FSL_LS_PPA
#include <asm/arch/ppa.h>
#endif
#include <asm/arch/mmu.h>
#include <asm/arch/soc.h>
#include <fsl_esdhc.h>
#include <hwconfig.h>
#include <environment.h>
#include <fsl_mmdc.h>
#include <netdev.h>
#include <fsl_sec.h>

DECLARE_GLOBAL_DATA_PTR;

static inline int get_board_version(void)
{
	struct ccsr_gpio *pgpio = (void *)(GPIO1_BASE_ADDR);
	int val;

	val = in_be32(&pgpio->gpdat);

	return val;
}

int checkboard(void)
{
#ifdef CONFIG_TARGET_LS1012AFRDM
	puts("Board: LS1012AFRDM ");
#else
	int rev;

	rev = get_board_version();

	puts("Board: FRWY-LS1012A ");

	puts("Version");

	switch (rev) {
	case BOARD_REV_A:
		puts(": RevA ");
		break;
	case BOARD_REV_B:
		puts(": RevB ");
		break;
	default:
		puts(": unknown");
		break;
	}
#endif

	return 0;
}

#ifdef CONFIG_TARGET_LS1012AFRWY
int esdhc_status_fixup(void *blob, const char *compat)
{
	char esdhc0_path[] = "/soc/esdhc@1560000";
	char esdhc1_path[] = "/soc/esdhc@1580000";

	do_fixup_by_path(blob, esdhc0_path, "status", "okay",
			 sizeof("okay"), 1);

	do_fixup_by_path(blob, esdhc1_path, "status", "disabled",
			 sizeof("disabled"), 1);
	return 0;
}
#endif

int dram_init(void)
{
#ifdef CONFIG_TARGET_LS1012AFRWY
	int board_rev;
#endif
	struct fsl_mmdc_info mparam = {
		0x04180000,	/* mdctl */
		0x00030035,	/* mdpdc */
		0x12554000,	/* mdotc */
		0xbabf7954,	/* mdcfg0 */
		0xdb328f64,	/* mdcfg1 */
		0x01ff00db,	/* mdcfg2 */
		0x00001680,	/* mdmisc */
		0x0f3c8000,	/* mdref */
		0x00002000,	/* mdrwd */
		0x00bf1023,	/* mdor */
		0x0000003f,	/* mdasp */
		0x0000022a,	/* mpodtctrl */
		0xa1390003,	/* mpzqhwctrl */
	};

#ifdef CONFIG_TARGET_LS1012AFRWY
	board_rev = get_board_version();

	if (board_rev & BOARD_REV_B) {
		mparam.mdctl = 0x05180000;
		gd->ram_size = SYS_SDRAM_SIZE_1024;
	} else {
		gd->ram_size = SYS_SDRAM_SIZE_512;
	}
#else
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
#endif
	mmdc_init(&mparam);

#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
	/* This will break-before-make MMU for DDR */
	update_early_mmu_table();
#endif

	return 0;
}

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

int board_init(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)(CONFIG_SYS_IMMR +
					CONFIG_SYS_CCI400_OFFSET);

	/*
	 * Set CCI-400 control override register to enable barrier
	 * transaction
	 */
	out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);

	return 0;
}
