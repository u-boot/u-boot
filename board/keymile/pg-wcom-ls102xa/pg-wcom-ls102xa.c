// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Hitachi Power Grids. All rights reserved.
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/ls102xa_devdis.h>
#include <asm/arch/ls102xa_soc.h>
#include <hwconfig.h>
#include <mmc.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_ifc.h>
#include <fsl_immap.h>
#include <netdev.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <fsl_sec.h>
#include <fsl_devdis.h>
#include <fsl_ddr.h>
#include <spl.h>
#include <fdt_support.h>
#include <fsl_qe.h>
#include <fsl_validate.h>

#include "../common/common.h"
#include "../common/qrio.h"

DECLARE_GLOBAL_DATA_PTR;

static uchar ivm_content[CONFIG_SYS_IVM_EEPROM_MAX_LEN];

int checkboard(void)
{
	show_qrio();

	return 0;
}

int dram_init(void)
{
	return fsl_initdram();
}

int board_early_init_f(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	struct fsl_ifc ifc = {(void *)CONFIG_SYS_IFC_ADDR, (void *)NULL};

	/* Disable unused MCK1 */
	setbits_be32(&gur->ddrclkdr, 2);

	/* IFC Global Configuration */
	setbits_be32(&ifc.gregs->ifc_gcr, 12 << IFC_GCR_TBCTL_TRN_TIME_SHIFT);
	setbits_be32(&ifc.gregs->ifc_ccr, IFC_CCR_CLK_DIV(3) |
					  IFC_CCR_INV_CLK_EN);

	/* clear BD & FR bits for BE BD's and frame data */
	clrbits_be32(&scfg->etsecdmamcr, SCFG_ETSECDMAMCR_LE_BD_FR);
	out_be32(&scfg->etsecmcr, SCFG_ETSECCMCR_GE2_CLK125);

	init_early_memctl_regs();

	/* QRIO Configuration */
	qrio_uprstreq(UPREQ_CORE_RST);

#if CONFIG_IS_ENABLED(TARGET_PG_WCOM_SELI8)
	qrio_prstcfg(KM_LIU_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_LIU_RST, true);

	qrio_prstcfg(KM_PAXK_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(KM_PAXK_RST, true);
#endif

#if CONFIG_IS_ENABLED(TARGET_PG_WCOM_EXPU1)
	qrio_prstcfg(WCOM_TMG_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(WCOM_TMG_RST, true);

	qrio_prstcfg(WCOM_PHY_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_prst(WCOM_PHY_RST, false, false);

	qrio_prstcfg(WCOM_QSFP_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_wdmask(WCOM_QSFP_RST, true);

	qrio_prstcfg(WCOM_CLIPS_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_prst(WCOM_CLIPS_RST, false, false);
#endif
	qrio_prstcfg(KM_DBG_ETH_RST, PRSTCFG_POWUP_UNIT_CORE_RST);
	qrio_prst(KM_DBG_ETH_RST, false, false);

	i2c_deblock_gpio_cfg();

	/* enable the Unit LED (red) & Boot LED (on) */
	qrio_set_leds();

	/* enable Application Buffer */
	qrio_enable_app_buffer();

	arch_soc_init();

	return 0;
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_SYS_FSL_ERRATUM_A010315))
		erratum_a010315();

	fsl_serdes_init();

	ls102xa_smmu_stream_id_init();

	u_qe_init();

	return 0;
}

int board_late_init(void)
{
	return 0;
}

int misc_init_r(void)
{
	if (IS_ENABLED(CONFIG_FSL_DEVICE_DISABLE))
		device_disable(devdis_tbl, ARRAY_SIZE(devdis_tbl));

	ivm_read_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN,
			CONFIG_PIGGY_MAC_ADDRESS_OFFSET);

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	ft_cpu_setup(blob, bd);

	if (IS_ENABLED(CONFIG_PCI))
		ft_pci_setup(blob, bd);

	return 0;
}

#if defined(CONFIG_POST)
int post_hotkeys_pressed(void)
{
	/* DIC26_SELFTEST: GPRTA0, GPA0 */
	qrio_gpio_direction_input(QRIO_GPIO_A, 0);
	return qrio_get_gpio(QRIO_GPIO_A, 0);
}

ulong post_word_load(void)
{
	/* POST word is located at the beginning of reserved physical RAM */
	void *addr = (void *)(CONFIG_SYS_SDRAM_BASE +
				gd->ram_size - CONFIG_KM_RESERVED_PRAM + 8);
	return in_le32(addr);
}

void post_word_store(ulong value)
{
	/* POST word is located at the beginning of reserved physical RAM */
	void *addr = (void *)(CONFIG_SYS_SDRAM_BASE +
				gd->ram_size - CONFIG_KM_RESERVED_PRAM + 8);
	out_le32(addr, value);
}

int arch_memory_test_prepare(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	/* Define only 1MiB range for mem_regions at the middle of the RAM */
	/* For 1GiB range mem_regions takes approx. 4min */
	*vstart = CONFIG_SYS_SDRAM_BASE + (gd->ram_size >> 1);
	*size = 1 << 20;
	return 0;
}
#endif

u8 flash_read8(void *addr)
{
	return __raw_readb(addr + 1);
}

void flash_write16(u16 val, void *addr)
{
	u16 shftval = (((val >> 8) & 0xff) | ((val << 8) & 0xff00));

	__raw_writew(shftval, addr);
}

u16 flash_read16(void *addr)
{
	u16 val = __raw_readw(addr);

	return (((val) >> 8) & 0x00ff) | (((val) << 8) & 0xff00);
}

int hush_init_var(void)
{
	ivm_analyze_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
	return 0;
}

int last_stage_init(void)
{
	set_km_env();
	return 0;
}
